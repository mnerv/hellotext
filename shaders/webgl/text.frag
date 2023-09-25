#version 300 es
precision mediump float;

layout(location = 0) out vec4 color;

#ifndef RENDER_MODE
#define RENDER_MODE 0
#endif
#define SUBPIXEL 1
#define SDF 2

in vec2 _uv;
in vec2 _size;
in vec2 _offset;
in vec4 _color;

uniform vec2 u_size;
uniform sampler2D u_texture;

void main() {
    vec2 uv = vec2(
        _uv.x * (_size.x / u_size.x) + (_offset.x / u_size.x),
        _uv.y * (_size.y / u_size.y) + (_offset.y / u_size.y)
    );

    vec4 color_mask = vec4(0);

#if RENDER_MODE == SUBPIXEL
    vec4 s = texture(u_texture, uv);  // Texture sample
    color = _color;
    color_mask = _color.a * s;
#elif RENDER_MODE == SDF
    float d = texture(u_texture, uv).r;
    float aaf = fwidth(d);
    float a = smoothstep(0.5 - aaf, 0.5 + aaf, d);
    color = vec4(_color.rgb, a);
#else
    float d = texture(u_texture, uv).r;
    color = vec4(_color.rgb, d);
#endif
}
