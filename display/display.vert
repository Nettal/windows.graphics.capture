#version 330

layout (location = 0) in vec4 vaIn;

out vec2 uv;

void main() {
    uv = vaIn.zw;
    gl_Position = vec4(vec3(vec2(vaIn.xy), 0.5), 1.0);
}
