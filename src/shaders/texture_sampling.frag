#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 outUv;
layout(location = 1) flat in int inTIndex;

layout(binding = 2) uniform sampler2D texSampler[10];

layout(location = 0) out vec4 finalColor;


void main() {
    finalColor = texture(texSampler[inTIndex], outUv);
}