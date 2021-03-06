#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;

#define LIGHT

layout(binding = 0) uniform SceneUniformBuffer {
    mat4 proj;
    mat4 view;
    vec3 camPos;
    LightSource lights[MAX_LIGHTS];
    int light_number;
} sb;

layout(binding = 1) uniform UniformBufferObject {
    mat4 model;
    int textureIndex;
    vec3 padding;
} ubo;

layout(location = 0) out vec2 outUv;
layout(location = 1) out int outTIndex;

void main() {
    gl_Position = sb.proj * sb.view * ubo.model * vec4(inPosition, 1.0);
    outUv = inUv;
    outTIndex = ubo.textureIndex;
}
