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
    float randc;
    float amp;
    vec2 padding;
} ubo;

layout(binding = 2) uniform sampler2D noise_texture;

layout(location = 0) out float noiseval;
layout(location = 1) out vec2 outUv;


void main() {
    vec4 tex = texture(noise_texture, inUv);
    vec3 pos = inPosition;
    pos.y += (tex.r * ubo.amp) - ubo.amp * 0.5;
    gl_Position = sb.proj * sb.view * ubo.model * vec4(pos, 1.0);
    noiseval = tex.r;
    outUv = inUv;
}
