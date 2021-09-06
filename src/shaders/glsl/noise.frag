#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 3) uniform sampler2D mount_texture;
layout(binding = 4) uniform sampler2D grass_texture;

layout(location = 0) in float noiseval;
layout(location = 1) in vec2 outUv;

layout(location = 0) out vec4 finalColor;


void main() {
    float s = smoothstep(0.50, 0.65, noiseval);
    vec4 a = mix(texture(mount_texture, outUv), texture(grass_texture, outUv), s);
    finalColor = a;
}
