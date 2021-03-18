#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 worldPosition;
layout(location = 1) in vec3 worldNormal;

layout(location = 0) out vec4 finalColor;

#define LIGHT

layout(binding = 0) uniform SceneUniformBuffer {
    mat4 proj;
    mat4 view;
    LightSource lights[MAX_LIGHTS];
    vec3 camPos;
    int light_number;
} sb;

layout(binding = 1) uniform UniformBufferObject {
    mat4 model;
    vec4 albedo;
    float roughness;
    float metallic;
    float specular;
    float exposure;
    float gamma;
    vec3 padding;
} ubo;

layout(binding = 2) uniform samplerCube samplerIrradiance;
layout(binding = 3) uniform sampler2D samplerBRDFLUT;
layout(binding = 4) uniform samplerCube prefilteredMap;

const float PI = 3.14159265359;

vec3 Uncharted2Tonemap(vec3 x) {
  float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

//Normal Distribution Function
float NormalDistribution(float dotNH, float roughness) {
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;

    return (alpha2)/(PI * denom*denom);
}

float GeometricShadowing(float dotNL, float dotNV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float GL = dotNL / (dotNL * (1.0 - k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);

    return GL*GV;
}

vec3 Fresnel(float cos_theta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cos_theta, 5.0);
}

vec3 FresnelR(float cos_theta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cos_theta, 5.0);
}

vec3 PrefilteredReflection(vec3 R, float roughness) {
    const float kReflectionLod = 9.0;
    float lod = roughness * kReflectionLod;
    float lodf = floor(lod);
    float lodc = ceil(lod);
    vec3 a = textureLod(prefilteredMap, R, lodf).rgb;
    vec3 b = textureLod(prefilteredMap, R, lodc).rgb;

    return mix(a, b, lod - lodf);
}


vec3 SpecularBRDF(vec3 L, vec3 V, vec3 N, float metallic, float roughness, vec3 F0) {
    vec3 H = normalize(V + L);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);

    vec3 color = vec3(0.0);
    if (dotNL > 0.0) {
        //Normal distribution of Microfacet
        float D = NormalDistribution(dotNH, roughness);
        //Microfacet Shadowing
        float G = GeometricShadowing(dotNL, dotNV, roughness);
        //Fresnel(Specular reflectance depending on angle of incidente)
        vec3 F = Fresnel(dotNV, F0);

        vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);
        vec3 kd = (vec3(1.0) - F) * (1.0 - metallic);

        color += (kd * ubo.albedo.rgb / PI + spec) * dotNL;
    }
    return color;
}

void main() {
  vec3 N = normalize(worldNormal);
  vec3 V = normalize(sb.camPos.xyz - worldPosition);
  vec3 R = reflect(-V, N);

  float metallic = ubo.metallic;
  float roughness = ubo.roughness;
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, ubo.albedo.rgb, metallic);

  vec3 Lo = vec3(0.0);
  for (int i = 0; i < sb.light_number; i++) {
      vec3 light_position = sb.lights[i].pos.xyz;
      vec3 L = normalize(vec3(light_position - worldPosition));
      float distance = length(light_position - worldPosition);
      float attenuation = 1.0 / (distance * distance);
      float c = step(distance, 5.0);
      vec3 radiance = sb.lights[i].lightcolor.xyz * attenuation;
      Lo += c * radiance * SpecularBRDF(L, V, N, metallic, roughness, F0);
  }

  vec2 brdf = texture(samplerBRDFLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
  vec3 reflection = PrefilteredReflection(R, roughness).rgb;
  vec3 irradiance = texture(samplerIrradiance, N).rgb;
  vec3 diffuse = irradiance * ubo.albedo.rgb;
  vec3 F = FresnelR(max(dot(N,V), 0.0), F0, roughness);

  //Specular reflectance
  vec3 specular = reflection * (F * brdf.x + brdf.y);

  //Ambient
  vec3 kd = 1.0 - F;
  kd *= 1.0 - metallic;
  vec3 ambient = kd * diffuse + specular;
  vec3 color = ambient + Lo;

  //Tone mapping
  color = Uncharted2Tonemap(color * ubo.exposure);
  color = color * (1.0 / Uncharted2Tonemap(vec3(11.2)));

  //Gamma correction
  color = pow(color, vec3(1.0 / ubo.gamma));
  finalColor = vec4(color, 1.0);
}