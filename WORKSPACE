workspace(name = "chip8")

load("@bazel_tools//tools/build_defs/repo:local.bzl", "new_local_repository")

new_local_repository(
    name = "sdl3",
    build_file_content = """
        cc_library(
            name = "sdl3",
            visibility = ["//visibility:public"],
            includes = ["include"],
            hdrs = glob(["include/SDL3/**/*.h"]),
            srcs = glob(["lib/libSDL3.dylib"]),
        )
    """,
    path = "/opt/homebrew/opt/sdl3",
)
