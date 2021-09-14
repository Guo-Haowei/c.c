#include "screen.hpp"

extern "C" void _start()
{
    kprint_color( VGA_DEFAULT_COLOR );
    // kprint_clear();
    kprint_at( "X", 1, 6 );
    kprint_at( "This text spans multiple lines", 75, 10 );

    kprint_at( "There is a line\nbreak", 0, 20 );
    kprint_color( VGA_ERROR_COLOR );
    kprint( "There is a line\nbreak" );

    kprint_color( VGA_WARNING_COLOR );
    kprint_at( "What happens when we run out of space?", 45, 24 );
    kprint_byte( 'H' );
}
