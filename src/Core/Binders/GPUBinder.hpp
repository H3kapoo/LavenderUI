#pragma once

#include "src/Utils/Logger.hpp"
#include "vendor/glm/glm.hpp"

namespace lav::core
{
class GPUBinder
{
public:
    enum class Function
    {
        SCISSORS,
        DEPTH,
        BLENDING
    };

    enum class TextureType
    {
        Single2D,
        Array2D
    };

    enum class ColorType { MONO, RGB, RGBA};

    enum class TextureWrap { CLAMP_TO_EDGE };

    enum class TextureFilter { LINEAR };

    struct TextureOptions
    {
        TextureWrap uWrap{TextureWrap::CLAMP_TO_EDGE};
        TextureWrap vWrap{TextureWrap::CLAMP_TO_EDGE};
        TextureFilter min{TextureFilter::LINEAR};
        TextureFilter mag{TextureFilter::LINEAR};
    };

    enum class ShaderPartType { VERTEX, FRAG };

private:
    enum class ShaderStatusQuerry { COMPILE, LINK };

public:
    static auto get() -> GPUBinder&;

    auto init() -> bool;
    auto setViewportArea(const glm::ivec4& area) -> void;
    auto setScissorsArea(const glm::ivec4& area) -> void;
    auto enable(const Function func, const bool enable = true) -> void;

    /* Rendering */
    auto clearColor(const glm::vec4& color) -> void;
    auto clearAllBufferBits() -> void;
    auto renderBoundQuad() const -> void;
    auto renderBoundQuadInstanced(const uint32_t size) const -> void;

    /* Shader */
    auto createProgram() const -> uint32_t;
    auto loadShaderPartType(const ShaderPartType type, const std::string& data) const -> uint32_t;
    auto linkPartsToProgram(const uint32_t programId, const uint32_t vertexId, const uint32_t fragId) -> bool;
    auto useProgram(const uint32_t programId) const -> void;
    template<typename T>
    auto uploadUniform(const uint32_t programId, const std::string& name, const T& val) const -> bool;
    auto uploadUniformTexture(const uint32_t programId, const std::string& name, const TextureType type,
        const uint32_t texSlot, const uint32_t texId) const -> bool;
    auto getUniformLocation(const uint32_t programId, const std::string& name) const -> int32_t;

    /* Textures */
    auto generateTexture() const -> uint32_t;
    auto activateTextureSlot(const uint8_t textureSlot) const -> void;
    auto bindIdToTextureType(const TextureType texType, const uint32_t texId) const -> void;
    auto createTexture(const uint32_t width, const uint32_t height, const uint32_t sliceCount,
        const TextureType texType, const ColorType colType, const TextureOptions texOpts,
        unsigned char* data) const -> uint32_t;
    auto bufferTextureData(const uint32_t width, const uint32_t height, const uint32_t sliceCount,
        const TextureType texType, const ColorType colType, unsigned char* data) -> void;
    auto unpackAlignment(const uint32_t bytes = 1) const -> void;

    /* Meshes */
    auto useVao(const uint32_t vao) const -> void;
    auto loadMeshData(const std::vector<float> eboData,
        const std::vector<uint32_t> indexData,
        const std::vector<uint32_t> eboComponentsSize) const -> uint32_t;

    auto convertTextureType(const TextureType type) const -> uint32_t;
    auto convertColorType(const ColorType type) const -> uint32_t;
    auto getMaxTextureSlots() const -> uint32_t;

private:
    GPUBinder() = default;
    GPUBinder(const GPUBinder&) = delete;
    GPUBinder(GPUBinder&&) = delete;
    auto operator=(const GPUBinder&) = delete;
    auto operator=(GPUBinder&&) = delete;

    auto convertShaderPartType(const ShaderPartType type) const -> uint32_t;
    auto convertShaderStatusQuerryType(const ShaderStatusQuerry type) const -> uint32_t;
    auto isStausOk(const uint32_t idToQuerry, const ShaderStatusQuerry type) const -> bool;

private:
    utils::Logger log_{"GPUBinder"};
};
} // namespace lav::core