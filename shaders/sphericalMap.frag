#version 450
#extension GL_ARB_shader_viewport_layer_array : enable

layout(location = 0) in vec3 outPosition;
layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D tex;

const vec2 invPi = vec2(0.1591549, 0.3183098862);

vec2 SampleSphericalMap(vec3 v) {
    return vec2(atan(v.z, v.x), asin(v.y)) * invPi + 0.5;
}

vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec2 uv = SampleSphericalMap(normalize(outPosition));
    vec3 color = texture(tex, uv).rgb;
    color = ACESFilm(color);
    fragColor = vec4(color, 1.0);
}
