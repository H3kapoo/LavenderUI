#version 440 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 vTex;
// layout (location = 2) in vec3 vCol;

uniform mat4[256] uModelMatrices;
uniform mat4 uMatrixProjection;

out vec2 fTexCoords;
flat out int fInstanceId;

void main()
{
    fTexCoords = vTex;
    fInstanceId = gl_InstanceID;
    gl_Position = uMatrixProjection * uModelMatrices[gl_InstanceID] * vec4(vPos.xyz, 1.0);
}