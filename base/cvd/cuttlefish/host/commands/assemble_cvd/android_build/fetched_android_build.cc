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

#include "cuttlefish/host/commands/assemble_cvd/android_build/fetched_android_build.h"
#include <android-base/file.h>

#include <functional>
#include <map>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "fmt/ostream.h"

#include "cuttlefish/common/libs/key_equals_value/key_equals_value.h"
#include "cuttlefish/common/libs/utils/result.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/image_provider.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/img_zip.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/target_files.h"
#include "cuttlefish/host/libs/config/fetcher_config.h"
#include "cuttlefish/host/libs/config/file_source.h"

namespace cuttlefish {

FetchedAndroidBuild::FetchedAndroidBuild(const FetcherConfig& config,
                                         FileSource source)
    : fetcher_config_(&config), source_(source) {}

Result<ImgZip*> FetchedAndroidBuild::MemoizeImgZip() {
  if (!img_.has_value()) {
    img_ = CF_EXPECT(ImgZip::FromFetcherConfig(*fetcher_config_, source_));
  }
  return &img_.value();
}

Result<TargetFiles*> FetchedAndroidBuild::MemoizeTargetFiles() {
  if (!target_files_.has_value()) {
    target_files_ =
        CF_EXPECT(TargetFiles::FromFetcherConfig(*fetcher_config_, source_));
  }
  return &target_files_.value();
}

std::vector<ImageProvider*> FetchedAndroidBuild::ImageProviders() {
  std::vector<ImageProvider*> providers;
  if (Result<ImgZip*> img = MemoizeImgZip(); img.ok()) {
    providers.emplace_back(img.value());
  }
  if (Result<TargetFiles*> target = MemoizeTargetFiles(); target.ok()) {
    providers.emplace_back(target.value());
  }
  return providers;
}

Result<std::set<std::string_view>> FetchedAndroidBuild::Images() {
  return CF_EXPECT(::cuttlefish::Images(ImageProviders()));
}

Result<std::string_view> FetchedAndroidBuild::ImageFile(
    std::string_view name, std::optional<std::string_view> extract_dir) {
  return CF_EXPECT(
      ::cuttlefish::ImageFile(ImageProviders(), name, extract_dir));
}

Result<std::set<std::string, std::less<void>>>
FetchedAndroidBuild::AbPartitions() {
  TargetFiles* target_files = CF_EXPECT(MemoizeTargetFiles());
  return CF_EXPECT(target_files->AbPartitionsTxt());
}

Result<std::map<std::string, std::string>> FetchedAndroidBuild::MiscInfo() {
  if (Result<TargetFiles*> target = MemoizeTargetFiles(); target.ok()) {
    return CF_EXPECT(target.value()->MiscInfoTxt());
  }
  std::string path =
      fetcher_config_->FindCvdFileWithSuffix(source_, "misc_info.txt");
  CF_EXPECT(!path.empty(), "Could not find misc_info.txt in fetcher config");
  std::string contents;
  CF_EXPECT(android::base::ReadFileToString(path, &contents));
  return CF_EXPECT(ParseKeyEqualsValue(contents));
}

std::ostream& FetchedAndroidBuild::Format(std::ostream& out) const {
  out << "FetchedAndroidBuild {\n";
  if (img_.has_value()) {
    fmt::print(out, "\t.img_ = {},\n", img_.value());
  }
  if (target_files_.has_value()) {
    fmt::print(out, "\t.target_files_ = {},\n", target_files_.value());
  }
  return out << "}";
}

std::ostream& operator<<(std::ostream& out,
                         const FetchedAndroidBuild& fetched_build) {
  return fetched_build.Format(out);
}

}  // namespace cuttlefish
