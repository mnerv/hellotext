#version 410
layout(location = 0, index = 0) out vec4 color;

in vec2 _uv;
in vec4 _color;
in vec2 _uv_offset;
in vec2 _uv_size;
// in vec4 _round;

void main() {
    // color = vec4(1.0, 0.0, 0.0, 1.0);
    // color = vec4(1.0);
    color = _color;
}
