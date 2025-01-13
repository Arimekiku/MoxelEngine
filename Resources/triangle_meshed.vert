#version 460

layout (location = 0) in uint inPosition;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 outColor;

layout (push_constant) uniform Chunk
{
    vec3 worldPosition;
} chunk;

layout (binding = 0) uniform GlobalData
{
    mat4 uCamera;
} ubo;

void main()
{
    outColor = inColor;

    float x = float(inPosition & uint(31)); //2^5 - 1
    float y = float((inPosition & uint(992)) >> 5); //2^10 - 2^5
    float z = float((inPosition & uint(31744)) >> 10); //2^15 - 2^10
    vec3 localPosition = vec3(x, y, z);

    vec4 position = vec4(chunk.worldPosition + localPosition, 1.0f);
    gl_Position = ubo.uCamera * position;
}