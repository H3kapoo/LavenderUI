#include "FileResourceBinder.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb/stbi_image.hpp"

namespace lav::core
{
auto FileResourceBinder::get() -> FileResourceBinder&
{
    static FileResourceBinder instance;
    return instance;
}

auto FileResourceBinder::loadTextureData(const std::filesystem::path& path) const -> LoadInfo
{
    LoadInfo info;
    unsigned char* data = stbi_load(path.c_str(), &info.width, &info.height, &info.numChannels, 0);
    if (!data)
    {
        log_.error("Cannot load texture '{}'. Check path correctness!", path.string());
        return info;
    }

    info.data = data;
    info.fileExt = path.string().ends_with(".png") ? FileExt::PNG : FileExt::JPEG;

    log_.debug("Texture data loaded to host for '{}'", path.string());

    return info;
}

auto FileResourceBinder::freeLoadedTextureData(const LoadInfo& info) const -> void
{
    if (info.data == nullptr)
    {
        log_.warn("Tried to free texture data pointing to null!");
        return;
    }

    stbi_image_free(info.data);
}

} // namespace lav::core