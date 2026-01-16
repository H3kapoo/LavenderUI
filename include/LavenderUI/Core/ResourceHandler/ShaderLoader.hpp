#pragma once

#include <cstdint>
#include <unordered_map>

#include "include/LavenderUI/Utils/Logger.hpp"
#include "include/LavenderUI/Core/Binders/GPUBinder.hpp"

namespace lav::core
{
namespace fs = std::filesystem;

class ShaderLoader
{
public:
    static auto get() -> ShaderLoader&;

    auto load(const fs::path& vertexPath, const fs::path& fragPath) -> uint32_t;
    auto checkCacheFirst(const bool value) -> void;

private:
    ShaderLoader();
    ~ShaderLoader() = default;
    ShaderLoader(const ShaderLoader&) = delete;
    ShaderLoader(ShaderLoader&&) = delete;
    ShaderLoader& operator=(const ShaderLoader&) = delete;
    ShaderLoader& operator=(ShaderLoader&&) = delete;

    auto loadPart(const core::GPUBinder::ShaderPartType type, const fs::path& partPath) -> uint32_t;

private:
    utils::Logger log_;
    bool checkCache_;
    std::unordered_map<std::string, uint32_t> programIds_;
};
} // namespace lav::core