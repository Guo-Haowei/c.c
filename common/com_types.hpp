#pragma once

using int8_t  = signed char;
using int16_t = short;
using int32_t = int;

using uint8_t  = unsigned char;
using uint16_t = unsigned short;
using uint32_t = unsigned int;

template<typename T>
constexpr uint16_t low_16( T address )
{
    return ( uint16_t )( address & 0xFFFF );
}

template<typename T>
constexpr uint16_t high_16( T address )
{
    return ( uint16_t )( ( address >> 16 ) & 0xFFFF );
}
