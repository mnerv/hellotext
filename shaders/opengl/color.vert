#version 410
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in mat4 a_model;     // Transform data

out vec4 _color;

uniform mat4 u_model       = mat4(1.0);
uniform mat4 u_view        = mat4(1.0);
uniform mat4 u_projection  = mat4(1.0);

void main() {
    _color = a_color;
    gl_Position = u_projection * u_view * u_model * a_model * vec4(a_position, 1.0);
}
