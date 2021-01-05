#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;

layout(binding = 0) uniform SceneUniformBuffer {
    mat4 view;
    mat4 proj;
} sb;

layout(binding = 1) uniform UniformBufferObject {
    mat4 model;
    vec4 color;
} ubo;

layout(location = 0) out vec4 outColor;
//layout(location = 0) out vec3 outNormal;
//layout(location = 1) out vec2 outUv;

void main() {
    gl_Position = sb.proj * sb.view * ubo.model * vec4(inPosition, 1.0);
    outColor = ubo.color;
    //outNormal = inNormal;
    //outUv = inUv;
}