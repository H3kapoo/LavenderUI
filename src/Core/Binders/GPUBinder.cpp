#include "GPUBinder.hpp"

#include "vendor/glew/include/GL/glew.h"
#include "vendor/glm/gtc/type_ptr.hpp"
#include <GL/glext.h>
#include <numeric>
#include <type_traits>

namespace lav::core
{
auto GPUBinder::get() -> GPUBinder&
{
    static GPUBinder instance;
    return instance;
}

auto GPUBinder::init() -> bool
{
    if (glewInit() != GLEW_OK)
    {
        log_.error("Couldn't initialize GLEW.");
        return false;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    glDebugMessageCallback(
        [](uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int32_t length,
            const char* message, const void* userParam)
        {
            (void) source;
            (void) id;
            (void) length;

            static_cast<const GPUBinder*>(userParam)
                ->log_.error("Type {} Severity {} Message {}", type, severity, message);
        }, this );

    log_.debug("GL Version {}", (const char*)glGetString(GL_VERSION));
    log_.debug("GLSL Version {}", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    enable(Function::DEPTH);
    enable(Function::SCISSORS);
    enable(Function::BLENDING);

    return true;
}

auto GPUBinder::setViewportArea(const glm::ivec4& area) -> void
{
    /* [x,y] start [z, w] end from bottom left to top right. */
    glViewport(area.x, area.y, area.z, area.w);
}

auto GPUBinder::setScissorsArea(const glm::ivec4& area) -> void
{
    /* [x,y] start [z, w] end from bottom left to top right. */
    glScissor(area.x, area.y, area.z, area.w);
}

auto GPUBinder::clearColor(const glm::vec4& color) -> void
{
    glClearColor(color.r, color.g, color.b, color.a);
}

auto GPUBinder::clearAllBufferBits() -> void
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

auto GPUBinder::enable(const Function func, const bool enable) -> void
{
    switch (func)
    {
        case Function::SCISSORS:
            enable ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST);
            return;
        case Function::DEPTH:
            enable ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
            return;
        case Function::BLENDING:
            if (enable)
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                return;
            }
            glDisable(GL_BLEND);
    }
}

auto GPUBinder::renderBoundQuad() const -> void
{
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

auto GPUBinder::renderBoundQuadInstanced(const uint32_t size) const -> void
{
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, size);
}

auto GPUBinder::createTexture(const uint32_t width, const uint32_t height, const TextureType texType,
    const ColorType colType, const TextureOptions texOpts, unsigned char* data) const -> uint32_t
{
    uint32_t id;
    glGenTextures(1, &id);

    /* Always bind to first texture slot while creating the texture on the GPU. */
    glActiveTexture(GL_TEXTURE0);

    const auto convertedTexType = convertTextureType(texType);
    const auto convertedColorType = convertColorType(colType);
    if (!convertedTexType || !convertedColorType) { return 0; }

    glBindTexture(convertedTexType, id);

    /* Wrapping, mag & min settings. */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        convertedTexType,
        0, /* Level */
        convertedColorType,
        width,
        height,
        0 /* Border */,
        convertedColorType,
        GL_UNSIGNED_BYTE,
        data);

    glBindTexture(convertedTexType, 0);

    return id;
}

auto GPUBinder::createProgram() const -> uint32_t
{
    return glCreateProgram();
}

auto GPUBinder::loadShaderPartType(const ShaderPartType type, const std::string& data) const -> uint32_t
{
    uint32_t id{glCreateShader(convertShaderPartType(type))};

    const char* dataIn = data.c_str();
    glShaderSource(id, 1, &dataIn, nullptr);
    glCompileShader(id);

    if (!isStausOk(id, ShaderStatusQuerry::COMPILE))
    {
        log_.error("Loading shader part failure!");
        return 0;
    }
    return id;
}

auto GPUBinder::linkPartsToProgram(const uint32_t programId, const uint32_t vertexId, const uint32_t fragId) -> bool
{
    glAttachShader(programId, vertexId);
    glAttachShader(programId, fragId);
    glLinkProgram(programId);
    glDeleteShader(vertexId);
    glDeleteShader(fragId);

    if (!isStausOk(programId, ShaderStatusQuerry::LINK))
    {
        log_.error("Linking failure for program '{}'!", programId);
        return false;
    }

    return true;
}

auto GPUBinder::useProgram(const uint32_t programId) const -> void
{
    glUseProgram(programId);
}

template<typename T>
auto GPUBinder::uploadUniform(const uint32_t programId, const std::string& name, const T& val) const -> bool
{
    const auto location = getUniformLocation(programId, name);
    if constexpr (std::is_same_v<T, glm::mat4>)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(val));
    }
    else if constexpr (std::is_same_v<T, std::vector<glm::mat4>>)
    {
        glUniformMatrix4fv(location, val.size(), GL_FALSE, glm::value_ptr(val[0]));
    }
    else if constexpr (std::is_same_v<T, glm::vec2>)
    {
        glUniform2f(location, val.x, val.y);
    }
    else if constexpr (std::is_same_v<T, glm::vec4>)
    {
        glUniform4f(location, val.x, val.y, val.z, val.w);
    }
    else if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t>)
    {
        glUniform1i(location, val);
    }
    else if constexpr (std::is_same_v<T, std::vector<int32_t>>)
    {
        glUniform1iv(location, val.size(), val.data());
    }
    else
    {
        log_.error("Unsupported upload type!");
    }

    return location == -1 ? false : true;
}

auto GPUBinder::uploadUniformTexture(const uint32_t programId, const std::string& name, const TextureType type,
        const uint32_t texSlot, const uint32_t texId) const -> bool
{
    const auto maxSlots = getMaxTextureSlots();
    if (texSlot + 1 > maxSlots)
    {
        log_.error(
            "GPU not able to support more than {} texture slots. Tried to use slot {}", maxSlots, texSlot);
        return false;
    }

    /* Shader needs texture slot location in range from [0..maxSlot], not from [GL_TEXTURE0..maxGL_TEXTURE] */
    uploadUniform(programId, name, texSlot);

    /* Active unit needs to be indeed [GL_TEXTURE0..maxGL_TEXTURE] */
    glActiveTexture(GL_TEXTURE0 + texSlot);

    const auto& convertedType = convertTextureType(type);
    glBindTexture(convertedType, texId);
    return convertedType;
}

auto GPUBinder::getMaxTextureSlots() const -> uint32_t
{
    int32_t maxTextureSlots;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureSlots);
    return maxTextureSlots;
}

auto GPUBinder::getUniformLocation(const uint32_t programId, const std::string& name) const -> int32_t
{
    return glGetUniformLocation(programId, name.c_str());
}

auto GPUBinder::convertTextureType(const TextureType type) const -> uint32_t
{
    switch (type)
    {
        case TextureType::Single2D:
            return GL_TEXTURE_2D;
        case TextureType::Array2D:
            return GL_TEXTURE_2D_ARRAY;
    }

    log_.error("Unknown texture type! '{}'", static_cast<uint8_t>(type));
    return 0;
}

auto GPUBinder::convertColorType(const ColorType type) const -> uint32_t
{
    switch (type)
    {
        case ColorType::RGBA:
            return GL_RGBA;
        case ColorType::RGB:
            return GL_RGB;
    }

    log_.error("Unknown type fed to texture! '{}'", static_cast<uint8_t>(type));
    return 0;
}

auto GPUBinder::convertShaderPartType(const ShaderPartType type) const -> uint32_t
{
    switch (type)
    {
        case ShaderPartType::VERTEX:
            return GL_VERTEX_SHADER;
        case ShaderPartType::FRAG:
            return GL_FRAGMENT_SHADER;
        break;
    }

    log_.error("Unknown shader part type! '{}'", static_cast<uint8_t>(type));
    return 0;
}

auto GPUBinder::convertShaderStatusQuerryType(const ShaderStatusQuerry type) const -> uint32_t
{
    switch (type)
    {
        case ShaderStatusQuerry::COMPILE:
            return GL_COMPILE_STATUS;
        case ShaderStatusQuerry::LINK:
            return GL_LINK_STATUS;
        break;
    }

    log_.error("Unknown link status querry! '{}'", static_cast<uint8_t>(type));
    return 0;
}

auto GPUBinder::isStausOk(const uint32_t idToQuerry, const ShaderStatusQuerry type) const -> bool
{
    static const uint32_t MSG_SIZE = 256;
    int32_t ok{0};
    char msg[MSG_SIZE];
    const auto converted = convertShaderStatusQuerryType(type);

    switch (type)
    {
        case ShaderStatusQuerry::COMPILE:
            glGetShaderiv(idToQuerry, converted, &ok);
            if (!ok)
            {
                glGetShaderInfoLog(idToQuerry, MSG_SIZE, NULL, msg);
                log_.error("Status check failed: {}", msg);
                return false;
            }
            break;
        case ShaderStatusQuerry::LINK:
            glGetProgramiv(idToQuerry, converted, &ok);
            if (!ok)
            {
                glGetProgramInfoLog(idToQuerry, MSG_SIZE, NULL, msg);
                log_.error("Status check failed: {}", msg);
                return false;
            }
          break;
    }

    return true;
}


auto GPUBinder::useVao(const uint32_t vao) const -> void
{
    glBindVertexArray(vao);
}

auto GPUBinder::loadMeshData(const std::vector<float> eboData,
    const std::vector<uint32_t> indexData,
    const std::vector<uint32_t> eboComponentsSize) const -> uint32_t
{
    const uint32_t stride = std::accumulate(eboComponentsSize.begin(), eboComponentsSize.end(), 0);

    /* Generate vertex attribute object to encapsulate the data */
    uint32_t vaoId;
    glCreateVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);

    /* Generate buffer to hold vertex and index data */
    uint32_t vboId;
    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * eboData.size(), eboData.data(), GL_STATIC_DRAW);

    uint32_t eboId;
    glGenBuffers(1, &eboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indexData.size(), indexData.data(), GL_STATIC_DRAW);

    /* Divide the buffer into chunks of usable data "subarrays" */
    uint64_t previousComponentSize{0};
    for (uint32_t compIndex = 0; compIndex < eboComponentsSize.size(); ++compIndex)
    {
        glVertexAttribPointer(compIndex, eboComponentsSize[compIndex],
            GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(previousComponentSize * sizeof(float)));
        glEnableVertexAttribArray(compIndex);
    }

    return vaoId;
}

template auto GPUBinder::uploadUniform(const uint32_t, const std::string&, const glm::mat4&) const -> bool;
template auto GPUBinder::uploadUniform(const uint32_t, const std::string&, const std::vector<glm::mat4>&) const -> bool;
template auto GPUBinder::uploadUniform(const uint32_t, const std::string&, const glm::vec2&) const -> bool;
template auto GPUBinder::uploadUniform(const uint32_t, const std::string&, const glm::vec4&) const -> bool;
template auto GPUBinder::uploadUniform(const uint32_t, const std::string&, const int32_t&) const -> bool;
template auto GPUBinder::uploadUniform(const uint32_t, const std::string&, const uint32_t&) const -> bool;
template auto GPUBinder::uploadUniform(const uint32_t, const std::string&, const std::vector<int32_t>&) const -> bool;
} // namespace lav::core