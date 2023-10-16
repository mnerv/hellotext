#include "text_engine.hpp"
#include "renderer.hpp"

namespace txt {
static constexpr float quad_vertices[] = {
//     x,     y,     z,       u,    v,
    0.0f,  0.0f,  0.0f,    0.0f, 0.0f,
    0.0f,  1.0f,  0.0f,    0.0f, 1.0f,
    1.0f,  1.0f,  0.0f,    1.0f, 1.0f,
    1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
};
static constexpr std::uint32_t quad_cw_indices[] = {
    0, 1, 2,
    0, 2, 3
};

text_engine::text_engine(font_manager_ref_t manager) : m_manager(manager) {
    m_index_buffer = make_index_buffer(quad_cw_indices, sizeof(quad_cw_indices), len(quad_cw_indices), type::u32, usage::static_draw);

    m_manager->load({
        .filename    = "./res/fonts/Cozette/CozetteVector.ttf",
        .size        = 13,
        .family      = "Cozette",
        .style       = "Regular",
        .render_mode = text_render_mode::raster
    });
    m_typeface = m_manager->family("Cozette")->typeface("Regular");
    reload();

    auto vs = read_text("./shaders/opengl/text.vert");
    auto fs = read_text("./shaders/opengl/text.frag");
    m_shader_normal = make_shader(vs, fs);
}

auto text_engine::text(std::string const& str, glm::vec3 const& position, glm::vec4 const& color, typeface_ref_t const& typeface) -> void {
    auto it = m_batches.find(typeface);
    if (it == std::end(m_batches)) it = m_batches.find(m_typeface);
    auto& tf = it->first;
    auto& batch = it->second;

    // TODO: Handle UTF-8 encoding
    for (auto const& c : str) {
        auto const& gh = tf->query(c);
        batch.push(gh, position, color);
    }
}
auto text_engine::calc_size(std::string const& str, typeface_ref_t const& typeface) const -> glm::vec2 {
    (void)str;
    (void)typeface;
    return {};
}

auto text_engine::reload() -> void {
    for (auto const& [name, family] : m_manager->families()) {
        for (auto const& [style, tf] : family->typefaces()) {
            create_batch(tf);
        }
    }
}
auto text_engine::begin() -> void {
    for (auto& [tf, batch] : m_batches)
        batch.count = 0;
}
auto text_engine::end() -> void {
    for (auto& [tf, batch] : m_batches) {
        if (batch.count == 0) continue;
        batch.vertex_buffer->bind();
        batch.vertex_buffer->resize(batch.count * sizeof(gpu));
        batch.vertex_buffer->sub(batch.cache.data(), batch.count * sizeof(gpu));
        batch.vertex_buffer->unbind();

        if (tf->mode() == text_render_mode::subpixel) {
            render_subpixel(batch);
        } else {
            render_normal(batch);
        }
    }
}

auto text_engine::batch::push(glyph const& gh, glm::vec3 const& position, glm::vec4 const& color) -> void {
    auto const& code_uv = code_uvs.at(gh.codepoint);
    auto const xpos = float(gh.bearing_left + position.x);
    auto const ypos = -(float(gh.bitmap->height()) - float(gh.bearing_top) - position.y);
    auto const w = float(gh.bitmap->width());
    auto const h = float(gh.bitmap->height());
    auto const u = float(code_uv.x);
    auto const v = float(code_uv.y);

    gpu character{
        .color     = color,
        .size      = {w, h},
        .uv_offset = {u, v},
        .position  = {xpos, ypos, position.z}
    };

    if (count < cache.size()) {
        cache[count] = character;
    } else {
        cache.push_back(character);
    }
    ++count;
}
auto text_engine::render_normal(batch& batch) -> void {
    m_shader_normal->bind();
    batch.texture->bind();
    batch.buffer_layout->bind();
    m_index_buffer->bind();
    glDrawElementsInstanced(GL_TRIANGLES, GLsizei(m_index_buffer->size()), gl_type(m_index_buffer->type()), nullptr, GLsizei(batch.count));
}
auto text_engine::render_subpixel(batch& batch) -> void {
    if (batch.count == 0) return;
}

auto text_engine::create_batch(typeface_ref_t tf) -> void {
    auto it = m_batches.find(tf);
    auto is_new = false;
    if (it == std::end(m_batches)) {
        m_batches.insert({tf, {}});
        it = m_batches.find(tf);
        is_new = true;
    }

    auto& batch = it->second;
    resize_atlas(tf, batch);
    for (auto const& [code, glyph] : tf->glyphs()) {
        insert_atlas(glyph, tf, batch);
    }

    texture_props tex_props{};
    if (tf->mode() == text_render_mode::raster) {
        tex_props.min_filter = tex_filter::nearest;
        tex_props.mag_filter = tex_filter::nearest;
    } else {
        tex_props.min_filter = tex_filter::linear;
        tex_props.mag_filter = tex_filter::linear;
    }
    tex_props.wrap_s = tex_wrap::clamp_to_edge;
    tex_props.wrap_t = tex_wrap::clamp_to_edge;
    tex_props.mipmap = false;

    if (is_new) {
        batch.buffer_layout = make_attribute_descriptor();
        batch.buffer_layout->add(make_vertex_buffer(quad_vertices, sizeof(quad_vertices), type::f32, usage::static_draw), {
            {type::vec3, false, 0},
            {type::vec2, false, 0}
        });
        batch.vertex_buffer = make_vertex_buffer(nullptr, sizeof(gpu), type::f32, usage::dynamic_draw);
        batch.buffer_layout->add(batch.vertex_buffer, {
            {type::vec4, false, 1},
            {type::vec2, false, 1},
            {type::vec2, false, 1},
            {type::vec3, false, 1},
        });
        batch.texture = make_texture(batch.atlas, tex_props);
    }
    batch.texture->set(batch.atlas, tex_props);
}
auto text_engine::resize_atlas(typeface_ref_t& typeface, batch& batch) -> void {
    constexpr auto round_up2 = [](auto const& value) {
        return std::pow(2, std::ceil(std::log2(value) / std::log2(2)));
    };
    auto const cols = static_cast<std::size_t>(std::ceil(std::sqrt(typeface->glyphs().size())));
    auto const msp2 = static_cast<std::size_t>(round_up2(typeface->glyph_size()));  // Glyph max size round to power of 2
    auto const size = static_cast<std::size_t>(round_up2(cols * msp2));
    if (batch.atlas == nullptr || batch.atlas->width() != size) {
        batch.atlas = make_image_u8(nullptr, size, size, typeface->channels());
        batch.uv = {0, std::int32_t(size) - 1};
    }
}
auto text_engine::insert_atlas(glyph const& gh, typeface_ref_t& tf, batch& batch) -> void {
    auto& atlas = batch.atlas;
    auto const& bm = gh.bitmap;

    for (std::size_t i = 0; i < bm->height(); ++i) {
        for (std::size_t j = 0; j < bm->width(); ++j) {
            auto const pixel = bm->pixel(j, i);
            atlas->set(std::size_t(batch.uv.x) + j, std::size_t(batch.uv.y) - i, pixel);
        }
    }

    auto& uv = batch.uv;
    batch.code_uvs.insert_or_assign(gh.codepoint,
        glm::vec2{
            uv.x,
            uv.y - std::int32_t(bm->height() - 1)
        }
    );

    auto const glyph_size = tf->glyph_size();
    uv.x += std::int32_t(glyph_size);
    if (uv.x >= std::int32_t(atlas->width() - glyph_size)) {
        uv.x = 0;
        uv.y -= std::int32_t(glyph_size);
    }
}
} // namespace txt
