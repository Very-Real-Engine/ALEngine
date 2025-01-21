#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inTangent;

layout(binding = 0) uniform ShadowUniformBufferObject {
    mat4 proj;
    mat4 view;
    mat4 model;         // 모델의 변환 행렬
};

void main() {
    // 월드 좌표계를 거쳐 그림자 맵 텍스처 좌표로 변환
    // gl_Position = lightViewProj * model * vec4(inPosition, 1.0);
    gl_Position = proj * view * model * vec4(inPosition, 1.0);
}
