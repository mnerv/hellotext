#include "text_engine.hpp"
#include "renderer.hpp"
#include "utf8.h"

namespace txt {
static constexpr float quad_vertices[]{
//     x,     y,     z,       u,    v,
    0.0f,  0.0f,  0.0f,    0.0f, 0.0f,
    0.0f,  1.0f,  0.0f,    0.0f, 1.0f,
    1.0f,  1.0f,  0.0f,    1.0f, 1.0f,
    1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
};
static constexpr std::uint32_t quad_cw_indices[]{
    0, 1, 2,
    0, 2, 3
};

text_batch::text_batch(typeface_ref_t typeface) : m_typeface(typeface) {
    generate_atlas();
}

auto text_batch::generate_atlas() -> void {
    resize_atlas();
    m_max_delta_origin_ymin = 0;
    m_max_bearing_left      = 0;
    m_max_bearing_top       = 0;
    for (auto const& [code, glyph] : m_typeface->data()) {
        insert_bitmap(glyph);
        m_max_delta_origin_ymin = std::max(std::int32_t(glyph.bitmap->height()) - glyph.bearing_top, m_max_delta_origin_ymin);
        m_max_bearing_left = std::max(glyph.bearing_left, m_max_bearing_left);
        m_max_bearing_top  = std::max(glyph.bearing_top, m_max_bearing_top);
    }

    texture_props tex_props{};
    if (m_typeface->render_mode() == text_render_mode::raster) {
        tex_props.min_filter = tex_filter::nearest;
        tex_props.mag_filter = tex_filter::nearest;
    } else {
        tex_props.min_filter = tex_filter::linear;
        tex_props.mag_filter = tex_filter::linear;
    }
    tex_props.wrap_s = tex_wrap::clamp_to_edge;
    tex_props.wrap_t = tex_wrap::clamp_to_edge;
    tex_props.mipmap = false;

    if (m_texture == nullptr)
        m_texture = make_texture(m_atlas, tex_props);
    else
        m_texture->set(m_atlas, tex_props);
    m_is_typeface_valid = true;
}
auto text_batch::reset() -> void {
    if (m_data.size() - m_size > 256) m_data.resize(m_size);
    m_size = 0;
}
auto text_batch::push(glyph const& gh, glm::vec3 const& position, glm::vec4 const& color, glm::vec2 const& scale) -> void {
    auto const xpos = float(gh.bearing_left) + position.x;
    auto const ypos = -(float(gh.bitmap->height()) - float(gh.bearing_top)) + position.y;
    auto const w = float(gh.bitmap->width());
    auto const h = float(gh.bitmap->height());
    auto const& uv = m_uv_map.at(gh.codepoint);

    gpu const new_char{
        .color     = color,
        .position  = {xpos, ypos, position.z},
        .scale     = {scale, 1.0f},
        .uv_offset = glm::vec2{uv},
        .uv_size   = {w, h}
    };

    if (m_size < m_data.size())
        m_data[m_size] = new_char;
    else
        m_data.push_back(new_char);
    ++m_size;
}

auto text_batch::resize_atlas() -> void {
    constexpr auto round_up2 = [](auto const& value) {
        return std::pow(2, std::ceil(std::log2(value) / std::log2(2)));
    };
    auto const cols = static_cast<std::size_t>(std::ceil(std::sqrt(m_typeface->size())));
    auto const msp2 = static_cast<std::size_t>(round_up2(m_typeface->max_size_dim()));  // Glyph max size round to power of 2
    auto const size = static_cast<std::size_t>(round_up2(cols * msp2));
    if (m_atlas == nullptr || m_atlas->width() != size) {
        m_atlas = make_image_u8(nullptr, size, size, m_typeface->channels());
        m_current_uv = {0, std::int32_t(size) - 1};
    }
}
auto text_batch::insert_bitmap(txt::glyph const& glyph) -> void {
    auto const& bm = glyph.bitmap;
    for (std::size_t i = 0; i < bm->height(); ++i) {
        for (std::size_t j = 0; j < bm->width(); ++j) {
            auto const pixel = bm->pixel(j, i);
            m_atlas->set(std::size_t(m_current_uv.x) + j, std::size_t(m_current_uv.y) - i, pixel);
        }
    }

    m_uv_map.insert_or_assign(glyph.codepoint,
        glm::vec2{
            float(m_current_uv.x),
            float(m_current_uv.y - std::int32_t(bm->height() - 1))
        }
    );

    auto const glyph_size = m_typeface->max_size_dim();
    m_current_uv.x += std::int32_t(glyph_size);
    if (m_current_uv.x >= std::int32_t(m_atlas->width() - glyph_size)) {
        m_current_uv.x = 0;
        m_current_uv.y -= std::int32_t(glyph_size);
    }
}

text_engine::text_engine(window_ref_t window, font_manager_ref_t manager) : m_window(window), m_manager(manager) {
    m_index_buffer = make_index_buffer(quad_cw_indices, sizeof(quad_cw_indices), len(quad_cw_indices), type::u32, usage::static_draw);
    m_instance_buffer = make_vertex_buffer(nullptr, sizeof(text_batch::gpu), type::f32, usage::dynamic_draw, {
        {type::vec4, false, 1},
        {type::vec3, false, 1},
        {type::vec3, false, 1},
        {type::vec2, false, 1},
        {type::vec2, false, 1},
    });
    m_descriptor = make_attribute_descriptor();
    m_descriptor->add(make_vertex_buffer(quad_vertices, sizeof(quad_vertices), type::f32, usage::static_draw, {
        {type::vec3, false, 0},
        {type::vec2, false, 0},
    }));
    m_descriptor->add(m_instance_buffer);

    m_manager->load({
        .filename    = "./res/fonts/Cozette/CozetteVector.ttf",
        .size        = 13,
        .family      = "Cozette",
        .style       = "Regular",
        .render_mode = text_render_mode::raster,
        .ranges      = default_character_range,
    });
    m_typeface = m_manager->family("Cozette")->typeface("Regular");
    reload();

#ifndef __EMSCRIPTEN__
    auto vs = read_text("./shaders/opengl/text.vert");
    auto fs = read_text("./shaders/opengl/text.frag");
#else
    auto vs = read_text("./shaders/webgl/text.vert");
    auto fs = read_text("./shaders/webgl/text.frag");
#endif
    m_shader_normal = make_shader(vs, fs);
}
auto text_engine::load(typeface_props const props) -> void {
    m_manager->load({
        .filename = props.filename,
        .size     = props.size,
        .family   = props.family,
        .style    = props.style,
        .render_mode = props.render_mode,
        .ranges      = props.ranges,
        .scale       = props.render_mode == text_render_mode::raster ? 1.0 : m_window->content_scale_x()
    });
}

auto text_engine::typeface(std::string const& family, std::string const& style) -> typeface_ref_t {
    auto const it = m_manager->families().find(family);
    if (it == std::end(m_manager->families())) return nullptr;
    return it->second->typeface(style);
}

auto text_engine::text(std::string const& str, glm::vec3 const& position, glm::vec4 const& color, glm::vec2 const& scale, typeface_ref_t const& typeface) -> void {
    typeface_ref_t current = typeface == nullptr ? m_typeface : typeface;
    auto batch_it = m_batches.find(current);
    if (batch_it == std::end(m_batches)) reload();
    batch_it = m_batches.find(current);
    auto& batch = batch_it->second;

    std::u32string tmp_str{};
    utf8::utf8to32(std::begin(str), std::end(str), std::back_inserter(tmp_str));
    glm::vec2 pos = position;
    // std::int64_t advance_y = 0;
    auto font_scale = 1.0f;
    if (current->render_mode() != text_render_mode::raster) {
        font_scale *= 1.0f / float(m_window->content_scale_x());
    }

    for (auto const& code : tmp_str) {
        auto it = current->find(code);
        if (it == std::end(*current)) {
            it = current->load(code);
            if (it == std::end(*current)) it = current->find(L'?');
            else batch.generate_atlas();
        }
        auto const& gh = it->second;
        batch.push(gh, {pos.x, pos.y + float(batch.max_delta_origin_ymin()), position.z}, color, scale * font_scale);
        pos.x += float(gh.advance_x >> 6) * scale.x * font_scale;
    }
}
auto text_engine::text_size(std::string const& str, glm::vec2 const& scale, typeface_ref_t const& typeface) -> glm::vec2 {
    typeface_ref_t current = typeface == nullptr ? m_typeface : typeface;
    auto batch_it = m_batches.find(current);
    if (batch_it == std::end(m_batches)) reload();
    batch_it = m_batches.find(current);
    auto& batch = batch_it->second;

    std::u32string tmp_str{};
    utf8::utf8to32(std::begin(str), std::end(str), std::back_inserter(tmp_str));
    glm::vec2 pos{0.0f};
    // std::int64_t advance_y = 0;
    auto font_scale = 1.0f;
    if (current->render_mode() != text_render_mode::raster) {
        font_scale *= 1.0f / float(m_window->content_scale_x());
    }

    glm::vec2 min_position{limits<float>::max()};
    glm::vec2 max_position{limits<float>::min()};
    for (auto const& code : tmp_str) {
        auto it = current->find(code);
        if (it == std::end(*current)) {
            it = current->load(code);
            if (it == std::end(*current)) it = current->find(L'?');
            else batch.generate_atlas();
        }
        auto const& gh = it->second;

        glm::vec2 const bl{
            pos.x,
            pos.y - float(batch.max_delta_origin_ymin()) * scale.y * font_scale
        };
        glm::vec2 const tr{
            pos.x + float(batch.max_bearing_left() + std::int32_t(gh.bitmap->width())) * scale.x * font_scale,
            // pos.y + float(batch.max_bearing_top()) * scale.y * font_scale
            pos.y + float(gh.bitmap->height()) * scale.y * font_scale
        };
        min_position.x = std::min(bl.x, min_position.x);
        min_position.y = std::min(bl.y, min_position.y);
        max_position.x = std::max(tr.x, max_position.x);
        max_position.y = std::max(tr.y, max_position.y);
        pos.x += float(gh.advance_x >> 6) * scale.x * font_scale;
    }

    return max_position - min_position;
}

auto text_engine::reload() -> void {
    m_manager->reload();
    m_batches.clear();
    for (auto const& [name, family] : m_manager->families()) {
        for (auto const& [style, typeface] : family->typefaces()) {
            m_batches.insert_or_assign(typeface, text_batch{typeface});
        }
    }
}
auto text_engine::begin() -> void {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (auto& [tf, batch] : m_batches)
        batch.reset();
}
auto text_engine::end() -> void {
    for (auto const& [tf, batch] : m_batches) {
        if (batch.size() == 0) continue;
        m_instance_buffer->bind();
        m_instance_buffer->resize(batch.size() * sizeof(text_batch::gpu));
        m_instance_buffer->sub(batch.chars().data(), batch.size() * sizeof(text_batch::gpu));
        m_instance_buffer->unbind();

        if (tf->render_mode() == text_render_mode::subpixel)
            render_subpixel(batch);
        else
            render_normal(batch);
    }
}

auto text_engine::render_normal(text_batch const& batch) -> void {
    m_shader_normal->bind();
    m_model = glm::mat4{1.0f};
    m_shader_normal->upload_mat4("u_model", m_model);
    m_shader_normal->upload_mat4("u_view", m_view);
    m_shader_normal->upload_mat4("u_projection", m_projection);
    m_shader_normal->upload_vec2("u_size", {float(batch.texture()->width()), float(batch.texture()->height())});
    m_shader_normal->upload_num("u_texture", 0);
    batch.texture()->bind(0);
    m_descriptor->bind();
    m_index_buffer->bind();
    glDrawElementsInstanced(GL_TRIANGLES, GLsizei(m_index_buffer->size()), gl_type(m_index_buffer->type()), nullptr, GLsizei(batch.size()));
}
auto text_engine::render_subpixel(text_batch const& batch) -> void {
    (void)batch;
}
} // namespace txt
