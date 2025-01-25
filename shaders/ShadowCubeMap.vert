#version 450
#extension GL_ARB_shader_viewport_layer_array : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;

layout(binding = 0) uniform ShadowUniformBufferObject {
    mat4 proj;
    mat4 view[6];
    mat4 model;
} ubo;

layout(binding = 1) uniform LayerIndex {
    uint layerIndex;
} layerData;

void main() {
    gl_Layer = int(layerData.layerIndex);
    gl_Position = ubo.proj * ubo.view[gl_Layer] * ubo.model * vec4(inPosition, 1.0);
}
