#version 410 core
layout(location = 0, index = 0) out vec4 color;

in vec2 _uv;
in vec4 _color;
in vec2 _uv_offset;
in vec2 _uv_size;
// in vec4 _round;

uniform sampler2D u_texture;

void main() {
    color = texture(u_texture, _uv);
}
