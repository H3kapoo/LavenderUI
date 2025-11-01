#pragma once

#include <stdint.h>
#include <memory>

#include "src/Core/Binders/GPUBinder.hpp"

namespace lav::core
{
class Texture
{
public:
enum class Type : uint8_t { UNKNOWN, PNG, JPG, JPEG };

struct Options
{
    GPUBinder::TextureType texType{GPUBinder::TextureType::Single2D};
};

public:
    uint32_t id{0};
    uint32_t height{0};
    uint32_t width{0};
    uint8_t numChannels{0};
    Type type{Type::UNKNOWN};
};

using TexturePtr = std::shared_ptr<Texture>;
} // namespace lav core