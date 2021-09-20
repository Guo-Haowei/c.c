#include <stdio.h>

int main() {
    int n = 6;
    while (n > 0) { printf("n: %d\n", n); n = n - 1; };
    n = 70;

    while (n < 90) {
        if (n == 80) break;
        printf("n: %d\n", n += 1);
    }

    int i = 0;
    while (i < 100) {
        i += 1;
        if (i % 5) continue;
        printf("%d is divisible by 5\n", i);
    }

    return 0;
}
