#include <stdio.h>

int main() {
    printf("test inc dec\n");
    int a = 15;
    printf("%d\n", a++);
    printf("%d\n", ++a);
    printf("%d\n", a);
    printf("%d\n", a -= 2);
    printf("%d\n", a += 123);
    printf("%d\n", --a);
    printf("%d\n", a);
    printf("%d\n", a--);
    printf("%d\n", a);
    int* ip = 0;
    printf("%p\n", ip++);
    printf("%p\n", ip);
    printf("%p\n", ++ip);
    printf("%p\n", ip);
    printf("%p\n", ip--);
    printf("%p\n", ip -= 2);
    printf("%p\n", ip += 29);
    printf("%p\n", --ip);
    printf("%p\n", ip);
    char* cp = 0;
    printf("%p\n", cp++);
    printf("%p\n", cp++);
    printf("%p\n", ++cp);
    printf("%p\n", cp += 20);
    printf("%p\n", cp -= 2);
    printf("%p\n", cp--);
    printf("%p\n", cp);
    printf("%p\n", --cp);
    char* cpp = 0;
    printf("%p\n", cpp++);
    printf("%p\n", cpp);
    printf("%p\n", cpp++);
    printf("%p\n", ++cpp);
    printf("%p\n", cpp);
    printf("%p\n", cpp += 15);
    printf("%p\n", cpp -= 2);
    printf("%p\n", cpp);
    printf("%p\n", cpp--);
    printf("%p\n", --cpp);

    char* str = "Hello, there\n";
    while (*str) {
        printf("%c", *str++);
    }
    return 0;
}
