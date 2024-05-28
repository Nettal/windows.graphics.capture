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
    if (outIndex == 0){
        //do comp
        outColor0 = mix(b1, s, s.a);

        outColor1 = b1;
    } else {
        //do comp
        outColor1 =  mix(b0, s, s.a);

        outColor0 = b0;
    }
}