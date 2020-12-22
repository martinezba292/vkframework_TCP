#version 330
#extension GL_ARB_separate_shader_objects : enable


/*vec3 colors[6] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);*/

layout(location = 0) in vec3 inPosition;
//layout(location = 1) in vec3 inColor;

//layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(inPosition, 1.0);
    //fragColor = colors[gl_VertexIndex];
}