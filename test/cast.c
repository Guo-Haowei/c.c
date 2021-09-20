#include <stdio.h>
#include <stdlib.h>

int main() {
    char* p = malloc(4096);
    char* src = "hello there one more time";
    int i = 0;
    while (src[i]) {
        p[i] = src[i];
        ++i;
    }
    p[i] = 0;
    printf("%s\n", p);

    int* ip = p;
    i = 0;
    while (i <= 3) {
        printf("%d\n", ip[i]);
        ++i;
    }

    printf("%d\n", *((int*)p));
    printf("%d\n", *(((int*)p) + 1));
    printf("%d\n", *(((int*)p) + 2));
 
    return 0;
}
