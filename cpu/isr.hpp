/// Interrupt Service Routines

#pragma once

#include "common/types.hpp"

extern "C" {

struct registers_t {
    uint32_t ds;                                     /* Data segment selector */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* Pushed by pusha. */
    uint32_t int_no, err_code;                       /* Interrupt number and error code (if applicable) */
    uint32_t eip, cs, eflags, useresp, ss;           /* Pushed by the processor automatically */
};

void isr_install();
void isr_handler( registers_t r );
}
