#pragma once

#include <fstream>

// Surely std::format could be used but if utility is included across multiple translation units
// there will be a performance penalty. Printf does an excellent job for now anyway.
// If you however want to use the new std::format, enable this.
// #define NEW_PRINT

#ifdef NEW_PRINT
#include <format>
#endif

#define MUTE_PRINT_

namespace utils
{

#define ERROR_COLOR "\033[38;5;196m"
#define INFO_COLOR "\033[38;5;255m"
#define RESET_COLOR "\033[0m"
#define NEW_LINE "\n"

#ifndef NEW_PRINT
#ifndef MUTE_PRINT

#define printlne(fmt, ...) printf(ERROR_COLOR "[ERR] " fmt RESET_COLOR NEW_LINE, ##__VA_ARGS__)
#define println(fmt, ...) printf(INFO_COLOR "[INF] " fmt RESET_COLOR NEW_LINE, ##__VA_ARGS__)
#define print(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define sprint(buff, fmt, ...)                                                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
        int32_t size = snprintf(nullptr, 0, fmt, ##__VA_ARGS__);                                                       \
        buff = std::string(size, '\0');                                                                                \
        sprintf(buff.data(), fmt, ##__VA_ARGS__);                                                                      \
    } while (0);

#else // MUTE_PRINT

#define printlne(fmt, ...)
#define println(fmt, ...)
#define print(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define sprint(buff, fmt, ...)

#endif // MUTE_PRINT

#else // NEW_PRINT

template <typename... Args> void println(std::format_string<Args...> fmt, Args&&... args)
{
    puts(std::format(fmt, std::forward<Args>(args)...).c_str());
}

#endif // NEW_PRINT

/**
    @brief Read 1 byte from _stream_ and return int8_t
*/
int8_t peek1(std::ifstream& stream);

/**
    @brief Read 1 byte from _stream_ and return int8_t
*/
int8_t read1(std::ifstream& stream);

/**
    @brief Read 2 big endian bytes from _stream_ and return int16_t
*/
int16_t read2(std::ifstream& stream);

/**
    @brief Read 4 big endian bytes from _stream_ and return int32_t
*/
int32_t read4(std::ifstream& stream);

/**
    @brief Read 8 big endian bytes from _stream_ and return int64_t
*/
int64_t read8(std::ifstream& stream);

/**
    @brief Read N bytes, supposedly ASCII, and return the string it forms
*/
std::string readStringBytes(std::ifstream& stream, int32_t n);

/**
    @brief Determine if the next 12 bytes form the magic number
*/
bool isMagicNumberNext(std::ifstream& stream);

} // namespace utils