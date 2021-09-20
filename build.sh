#!/usr/bin/bash
echo "//< this file is generated by build.sh" | cat > c.c
echo "#include <stdio.h>" | cat >> c.c
echo "#include <stdlib.h>" | cat >> c.c
gcc -E -DPREPROC c-source.c > tmp.c || exit 1
cat tmp.c | sed  '/^#/d' tmp.c | sed '/^\s*$/d' | cat >> c.c
rm tmp.c
gcc -m32 -Wno-int-conversion -Wno-format -Wno-sign-compare -Wno-builtin-declaration-mismatch -Wno-implicit-function-declaration c.c -o c || exit 1