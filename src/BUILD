cc_library(
    name = "core",
    srcs = ["core.cc"],
    hdrs = ["core.h"],
)

cc_library(
    name = "chip8",
    hdrs = ["chip8.h"],
)

cc_binary(
    name = "hello-world",
    srcs = ["main.cc"],
    deps = [
        ":chip8",
        ":core",
    ],
)
