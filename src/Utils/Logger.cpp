#include "Logger.hpp"
#include <filesystem>

namespace lav::utils
{
std::ostream* Logger::outStream_ = &std::cout;
std::ofstream Logger::fileStream_ = std::ofstream{std::filesystem::current_path() / "test.txt"};
Logger::Level Logger::maxAllowedLevel_ = Logger::Level::INFO;
const char* Logger::ERROR_COLOR_{"\033[38;2;255;80;80m"};
const char* Logger::WARN_COLOR_{"\033[38;2;0;200;255m"};
const char* Logger::DEBUG_COLOR_{"\033[38;2;150;150;150m"};
const char* Logger::INFO_COLOR_{"\033[230;240;255;200;0m"};

auto Logger::useFileForLogging(const std::filesystem::path& filePath) -> void
{
    if (filePath.empty()) { outStream_ = &std::cout; };

    fileStream_ = std::ofstream{filePath};
    fileStream_.is_open() ? outStream_ = &fileStream_ : &std::cout;
}

auto Logger::setMaxAllowedLevel(const Level lvl) -> void
{
    maxAllowedLevel_ = lvl;
}
} // namespace lav::utils