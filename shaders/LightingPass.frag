#version 450

layout(input_attachment_index = 0, binding = 0) uniform subpassInput positionAttachment;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput normalAttachment;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput albedoAttachment;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput pbrAttachment;

struct Light {
    float intensity;
    float innerCutoff;
    float outerCutoff;
    uint type;
    uint onShadowMap;
    uint padding1;
    uint padding2;
    uint padding3;
    vec3 position;
    vec3 direction;
    vec3 color;
};

layout(binding = 4) uniform LightingInfo {
    Light lights[16];
    vec3 cameraPos;
    mat4 view[4][6];
    mat4 proj[4];
    uint numLights;
    float ambientStrength;
    uint padding1;
    uint padding2;
};

layout(binding = 5) uniform sampler2DShadow shadowMap[4];
layout(binding = 6) uniform samplerCube shadowCubeMap[4];
layout(binding = 7) uniform sampler2D background;
// layout(binding = 8) uniform samplerCube skybox;


layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

float PCFShadow(sampler2DShadow shadowMap, vec3 shadowCoord, float currentDepth) {
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0); // Shadow map texel size

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(x, y) * texelSize;
            shadow += texture(shadowMap, vec3(shadowCoord.xy + offset, shadowCoord.z - 0.005));
        }
    }

    return shadow / 9.0; // 3x3 필터 적용
}

vec3 rotatedVectors[8];

vec3 getRotationAxis(vec3 direction) {
    vec3 worldUp = vec3(0.0, 1.0, 0.0);
    
    if (abs(dot(direction, worldUp)) > 0.99) {
        return normalize(vec3(1.0, 0.0, 0.0));
    }

    return normalize(cross(direction, worldUp));
}

vec3 rotateVector(vec3 v, vec3 axis, float angle) {
    float cosTheta = cos(angle);
    float sinTheta = sin(angle);
    
    return v * cosTheta + cross(axis, v) * sinTheta + axis * dot(axis, v) * (1.0 - cosTheta);
}


void generateRotatedVectors(vec3 direction) {
    vec3 rotationAxis = getRotationAxis(direction);
    float rotationAngle = radians(360.0 / 8.0);
    float angleStep = radians(0.1);
    for (int i = 0; i < 8; i++) {
        vec3 newRotationAxis = rotateVector(rotationAxis, direction, rotationAngle * float(i));
        rotatedVectors[i] = rotateVector(direction, newRotationAxis, angleStep);
    }
}

float PCFShadowCube(samplerCube shadowMap, vec3 fragToLight, float currentDepth) {
    float shadow = 0.0;
    generateRotatedVectors(fragToLight);

    float closestDepth = texture(shadowMap, fragToLight).r;
    shadow += (currentDepth - 0.005 > closestDepth) ? 0.0 : 1.0;

    for (int i = 0; i < 8; i++) {
        closestDepth = texture(shadowMap, rotatedVectors[i]).r;
        shadow += (currentDepth - 0.005 > closestDepth) ? 0.0 : 1.0;
    }
    
    return shadow / 9.0;
}


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

uint getCubeFace(vec3 L) {
    vec3 absL = abs(L);
    uint faceIndex;

    if (absL.x > absL.y && absL.x > absL.z) {
        // +X or -X
        faceIndex = (L.x > 0.0) ? 0u : 1u;
    } else if (absL.y > absL.x && absL.y > absL.z) {
        // +Y or -Y
        faceIndex = (L.y > 0.0) ? 2u : 3u;
    } else {
        // +Z or -Z
        faceIndex = (L.z > 0.0) ? 4u : 5u;
    }

    return faceIndex;
}

const float FLT_MAX = 3.4028235e+38 - 1; 

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

    uint shadowMapIndex = 0;

    if (fragPosition.x >= FLT_MAX) {
        outColor = texture(background, fragTexCoord);
        return;
    }

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

            // Shadow Cube Map 샘플링
            if (lights[i].onShadowMap == 1) {
                float closestDepth = texture(shadowCubeMap[shadowMapIndex], -L).r; // -L: light 방향
                uint faceIndex = getCubeFace(-L);
                mat4 lightView = view[shadowMapIndex][faceIndex];
                mat4 lightProj = proj[shadowMapIndex];
                mat4 lightViewProj = lightProj * lightView;
                vec4 lightSpacePosition = lightViewProj * vec4(fragPosition, 1.0);
                float currentDepth = lightSpacePosition.z / lightSpacePosition.w;
                float bias = 0.005;
                // shadowFactor = currentDepth - bias > closestDepth ? 0.0 : 1.0;  
                shadowFactor = PCFShadowCube(shadowCubeMap[shadowMapIndex], -L, currentDepth);
                attenuation *= shadowFactor;
                shadowMapIndex++;
            }
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


            if (lights[i].onShadowMap == 1 && shadowMapIndex < 4) {
                mat4 lightViewProj = proj[shadowMapIndex] * view[shadowMapIndex][0];
                vec4 lightSpacePosition = lightViewProj * vec4(fragPosition, 1.0);
                vec3 shadowCoord = lightSpacePosition.xyz / lightSpacePosition.w; // NDC 변환
                shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;

                float closestDepth = texture(shadowMap[shadowMapIndex], shadowCoord);
                float currentDepth = shadowCoord.z;
                float bias = 0.005;
                // shadowFactor = currentDepth - bias > closestDepth ? 0.0 : 1.0;

                shadowFactor = PCFShadow(shadowMap[shadowMapIndex], shadowCoord, shadowCoord.z);
                attenuation *= shadowFactor;
                shadowMapIndex++;
            }
        }
        else { // Directional Light
            L = normalize(-lights[i].direction);
            attenuation = 1.0;

            if (lights[i].onShadowMap == 1 && shadowMapIndex < 4) {
                mat4 lightViewProj = proj[shadowMapIndex] * view[shadowMapIndex][0];
                vec4 lightSpacePosition = lightViewProj * vec4(fragPosition, 1.0);
                vec3 shadowCoord = lightSpacePosition.xyz / lightSpacePosition.w; // NDC 변환
                shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;

                float closestDepth = texture(shadowMap[shadowMapIndex], shadowCoord);
                float currentDepth = shadowCoord.z;
                float bias = 0.005;
                // shadowFactor = currentDepth - bias > closestDepth ? 0.0 : 1.0;

                shadowFactor = PCFShadow(shadowMap[shadowMapIndex], shadowCoord, shadowCoord.z);
                attenuation *= shadowFactor;
                shadowMapIndex++;
            }
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

        float NdotL = max(dot(N, L), 0.05);
        vec3 diffuse = kD * albedo / 3.14159265359;
        vec3 radiance = lights[i].color * lights[i].intensity * NdotL * attenuation;

        finalColor += (diffuse + specular) * radiance;
    }
    outColor = vec4(clamp(finalColor, 0.0, 1.0), 1.0);
}
