#include <stdio.h>

int streq(char* p1, char* p2, int len) {
    while (len > 0) {
        if (*p1 == 0 || *p2 == 0) return 1;
        if (*p1 != *p2) return 0;
        p1 += 1; p2 += 1; len -= 1;
    }
    return 1;
}

void test_streq(char* p1, char* p2, int len) {
    printf("streq(%s, %s, %d) == %s\n", p1, p2, len, streq(p1, p2, len) ? "true" : "false");
    return;
}

int main() {
    printf("This " "is" " a " "'%s'\n\0test", "dummy");
    int i = 0;
    while (i < 7) {
        printf("abcdefg[%d] = %c\n", i, "a" "b" "c" "d" "e" "f" "g"[i]);
        i += 1;
    }

    test_streq("abc", "ab", 2);
    test_streq("abc", "ab", 3);
    test_streq("ab", "abc", 2);
    test_streq("ab", "abc", 3);
    test_streq("continue", "continue", 9);
    test_streq("continue", "continue", 10);
    test_streq("continue", "continue", 8);
    test_streq("return 0", "return", 8);
    test_streq("int8", "int", 3);

    return 0;
}
