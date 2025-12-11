#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s file [args...]\n", *argv);
        return 1;
    }

    void* fp = fopen(*(argv + 1), "r");
    if (!fp) {
        printf("file '%s' does not exist\n", *(argv + 1));
        return 1;
    }

    char* p = calloc(4096, 1);
    char* pp = p;

    int c;
    while ((c = fgetc(fp)) != -1) {
        *pp = c;
        pp += 1;
    }
    *pp = 0;

    printf("%s\n", p);

    return 0;
}

