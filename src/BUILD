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

cc_test(
    name = "chip8_test",
    srcs = ["chip8_test.cc"],
    args = [
        "--test_output=all",
    ],
    copts = [
        "-std=c++23",
    ],
    deps = [
        ":chip8",
        "@googletest//:gtest",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "sdl_lib",
    srcs = ["SDL_system.cc"],
    hdrs = [
        "AV_system.h",
        "SDL_system.h",
    ],
    deps = [
        ":chip8",
        "@sdl3",
    ],
)

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    copts = [
        "-Iexternal/sdl3/include",
        "-std=c++23",
    ],
    deps = [
        ":app_error",
        ":chip8",
        ":sdl_lib",
        "@sdl3",
    ],
)
