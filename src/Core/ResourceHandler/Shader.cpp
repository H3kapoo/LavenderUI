#include "Shader.hpp"

#include "src/Utils/Logger.hpp"
#include "vendor/glew/include/GL/glew.h"
#include "vendor/glm/gtc/type_ptr.hpp"

namespace lav::core
{
Shader::Shader(const uint32_t programId)
    : programId_(programId)
{}

auto Shader::uploadMat4(const std::string& name, const glm::mat4& val) const -> void
{
    constexpr uint32_t transposeMatrix = GL_FALSE;
    glUniformMatrix4fv(getLocation(name), 1, transposeMatrix, glm::value_ptr(val));
}

auto Shader::uploadMat4v(const std::string& name, const std::vector<glm::mat4>& vals) const -> void
{
    constexpr uint32_t transposeMatrix = GL_FALSE;
    glUniformMatrix4fv(getLocation(name), vals.size(), transposeMatrix, glm::value_ptr(vals[0]));
}

auto Shader::uploadVec2f(const std::string& name, const glm::vec2& val) const -> void
{
    glUniform2f(getLocation(name), val.x, val.y);
}

auto Shader::uploadVec4f(const std::string& name, const glm::vec4& val) const -> void
{
    glUniform4f(getLocation(name), val.x, val.y, val.z, val.w);
}

auto Shader::uploadInt(const std::string& name, const int32_t val) const -> void
{
    glUniform1i(getLocation(name), val);
}

auto Shader::uploadIntv(const std::string& name, const std::vector<int32_t>& val) const -> void
{
    glUniform1iv(getLocation(name), val.size(), val.data());
}

auto Shader::uploadTexture2DArray(const std::string& name, const uint32_t texUnit,
    const uint32_t texId) const -> void
{
    int32_t maxTextureUnitIds;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnitIds);
    if (texUnit + 1 > GL_TEXTURE0 + (uint32_t)maxTextureUnitIds)
    {
        utils::Logger("Shader").error(
            "GPU not able to support more than {} texture units. Tried to use unit {}",
            maxTextureUnitIds, (int32_t)texUnit - (int32_t)GL_TEXTURE0);
        return;
    }

    /* Shader needs texture unit location in range from [0..maxUnits], not from [GL_TEXTURE0..maxGL_TEXTURE] */
    uploadInt(name, texUnit - GL_TEXTURE0);

    /* Active unit needs to be indeed [GL_TEXTURE0..maxGL_TEXTURE] */
    glActiveTexture(texUnit);

    glBindTexture(GL_TEXTURE_2D_ARRAY, texId);
}

auto Shader::getLocation(const std::string& name) const -> int32_t
{
    int32_t location = glGetUniformLocation(programId_, name.c_str());
    if (location == -1)
    {
        utils::Logger log("Shader({})", programId_);

        log.warn("Can't find location named: '{}'", name);
        return -1;
    }
    return location;
}

auto Shader::bind() const -> void { glUseProgram(programId_); }

auto Shader::unbind() const -> void { glUseProgram(0); }

auto Shader::getId() const -> uint32_t { return programId_; }
} // namespace lav::core