#include <stdio.h>

void test_literal() {
    printf("test literal\n");
    printf("%d\n", 123);
    printf("%X\n", 0xF);
    printf("%X\n", 0x1F2C);
    printf("%X\n", 0xDEADBEEF);
    printf("%X\n", 0xFFFFFFFF);
    printf("%c\n", 'a');
    printf("%c\n", '0');
    printf("%c\n", '-');
    return;
}

void test_arith() {
    printf("test arith\n");
    printf("%d\n", 0);
    printf("%d\n", 31 + 15);
    printf("%d\n", 145 - 199);
    printf("%d\n", 324 * 71);
    printf("%d\n", 56 / 8);
    printf("%d\n", 54 % 8);
    printf("%d\n", 1 + 2 + 3);
    printf("%d\n", 1 - 2 - 3);
    printf("%d\n", 64 / 4 / 4);
    printf("%d\n", 64 / (4 / 4));
    printf("%d\n", 14 + 12 * 3 - 4 / 2 + (53 - 23) - 456 % 34);
    printf("%d\n", 1 + 2 + -3 + 4);
    printf("%d\n", 1 + 2 * -3 + 4);
    printf("%d\n", 1 * 2 + 3 * +4);
    printf("%d\n", 4 / 2 + 6 / +3);
    printf("%d\n", 24 / 2 / 3);
    printf("%d\n", 24 % 7);
    printf("%d\n", 24 % 3);
    printf("%d\n", 'a' + 1);
    int a = 0 - 1;
    printf("%d\n", a);
    printf("%d\n", a + -1);
    return;
}

void test_relational() {
    printf("test ralational\n");
    printf("%d\n", 1 > 0);
    printf("%d\n", 0 < 1);
    printf("%d\n", 1 < 0);
    printf("%d\n", 0 > 1);
    printf("%d\n", 1 > 1);
    printf("%d\n", 1 < 1);
    printf("%d\n", 1 >= 0);
    printf("%d\n", 0 <= 1);
    printf("%d\n", 1 <= 0);
    printf("%d\n", 0 >= 1);
    printf("%d\n", 1 >= 1);
    printf("%d\n", 1 <= 1);
    printf("%d\n", 1 == 1);
    printf("%d\n", 1 == 2);
    printf("%d\n", 1 != 2 - 1);
    printf("%d\n", 0 != 2 - 1);
    printf("%d\n", 6 > 6);
    printf("%d\n", 6 >= 6);
    printf("%d\n", 7 < 6);
    printf("%d\n", 7 <= 6);
    printf("%d\n", 7 <= 7);
    printf("%d\n", 0xFFFF > 1);
    return;
}

void test_assign() {
    printf("test assignment\n");
    int v;
    v = 5;
    printf("%d\n", v -= 1);
    printf("%d\n", v);
    v += v + 5;
    printf("%d\n", v);
    v -= 2 - v;
    printf("%d\n", v);
    printf("%d\n", v += v * v);
    return;
}

void test_unary() {
    printf("test unary\n");
    int x = 2, y = 9, z = 14214;
    printf("%d\n", -x);
    printf("%d\n", +y);
    printf("%d\n", -z);
    return;
}

void test_ternary() {
    printf("test ternary\n");
    int x = 2, y = 9, z = 14214;
    printf("%s\n", !1 ? "false" : "true");
    printf("%s\n" ,!0 ? "true" : "false");
    printf("%d\n" , y > x ? y += z : x);
    printf("%d\n", (1 + 2) ? 51 : 52);
    printf("%d\n", (1 - 1) ? 51 : 52);
    printf("%d\n", (1 - 1) ? 51 : 52 / 2);
    printf("%d\n", (1 - 0) ? 51 / 3 : 52);
    return;
}

int main() {
    test_literal();
    test_arith();
    test_relational();
    test_assign();
    test_unary();
    return 0;
}
