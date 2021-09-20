#include <stdio.h>

int main() {
    printf("%d\n", fib(0));
    printf("%d\n", fib(1));
    printf("%d\n", fib(2));
    printf("%d\n", fib(3));
    printf("%d\n", fib(4));
    printf("%d\n", fib(5));
    printf("%d\n", fib(6));
    printf("%d\n", fib(7));
    printf("%d\n", fib(8));
    printf("%d\n", fib(9));
    printf("%d\n", fib(10));
    printf("%d\n", fib(11));
    printf("%d\n", fib(12));
    return 0;
}

int fib(int i) {
    if (i < 2) return i;
    return fib(i - 2) + fib(i - 1);
}
