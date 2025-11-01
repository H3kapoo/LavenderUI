#include "MeshLoader.hpp"

#include <vector>

#include "src/Core/Binders/GPUBinder.hpp"

namespace lav::core
{
MeshLoader::MeshLoader()
    : log_("MeshLoader")
{}

auto MeshLoader::loadQuad() -> uint32_t
{
    /* Easy fetch with no re-loading */
    if (vaos_.contains("q"))
    {
        return vaos_.at("q");
    }

    /* Note: clockwise winding */
    std::vector<float> vertexData =
    {
         /*    Pos              TC */
         1.0f, 1.0f, 0.0f,  1.0f, 1.0f, /* top right */
         1.0f, 0.0f, 0.0f,  1.0f, 0.0f, /* bottom right */
         0.0f, 0.0f, 0.0f,  0.0f, 0.0f, /* bottom left */
         0.0f, 1.0f, 0.0f,  0.0f, 1.0f, /* top left */ 
    };

    std::vector<uint32_t> eboData =
    {
        0, 1, 3, /* First triangle */
        1, 2, 3  /* Second triangle */
    };

    std::vector<uint32_t> eboComponentsSize = { 3, 2 };

    uint32_t vaoId = GPUBinder::get().loadMeshData(vertexData, eboData, eboComponentsSize);

    log_.debug("Loaded quad mesh with vaoID {}", vaoId);
    vaos_["q"] = vaoId;

    return vaoId;
}

auto MeshLoader::get() -> MeshLoader&
{
    static MeshLoader instance;
    return instance;
}
} // namespace lav::core