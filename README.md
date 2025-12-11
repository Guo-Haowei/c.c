# c.c

## Build
```shell
$ cmake -S . -B build -A Win32
$ cmake --build build --config Debug
```

## Self Hosting
```shell
$ py -u scripts/preprocess.py # This preprocess the source code, because c.c doesn't have preprocessor
$ ./c hello.c                 # hello world
$ ./c c.c c.c hello.c         # self hosting
```

## Testing
```shell
$ # copy c.c.exe to root dir
$ py -u scripts/run_tests.py
```

This project is a tiny C interpreter inspired by [c4](https://github.com/rswier/c4).
Only works on 32-bit arch becase the program assumes int and pointer has the same size.
