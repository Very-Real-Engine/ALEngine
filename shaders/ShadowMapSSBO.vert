#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;

layout(set = 0, binding = 0) uniform ShadowUBO {
    mat4 proj;
    mat4 view;
} ubo;

layout(set = 0, binding = 1) buffer ObjectBuffer {
    mat4 model[];
} ssbo;

void main() {
    mat4 modelMatrix = ssbo.model[gl_InstanceIndex];
    gl_Position = ubo.proj * ubo.view * modelMatrix * vec4(inPosition, 1.0);
}
