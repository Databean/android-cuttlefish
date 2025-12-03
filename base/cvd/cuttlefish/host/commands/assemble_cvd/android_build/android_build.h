//
// Copyright (C) 2025 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <functional>
#include <map>
#include <string>

#include "cuttlefish/common/libs/utils/result.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/image_provider.h"

namespace cuttlefish {

/**
 * Represents one of the places that can describe an Android build.
 * - A build downloaded by `cvd fetch`.
 * - A local `m` build.
 * - A local `m dist` build.
 * - An unknown case where `--system_image_dir` or `ANDROID_PRODUCT_OUT` points
 *   to a directory with partition files.
 */
class AndroidBuild : public ImageProvider {
 public:
  virtual ~AndroidBuild() = default;

  virtual Result<std::set<std::string, std::less<void>>> AbPartitions() = 0;

  virtual Result<std::map<std::string, std::string>> MiscInfo() = 0;
};

}  // namespace cuttlefish
