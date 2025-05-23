/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <string>

#include "EGL/egl.h"
#include "EGL/eglext.h"

#include "cuttlefish/host/graphics_detector/egl_funcs.h"
#include "cuttlefish/host/graphics_detector/expected.h"
#include "cuttlefish/host/graphics_detector/lib.h"

namespace gfxstream {

class Egl {
 public:
  static gfxstream::expected<Egl, std::string> Load();

  Egl(const Egl&) = delete;
  Egl& operator=(const Egl&) = delete;

  Egl(Egl&&) = default;
  Egl& operator=(Egl&&) = default;

#define DECLARE_EGL_FUNCTION_MEMBER_POINTER(return_type, function_name, \
                                            signature)                  \
  return_type(EGLAPIENTRY* function_name) signature = nullptr;

  FOR_EACH_EGL_FUNCTION(DECLARE_EGL_FUNCTION_MEMBER_POINTER);

 private:
  Egl() = default;

  gfxstream::expected<Ok, std::string> Init();

  Lib mLib;
};

}  // namespace gfxstream
