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

### Decreasing Branch Prediction

One of the things that our chip8 emulator needs to do is determine which opcode to run from the ROM and execute that operation. There are 36 different opcodes.

The traditional if-else chain involves a linear scan through the if statements, which could result in a branch misprediction at every step of the chain. 
This requires the hardware to rollback its speculative execution if a branch is mispredicted. A switch statement might be a better alternative here as the compiler could create a jump table, 
but alternatively the compiler might not if the case conditions are sparse (i.e. the values are not one after another, like 1, 20, 45, ...)

If we were to use a table of function pointers, however we remove the branch predictions of the if-else  and instead have a table lookup in the data cache/memory (as opposed to the instruction cache/memory). 
This lookup cost is the same regardless of which item in the table you are searching for compared to the cost of a if statement further down the chain. While we do remove branch prediction in the sense of 
if-else statements, we now have to contend with a indirect branch prediction in determining which function from the table we actually execute. 
As there are a high number of potential entries, the indirect branch predictor is likely to fail. The choice of using a table of function pointers, 
changes the equation of having potentially multiple branch predictions fail, to almost always incurring a singular indirect branch misprediction. 
The belief here is this will keep functions runtime cost stable regardless of which opcode is invoked.

**Hypothesis: If our chip8 emulator uses a table of function pointers (as opposed to chained if-else statements), the runtime of the function that determines which opcode to run will have a 
lower standard deviation through the multiple calls to the function.**

### Data Layout

The chip8 emulator stores many variables within its class that help to simulate the hardware of the original chip8. There's memory, the registers, the stack, etc.

One potential slowdown during a programs execution is that its data members are not arranged in such a way that makes the hardwares life simpler. To go further into detail, two specific cases are:

1. **Data is not aligned for minimal padding**
 - The primary issue here is that the compiler does alignment relative to each element in the class/struct, and if your data is not aligned, there could be excessive padding.
 - Suppose our class has a char, double, and int (defined in that order). The char takes 1 byte, the double takes 8 bytes, and the int takes 4 bytes.
    - In this example the compiler will insert 7 bytes of padding to the char and 4 bytes of padding to the int, so that the hardware can more easily access the double
    - Alternatively we could restructure the so that its double, int, char. This way we only have 3 bytes of padding for the 8 - 4 (int) - 1 (char).
 - The benefit of alignment here is that we utilize less memory and as such our cache lines are better utilized, improving the runtime and memory efficiency of our program
 - *NOTE: Sometimes we want padding, such as in the case of false sharing for multithread applications, but that's outside the scope of this emulator*

2. **Data that is related to each other is not packed together**
 - The primary issue here is, if data that is often used together is not packed closely, reads might span multiple cache lines.
 - On most modern x86 systems, L1 cache lines are 64 bytes
 - Suppose we had 3 variables that were always used in a tight loop. We have a int, double, char again.
   - If we have some number of variables that take up a substantial part of memory between the int, double, and char, Each of these three variables would live on a separate cache line
   - Now if this tight loop is being run through many iterations, our L1 cache has 3 separate lines that are all being loaded into our cache which is wasteful and could evict lines that are needed further down the line
   - Even worse depending on the associativity of our L1 cache, loading one of these three variables could evict the cache line of another one our variables, resulting in a large negative performance impact
 - The benefit of packing here is that data that is used together can be on the same cacheline, resulting in improved runtime and better hardware cache utilization.

**Hypothesis 1: If our chip8 emulator aligns its data member variables to reduce padding, we will have an overall lower program memory footprint and improved runtime execution.**

**Hypothesis 2: If our chip8 emulator packs its data member variables relative to their usage, we will have even further improved runtime execution.**

### SIMD

## Linker Optimizations

### LTO

### PGO

## Post Link Optimizations

### BOLT

### Propeller

## Acknowledgments
- [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [Timendus chip8-test-suite](https://github.com/Timendus/chip8-test-suite)
- [Austin Morlan chip8_emulator](https://austinmorlan.com/posts/chip8_emulator/)
- [Denis Bakhvalov's Performance Analysis and Tuning on Modern CPUs](https://github.com/dendibakh/perf-book)
