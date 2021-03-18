#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 worldPosition;
layout(location = 1) out vec3 worldNormal;

#define LIGHT

layout(binding = 0) uniform SceneUniformBuffer {
    mat4 proj;
    mat4 view;
    LightSource lights[MAX_LIGHTS];
    vec3 camPos;
    int light_number;
} sb;

layout(binding = 1) uniform UniformBufferObject {
    mat4 model;
    vec4 albedo;
    float roughness;
    float metallic;
    float specular;
    float exposure;
    float gamma;
    vec3 padding;
} ubo;


void main() {
    worldPosition = vec3(ubo.model * vec4(inPosition, 1.0));
    worldNormal = mat3(ubo.model) * inNormal;
    gl_Position = sb.proj * sb.view * vec4(worldPosition, 1.0);
}
