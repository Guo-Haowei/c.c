#include "screen.hpp"

#include "drivers/ports.hpp"

struct ScreenChar {
    uint8_t character;
    uint8_t color;
};

static_assert( sizeof( ScreenChar ) == 2 * sizeof( uint8_t ) );

static struct {
    uint8_t color;
    int position;
} s_screenGlob;

/// forward declaration of helper functions
static int get_cursor_offset();
static void set_cursor_offset( int offset );
static int kprint_byte_internal( uint8_t c, int offset );

void kprint_init()
{
    s_screenGlob.color    = VGA_DEFAULT_COLOR;
    s_screenGlob.position = get_cursor_offset();
}

void kprint_color( uint8_t color )
{
    s_screenGlob.color = color;
}

void kprint( uint8_t c )
{
    s_screenGlob.position = kprint_byte_internal( c, s_screenGlob.position );
    set_cursor_offset( s_screenGlob.position );
}

void kprint( const char *str )
{
    int &pos = s_screenGlob.position;

    for ( int idx = 0; str[idx]; ++idx )
    {
        pos = kprint_byte_internal( str[idx], pos );
    }

    set_cursor_offset( pos );
}

/// implementations of helper functions
static int get_cursor_offset()
{
    /* Use the VGA ports to get the current cursor position
     * 1. Ask for high byte of the cursor offset (data 14)
     * 2. Ask for low byte (data 15)
     */
    port_byte_out( REG_SCREEN_CTRL, 14 );
    int offset = port_byte_in( REG_SCREEN_DATA ) << 8; /* High byte: << 8 */
    port_byte_out( REG_SCREEN_CTRL, 15 );
    offset += port_byte_in( REG_SCREEN_DATA );
    return offset;
}

static void set_cursor_offset( int offset )
{
    /* Similar to get_cursor_offset, but instead of reading we write data */
    port_byte_out( REG_SCREEN_CTRL, 14 );
    port_byte_out( REG_SCREEN_DATA, (unsigned char)( offset >> 8 ) );
    port_byte_out( REG_SCREEN_CTRL, 15 );
    port_byte_out( REG_SCREEN_DATA, (unsigned char)( offset & 0xff ) );
}

static int kprint_byte_internal( uint8_t c, int offset )
{
    ScreenChar *vidmem = reinterpret_cast<ScreenChar *>( VIDEO_ADDRESS );
    if ( offset >= SCREEN_WIDTH * SCREEN_HEIGHT )
    {
        vidmem[SCREEN_WIDTH * SCREEN_HEIGHT - 1].character = 'E';
        vidmem[SCREEN_WIDTH * SCREEN_HEIGHT - 1].color     = make_vga_color( VgaColor::Red, VgaColor::White );
        return offset;
    }

    // what if overflow
    if ( c == '\n' )
    {
        return ( offset / SCREEN_WIDTH + 1 ) * SCREEN_WIDTH;
    }

    vidmem[offset].character = c;
    vidmem[offset].color     = s_screenGlob.color;
    return offset + 1;
}
