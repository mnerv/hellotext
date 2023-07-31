#version 410 core

layout(location = 0, index = 0) out vec4 color;
layout(location = 0, index = 1) out vec4 color_mask;

in vec2 _uv;

uniform vec4 u_color = vec4(1.0, 0.0, 1.0, 1.0);

float circle(vec2 position, vec2 center, float radius) {
    float d = length(position - center);
    return smoothstep(fwidth(d), 0.0, d - radius);
}

float line(vec2 position, vec2 a, vec2 b, float thickness) {
    vec2 pa = position - a;
    vec2 ba = b - a;
    float t = clamp(dot(pa, ba) / dot(ba, ba), 0.0f, 1.0f);
    vec2 c = a + ba * t;
    float d = length(c - position);
    return smoothstep(fwidth(d), 0.0f, d - thickness);
}

void main() {
    vec2 uv = _uv - 0.5;

    vec4 P_color = vec4(1.0, 0.0, 0.0, 1.0);
    vec4 L_color = vec4(1.0);
    vec4 col = vec4(0);

    col += L_color * line(uv, vec2(0.0), vec2(0.0, 0.5), 0.01);
    col += vec4(0.0, 1.0, 0.0, 1.0) * line(uv, vec2(0.0), vec2(0.5, 0.5), 0.01);
    col += P_color * circle(uv, vec2(0.0), 0.1);

    color = col;
}
