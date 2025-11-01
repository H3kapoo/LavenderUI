#include "Mesh.hpp"

#include "src/Core/Binders/GPUBinder.hpp"

namespace lav::core
{
Mesh::Mesh(const uint32_t vaoId)
    : vaoId_(vaoId)
{}

auto Mesh::bind() const -> void { GPUBinder::get().useVao(vaoId_); }

auto Mesh::unbind() const -> void { GPUBinder::get().useVao(0); }

auto Mesh::vao() const -> uint32_t { return vaoId_; }
} // namespace lav::resourceloaders