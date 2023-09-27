#version 410
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;

layout(location = 2) in vec4 a_color;
layout(location = 3) in vec3 a_transform_position;
layout(location = 4) in vec3 a_transform_scale;
layout(location = 5) in vec3 a_transform_rotation;
layout(location = 6) in vec2 a_uv_offset;
layout(location = 7) in vec2 a_uv_size;
layout(location = 8) in vec4 a_round;

out vec2 _uv;
out vec4 _color;
out vec2 _uv_offset;
out vec2 _uv_size;
out vec4 _round;

uniform mat4 u_model       = mat4(1.0);
uniform mat4 u_view        = mat4(1.0);
uniform mat4 u_projection  = mat4(1.0);

void main() {
    _uv        = a_uv;
    _color     = a_color;
    _uv_offset = a_uv_offset;
    _uv_size   = a_uv_size;
    _round     = a_round;

    gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0);
}
