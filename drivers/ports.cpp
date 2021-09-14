#include "ports.h"

uint8_t port_byte_in( uint16_t port )
{
    unsigned char result;
    //  Inline assembler syntax
    //  '"=a" (result)'; set '=' the C variable '(result)' to the value of
    //  register e'a'x
    //  '"d" (port)': map the C variable '(port)' into e'd'x register
    //  Inputs and outputs are separated by colons

    // clang-format off
    __asm__( "in %%dx, %%al" : "=a"( result ) : "d"( port ) );
    // clang-format on
    return result;
}

void port_byte_out( uint16_t port, uint8_t data )
{
    //  Notice how here both registers are mapped to C variables and
    //  nothing is returned, thus, no equals '=' in the asm syntax
    //  However we see a comma since there are two variables in the input area
    //  and none in the 'return' area

    // clang-format off
    __asm__( "out %%al, %%dx" : : "a"( data ), "d"( port ) );
    // clang-format on
}

uint16_t port_word_in( uint16_t port )
{
    unsigned short result;
    // clang-format off
    __asm__( "in %%dx, %%ax" : "=a"( result ) : "d"( port ) );
    // clang-format on
    return result;
}

void port_word_out( uint16_t port, uint16_t data )
{
    // clang-format off
    __asm__( "out %%ax, %%dx" : : "a"( data ), "d"( port ) );
    // clang-format on
}
