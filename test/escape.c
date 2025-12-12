#include <stdio.h>

int main() {
    int a = '\n';
    printf("'\\n' = %d\n", a);
    a = '\0';
    printf("'\\0' = %d\n", a);
    a = '\t';
    printf("'\\t' = %d\n", a);
    a = '\\';
    printf("'\\\\' = %d\n", a);
    return 0;
}
