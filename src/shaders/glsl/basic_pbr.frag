#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 worldPosition;
layout(location = 1) in vec3 worldNormal;
layout(location = 0) out vec4 finalColor;

#define LIGHT

layout(binding = 0) uniform SceneUniformBuffer {
    mat4 view;
    mat4 proj;
    LightSource lights[MAX_LIGHTS];
    vec3 camPos;
    int light_number;
} sb;

layout(binding = 1) uniform UniformBufferObject {
    mat4 model;
    vec4 albedo;
    float roughness;
    float metallic;
    vec2 padding;
} ubo;

const float PI = 3.14159265359;

//Normal Distribution Function
float NormalDistribution(float dotNH, float roughness) {
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;

    return (alpha2)/(PI * denom*denom);
}

float GeometricShadowing(float dotNL, float dotNV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float GL = dotNL / (dotNL * (1.0 - k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);

    return GL*GV;
}

vec3 Fresnel(float cos_theta, float metallic) {
    vec3 F0 = mix(vec3(0.04), vec3(ubo.albedo.x, ubo.albedo.y, ubo.albedo.z), metallic); // material.specular;
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - cos_theta, 5.0);
    return F;
}

vec3 SpecularBRDF(vec3 L, vec3 V, vec3 N, float metallic, float roughness, vec3 albedo) {
    vec3 H = normalize(V + L);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);
    float dotLH = clamp(dot(L, H), 0.0, 1.0);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);

    vec3 color = vec3(0.0);
    if (dotNL > 0.0) {
        float rroughness = max(0.05, roughness);

        //Normal distribution of Microfacet
        float D = NormalDistribution(dotNH, roughness);
        //Microfacet Shadowing
        float G = GeometricShadowing(dotNL, dotNV, roughness);
        //Fresnel(Specular reflectance depending on angle of incidente)
        vec3 F = Fresnel(dotNV, metallic);

        vec3 spec = D * F * G / (4.0 * dotNL * dotNV);
        vec3 ks = F;
        vec3 kd = vec3(1.0) - ks;
        kd *= 1.0 - metallic;

        color += (kd * albedo / PI + spec) * dotNL;
    }
    return color;
}

void main() {
    vec3 N = normalize(worldNormal);
    vec3 V = normalize(sb.camPos.xyz - worldPosition);

    float roughness = ubo.roughness;

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < sb.light_number; i++) {
        vec3 light_position = sb.lights[i].pos.xyz;
        vec3 L = normalize(vec3(light_position - worldPosition));
        float distance = length(light_position - worldPosition);
        float attenuation = 1.0 / (distance * distance);
        float c = step(distance, 5.0);
        vec3 radiance = sb.lights[i].lightcolor.xyz * attenuation;
        Lo += c * radiance * SpecularBRDF(L, V, N, ubo.metallic, roughness, ubo.albedo.xyz);
    }
    vec3 color = vec3(ubo.albedo.x, ubo.albedo.y, ubo.albedo.z) * 0.03;
    color += Lo;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(0.4545));

    finalColor = vec4(color, 1.0);
}
