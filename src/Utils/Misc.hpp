#pragma once

#include "vendor/glm/ext/vector_float4.hpp"
#include "vendor/glm/ext/vector_float2.hpp"
#include <print>
#include <memory>
#include <random>
#include <cxxabi.h>

namespace src::utils
{
inline auto genId() -> uint64_t
{
    static uint64_t id{1};
    return id++;
}

// inline auto demangleName(const char* name) -> std::string
// {
//     /* Works on linux+gcc for now, not sure about windows. */
//     int status = 0;
//     std::unique_ptr<char, void(*)(void*)> res {
//         abi::__cxa_demangle(name, nullptr, nullptr, &status),
//         std::free
//     };
//     if (status == 0)
//     {
//         std::string s{res.get()};
//         return s.substr(s.find_last_of(":") + 1);
//     }
//     return (status == 0) ? res.get() : name;
// }

template <typename T>
inline auto getTypeId() -> uint64_t
{
    // using PlainT = std::remove_cvref_t<T>;
    static uint64_t id = genId();
    // std::println("{} {}", id, demangleName(typeid(PlainT).name()));
    return id;
}

inline auto random01() -> float
{
    static std::random_device rd;
    static std::mt19937 generator(rd());

    static std::uniform_real_distribution<> distance(0.0, 1.0);
    return distance(generator);
}

inline auto randomRGB() -> glm::vec4 
{
    return {random01(), random01(), random01(), 1.0f};
}

inline auto randomRGBA() -> glm::vec4 
{
    return {random01(), random01(), random01(), random01()};
}

inline auto randomInt(const int32_t min, const int32_t max) -> int32_t
{
    return min + (random01() * (max - min));
}

/**
    Convert a hex string to a normalized RGBA vector.

    @param hexColor Hex color to convert (#RRGGBBAA)

    @return Normalized color vector
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
    Remamps a value that is normally between A and B to a value between C and D linearly.

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

template<typename T, typename... Args>
inline auto make(Args&&... args) -> std::shared_ptr<T>
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}
} // namespace src::utils