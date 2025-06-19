#include "Mesh.hpp"

#include "vendor/glew/include/GL/glew.h"

namespace src::resourceloaders
{
Mesh::Mesh(const uint32_t vaoId)
    : vaoId_(vaoId)
{}

auto Mesh::bind() const -> void { glBindVertexArray(vaoId_); }

auto Mesh::unbind() const -> void { glBindVertexArray(0); }

auto Mesh::vao() const -> uint32_t { return vaoId_; }
} // namespace src::resourceloaders