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

#include "cuttlefish/host/commands/assemble_cvd/android_build/android_product_dir.h"

#include <functional>
#include <map>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <utility>

#include "absl/strings/str_cat.h"
#include "absl/strings/strip.h"
#include "fmt/ostream.h"
#include "fmt/ranges.h"

#include "cuttlefish/common/libs/utils/files.h"
#include "cuttlefish/common/libs/utils/result.h"

namespace cuttlefish {

static constexpr std::string_view kImgSuffix = ".img";

Result<AndroidProductDir> AndroidProductDir::Create(
    std::string path) {
  std::map<std::string, std::string, std::less<void>> images;
  for (std::string_view member : CF_EXPECT(DirectoryContents(path))) {
    if (!absl::ConsumeSuffix(&member, kImgSuffix)) {
      continue;
    }
    images.emplace(member, absl::StrCat(path, "/", member, kImgSuffix));
  }
  CF_EXPECT(!images.empty());
  return AndroidProductDir(std::move(path), std::move(images));
}

Result<std::set<std::string_view>> AndroidProductDir::Images() {
  std::set<std::string_view> images;
  for (auto& [name, unused_path] : images_) {
    images.emplace(name);
  }
  return images;
}

Result<std::string_view> AndroidProductDir::ImageFile(
    std::string_view name, std::optional<std::string_view>) {
  auto image_it = images_.find(name);
  CF_EXPECT(image_it != images_.end());
  return image_it->second;
}

Result<std::set<std::string, std::less<void>>> AbPartitions() {
  return CF_ERR("ab_partitions.txt not present in an `m` build");
}

Result<std::map<std::string, std::string>> MiscInfo() {
  return CF_ERR("misc_info.txt not present in an `m` build");
}

AndroidProductDir::AndroidProductDir(
    std::string path,
    std::map<std::string, std::string, std::less<void>> images)
    : path_(std::move(path)), images_(std::move(images)) {}

std::ostream& AndroidProductDir::Format(std::ostream& out) const {
  out << "AndroidProductDir { ";
  fmt::print(out, " .path = '{}'", path_);
  fmt::print(out, " .images = ['{}'] ", fmt::join(images_, "', '"));
  return out << "}";
}

std::ostream& operator<<(std::ostream& out,
                         const AndroidProductDir& provider) {
  return provider.Format(out);
}

}  // namespace cuttlefish
