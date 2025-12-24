#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 texCoords;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo
{
  mat4 projection;// projection * view * model
  mat4 view;
  vec4 ambientLightColor; // w is intensity
  vec3 lightPosition;
  vec4 lightColor;
} ubo;

layout(push_constant) uniform Push 
{
  mat4 modelMatrix; 
  mat4 normalMatrix;
} push;

layout(set = 1, binding = 0) uniform sampler2D TextureImage;

void main() 
{
  vec4 texColor = texture(TextureImage, texCoords);

  vec3 directionToLight = ubo.lightPosition - fragPosWorld;
  float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared
  vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
  vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  float angleLight = dot(normalize(fragNormalWorld), normalize(directionToLight));
  vec3 diffuseLight = lightColor * max(angleLight, 0.0);

  //outColor = vec4(texCoords, 0.0, 1.0);
  //outColor = vec4((diffuseLight + ambientLight) * fragColor, 1.0);
  outColor = vec4((diffuseLight + ambientLight) * texColor.rgb, 1.0);
}