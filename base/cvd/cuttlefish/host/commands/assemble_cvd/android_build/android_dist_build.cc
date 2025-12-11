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

#include "cuttlefish/host/commands/assemble_cvd/android_build/android_dist_build.h"

#include <functional>
#include <map>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "fmt/ostream.h"

#include "cuttlefish/common/libs/utils/result.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/android_product_dir.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/image_provider.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/img_zip.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/target_files.h"

namespace cuttlefish {

Result<AndroidDistBuild> AndroidDistBuild::Create(AndroidProductDir product_dir,
                                                  const std::string& dist_dir) {
  ImgZip img_zip = CF_EXPECT(ImgZip::FromDirectory(dist_dir));
  TargetFiles target_files = CF_EXPECT(TargetFiles::FromDirectory(dist_dir));
  return AndroidDistBuild(std::move(product_dir), std::move(img_zip),
                          std::move(target_files));
}

Result<std::set<std::string_view>> AndroidDistBuild::Images() {
  return CF_EXPECT(::cuttlefish::Images(Providers()));
}

Result<std::string_view> AndroidDistBuild::ImageFile(
    std::string_view name, std::optional<std::string_view> extract_dir) {
  return CF_EXPECT(::cuttlefish::ImageFile(Providers(), name, extract_dir));
}

Result<std::set<std::string, std::less<void>>>
AndroidDistBuild::AbPartitions() {
  return CF_EXPECT(target_files_.AbPartitionsTxt());
}

Result<std::map<std::string, std::string>> AndroidDistBuild::MiscInfo() {
  return CF_EXPECT(target_files_.MiscInfoTxt());
}

AndroidDistBuild::AndroidDistBuild(AndroidProductDir product_dir,
                                   ImgZip img_zip, TargetFiles target_files)
    : product_dir_(std::move(product_dir)),
      img_zip_(std::move(img_zip)),
      target_files_(std::move(target_files)) {}

std::vector<ImageProvider*> AndroidDistBuild::Providers() {
  return {&product_dir_, &img_zip_, &target_files_};
}

std::ostream& AndroidDistBuild::Format(std::ostream& out) const {
  out << "AndroidDistBuild { ";
  fmt::print(out, " .product_dir_ = {}, ", product_dir_);
  fmt::print(out, " .img_zip_ = {}, ", img_zip_);
  fmt::print(out, " .target_files_ = {}, ", target_files_);
  return out << "}";
}

std::ostream& operator<<(std::ostream& out, const AndroidDistBuild& dist) {
  return dist.Format(out);
}

}  // namespace cuttlefish
