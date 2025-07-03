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

    auto uploadMat4(const std::string& name, const glm::mat4& val) const -> void;
    auto uploadMat4v(const std::string& name, const std::vector<glm::mat4>& vals) const -> void;
    auto uploadVec4f(const std::string& name, const glm::vec4& val) const -> void;
    auto uploadInt(const std::string& name, const int32_t val) const -> void;
    auto uploadIntv(const std::string& name, const std::vector<int32_t>& val) const -> void;
    auto uploadTexture2DArray(const std::string& name, const uint32_t texUnit,
        const uint32_t texId) const -> void;

    auto bind() const -> void;
    auto unbind() const -> void;
    auto getId() const -> uint32_t;

private:
    auto getLocation(const std::string& name) const -> int32_t;

private:
    uint32_t programId_;
};
} // namespace src::resourceloaders