#pragma once

#include <cstdint>
#include <unordered_map>

#include "src/Utils/Logger.hpp"
#include "vendor/glew/include/GL/glew.h"

namespace lav::core
{
namespace fs = std::filesystem;

class ShaderLoader
{
public:
    auto load(const fs::path& vertexPath, const fs::path& fragPath) -> uint32_t;
    auto checkCacheFirst(const bool value) -> void;

public:
    static auto get() -> ShaderLoader&;

private:
    enum ShaderType
    {
        VERTEX = GL_VERTEX_SHADER,
        FRAG = GL_FRAGMENT_SHADER
    };

    enum ShaderStatus
    {
        COMPILE = GL_COMPILE_STATUS,
        LINK = GL_LINK_STATUS
    };

    ShaderLoader();
    ~ShaderLoader() = default;
    ShaderLoader(const ShaderLoader&) = delete;
    ShaderLoader(ShaderLoader&&) = delete;
    ShaderLoader& operator=(const ShaderLoader&) = delete;
    ShaderLoader& operator=(ShaderLoader&&) = delete;

    auto loadPart(const ShaderType type, const fs::path& partPath) -> uint32_t;
    auto checkStatus(const uint32_t id, const ShaderStatus status) -> bool;

private:
    utils::Logger log_;
    std::unordered_map<std::string, uint32_t> programIds_;
    bool checkCache_{true};
};
} // namespace lav::core