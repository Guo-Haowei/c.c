#pragma once
#include "vga_color.h"

void kprint_clear();

void kprint_byte( uint8_t c );

void kprint_at( const char *str, int col, int row );
void kprint( const char *str );