#include "idt.hpp"

idt_gate_t g_idt[IDT_ENTRIES];
idt_register_t g_idt_reg;

void set_idt_gate( int n, uint32_t handler )
{
    g_idt[n].low_offset  = low_16( handler );
    g_idt[n].sel         = KERNEL_CS;
    g_idt[n].always0     = 0;
    g_idt[n].flags       = 0x8E;
    g_idt[n].high_offset = high_16( handler );
}

void set_idt()
{
    g_idt_reg.base  = (uint32_t)&g_idt;
    g_idt_reg.limit = IDT_ENTRIES * sizeof( idt_gate_t ) - 1;
    // clang-format off
    __asm__ __volatile__( "lidtl (%0)" : : "r"( &g_idt_reg ) );
    // clang-format on
}