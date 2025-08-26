## Run

`bazel run src:main <example_rom> --copt=<example_copts>`

### copts
1. O2, O3
2. DUSE_VECTOR_DRAW

## Trace

`xctrace record --template "Time Profiler" --target-stdout - --launch ./bazel-bin/src/main <example_rom> --copt=<example_copts>`

## IDE Feature Support

`bazel run @hedron_compile_commands//:refresh_all`


