# chip8-cpp20
A chip-8 emulator using some C++ 20/23 features.

Some Language + STL features used include concepts, std::expected

## Prerequisites
1. [Bazel](https://bazel.build/) - Build Tool/System
2. [LLVM](https://llvm.org/) - Compiler Infrastructure, Includes multiple subprojects used below (clang, libc++, BOLT, etc.)
3. [SDL3](https://wiki.libsdl.org/SDL3/FrontPage) - Audio/Visual/IO Library

## Run

```
bazel run src:main {$PATH_TO_YOUR_CHIP8_ROM}
```

### Optimizations

#### Removing Branch Prediction

#### Data Layout

#### SIMD

### Linker Optimizations

#### LTO

#### PGO

### Post Link Optimizations

#### BOLT

#### Propeller

