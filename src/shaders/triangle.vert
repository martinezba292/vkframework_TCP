#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inUv;

layout(binding = 0) uniform SceneUniformBuffer {
    mat4 view;
    mat4 proj;
} sb;

layout(binding = 1) uniform UniformBufferObject {
    mat4 model;
} ubo;

layout(location = 0) out vec3 outNormal;

/*mat4 alt_model = mat4(
1.0, 0.0,  0.0, 0.0,
0.0, 1.0,  0.0, 0.0,
0.0, 0.0,  1.0, 0.0,
0.0, 0.0,  0.0, 1.0
);

mat4 alt_view = mat4(
1.0, 0.0,  0.0, 0.0,
0.0, 1.0,  0.0, 0.0,
0.0, 0.0,  1.0, 0.0,
0.0, 0.0, -1.0, 1.0
);

mat4 alt_proj = mat4(
1.8, 0.0,  0.0, 0.0,
0.0, 2.4,  0.0, 0.0,
0.0, 0.0,  -1.02, -1.0,
0.0, 0.0, -0.2, 0.0
);*/

void main() {
    gl_Position = sb.proj * sb.view * ubo.model * vec4(inPosition, 1.0);
    outNormal = inNormal;
}