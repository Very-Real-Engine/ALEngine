#version 450

layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 color;
} mvp;

layout(location = 0) out vec3 outColor;

void main() {
    gl_Position = mvp.proj * mvp.view * mvp.model * vec4(inPosition, 1.0);
    outColor = mvp.color;
}
