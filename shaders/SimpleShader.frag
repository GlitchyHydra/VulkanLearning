#version 450

#ifdef GL_ES
precision mediump float;
#endif

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv_Coords;

layout(binding = 1) uniform sampler2D TextureImage;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(TextureImage, uv_Coords);
    //outColor = vec4(uv_Coords, 0.0, 1.0);
}