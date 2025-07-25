cc_library(
    name = "core",
    srcs = ["core.cc"],
    hdrs = ["core.h"],
)

cc_library(
    name = "chip8",
    srcs = ["chip8.cc"],
    hdrs = ["chip8.h"],
)

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    copts = ["-Iexternal/sdl3/include"],
    deps = [
        ":chip8",
        ":core",
        "@sdl3",
    ],
)
