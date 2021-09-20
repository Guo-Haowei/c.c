#include <stdio.h>

int main() {
    int six_6 = 6;

    if (six_6 != 0) { printf("%d != 0\n", six_6); }
    if (six_6 < 0) printf("false\n");
    else printf("true\n");
    { int div = 6; if (six_6 % div == 0) printf("%d divisible by %d\n", six_6, div); }
    { int div = 3; if (six_6 % div == 0) printf("%d divisible by %d\n", six_6, div); }
    { int div = 2; if (six_6 % div == 0) printf("%d divisible by %d\n", six_6, div); }
    { int div = 5; if (six_6 % div == 0) printf("%d divisible by %d\n", six_6, div); }

    {
        int num = 128;
        if (num && (num = num / 2) && (num = num / 4)) {
            printf("num is %d\n", num);
        }
    }
    {
        int num = 128;
        (1 && 0 && (num = num / 4));
        printf("num is %d\n", num);
    }
    {
        int num = 77;
        if (num % 5 == 0 || num % 7 == 0 || num % 4 == 0) {
            printf("%d is divisible by 4, 5 or 7\n", num);
        }
    }
    {
        int num = 35;
        if (num % 5 || num % 7 || num % 4 == 0) {
            printf("%d is divisible by 4, 5 or 7\n", num);
        }
    }
    {
        int i = 12;
        i = (i < 12 ? i : (i += 24));
        printf("i is now %d\n", i);
    }
    {
        int i = 12;
        i = (i == 12 ? i : (i += 24));
        printf("i is now %d\n", i);
    }

    0 && printf("you should not see this, 0 &&\n");
    1 && printf("you should see this, 1 &&\n");
    0 || printf("you should see this, 0 ||\n");
    1 || printf("you should not see this, 1 ||\n");

    return 0;
}
