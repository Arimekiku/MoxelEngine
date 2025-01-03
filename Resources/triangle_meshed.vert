#version 460
#extension GL_EXT_buffer_reference : require

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 outColor;

layout (binding = 0) uniform RenderData
{
    mat4 uModel;
    mat4 uCamera;
} ubo;

void main()
{
    outColor = inColor;

    gl_Position = ubo.uCamera * ubo.uModel * vec4(inPosition, 1.0f);
}