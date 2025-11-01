#include "TextureLoader.hpp"
#include "src/Core/Binders/GPUBinder.hpp"
#include "src/Core/Binders/FileResourceBinder.hpp"


namespace lav::core
{
TextureLoader& TextureLoader::get()
{
    static TextureLoader instance;
    return instance;
}

TextureLoader::TextureLoader()
{}

TextureLoader::~TextureLoader()
{}

auto TextureLoader::load(const std::filesystem::path& texPath, const Texture::Options& opts) -> Texture
{
    if (texPathToObject_.count(texPath))
    {
        return texPathToObject_.at(texPath);
    }

    texPathToObject_[texPath] = loadInternal(texPath, opts);

    return texPathToObject_.at(texPath);
}

auto TextureLoader::loadInternal(const std::filesystem::path& texPath, const Texture::Options& opts) -> Texture
{
    Texture texture;

    /* Load into host memory. */
    core::FileResourceBinder::LoadInfo info =
        core::FileResourceBinder::get().loadTextureData(texPath);
    if (!info.data)
    {
        log_.error("Load from path failed for '{}'", texPath.string());
        return texture;
    }

    const auto colorType =
        info.fileExt == FileResourceBinder::FileExt::PNG
        ? GPUBinder::ColorType::RGBA : GPUBinder::ColorType::RGB;

    texture.width = info.width;
    texture.height = info.height;
    texture.numChannels = info.numChannels;
    texture.id = GPUBinder::get().createTexture(
        texture.width,
        texture.height,
        GPUBinder::TextureType::Single2D,
        colorType,
        GPUBinder::TextureOptions{},
        info.data);
    
    if (!texture.id)
    {
        log_.error("Could not create GPU texture for '{}'", texPath.string());
        return texture;
    }

    /* Free host data */
    core::FileResourceBinder::get().freeLoadedTextureData(info);

    log_.debug("Created textureId '{}' for GPU from '{}'", texture.id, texPath.string());

    return texture;
}
} // namespace lav::core