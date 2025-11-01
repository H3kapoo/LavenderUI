#version 440 core

uniform vec4 uColor = vec4(0.3, 0.1, 0.4, 1.0);
uniform vec4 uBorderColor = vec4(0.0, 0.0, 1.0, 1.0);
uniform vec4 uBorderSize = vec4(0);
uniform vec4 uBorderRadii = vec4(0);
uniform vec2 uResolution;
uniform int uUseTexture = 0;
uniform sampler2D uTexture;

in vec2 vTexCoords;
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

void main()
{
    vec2 p = vTexCoords;
    p.x *= uResolution.x;
    p.y *= uResolution.y;
    p -= vec2(uResolution.x / 2.0, uResolution.y / 2.0);

    /* Wanted size of the outside box. */
    vec2 outerBoxSize = vec2(uResolution.x, uResolution.y);

    /* Size of the content after the borders are in place. */
    vec2 contentSize = vec2(uResolution.x, uResolution.y);
    contentSize -= vec2(uBorderSize.z + uBorderSize.w, uBorderSize.x + uBorderSize.y);

    /* Center of the inner box aka the box formed after applying the border sizes for the outer box.*/
    vec2 innerBoxCenter = vec2(
        ((uBorderSize.z + uBorderSize.w) * 0.5) - uBorderSize.w - 0,
        ((uBorderSize.x + uBorderSize.y) * 0.5) - uBorderSize.y);

    /* Invert inner border radius..for some reason. */
    vec4 innerBorderRadius = uBorderRadii;
    float temp = innerBorderRadius.z;
    innerBorderRadius.z = innerBorderRadius.x;
    innerBorderRadius.x = temp;

    temp = innerBorderRadius.w;
    innerBorderRadius.w = innerBorderRadius.y;
    innerBorderRadius.y = temp;

    float outerBoxDist = roundedBoxSDF(p, (outerBoxSize / 2.0), uBorderRadii);
    float innerBoxDist = roundedBoxSDF((innerBoxCenter) - p, (contentSize / 2.0), innerBorderRadius / 2.0f);

    float outerBoxSdf = step(0.0001, outerBoxDist);
    float innerBoxSdf = step(0.0001, innerBoxDist);
    float inOutDiffSdf = innerBoxSdf - outerBoxSdf;

    if (outerBoxSdf >= 1) { discard; }

    /* Set the color of the inner content */
    vec4 finalColor = uUseTexture == 1 ? texture(uTexture, vTexCoords) : mix(uColor, vec4(0.0), innerBoxSdf);

    /* Set the color of the border */
    finalColor += mix(vec4(0.0), uBorderColor, inOutDiffSdf);

    fragColor = finalColor;
}