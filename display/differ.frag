#version 330

in vec2 uv;

uniform sampler2D src;
uniform sampler2D back0;
uniform sampler2D back1;

layout(location = 0) out vec4 outColor0;
layout(location = 1) out vec4 outColor1;

uniform int outIndex;

void main() {
    vec2 nuv = vec2(uv.x, 1 - uv.y);
    vec4 b0 = texture(back0, nuv);
    vec4 b1 = texture(back1, nuv);
    vec4 s = texture(src, uv);
    bool no_differ = length(s) < 0.0001;// no differ, use back
    vec4 differ_color = distance(s, vec4(1, 1, 1, 0)) < 0.0001 ? vec4(0) : s;// 1,1,1,0 means origin is transparent
    vec4 dest = no_differ ? (outIndex == 0 ? b1 : b0) : differ_color;
    outColor0 = outColor1 = dest;
}