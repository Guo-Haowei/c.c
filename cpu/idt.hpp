#pragma once

#include "common/types.hpp"

static constexpr uint32_t KERNEL_CS = 0x08;
static constexpr int IDT_ENTRIES    = 256;

#pragma pack( push, 1 )
struct idt_gate_t {
    uint16_t low_offset; /* Lower 16 bits of handler function address */
    uint16_t sel;        /* Kernel segment selector */
    uint8_t always0;
    /* First byte
     * Bit 7: "Interrupt is present"
     * Bits 6-5: Privilege level of caller (0=kernel..3=user)
     * Bit 4: Set to 0 for interrupt gates
     * Bits 3-0: bits 1110 = decimal 14 = "32 bit interrupt gate" */
    uint8_t flags;
    uint16_t high_offset; /* Higher 16 bits of handler function address */
};

struct idt_register_t {
    uint16_t limit;
    uint32_t base;
};
#pragma pack( pop )

extern idt_gate_t g_idt[IDT_ENTRIES];
extern idt_register_t g_idt_reg;

void set_idt_gate( int n, uint32_t handler );
void set_idt();

static_assert( sizeof( idt_gate_t ) == 8 );
static_assert( sizeof( idt_register_t ) == 6 );
