# my_project/local_deps.bzl

# --- STEP 1: Define the implementation for our custom repository rule ---
def _my_local_lib_impl(ctx):
    """Implementation of the my_local_repository rule."""
    # This function gets a `repository_ctx` object.

    # 1. Make the local directory's contents available inside the new repo.
    # The `path` attribute comes from the rule's call site.
    # `ctx.symlink` is efficient. It creates a symlink from the source path
    # into the root of this external repository.
    ctx.symlink(ctx.attr.path, "")

    # 2. Write the BUILD file content programmatically.
    # The `build_file_content` attribute also comes from the rule's call site.
    ctx.file(
        "BUILD.bazel",
        content = ctx.attr.build_file_content,
        executable = False,
    )

# --- STEP 2: Define the repository rule itself ---
# This creates a new rule that we can call, like `http_archive`.
my_local_repository = repository_rule(
    implementation = _my_local_lib_impl,
    # Define the attributes (parameters) our rule will accept.
    attrs = {
        "path": attr.string(mandatory = True),
        "build_file_content": attr.string(mandatory = True),
    },
)

# --- STEP 3: Update the module extension to use our new rule ---
def _local_deps_extension_impl(ctx):
    """Implementation of the module extension."""
    # This function gets a `module_ctx` object.

    for mod in ctx.modules:
        for dep in mod.tags.local_lib:
            # Instead of `native.new_local_repository`, we now call our own rule!
            my_local_repository(
                name = dep.name,
                path = dep.path,
                build_file_content = dep.build_file_content,
            )

# The tag and extension definitions remain the same.
local_lib_tag = tag_class(attrs = {
    "name": attr.string(mandatory = True),
    "path": attr.string(mandatory = True),
    "build_file_content": attr.string(mandatory = True),
})

local_deps_extension = module_extension(
    implementation = _local_deps_extension_impl,
    tag_classes = {"local_lib": local_lib_tag},
)
