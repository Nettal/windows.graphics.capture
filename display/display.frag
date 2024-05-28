#version 330

in vec2 uv;

uniform sampler2D src0;
uniform sampler2D src1;

uniform int srcIndex;

void main() {
    if (uv.x < 0.5) {
        gl_FragColor = texture(src0, vec2(uv.x, 1 - uv.y));
    } else {
        gl_FragColor = texture(src1, vec2(uv.x, 1 - uv.y));
    }
}