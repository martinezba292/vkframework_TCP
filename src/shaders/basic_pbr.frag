#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "glsl_common.h"

layout(binding = 0) uniform SceneUniformBuffer {
    mat4 view;
    mat4 proj;
    //LightParams params[NLIGHTS];
    vec4 lights[4];
    vec4 camPos;
    int light_number;
} sb;

layout(binding = 1) uniform UniformBufferObject {
    mat4 model;
    vec4 color;
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
    vec3 F0 = mix(vec3(0.04), vec3(ubo.color.x, ubo.color.y, ubo.color.z), metallic); // material.specular;
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - cos_theta, 5.0);
    return F;
}

vec3 SpecularBRDF(vec3 L, vec3 V, vec3 N, float metallic, float roughness) {
    vec3 H = normalize(V + L);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);
    float dotLH = clamp(dot(L, H), 0.0, 1.0);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);

    vec3 light_color = vec3(1.0);
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
        color += spec * dotNL * light_color;
    }

    return color;
}


layout(location = 0) in vec3 worldPosition;
layout(location = 1) in vec3 worldNormal;

layout(location = 0) out vec4 finalColor;

void main() {

    vec3 N = normalize(worldNormal);
    vec3 V = normalize(sb.camPos.xyz - worldPosition);

    float roughness = ubo.roughness;

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < sb.light_number; i++) {
        vec3 light_position = sb.lights[i].xyz;
        vec3 L = normalize(vec3(light_position - worldPosition));
        Lo += SpecularBRDF(L, V, N, ubo.metallic, roughness);
    }
    
    vec3 color = vec3(ubo.color.x, ubo.color.y, ubo.color.z) * 0.02;
    color += Lo;
    
    color = pow(color, vec3(0.4545));

    finalColor = vec4(color, 1.0);
}