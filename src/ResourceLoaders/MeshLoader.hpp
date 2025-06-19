#pragma once

#include <cstdint>
#include <unordered_map>

#include "src/Utils/Logger.hpp"

namespace src::resourceloaders
{
class MeshLoader
{
public:
    auto loadQuad() -> uint32_t;

public:
    static auto get() -> MeshLoader&;

private:
    MeshLoader();
    ~MeshLoader() = default;
    MeshLoader(const MeshLoader&) = delete;
    MeshLoader(MeshLoader&&) = delete;
    MeshLoader& operator=(const MeshLoader&) = delete;
    MeshLoader& operator=(MeshLoader&&) = delete;

private:
    utils::Logger log_;
    std::unordered_map<std::string, uint32_t> vaos_;
};
} // namespace src::resourceloaders