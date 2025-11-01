#pragma once

#include <string>
#include <unordered_map>

#include "src/Utils/Logger.hpp"
#include "src/Core/ResourceHandler/Texture.hpp"
#include "vendor/glm/glm.hpp"

namespace lav::core
{
class TextureLoader
{
public:
    static TextureLoader& get();

    auto load(const std::filesystem::path& texPath, const Texture::Options& opts) -> Texture;

private:
    TextureLoader();
    ~TextureLoader();
    /* Cannot be copied or moved */
    TextureLoader(const TextureLoader&) = delete;
    TextureLoader(TextureLoader&&) = delete;
    TextureLoader& operator=(const TextureLoader&) = delete;
    TextureLoader& operator=(TextureLoader&&) = delete;

    auto loadInternal(const std::filesystem::path& texPath, const Texture::Options& opts) -> Texture;

private:
    utils::Logger log_{"TextureLoader"};

    std::unordered_map<std::filesystem::path, Texture> texPathToObject_;
};
} // namespace lav::core