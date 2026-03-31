#version 440 core

uniform vec4 uColor = vec4(0.3, 0.1, 0.4, 1.0);
uniform vec4 uBorderColor = vec4(0.0, 0.0, 1.0, 1.0);
uniform vec4 uBorderSize = vec4(0); // TBLR
uniform vec4 uBorderRadii = vec4(0); // TL TR BL BR
uniform vec2 uResolution;
uniform vec2 uBotLeft;
uniform int uUseTexture = 0;
uniform sampler2D uTexture;

in vec2 vTexCoords;
in mat4 vProjection;
out vec4 fragColor;

/**
    Compute sdf of a box whose corners can be rounded individually.

    @param uv Uv coord
    @param halfSize Half size of the box
    @radii top/bot/left/right sizes of the corner radii
*/
float roundedBoxSDF(vec2 uv, vec2 halfSize, vec4 radii)
{
    vec2 absPos = abs(uv) - halfSize;
    float radius = uv.x > 0 ? (uv.y > 0 ? radii.z : radii.y) : (uv.y > 0 ? radii.w : radii.x);
    return length(max(absPos + radius, 0.0)) - radius;
}

float box(vec2 p, vec2 b)
{
    vec2 d = abs(p) - b;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

void main()
{
    vec2 uv = vTexCoords * uResolution;
    uv = floor(uv) + 0.5;
    // uv -= vec2(uResolution * 0.5);

    vec2 halfSize = 0.5 * (uResolution);

    float d = box(uv, halfSize);

    float border = step(-1.0, d) - step(0.0, d);

    // fragColor = vec4(border, 0.0, 0.0, 1.0);
    // fragColor = vec4(d, 0.0, 0.0, 1.0);
    // if (d == -1.0)
//     if (uv.x == 0.0)
//     {
//         fragColor = vec4(1.0, 0.0, 0.0, 1.0);
//     }
//     else
//     {
//         fragColor = vec4(0.0, 0.0, 0.0, 1.0);
//     }

    // fragColor = vec4(border, 0.0, 0.0, 1.0);
    fragColor = vec4(uv.x == 0.5 ? 1.0 : 0.0, 0.0, 0.0, 1.0);
    // fragColor = vec4(fract(uv.x), 0.0, 0.0, 1.0);
}