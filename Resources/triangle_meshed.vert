#version 460
#extension GL_EXT_buffer_reference : require

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 outColor;

layout (binding = 0) uniform UniformBufferObject_TEST {
    mat4 uModel;
    mat4 uView;
    mat4 uProj;
} ubo;

void main()
{
    outColor = inColor;

    //vec4 result = vec4(ubo.uProj * ubo.uView * ubo.uModel);
    gl_Position = ubo.uModel * vec4(inPosition, 1.0f);
}