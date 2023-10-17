#version 410
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;

// Instancing
layout(location = 2) in vec4 a_color;
layout(location = 3) in vec2 a_uv_size;
layout(location = 4) in vec2 a_uv_offset;
layout(location = 5) in vec3 a_tp;

out vec2 _uv;
out vec2 _size;
out vec2 _offset;
out vec4 _color;

uniform mat4 u_model       = mat4(1.0);
uniform mat4 u_view        = mat4(1.0);
uniform mat4 u_projection  = mat4(1.0);

void main() {
    _uv     = a_uv;
    _size   = a_uv_size;
    _offset = a_uv_offset;
    _color  = a_color;
    // vec3 position = a_position;
    // position.xy = (position.xy * _size + a_tp.xy);

    mat4 model = mat4(1.0);
    // Translation
    model = transpose(mat4(
        vec4(1.0, 0.0, 0.0, a_tp.x),
        vec4(0.0, 1.0, 0.0, a_tp.y),
        vec4(0.0, 0.0, 1.0, a_tp.z),
        vec4(0.0, 0.0, 0.0, 1.0)
    ));
    // Scale
    model *= transpose(mat4(
        vec4(_size.x, 0.0, 0.0, 0.0),
        vec4(0.0, _size.y, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    ));

    gl_Position = u_projection * u_view * u_model * model * vec4(a_position, 1.0);
}
