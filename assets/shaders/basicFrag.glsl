#version 440

out vec4 fragColor;

in vec2 vTexCoords;

uniform vec4 uColor;

void main()
{
    // fragColor = vec4(0.5f, 1.0f, 1.0f, 1.0f);
    fragColor = uColor;
    // fragColor = vec4(vTexCoords, 0.0f, 0.5f);
}