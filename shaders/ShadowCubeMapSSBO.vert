#version 450
#extension GL_ARB_shader_viewport_layer_array : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;

layout(set = 0, binding = 0) uniform ShadowUniformBufferObject {
    mat4 proj;
    mat4 view[6];
} ubo;

layout(set = 0, binding = 1) buffer SSBO {
    mat4 model[];
} ssbo;

layout(push_constant) uniform PushConstants {
    uint layerIndex;
} pushData;

void main() {
    gl_Layer = int(pushData.layerIndex);
    gl_Position = ubo.proj * ubo.view[gl_Layer] * ssbo.model[gl_InstanceIndex] * vec4(inPosition, 1.0);
}
