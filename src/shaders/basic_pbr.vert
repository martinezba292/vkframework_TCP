#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "glsl_common.h"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;

layout(binding = 0) uniform SceneUniformBuffer {
    mat4 view;
    mat4 proj;
    //LightParams params[NLIGHTS];
    vec4 lights[4];
    int light_number;
    vec3 camPos;
} sb;

layout(binding = 1) uniform UniformBufferObject {
    mat4 model;
    vec4 color;
    float roughness;
    float metallic;
    vec2 padding;
} ubo;

layout(location = 0) out vec3 worldPosition;
layout(location = 1) out vec3 worldNormal;


void main() {
    worldPosition = vec3(ubo.model * vec4(inPosition, 1.0));
    worldNormal = mat3(ubo.model) * inNormal;
    gl_Position = sb.proj * sb.view * vec4(worldPosition, 1.0);
}