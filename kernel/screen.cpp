#include "screen.hpp"

#include "drivers/ports.hpp"

static constexpr int MAX_ROWS = 25;
static constexpr int MAX_COLS = 80;

static constexpr size_t VIDEO_ADDRESS = 0xb8000;

// TODO: refactor

struct ScreenChar {
    uint8_t character;
    uint8_t color;
};

static_assert( sizeof( ScreenChar ) == 2 * sizeof( uint8_t ) );

static struct {
    uint8_t color;
    int position;
} s_screenGlob;

/// forward declaration
static int get_cursor_offset();
static void set_cursor_offset( int offset );

/**
 * @brief
 *
 * @param c      The character to print
 * @param offset The offset
 * @return
 */
static int kprint_byte_internal( uint8_t c, int offset );

void kprint_init()
{
    s_screenGlob.color    = VGA_DEFAULT_COLOR;
    s_screenGlob.position = get_cursor_offset();
}

void kprint_byte( uint8_t c )
{
    s_screenGlob.position = kprint_byte_internal( c, s_screenGlob.position );
    set_cursor_offset( s_screenGlob.position );
}

void kprint_color( uint8_t color )
{
    s_screenGlob.color = color;
}

/**
 * Print a message on the specified location
 * If col, row, are negative, we will use the current offset
 */

// void kprint( const char *str )
// {
// }

/**********************************************************
 * Private kernel functions                               *
 **********************************************************/

/**
 * Innermost print function for our kernel, directly accesses the video memory 
 *
 * If 'col' and 'row' are negative, we will print at current cursor location
 * If 'attr' is zero it will use 'white on black' as default
 * Returns the offset of the next character
 * Sets the video cursor to the returned offset
 */

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
    if ( offset >= MAX_COLS * MAX_ROWS )
    {
        offset = MAX_COLS * MAX_ROWS - 1;

        vidmem[offset].character = 'E';
        vidmem[offset].color     = make_vga_color( VgaColor::Red, VgaColor::White );
        return offset;
    }

    // what if overflow
    if ( c == '\n' )
    {
        offset = ( offset / MAX_COLS + 1 ) * MAX_COLS;
    }
    else
    {
        vidmem[offset].character = c;
        vidmem[offset].color     = s_screenGlob.color;
        ++offset;
    }

    return offset;
}

// void kprint_clear()
// {
//     int screen_size = MAX_COLS * MAX_ROWS;
//     int i;

//     uint8_t *vidmem = reinterpret_cast<uint8_t *>( VIDEO_ADDRESS );

//     for ( i = 0; i < screen_size; i++ )
//     {
//         vidmem[i * 2]     = ' ';
//         vidmem[i * 2 + 1] = VGA_DEFAULT_COLOR;
//     }
//     set_cursor_offset( get_offset( 0, 0 ) );
// }