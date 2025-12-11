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

    char* str = "Hello, there\n";
    while (*str) {
        printf("%c", *str++);
    }
    return 0;
}
