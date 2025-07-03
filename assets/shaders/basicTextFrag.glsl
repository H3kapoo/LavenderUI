#version 440 core

uniform sampler2DArray uTextureArray;
uniform int[256] uCharIndices;
uniform vec4 uColor;

out vec4 fragColor;

in vec2 fTexCoords;
flat in int fInstanceId;

void main()
{
    int zSliceIndex = uCharIndices[fInstanceId];
    float t = texture(uTextureArray, vec3(fTexCoords, zSliceIndex)).r;

    fragColor = vec4(uColor.xyz, t);
}