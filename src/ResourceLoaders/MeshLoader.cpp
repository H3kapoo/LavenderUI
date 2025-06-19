#include "MeshLoader.hpp"

#include <vector>

#include "vendor/glew/include/GL/glew.h"

namespace src::resourceloaders
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
    static std::vector<float> vertexData =
    {
         /*    Pos              TC */
         1.0f, 1.0f, 0.0f,  1.0f, 1.0f, /* top right */
         1.0f, 0.0f, 0.0f,  1.0f, 0.0f, /* bottom right */
         0.0f, 0.0f, 0.0f,  0.0f, 0.0f, /* bottom left */
         0.0f, 1.0f, 0.0f,  0.0f, 1.0f, /* top left */ 
    };

    static std::vector<uint32_t> indexData =
    {
        0, 1, 3, /* First triangle */
        1, 2, 3  /* Second triangle */
    };

    /* Generate vertex attribute object to encapsulate the data */
    uint32_t vaoId;
    glCreateVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);

    /* Generate buffer to hold vertex and index data */
    uint32_t vboId;
    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexData.size(), vertexData.data(), GL_STATIC_DRAW);

    uint32_t eboId;
    glGenBuffers(1, &eboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indexData.size(), indexData.data(), GL_STATIC_DRAW);

    /* Divide the buffer into chunks of usable data "subarrays" */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    log_.debug("Loaded quad mesh with vaoID {}", vaoId);
    vaos_["q"] = vaoId;

    return vaoId;
}

auto MeshLoader::get() -> MeshLoader&
{
    static MeshLoader instance;
    return instance;
}
} // namespace src::resourceloaders