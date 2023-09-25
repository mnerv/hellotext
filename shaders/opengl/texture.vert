#version 410
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;
layout(location = 2) in vec4 a_color;
layout(location = 3) in mat4 a_model;    // Transform data
layout(location = 3) in mat3 a_model_uv; // UV transform data
layout(location = 4) in vec4 a_round;

out vec2 _uv;
out vec4 _color;
out mat3 _model_uv;
out vec4 _round;

uniform mat4 u_model       = mat4(1.0);
uniform mat4 u_view        = mat4(1.0);
uniform mat4 u_projection  = mat4(1.0);

void main() {
    _uv       = a_uv;
    _color    = a_color;
    _model_uv = a_model_uv;
    _round    = a_round;

    gl_Position = u_projection * u_view * u_model * a_model * vec4(a_position, 1.0);
}
