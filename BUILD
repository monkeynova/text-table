load("@com_monkeynova_base_workspace//:default_rules.bzl", "default_rules")

default_rules(workspace_dep = "update_workspace.date")


cc_library(
    name = "text_table",
    srcs = ["text_table.cc"],
    hdrs = ["text_table.h"],
    visibility = ["//visibility:public"],
    deps = [
       "@abseil-cpp//absl/algorithm:container",
       "@abseil-cpp//absl/container:flat_hash_set",
       "@abseil-cpp//absl/flags:flag",
       "@abseil-cpp//absl/log:check",
       "@com_monkeynova_gunit_main//:vlog",
    ],
)
