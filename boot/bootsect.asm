[org 0x7c00]
KERNEL_OFFSET equ 0x1000

    mov [BOOT_DRIVE], dl ; Remember that the BIOS sets us the boot drive in 'dl' on boot
    mov bp, 0x9000
    mov sp, bp

    mov bx, MSG_REAL_MODE
    call println

    call load_kernel    ; read the kernel from disk
    call switch_to_pm   ; disable interrupts, load GDT, etc.
    hlt

%include "boot/print.asm"
%include "boot/disk.asm"
%include "boot/gdt.asm"
%include "boot/switch_pm.asm"

[bits 16]
load_kernel:
    mov bx, MSG_LOAD_KERNEL
    call println

    mov bx, KERNEL_OFFSET ; Read from disk and store in 0x1000
    mov dh, 16 ; Our future kernel will be larger, make this big
    mov dl, [BOOT_DRIVE]
    call disk_load
    ret

[bits 32]
BEGIN_PM:
    ; mov ebx, MSG_PROT_MODE
    ; call print32
    call KERNEL_OFFSET ; give control to the kernel
    hlt

; data
BOOT_DRIVE db 0
MSG_REAL_MODE db "Started in 16-bit Real Mode", 0
MSG_LOAD_KERNEL db "Loading kernel into memory", 0

; padding
times 510 - ($ - $$) db 0
; magic number
dw 0xaa55