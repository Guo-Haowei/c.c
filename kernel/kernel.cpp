#include "screen.hpp"

extern "C" void _start()
{
    kprint_init();
    kprint_color( VGA_DEFAULT_COLOR );
    kprint_byte( 'H' );
    kprint_color( VGA_WARNING_COLOR );
    kprint_byte( 'e' );
    kprint_color( VGA_ERROR_COLOR );
    kprint_byte( 'l' );
    kprint_byte( 'l' );
    kprint_color( VGA_DEFAULT_COLOR );
    kprint_byte( 'o' );
}
