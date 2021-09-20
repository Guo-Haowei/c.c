#include <stdio.h>

char *p;
int a, b, c, d;

int main() {
    printf("a: %d, b: %d, c: %d, d: %d\n", a, b, c, d);
    a = 1, b = 2, c = 3, d = 4;
    printf("a: %d, b: %d, c: %d, d: %d\n", a, b, c, d);
    {
        int a = 10, b = 20, c = 30, d = 40;
        printf("a: %d, b: %d, c: %d, d: %d\n", a, b, c, d);
    }
    printf("a: %d, b: %d, c: %d, d: %d\n", a, b, c, d);
    while (b < 10) {
        int e1 = b++;
        int e2 = b;
        printf("e1 %d, e2 %d\n", e1, e2);
    }
    return 0;
}
