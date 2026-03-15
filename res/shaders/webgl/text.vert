#version 300 es
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;

// Instancing
layout(location = 2) in vec4 a_color;
layout(location = 3) in vec3 a_tp;  // Transform position
layout(location = 4) in vec3 a_ts;  // Transform scale
layout(location = 5) in vec2 a_uv_offset;
layout(location = 6) in vec2 a_uv_size;

out vec2 _uv;
out vec2 _uv_offset;
out vec2 _uv_size;
out vec4 _color;
// out vec2 _scale;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    _uv        = a_uv;
    _color     = a_color;
    _uv_offset = a_uv_offset;
    _uv_size   = a_uv_size;

    mat4 model = transpose(mat4(
        vec4(1.0, 0.0, 0.0, a_tp.x),
        vec4(0.0, 1.0, 0.0, a_tp.y),
        vec4(0.0, 0.0, 1.0, a_tp.z),
        vec4(0.0, 0.0, 0.0, 1.0)
    ));
    model *= transpose(mat4(
        vec4(_uv_size.x * a_ts.x, 0.0, 0.0, 0.0),
        vec4(0.0, _uv_size.y * a_ts.y, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    ));

    gl_Position = u_projection * u_view * u_model * model * vec4(a_position, 1.0);
}
