#version 460

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 outColor;

layout (push_constant) uniform PerChunkData
{
    mat4 uModel;
} constants;

layout (binding = 0) uniform GlobalData
{
    mat4 uCamera;
} ubo;

void main()
{
    outColor = inColor;

    gl_Position = ubo.uCamera * constants.uModel * vec4(inPosition, 1.0f);
}