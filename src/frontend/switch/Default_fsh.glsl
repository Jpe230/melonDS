#version 460

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor; 

layout (binding = 0) uniform sampler2D inTexture;

void main()
{
    outColor = texture(inTexture, inUV) * inColor;
}