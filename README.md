# chip8-cpp20
A chip-8 emulator using some C++ 20/23 features.

Some Language + STL features used include concepts, std::expected

## Prerequisites
1. [Bazel](https://bazel.build/) - Build Tool/System
2. [LLVM](https://llvm.org/) - Compiler Infrastructure, Includes multiple subprojects used below (clang, libc++, BOLT, etc.)
3. [SDL3](https://wiki.libsdl.org/SDL3/FrontPage) - Audio/Visual/IO Library
4. [xctrace](https://keith.github.io/xcode-man-pages/xctrace.1.html) - Profile programs running on MacOS

## Run

```
bazel run src:main {$PATH_TO_YOUR_CHIP8_ROM}
```

## Optimizations

#### Removing Branch Prediction

#### Data Layout

#### SIMD

### Linker Optimizations

#### LTO

#### PGO

### Post Link Optimizations

#### BOLT

#### Propeller

## Benchmarks

## Acknowledgments
- [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [Timendus chip8-test-suite](https://github.com/Timendus/chip8-test-suite)
- [Austin Morlan chip8_emulator](https://austinmorlan.com/posts/chip8_emulator/)
