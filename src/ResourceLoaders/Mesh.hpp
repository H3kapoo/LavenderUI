#pragma once

#include <cstdint>

namespace src::resourceloaders
{
class Mesh
{
public:
    Mesh(const uint32_t vaoId);
    ~Mesh() = default;
    Mesh(Mesh&&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh& operator=(Mesh&&) = delete;

    auto bind() const -> void;
    auto unbind() const -> void;
    auto vao() const -> uint32_t;

private:
    uint32_t vaoId_;
};
} // namespace src::resourceloaders