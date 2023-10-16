#version 410
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;

layout(location = 2) in vec4 a_color;
layout(location = 3) in vec3 a_tp;
layout(location = 4) in vec3 a_ts;
layout(location = 5) in vec3 a_tr;
layout(location = 6) in vec2 a_uv_offset;
layout(location = 7) in vec2 a_uv_size;
// layout(location = 8) in vec4 a_round;

out vec2 _uv;
out vec4 _color;
out vec2 _uv_offset;
out vec2 _uv_size;
// out vec4 _round;

out vec2 _scale;

uniform mat4 u_model       = mat4(1.0);
uniform mat4 u_view        = mat4(1.0);
uniform mat4 u_projection  = mat4(1.0);
uniform float u_time = 1.0;

void main() {
    _uv        = a_uv;
    _color     = a_color;
    _uv_offset = a_uv_offset;
    _uv_size   = a_uv_size;
    _scale     = a_ts.xy;
    // _round     = a_round;

    mat4 model = mat4(1.0);
    // Translation
    model = transpose(mat4(
        vec4(1.0, 0.0, 0.0, a_tp.x),
        vec4(0.0, 1.0, 0.0, a_tp.y),
        vec4(0.0, 0.0, 1.0, a_tp.z),
        vec4(0.0, 0.0, 0.0, 1.0)
    ));
    // Rotation - Z
    float theta = a_tr.z;
    model *= transpose(mat4(
        vec4(cos(theta), -sin(theta), 0.0, 0.0),
        vec4(sin(theta),  cos(theta), 0.0, 0.0),
        vec4(       0.0,         0.0, 1.0, 0.0),
        vec4(       0.0,         0.0, 0.0, 1.0)
    ));
    // Scale
    model *= transpose(mat4(
        vec4(a_ts.x, 0.0, 0.0, 0.0),
        vec4(0.0, a_ts.y, 0.0, 0.0),
        vec4(0.0, 0.0, a_ts.z, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    ));

    gl_Position = u_projection * u_view * u_model * model * vec4(a_position, 1.0);
}
