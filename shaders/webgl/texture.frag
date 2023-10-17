#version 300 es
precision mediump float;
layout(location = 0) out vec4 color;

in vec2 _uv;
in vec4 _color;
in vec2 _uv_offset;
in vec2 _uv_size;
in vec2 _scale;

uniform sampler2D u_texture;

void main() {
    vec2 uv = (_uv * _uv_size) + _uv_offset;
    color = texture(u_texture, uv);
}

