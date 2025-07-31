#version 440 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 vTex;

uniform mat4 uMatrixTransform;
uniform mat4 uMatrixProjection;

out vec2 vTexCoords;

void main()
{
    vTexCoords = vTex;
    gl_Position = uMatrixProjection * uMatrixTransform * vec4(vPos, 1.0f);
}
