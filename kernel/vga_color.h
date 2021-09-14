#pragma once
#include <stdint.h>

enum class VgaColor : uint8_t {
    Black,
    Blue,
    Green,
    Cyan,
    Red,
    Magenta,
    Brown,
    Lightgray,
    DarkGray,
    Lightblue,
    Lightgreen,
    Lightcyan,
    Lightred,
    Pink,
    Yellow,
    White,
};

static constexpr uint8_t make_vga_color( VgaColor foreground, VgaColor background )
{
    return ( static_cast<uint8_t>( background ) << 4 ) | ( static_cast<uint8_t>( foreground ) );
}

static constexpr uint8_t WHITE_ON_BLACK = make_vga_color( VgaColor::White, VgaColor::Black );
static constexpr uint8_t RED_ON_WHITE   = make_vga_color( VgaColor::Red, VgaColor::White );
