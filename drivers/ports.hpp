#include "common/types.hpp"

/* Screen i/o ports */
static constexpr uint16_t REG_SCREEN_CTRL = 0x3d4;
static constexpr uint16_t REG_SCREEN_DATA = 0x3d5;

uint8_t port_byte_in( uint16_t port );
void port_byte_out( uint16_t port, uint8_t data );

uint16_t port_word_in( uint16_t port );
void port_word_out( uint16_t port, uint16_t data );
