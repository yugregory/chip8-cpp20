cc_library(
    name = "app_error",
    hdrs = ["app_error.h"],
)

cc_library(
    name = "chip8",
    srcs = ["chip8.cc"],
    hdrs = ["chip8.h"],
    deps = [
        ":app_error",
    ],
)

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    copts = ["-Iexternal/sdl3/include"],
    deps = [
        ":app_error",
        ":chip8",
        "@sdl3",
    ],
)
