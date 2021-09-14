#include "screen.h"

#include "drivers/ports.h"

static constexpr int MAX_ROWS = 25;
static constexpr int MAX_COLS = 80;

static constexpr size_t VIDEO_ADDRESS = 0xb8000;

/* Screen i/o ports */
#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

struct ScreenChar {
    uint8_t character;
    uint8_t color;
};

/* Declaration of private functions */
int get_cursor_offset();
void set_cursor_offset( int offset );
int print_char( char c, int col, int row, char attr );

/* offset helpers */
static constexpr int get_offset( int col, int row )
{
    return 2 * ( row * MAX_COLS + col );
}

static constexpr int get_offset_row( int offset )
{
    return offset / ( 2 * MAX_COLS );
}

static constexpr int get_offset_col( int offset )
{
    return ( offset - ( get_offset_row( offset ) * 2 * MAX_COLS ) ) / 2;
}

/**
 * Print a message on the specified location
 * If col, row, are negative, we will use the current offset
 */
void kprint_at( const char *str, int col, int row )
{
    /* Set cursor if col/row are negative */
    int offset;
    if ( col >= 0 && row >= 0 )
    {
        offset = get_offset( col, row );
    }
    else
    {
        offset = get_cursor_offset();
        row    = get_offset_row( offset );
        col    = get_offset_col( offset );
    }

    for ( const char *p = str; *p; ++p )
    {
        offset = print_char( *p, col, row, WHITE_ON_BLACK );
        row    = get_offset_row( offset );
        col    = get_offset_col( offset );
    }
}

void kprint( const char *str )
{
    kprint_at( str, -1, -1 );
}

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

void kprint_byte( uint8_t c )
{
    print_char( c, 0, 0, make_vga_color( VgaColor::Yellow, VgaColor::Black ) );
}

int print_char( char c, int col, int row, char attr )
{
    uint8_t *vidmem = reinterpret_cast<uint8_t *>( VIDEO_ADDRESS );
    if ( !attr )
        attr = WHITE_ON_BLACK;

    /* Error control: print a red 'E' if the coords aren't right */
    if ( col >= MAX_COLS || row >= MAX_ROWS )
    {
        vidmem[2 * ( MAX_COLS ) * (MAX_ROWS)-2] = 'E';
        vidmem[2 * ( MAX_COLS ) * (MAX_ROWS)-1] = RED_ON_WHITE;
        return get_offset( col, row );
    }

    int offset;
    if ( col >= 0 && row >= 0 )
        offset = get_offset( col, row );
    else
        offset = get_cursor_offset();

    if ( c == '\n' )
    {
        row    = get_offset_row( offset );
        offset = get_offset( 0, row + 1 );
    }
    else
    {
        vidmem[offset]     = c;
        vidmem[offset + 1] = attr;
        offset += 2;
    }
    set_cursor_offset( offset );
    return offset;
}

int get_cursor_offset()
{
    /* Use the VGA ports to get the current cursor position
     * 1. Ask for high byte of the cursor offset (data 14)
     * 2. Ask for low byte (data 15)
     */
    port_byte_out( REG_SCREEN_CTRL, 14 );
    int offset = port_byte_in( REG_SCREEN_DATA ) << 8; /* High byte: << 8 */
    port_byte_out( REG_SCREEN_CTRL, 15 );
    offset += port_byte_in( REG_SCREEN_DATA );
    return offset * 2; /* Position * size of character cell */
}

void set_cursor_offset( int offset )
{
    /* Similar to get_cursor_offset, but instead of reading we write data */
    offset /= 2;
    port_byte_out( REG_SCREEN_CTRL, 14 );
    port_byte_out( REG_SCREEN_DATA, (unsigned char)( offset >> 8 ) );
    port_byte_out( REG_SCREEN_CTRL, 15 );
    port_byte_out( REG_SCREEN_DATA, (unsigned char)( offset & 0xff ) );
}

void kprint_clear()
{
    int screen_size = MAX_COLS * MAX_ROWS;
    int i;

    uint8_t *vidmem = reinterpret_cast<uint8_t *>( VIDEO_ADDRESS );

    for ( i = 0; i < screen_size; i++ )
    {
        vidmem[i * 2]     = ' ';
        vidmem[i * 2 + 1] = WHITE_ON_BLACK;
    }
    set_cursor_offset( get_offset( 0, 0 ) );
}