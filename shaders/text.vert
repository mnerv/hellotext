#version 410 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;

// Instancing
layout(location = 2) in vec2 a_size;
layout(location = 3) in vec2 a_offset;
layout(location = 4) in vec2 a_uv_offset;

out vec2 _uv;
out vec2 _size;
out vec2 _offset;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    _uv = a_uv;
    _size = a_size;
    _offset = a_uv_offset;
    vec3 position = a_position;
    position.xy = (position.xy * a_size + a_offset);
    gl_Position = u_projection * u_view * u_model * vec4(position, 1.0);
}
