#include "drivers/ports.h"

extern "C" void _start()
{
    /* Screen cursor position: ask VGA control register (0x3d4) for bytes
     * 14 = high byte of cursor and 15 = low byte of cursor. */
    port_byte_out( 0x3d4, 14 ); /* Requesting byte 14: high byte of cursor pos */
    /* Data is returned in VGA data register (0x3d5) */
    int position = port_byte_in( 0x3d5 );
    position     = position << 8; /* high byte */

    port_byte_out( 0x3d4, 15 ); /* requesting low byte */
    position += port_byte_in( 0x3d5 );

    int offset_from_vga = position * 2;

    char *vga           = (char *)0xb8000;
    const char buffer[] = "Landed in 32-bit Protected Mode!";

    for ( size_t offset = 0; offset < sizeof( buffer ) - 1; ++offset )
    {
        vga[offset_from_vga + 2 * offset]     = buffer[offset];
        vga[offset_from_vga + 2 * offset + 1] = 0xe;
    }

    position += sizeof( buffer ) - 1;
    port_byte_out( 0x3d5, position & 0xFF );
}
