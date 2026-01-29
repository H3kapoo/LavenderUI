#pragma once

#include <unordered_map>

#include <LavenderUI/Utils/Logger.hpp>
#include <LavenderUI/Core/ResourceHandler/Texture.hpp>

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
    utils::Logger log_;
    std::unordered_map<std::filesystem::path, Texture> texPathToObject_;
};
} // namespace lav::core