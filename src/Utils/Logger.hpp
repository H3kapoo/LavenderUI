#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <print>
#include <chrono>

namespace src::utils
{
/**
    @brief:
        Simple colored logger than can output to either the console of a text file.

    @notes:
    (1) Class is not trivially destructible/constructible so avoid using it as a member variable
        inside the classes that need to be trivial in that sense.
*/
class Logger
{
public:
    enum class Level : uint8_t
    {
        ERROR = 0,
        WARN,
        DEBUG,
        INFO
    };

public:
    template<typename... Args>
    Logger(std::format_string<Args...> fmt, Args&&... args)
        : name_(std::format(fmt, std::forward<Args>(args)...))
    {}

    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) noexcept = default;
    Logger& operator=(Logger&&) noexcept = default;

#define LOGGER_GENERATE_LOGGERS(name, level)\
    template<typename... Args>\
    auto name(std::format_string<Args...> fmt, Args&&... args) const -> void\
    {\
        log(level, fmt, std::forward<Args>(args)...);\
    }\

    LOGGER_GENERATE_LOGGERS(error, Level::ERROR);
    LOGGER_GENERATE_LOGGERS(warn, Level::WARN);
    LOGGER_GENERATE_LOGGERS(debug, Level::DEBUG);
    LOGGER_GENERATE_LOGGERS(info, Level::INFO);
#undef LOGGER_GENERATE_LOGGERS

    static auto useFileForLogging(const std::filesystem::path& filePath) -> void;
    static auto setMaxAllowedLevel(const Level lvl) -> void;

private:
    template<typename... Args>
    auto log(const Level& level, std::format_string<Args...> fmt, Args&&... args) const -> void
    {
        if (static_cast<uint8_t>(level) > static_cast<uint8_t>(maxAllowedLevel_))
        {
            return;
        }

        const char* prefix;
        const char* color;
        switch (level)
        {
            case Level::ERROR:
                prefix = "ERR";
                color = ERROR_COLOR_;
                break;
            case Level::WARN:
                prefix = "WRN";
                color = WARN_COLOR_;
                break;
            case Level::DEBUG:
                prefix = "DBG";
                color = DEBUG_COLOR_;
                break;
            case Level::INFO:
                prefix = "INF";
                color = INFO_COLOR_;
                break;
        }

        using namespace std::chrono;
        zoned_time nowLocal{current_zone(), time_point_cast<milliseconds>(system_clock::now())};

        std::print(*outStream_, "[{:%F %T}]{}[{}] [{}] ", nowLocal, color, prefix, name_);
        std::println(*outStream_, fmt, std::forward<Args>(args)...);
        std::print(*outStream_, "\033[m");
    }

private:
    static std::ostream* outStream_;
    static std::ofstream fileStream_;
    static Level maxAllowedLevel_;
    static const char* ERROR_COLOR_;
    static const char* WARN_COLOR_;
    static const char* DEBUG_COLOR_;
    static const char* INFO_COLOR_;
    std::string name_;
};
} // namespace src::utils