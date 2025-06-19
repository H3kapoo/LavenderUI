#include "Shader.hpp"

#include "vendor/glew/include/GL/glew.h"
#include "vendor/glm/gtc/type_ptr.hpp"

namespace src::resourceloaders
{
Shader::Shader(const uint32_t programId)
    : programId_(programId)
{}

auto Shader::uploadMat4(const std::string& name, const glm::mat4& val) -> void
{
    constexpr uint32_t transposeMatrix = GL_FALSE;
    glUniformMatrix4fv(getLocation(name), 1, transposeMatrix, glm::value_ptr(val));
}

auto Shader::uploadVec4f(const std::string& name, const glm::vec4& val) -> void
{
    glUniform4f(getLocation(name), val.x, val.y, val.z, val.w);
}

auto Shader::getLocation(const std::string& name) -> int32_t
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
} // namespace src::resourceloaders