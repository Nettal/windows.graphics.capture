#version 330

in vec2 uv;

uniform sampler2D tex;

void main() {
    vec4 color = texture(tex, uv);
    gl_FragColor = color;
}