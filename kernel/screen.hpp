#pragma once
#include "vga_color.hpp"

// TODO: refactor

void kprint_init();

void kprint_color( uint8_t color );
void kprint_clear();

void kprint_byte( uint8_t c );
void kprint( const char *str );
