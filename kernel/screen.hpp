#pragma once
#include "vga_color.hpp"

void kprint_clear();

void kprint_byte( uint8_t c );

void kprint_at( const char *str, int col, int row );
void kprint( const char *str );