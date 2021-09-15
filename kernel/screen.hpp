#pragma once
#include "vga_color.hpp"

static constexpr int SCREEN_WIDTH       = 80;
static constexpr int SCREEN_HEIGHT      = 25;
static constexpr uint32_t VIDEO_ADDRESS = 0xb8000;

void kprint_init();

void kprint_color( uint8_t color );
void kprint_clear();

void kprint( uint8_t c );
void kprint( const char *str );
