#pragma once

#include <cstdint>
#include <string>

#include "src/Utils/Logger.hpp"
#include "vendor/glm/glm.hpp"

namespace src::resourceloaders
{
class Shader
{
public:
    Shader(const uint32_t programId);
    ~Shader() = default;
    Shader(Shader&&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader& operator=(Shader&&) = delete;

    auto uploadMat4(const std::string& name, const glm::mat4& val) -> void;
    auto uploadVec4f(const std::string& name, const glm::vec4& val) -> void;

    auto bind() const -> void;
    auto unbind() const -> void;
    auto getId() const -> uint32_t;

private:
    auto getLocation(const std::string& name) -> int32_t;

private:
    uint32_t programId_;
};
} // namespace src::resourceloaders