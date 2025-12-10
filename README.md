# c.c

## CMake
```shell
$ cmake -S . -B build -A Win32
```

## Shell

```shell
$ sh build.sh
$ ./c hello.c
$ ./c c.c hello.c
$ ./c c.c c.c hello.c
$ ./c c.c c.c c.c hello.c
$ sh run-test.sh
```

This project is a tiny C interpreter inspired by [c4](https://github.com/rswier/c4).
Only works on 32-bit arch becase the program assumes int and pointer has the same size.
