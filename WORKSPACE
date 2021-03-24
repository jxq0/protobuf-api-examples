load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
  name="com_google_protobuf",
  remote="https://github.com/protocolbuffers/protobuf.git",
  tag="v3.15.6",
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")
protobuf_deps()
