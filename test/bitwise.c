#include <stdio.h>

int main() {
    printf("%08x\n", 0xFFFFFFFF & 1);
    printf("%08x\n", 0 | 0);
    printf("%08x\n", 0xFF0 & 0xFF);
    int a = 1, b = 2, c = 3, d = 4;
    int abcd = a | (b << 8) | (c << 16) | (d << 24);
    printf("%08x\n", abcd);
    printf("%08x\n", (abcd & 0xFF));
    printf("%08x\n", (abcd & 0xFF00) >> 8);
    printf("%08x\n", (abcd & 0xFF0000) >> 16);
    printf("%08x\n", (abcd & 0xFF000000) >> 24);
    return 0;
}