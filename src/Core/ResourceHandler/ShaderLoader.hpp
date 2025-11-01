#pragma once

#include <cstdint>
#include <unordered_map>

#include "src/Utils/Logger.hpp"
#include "src/Core/Binders/GPUBinder.hpp"

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
    std::unordered_map<std::string, uint32_t> programIds_;
    bool checkCache_{true};
};
} // namespace lav::core