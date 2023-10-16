#version 410
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;

// Instancing
layout(location = 2) in vec4 a_color;
layout(location = 3) in vec2 a_size;
layout(location = 4) in vec2 a_offset;
layout(location = 5) in vec3 a_position_offset;

out vec2 _uv;
out vec2 _size;
out vec2 _offset;
out vec4 _color;

uniform mat4 u_model       = mat4(1.0);
uniform mat4 u_view        = mat4(1.0);
uniform mat4 u_projection  = mat4(1.0);

void main() {
    _uv     = a_uv;
    _size   = a_size;
    _offset = a_offset;
    _color  = a_color;
    vec3 position = a_position;
    position.xy = (position.xy * a_size + a_position_offset.xy);
    gl_Position = u_projection * u_view * u_model * vec4(position, 1.0);
}
