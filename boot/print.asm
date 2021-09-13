; usage: println(str: bx)
println:
    pusha ; save all registers

    mov ah, 0x0e ; tty mode
print_start:
    mov al, [bx] ; bx contains the base address of the string
    cmp al, 0
    je print_done

    int 0x10

    inc bx
    jmp print_start

print_done:
    mov al, 0x0a ; '\n'
    int 0x10
    mov al, 0x0d ; '\r'
    int 0x10

    popa
    ret

; usage: print_hex(hex: dx)
print_hex:
    pusha

    mov cx, 0

hex_loop:
    cmp cx, 4
    je end

    ; 1. convert last char of 'dx' to ascii
    mov ax, dx
    and ax, 0x000f
    add al, 0x30
    cmp al, 0x39 ; if > 9, add extra 8 to represent 'A' to 'F'
    jle step2
    add al, 7 ; 'A' is ASCII 65 instead of 58, 65 - 58 = 7

step2:
    ; 2. get the correct position of the string to place our ASCII char
    ; bx <- base address + string length - index of char
    mov bx, HEX_OUT + 5 ; base + length
    sub bx, cx
    mov [bx], al
    ror dx, 4

    ; increment index
    inc cx
    jmp hex_loop

end:
    mov bx, HEX_OUT
    call println

    popa
    ret

HEX_OUT:
    db '0x0000',0 ; reserve memory for our new string
