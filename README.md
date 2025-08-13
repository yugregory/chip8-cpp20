# chip8-cpp20
A chip-8 emulator using some C++ 20/23 features.

Some Language + STL features used include concepts, std::expected

This was a starter emulator project to serve as a launchpad for eventually writing more complicated emulators.
Additionally it was a way for me to learn more about some of the concepts talked about in Denis Bakhvalov's [Perf-Book](https://github.com/dendibakh/perf-book)

Below I discuss some of the optimizations I utilized in the code and give a hypothesis about how I expect the performance of the emulator to change relative to the optimization. 
Below each optimization there is benchmarks section, where I detail how I benchmarked the emulator for the specific optimization and elaborate on the results.

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

## Acknowledgments
- [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [Timendus chip8-test-suite](https://github.com/Timendus/chip8-test-suite)
- [Austin Morlan chip8_emulator](https://austinmorlan.com/posts/chip8_emulator/)
- [Denis Bakhvalov's Performance Analysis and Tuning on Modern CPUs](https://github.com/dendibakh/perf-book)
