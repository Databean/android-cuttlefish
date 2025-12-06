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

#include "cuttlefish/host/commands/assemble_cvd/android_build/image_provider.h"

#include <optional>
#include <ostream>
#include <set>
#include <string_view>
#include <vector>

#include "cuttlefish/common/libs/utils/result.h"

namespace cuttlefish {

std::ostream& operator<<(std::ostream& out, const ImageProvider& build) {
  return build.Format(out);
}

Result<std::set<std::string_view>> Images(
    const std::vector<ImageProvider*>& providers) {
  std::set<std::string_view> ret_images;
  for (ImageProvider* provider : providers) {
    CF_EXPECT_NE(provider, nullptr);
    std::set<std::string_view> images = CF_EXPECT(provider->Images());
    ret_images.insert(images.begin(), images.end());
  }
  return ret_images;
}

Result<std::string_view> ImageFile(
    const std::vector<ImageProvider*>& providers, std::string_view name,
    std::optional<std::string_view> extract_dir) {
  for (ImageProvider* provider : providers) {
    CF_EXPECT_NE(provider, nullptr);
    Result<std::string_view> already_extracted = provider->ImageFile(name, {});
    if (already_extracted.ok()) {
      return *already_extracted;
    }
  }
  CF_EXPECTF(extract_dir.has_value(), "need extract_dir to extract '{}'", name);
  for (ImageProvider* provider : providers) {
    Result<std::string_view> attempt = provider->ImageFile(name, extract_dir);
    if (attempt.ok()) {
      return *attempt;
    }
  }
  return CF_ERRF("Failed to extract '{}'", name);
}

}  // namespace cuttlefish
