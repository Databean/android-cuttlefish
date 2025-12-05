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

#include "cuttlefish/host/commands/assemble_cvd/android_build/target_files.h"

#include <functional>
#include <map>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <utility>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "fmt/ostream.h"

#include "cuttlefish/common/libs/key_equals_value/key_equals_value.h"
#include "cuttlefish/common/libs/utils/result.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/find_build_archive.h"
#include "cuttlefish/host/libs/config/build_archive.h"
#include "cuttlefish/host/libs/config/fetcher_config.h"
#include "cuttlefish/host/libs/config/file_source.h"

namespace cuttlefish {

static constexpr std::string_view kTargetFilesMatch = "-target_files-";
static constexpr std::string_view kImgSuffix = ".img";

Result<TargetFiles> TargetFiles::FromFetcherConfig(const FetcherConfig& config,
                                                   FileSource source) {
  BuildArchive archive =
      CF_EXPECT(FindBuildArchive(config, source, kTargetFilesMatch));
  return CF_EXPECT(TargetFiles::FromBuildArchive(std::move(archive)));
}

Result<TargetFiles> TargetFiles::FromDirectory(const std::string& path) {
  BuildArchive archive = CF_EXPECT(FindBuildArchive(path, kTargetFilesMatch));
  return CF_EXPECT(TargetFiles::FromBuildArchive(std::move(archive)));
}

Result<TargetFiles> TargetFiles::FromBuildArchive(BuildArchive archive) {
  TargetFiles target_files(std::move(archive));

  CF_EXPECT(!target_files.archive_.Members().empty());

  return std::move(target_files);
}

Result<std::set<std::string_view>> TargetFiles::Images() {
  std::set<std::string_view> partitions;
  for (std::string_view member : archive_.Members()) {
    if (!absl::ConsumeSuffix(&member, kImgSuffix)) {
      continue;
    }
    absl::ConsumePrefix(&member, "/");
    if (!absl::ConsumePrefix(&member, "IMAGES/")) {
      continue;
    }
    partitions.emplace(std::move(member));
  }
  return partitions;
}

Result<std::string_view> TargetFiles::ImageFile(
    std::string_view name, std::optional<std::string_view> extract_dir) {
  std::string member_name = absl::StrCat(name, kImgSuffix);
  return CF_EXPECT(archive_.MemberFilepath(member_name, extract_dir));
}

Result<std::set<std::string, std::less<void>>> TargetFiles::AbPartitionsTxt() {
  static constexpr std::string_view kAbTxt = "META/ab_partitions.txt";
  return absl::StrSplit(CF_EXPECT(archive_.MemberContents(kAbTxt)), "\n");
}

Result<std::map<std::string, std::string>> TargetFiles::MiscInfoTxt() {
  std::string contents =
      CF_EXPECT(archive_.MemberContents("META/misc_info.txt"));
  return CF_EXPECT(ParseKeyEqualsValue(contents));
}

std::ostream& TargetFiles::Format(std::ostream& out) const {
  fmt::print(out, "TargetFiles {{ {} }}", archive_);
  return out;
}

std::ostream& operator<<(std::ostream& out, const TargetFiles& target_files) {
  return target_files.Format(out);
}

}  // namespace cuttlefish
