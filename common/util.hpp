#pragma once
#include "types.hpp"

template<typename T>
constexpr void UNUSED( T ){};

template<typename T, int N>
constexpr int array_length( T ( &buffer )[N] )
{
    UNUSED( buffer );
    return N;
}

void itoa( int value, char* str );
