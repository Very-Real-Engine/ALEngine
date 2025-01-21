#version 450

layout(input_attachment_index = 0, binding = 0) uniform subpassInput positionAttachment;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput normalAttachment;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput albedoAttachment;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput pbrAttachment;

struct Light {
    mat4 view;
    mat4 proj;
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float innerCutoff;
    float outerCutoff;
    uint type;
};

layout(binding = 4) uniform LightingInfo {
    Light lights[16];
    vec3 cameraPos;
    uint numLights;
    float ambientStrength;
};

layout(binding = 5) uniform sampler2DShadow shadowMap;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

// Fresnel-Schlick Approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Normal Distribution Function (NDF)
float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denominator = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (3.14159265359 * denominator * denominator);
}

// Geometry Function
float geometrySchlickGGX(float NdotV, float roughness) {
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return geometrySchlickGGX(NdotV, roughness) * geometrySchlickGGX(NdotL, roughness);
}

void main() {
    vec3 fragPosition = subpassLoad(positionAttachment).rgb;
    vec3 fragNormal = normalize(subpassLoad(normalAttachment).rgb);
    vec3 albedo = subpassLoad(albedoAttachment).rgb;

    vec4 pbr = subpassLoad(pbrAttachment);
    float roughness = pbr.r;
    float metallic = pbr.g;
    float ao = pbr.b;

    vec3 N = fragNormal;
    vec3 V = normalize(cameraPos - fragPosition);

    vec3 finalColor = vec3(0.0);

    vec3 ambient = ambientStrength * albedo * ao;
    finalColor += ambient;

    float shadowFactor = 1.0;

    for (uint i = 0; i < numLights; ++i) {
        vec3 L;
        float attenuation = 1.0;

        if (lights[i].type == 0) { // Point Light
            L = normalize(lights[i].position - fragPosition);
            float distance = length(lights[i].position - fragPosition);
            float constant = 1.0;
            float linear = 0.09;
            float quadratic = 0.032;
            attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));
        }
        else if (lights[i].type == 1) { // Spot Light
            L = normalize(lights[i].position - fragPosition);
            float distance = length(lights[i].position - fragPosition);
            float constant = 1.0;
            float linear = 0.09;
            float quadratic = 0.032;
            attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));

            float theta = dot(L, normalize(-lights[i].direction));
            float epsilon = max(lights[i].innerCutoff - lights[i].outerCutoff, 0.001);
            attenuation *= clamp((theta - lights[i].outerCutoff) / epsilon, 0.0, 1.0);

            // Shadow Mapping (Spotlight만)
            mat4 lightViewProj = lights[i].proj * lights[i].view;
            vec4 lightSpacePosition = lightViewProj * vec4(fragPosition, 1.0);
            vec3 shadowCoord = lightSpacePosition.xyz / lightSpacePosition.w; // NDC 변환
            shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;

            float closestDepth = texture(shadowMap, shadowCoord);
            float currentDepth = shadowCoord.z;
            float bias = 0.005;
            shadowFactor = currentDepth - bias > closestDepth ? 0.0 : 1.0;

            attenuation *= shadowFactor;
        }
        else { // Directional Light
            L = normalize(-lights[i].direction);
            attenuation = 1.0;
        }

        vec3 H = normalize(V + L);
        vec3 F0 = mix(vec3(0.04), albedo, metallic);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        float NDF = distributionGGX(N, H, roughness);
        float G = geometrySmith(N, V, L, roughness);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        vec3 diffuse = kD * albedo / 3.14159265359;
        vec3 radiance = lights[i].color * lights[i].intensity * NdotL * attenuation;

        finalColor += (diffuse + specular) * radiance;
    }
    outColor = vec4(clamp(finalColor, 0.0, 1.0), 1.0);
}
