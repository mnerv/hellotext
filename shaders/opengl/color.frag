#version 410
layout(location = 0, index = 0) out vec4 color;

in vec2 _uv;
in vec4 _color;
in vec2 _uv_offset;
in vec2 _uv_size;
// in vec4 _round;

in vec2 _scale;

float circle(vec2 position, vec2 center, float radius) {
    float d = length(position - center);
    return smoothstep(fwidth(d), 0.0, d - radius);
}

// float rect(vec2 position, vec2 center, vec2 size) {
//     vec2 d = abs(position) - size;
//     float od = length(max(d, 0.0));
//     float id = min(max(d.x, d.y), 0.0);
//     return od + id;
// }

void main() {
    // vec2 aspect = _scale / _scale.y;
    // vec2 uv = (_uv - 0.5) * aspect;
    // vec4 radiuses = _round / _scale.y;
    // vec2 bl = vec2(radiuses.x, radiuses.x);

    // color = _color * (1 - circle(uv, bl, radiuses.x));
    color = _color;
    // color = color * circle(uv, vec2(0), 0.5);
    // color += _color * vec4(1.0) * circle(uv, tl + vec2(0.5, -0.5), 0.5);
}
