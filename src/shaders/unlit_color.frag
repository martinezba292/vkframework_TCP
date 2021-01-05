#version 450
#extension GL_ARB_separate_shader_objects : enable

//layout(location = 0) in vec3 outNormal;
//layout(location = 1) in vec2 outUv;

//layout(binding = 2) uniform sampler2D texSampler;

layout(location = 0) in vec4 outColor;

layout(location = 0) out vec4 finalColor;


void main() {
    finalColor = outColor;

    //vec4 fcolor = mix(vec4(1.0, 0.2, 0.3, 1.0), vec4(0.0, 0.0, 1.0, 1.0), outNormal.z);
    //fcolor = mix(fcolor, vec4(0.0, 1.0, 0.0, 1.0), outNormal.y);
    //outColor = fcolor;

    //outColor = texture(texSampler, outUv);
}