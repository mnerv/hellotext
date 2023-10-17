#version 300 es
precision mediump float;
layout(location = 0) out vec4 color;

in vec2 _uv;
in vec4 _color;
in vec2 _uv_offset;
in vec2 _uv_size;
in vec2 _scale;

void main() {
    color = _color;
}
