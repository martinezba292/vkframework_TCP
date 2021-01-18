#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform samplerCube cubeMap;

layout(location = 0) in vec3 inPos;

layout(location = 0) out vec4 finalColor;


void main() {
    finalColor = texture(cubeMap, inPos);
}
