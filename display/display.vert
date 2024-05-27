#version 330

layout (location = 0) in vec2 vaPos;

void main() {
    gl_Position = vec4(vec3(vec2(vaPos), 0.5), 1.0);
}
