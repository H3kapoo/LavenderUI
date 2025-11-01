#include "Shader.hpp"

#include "src/Core/Binders/GPUBinder.hpp"
#include "src/Utils/Logger.hpp"

namespace lav::core
{
Shader::Shader(const uint32_t programId)
    : programId_(programId)
{}

auto Shader::uploadMat4(const std::string& name, const glm::mat4& val) const -> void
{
    reportFailure(
        name,
        GPUBinder::get().uploadUniform(programId_, name, val)
    );
}

auto Shader::uploadMat4v(const std::string& name, const std::vector<glm::mat4>& vals) const -> void
{
    reportFailure(
        name,
        GPUBinder::get().uploadUniform(programId_, name, vals)
    );
}

auto Shader::uploadVec2f(const std::string& name, const glm::vec2& val) const -> void
{
    reportFailure(
        name,
        GPUBinder::get().uploadUniform(programId_, name, val)
    );
}

auto Shader::uploadVec4f(const std::string& name, const glm::vec4& val) const -> void
{
    reportFailure(
        name,
        GPUBinder::get().uploadUniform(programId_, name, val)
    );
}

auto Shader::uploadInt(const std::string& name, const int32_t val) const -> void
{
    reportFailure(
        name,
        GPUBinder::get().uploadUniform(programId_, name, val)
    );
}

auto Shader::uploadIntv(const std::string& name, const std::vector<int32_t>& val) const -> void
{
    reportFailure(
        name,
        GPUBinder::get().uploadUniform(programId_, name, val)
    );
}

auto Shader::uploadTexture2D(const std::string& name, const uint32_t texSlot,
    const uint32_t texId) const -> void
{
    reportFailure(
        name,
        GPUBinder::get().uploadUniformTexture(
            programId_, name, GPUBinder::TextureType::Single2D, texSlot, texId)
    );
}

auto Shader::uploadTexture2DArray(const std::string& name, const uint32_t texSlot,
    const uint32_t texId) const -> void
{
    reportFailure(
        name,
        GPUBinder::get().uploadUniformTexture(
            programId_, name, GPUBinder::TextureType::Array2D, texSlot, texId)
    );
}

auto Shader::reportFailure(const std::string& name, const bool success) const -> void
{
    if (success) { return; }

    utils::Logger log("Shader({})", programId_);
    log.warn("Can't find location named: '{}'", name);
}

auto Shader::bind() const -> void { GPUBinder::get().useProgram(programId_); }

auto Shader::unbind() const -> void { GPUBinder::get().useProgram(0); }

auto Shader::getId() const -> uint32_t { return programId_; }
} // namespace lav::core