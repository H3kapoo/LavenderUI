#include "ShaderLoader.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace lav::core
{
auto ShaderLoader::get() -> ShaderLoader&
{
    static ShaderLoader instance;
    return instance;
}

ShaderLoader::ShaderLoader()
    : log_("ShaderLoader")
{}

auto ShaderLoader::load(const fs::path& vertexPath, const fs::path& fragPath) -> uint32_t
{
    /* Easy fetch with no re-loading */
    std::string allPathKey = (vertexPath / fragPath).string();
    if (checkCache_ && programIds_.contains(allPathKey))
    {
        return programIds_.at(allPathKey);
    }

    uint32_t vertexId{loadPart(GPUBinder::ShaderPartType::VERTEX, vertexPath)};
    uint32_t fragId{loadPart(GPUBinder::ShaderPartType::FRAG, fragPath)};

    if (!vertexId || !fragId)
    {
        log_.error("One or more shader parts failed to load!");
        return 0;
    }

    uint32_t programId{core::GPUBinder::get().createProgram()};
    if (!core::GPUBinder::get().linkPartsToProgram(programId, vertexId, fragId))
    {
        log_.error("Program '{}' failed to link!", programId);
        return 0;
    }

    log_.debug("Loaded shader with programID {}", programId);
    programIds_[allPathKey] = programId;

    return programId;
}

auto ShaderLoader::checkCacheFirst(const bool value) -> void { checkCache_ = value; }

auto ShaderLoader::loadPart(const GPUBinder::ShaderPartType type, const fs::path& partPath) -> uint32_t
{
    std::ifstream partFile{partPath};
    if (!partFile.is_open())
    {
        log_.error("Could not open shader part for: {}", partPath.string());
        return 0;
    }
    log_.debug("Resolving {}..", partPath.string());

    std::ostringstream ss;
    ss << partFile.rdbuf();
    partFile.close();

    return core::GPUBinder::get().loadShaderPartType(type, ss.str());
}
} // namespace lav::core