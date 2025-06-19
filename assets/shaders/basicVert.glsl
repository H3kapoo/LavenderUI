#version 440

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoords;

out vec2 vTexCoords;

uniform mat4 uMatrixProjection;
uniform mat4 uMatrixTransform;

void main()
{
    vTexCoords = texCoords;
    gl_Position = uMatrixProjection * uMatrixTransform * vec4(pos, 1.0f);
}