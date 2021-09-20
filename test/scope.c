#include <stdio.h>

void one() {
    int a = 31;
    {
        int a = 64;
        printf("%d\n", a);
    }
    printf("%d\n", a);
    {
        int a = 0x10;
        printf("%d\n", a);
    }
    return;
}

void two(int a) {
    int b = 102;
    printf("%d\n", a);
    printf("%d\n", b);
    {
        int a = 10;
        printf("%d\n", a);
        {
            int b = 12213;
            printf("%d\n", b);
        }
        int c = 6789;
        printf("%d\n", b);
        printf("%d\n", c);
        {
            printf("%d\n", c);
            int b = 9 + a;
            printf("%d\n", b);
            {
                printf("%d\n", a);
                int a = b + c;
                printf("%d\n", a);
            }
        }
        printf("%d\n", b);
    }
    int c = 111 + b - a;
    printf("%d\n", a);
    printf("%d\n", c);
    return;
}

int main() {
    one();
    two(0xAA9);
    int a = 42, b = 1, c = 6;
    printf("a: %d, b: %d, c: %d\n", a, b, c);
    a = b, b = c, c = a;
    printf("a: %d, b: %d, c: %d\n", a, b, c);
    return 0;
}
