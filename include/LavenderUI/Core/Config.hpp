#pragma once

#include <filesystem>

namespace lav::core
{
/**
    @brief
    Config paths for different resources needed by the app.
    All paths are relative to the location the executable is ran from.
*/
namespace fs = std::filesystem;

struct Config
{
    static inline std::filesystem::path testsPath     { "test" };
    static inline std::filesystem::path testViewsPath { testsPath / "views" };

    static inline std::filesystem::path assetsPath    { "assets" };
    static inline std::filesystem::path shadersPath   { assetsPath / "shaders" };
    static inline std::filesystem::path texturesPath  { assetsPath / "textures" };
    static inline std::filesystem::path fontsPath     { assetsPath / "fonts" };
};
} // namespace lav::core