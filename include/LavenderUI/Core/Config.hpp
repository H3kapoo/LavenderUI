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
    static inline const fs::path testsPath     { "test" };
    static inline const fs::path testViewsPath { testsPath / "views" };

    static inline const fs::path assetsPath    { "assets" };
    static inline const fs::path shadersPath   { assetsPath / "shaders" };
    static inline const fs::path texturesPath  { assetsPath / "textures" };
    static inline const fs::path fontsPath     { assetsPath / "fonts" };
};
} // namespace lav::core