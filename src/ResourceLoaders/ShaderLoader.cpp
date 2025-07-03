#include "ShaderLoader.hpp"

#include <GL/glext.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#include "vendor/glew/include/GL/glew.h"

namespace src::resourceloaders
{
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

    uint32_t vertexId{loadPart(ShaderType::VERTEX, vertexPath)};
    uint32_t fragId{loadPart(ShaderType::FRAG, fragPath)};

    if (!vertexId || !fragId)
    {
        log_.error("One or more shader parts failed to load!");
        return 0;
    }

    uint32_t programId{glCreateProgram()};
    glAttachShader(programId, vertexId);
    glAttachShader(programId, fragId);
    glLinkProgram(programId);
    glDeleteShader(vertexId);
    glDeleteShader(fragId);

    if (!checkStatus(programId, ShaderStatus::LINK))
    {
        log_.error("Linking shaders failure!");
        return 0;
    }

    log_.debug("Loaded shader with progranID {}", programId);
    programIds_[allPathKey] = programId;

    return programId;
}

auto ShaderLoader::checkCacheFirst(const bool value) -> void { checkCache_ = value; }

auto ShaderLoader::get() -> ShaderLoader&
{
    static ShaderLoader instance;
    return instance;
}

auto ShaderLoader::loadPart(const ShaderType type, const fs::path& partPath) -> uint32_t
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

    const std::string buffer = ss.str();
    uint32_t id{glCreateShader(type)};

    const char* data = buffer.c_str();
    glShaderSource(id, 1, &data, nullptr);
    glCompileShader(id);

    if (!checkStatus(id, ShaderStatus::COMPILE))
    {
        log_.error("Loading shader part failure!");
        return 0;
    }
    return id;
}

auto ShaderLoader::checkStatus(const uint32_t id, const ShaderStatus status) -> bool
{
    int32_t ok{0};
    char msg[512];

    switch (status)
    {
    case COMPILE:
        glGetShaderiv(id, status, &ok);
        if (!ok)
        {
            glGetShaderInfoLog(id, 512, NULL, msg);
            log_.error("Status check failed: {}", msg);
            return false;
        }
        break;
    case LINK:
        glGetProgramiv(id, status, &ok);
        if (!ok)
        {
            glGetProgramInfoLog(id, 512, NULL, msg);
            log_.error("Status check failed: {}", msg);
            return false;
        }
        break;
    }

    return true;
}
} // namespace src::resourceloaders