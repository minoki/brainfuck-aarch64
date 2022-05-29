# Brainfuck compiler for AArch64

Usage:

```
$ make
$ ./bfinter hello-simple.bf
Hello world!
$ ./bfcompile hello-simple.bf
$ clang out.s  # or gcc out.s
$ ./a.out
Hello world!
```
