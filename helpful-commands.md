## Run

`bazel run src:main <example_rom> --copt=<example_copts>`

### copts
1. O2, O3
2. DUSE_VECTOR_DRAW
3. flto
4. fprofile-instr-generate (To generate profile) 
  - Creates a default .profraw file
  - llvm-profdat merge -o <prof-file.profdata> <prof-raw-file.profraw>
5. fprofile-instr-use=<prof-file.profdata> (To build binary with profile data)

## Trace

`xctrace record --template "Time Profiler" --target-stdout - --launch ./bazel-bin/src/main <example_rom> --copt=<example_copts>`

## IDE Feature Support

`bazel run @hedron_compile_commands//:refresh_all`


