#pragma once

#include <print>
#include <memory>
#include <random>

#include "vendor/glm/glm.hpp"


/* Fwd declaration for UIBase */
namespace src::uielements
{
    class UIBase;
}

namespace src::utils
{
/**
    @brief Generate a new Id.

    @return Newly generated id.
*/
inline auto genId() -> uint64_t
{
    static uint64_t id{1};
    return id++;
}

/**
    @brief Generate an Id based on a template type T.

    @return Newly generated id.
*/
template <typename T>
inline auto getTypeId() -> uint64_t
{
    static uint64_t id = genId();
    return id;
}

/**
    @brief Generate a random float between [0, 1].

    @return Newly generated random value.
*/
inline auto random01() -> float
{
    static std::random_device rd;
    static std::mt19937 generator(rd());

    static std::uniform_real_distribution<> distance(0.0, 1.0);
    return distance(generator);
}

/**
    @brief Generate a random RGB value.

    @return Newly generated random value.
*/
inline auto randomRGB() -> glm::vec4 
{
    return {random01(), random01(), random01(), 1.0f};
}

/**
    @brief Generate a random RGBA value.

    @return Newly generated random value.
*/
inline auto randomRGBA() -> glm::vec4 
{
    return {random01(), random01(), random01(), random01()};
}

/**
    @brief Generate a random integer between [min, max].

    @param min Minimum value
    @param max Maximum value

    @return Newly generated random integer.
*/
inline auto randomInt(const int32_t min, const int32_t max) -> int32_t
{
    return min + (random01() * (max - min));
}

/**
    @brief Convert a hex string to a normalized RGBA vector.

    @param hexColor hex color to convert (#RRGGBBAA)

    @return Normalized color vector.
*/
inline auto hexToVec4(const std::string& hexColor) -> glm::vec4
{
    /* A bit restrictive but good enough for now */
    if ((hexColor.size() != 7 && hexColor.size() != 9) || hexColor[0] != '#')
    {
        fprintf(stderr, "Invalid hex color format!\n");
        return {0.0f, 0.0f, 0.0f, 1.0f};
    }

    uint32_t r, g, b, a;
    sscanf(hexColor.c_str(), "#%02x%02x%02x%02x", &r, &g, &b, &a);

    if (hexColor.size() == 7) { a = 1.0f; }

    // Normalize the color values to the range [0, 1]
    glm::vec4 normalizedColor;
    normalizedColor.r = static_cast<float>(r) / 255.0f;
    normalizedColor.g = static_cast<float>(g) / 255.0f;
    normalizedColor.b = static_cast<float>(b) / 255.0f;
    normalizedColor.a = static_cast<float>(a) / 255.0f;

    return normalizedColor;
}

/**
    @brief Remamps a value that is normally between A and B to a value between C and D linearly.

    @param value Value to remap
    @param startA Value range start before remap
    @param startB Value range end before remap
    @param endA Value range start after remap
    @param endB Value range end after remap

    @return Remapped value
*/
inline auto remap(float value, const float startA, const float endA,
    const float startB, const float endB) -> float
{
    /*
        [a,b]
        [c,d]
        x from [a,b]
        t1 = (x-a)/(b-a)
        y = (1-t1)*c + t1*d
    */
    if (value > endA) { return endB; }
    if (value < startA) { return startB; }
    if (endA - startA < 0.001f) {return startA;}

    const float t = (value - startA) / (endA - startA);
    return (1.0f - t) * startB + t * endB;
}

/**
    @brief Simple wrapper around std::make_shared.

    @param args.. Arguments with which to create the object

    @return Newly constructed T object
*/
template<typename T, typename... Args>
inline auto make(Args&&... args) -> std::shared_ptr<T>
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

/**
    @brief Simple wrapper around static_pointer_cast<T>.

    @param obj Object to be downcasted statically to T

    @return Newly cast T object
*/
template<typename T>
inline auto as(std::shared_ptr<src::uielements::UIBase>& val) -> std::shared_ptr<T>
{
    return std::static_pointer_cast<T>(val);
}

} // namespace src::utils
