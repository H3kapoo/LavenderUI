#pragma once

#include "src/Utils/Logger.hpp"
#include <filesystem>

namespace lav::core
{

class FileResourceBinder
{
public:
    enum class FileExt : uint8_t
    {
        JPEG,
        JPG,
        PNG
    };

    struct LoadInfo
    {
        unsigned char* data{nullptr};
        int32_t width{0};
        int32_t height{0};
        int32_t numChannels{0};
        FileExt fileExt{FileExt::JPEG};
    };

public:
    static auto get() -> FileResourceBinder&;

    auto loadTextureData(const std::filesystem::path& path) const -> LoadInfo;
    auto freeLoadedTextureData(const LoadInfo&) const -> void;

private:
    FileResourceBinder() = default;
    FileResourceBinder(const FileResourceBinder&) = delete;
    FileResourceBinder(FileResourceBinder&&) = delete;
    auto operator=(const FileResourceBinder&) = delete;
    auto operator=(FileResourceBinder&&) = delete;

private:
    utils::Logger log_{"FileResourceBinder"};
};

} // namespace lav::core