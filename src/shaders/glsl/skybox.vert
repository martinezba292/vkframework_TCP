#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;

layout(binding = 0) uniform SceneUniformBuffer {
    mat4 view;
    mat4 proj;
    //vec3 camPos;
    //LightSource lights[MAX_LIGHTS];
    //int light_number;
} sb;

//layout(binding = 1) uniform UniformBufferObject {
//    mat4 model;
//} ubo;

layout(location = 0) out vec3 outPos;

void main() {
    outPos = inPosition;
    outPos.xy *= -1.0;
    mat4 cube_view = sb.view;
    cube_view[3] = vec4(0.0, 0.0, 0.0, 1.0);
    gl_Position = sb.proj * cube_view * vec4(inPosition, 1.0);
}
