# toy-os

### Prereq

[QEMU](https://www.qemu.org/download/) for system emulation

[Docker](https://www.docker.com/products/docker-desktop) for setting up dev environment (cross compiler)

### Build

```shell
# [shell] build env
$ docker build buildenv -t toy-os-buildenv
# [shell] run env
$ docker run --rm -it -v $pwd:/root/env toy-os-buildenv
# or if win
$ docker run --rm -it -v %CD%:/root/env toy-os-buildenv
# [docker] build
$ make os-image.bin
# [shell] run
$ qemu-system-i386 -fda os-image.bin
```

### References

[os-tutorial](https://github.com/cfenollosa/os-tutorial) by `cfenollosa`

[Write Your Own 64-bit Operating System Kernel](https://www.youtube.com/watch?v=FkrpUaGThTQ&ab_channel=CodePulse) by `CodePulse`
