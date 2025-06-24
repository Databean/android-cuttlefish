# Copyright (C) 2025 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

load("//:build_variables.bzl", BUILD_VAR_COPTS = "COPTS")
load("//tools/lint:linters.bzl", "clang_tidy_test")

COPTS = BUILD_VAR_COPTS

def _cf_cc_binary_implementation(name, clang_tidy_enabled, copts, **kwargs):
    if not clang_tidy_enabled and "deprecation" not in kwargs:
        kwargs["deprecation"] = "Target '" + name + "' not covered by clang-tidy"
    native.cc_binary(
        name = name,
        copts = (copts or []) + COPTS,
        **kwargs,
    )
    if clang_tidy_enabled:
        clang_tidy_test(
            name = name + "_clang_tidy",
            srcs = [":" + name],
            tags = ["clang_tidy", "clang-tidy"],
            visibility = ["//visibility:private"],
        )

cf_cc_binary = macro(
    inherit_attrs = native.cc_binary,
    attrs = {
        "clang_tidy_enabled": attr.bool(configurable = False, default = True, doc = "Decide if a corresponding clang_tidy_test target is generated"),
        "copts": attr.string_list(configurable = False, default = []),
    },
    implementation = _cf_cc_binary_implementation,
)

def _cf_cc_library_implementation(name, clang_tidy_enabled, copts, strip_include_prefix, **kwargs):
    if not clang_tidy_enabled and "deprecation" not in kwargs:
        kwargs["deprecation"] = "Target '" + name + "' not covered by clang-tidy"
    native.cc_library(
        name = name,
        copts = (copts or []) + COPTS,
        strip_include_prefix = strip_include_prefix or "//cuttlefish",
        **kwargs,
    )
    if clang_tidy_enabled:
        clang_tidy_test(
            name = name + "_clang_tidy",
            srcs = [":" + name],
            tags = ["clang_tidy", "clang-tidy"],
            visibility = ["//visibility:private"],
        )

cf_cc_library = macro(
    inherit_attrs = native.cc_library,
    attrs = {
        "clang_tidy_enabled": attr.bool(configurable = False, default = True, doc = "Decide if a corresponding clang_tidy_test target is generated"),
        "copts": attr.string_list(configurable = False, default = []),
        "strip_include_prefix": attr.string(configurable = False, default = ""),
    },
    implementation = _cf_cc_library_implementation,
)

def _cf_cc_test_implementation(name, clang_tidy_enabled, copts, **kwargs):
    if not clang_tidy_enabled and "deprecation" not in kwargs:
        kwargs["deprecation"] = "Target '" + name + "' not covered by clang-tidy"
    native.cc_test(
        name = name,
        copts = (copts or []) + COPTS,
        **kwargs,
    )
    if clang_tidy_enabled: 
        clang_tidy_test(
            name = name + "_clang_tidy",
            srcs = [":" + name],
            tags = ["clang_tidy", "clang-tidy"],
            visibility = ["//visibility:private"],
        )

cf_cc_test = macro(
    inherit_attrs = native.cc_test,
    attrs = {
        "clang_tidy_enabled": attr.bool(configurable = False, default = True, doc = "Decide if a corresponding clang_tidy_test target is generated"),
        "copts": attr.string_list(configurable = False, default = []),
    },
    implementation = _cf_cc_test_implementation,
)
