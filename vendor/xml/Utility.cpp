#include "Utility.hpp"
#include <cstdio>

namespace utils
{

/**
    @brief Look ahead 1 byte from _stream_ and return int8_t
*/
int8_t peek1(std::ifstream& stream)
{
    return stream.peek();
}

/**
    @brief Read 1 byte from _stream_ and return int8_t
*/
int8_t read1(std::ifstream& stream)
{
    // Hack because 'readsome' needs char* but for int composition later we need unsigned chars
    // otherwise int returned will be faulty (idk, out of bounds shananigans)
    unsigned char tmp[1];
    stream.readsome((char*)tmp, 1);

    return tmp[0];
}

/**
    @brief Read 2 big endian bytes from _stream_ and return int16_t
*/
int16_t read2(std::ifstream& stream)
{
    // Hack because 'readsome' needs char* but for int composition later we need unsigned chars
    // otherwise int returned will be faulty (idk, out of bounds shananigans)
    unsigned char tmp[2];
    stream.readsome((char*)tmp, 2);

    return tmp[1] | tmp[0] << 8;
}

/**
    @brief Read 4 big endian bytes from _stream_ and return int32_t
*/
int32_t read4(std::ifstream& stream)
{
    // Hack because 'readsome' needs char* but for int composition later we need unsigned chars
    // otherwise int returned will be faulty (idk, out of bounds shananigans)
    unsigned char tmp[4];
    stream.readsome((char*)tmp, 4);

    return tmp[3] | tmp[2] << 8 | tmp[1] << 16 | tmp[0] << 24;
}

/**
    @brief Read 8 big endian bytes from _stream_ and return int64_t
*/
int64_t read8(std::ifstream& stream)
{
    // promote to 64 directly as we will hold in it final result
    uint64_t high = read4(stream);
    uint32_t low = read4(stream);

    return high << 32 | low;
}

/**
    @brief Read N bytes, supposedly ASCII, and return the string it forms
*/
std::string readStringBytes(std::ifstream& stream, int32_t n)
{
    std::string result(n, '\0');

    stream.readsome(result.data(), n);
    return result;
}

/**
    @brief Determine if the next 12 bytes form the magic number
*/

bool isMagicNumberNext(std::ifstream& stream)
{
    // Magic hex: e91100a843a0412d94b306da
    uint64_t highMagic = utils::read8(stream);
    uint32_t lowMagic = utils::read4(stream);

    return highMagic == 0xe91100a843a0412d && lowMagic == 0x94b306da;
}

} // namespace utils