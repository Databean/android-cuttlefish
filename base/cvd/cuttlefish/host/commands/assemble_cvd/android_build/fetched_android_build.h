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
#include <optional>
#include <set>
#include <string>
#include <string_view>

#include "absl/strings/str_format.h"
#include "fmt/ostream.h"

#include "cuttlefish/common/libs/utils/result.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/android_build.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/image_provider.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/img_zip.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/target_files.h"
#include "cuttlefish/host/libs/config/fetcher_config.h"
#include "cuttlefish/host/libs/config/file_source.h"

namespace cuttlefish {

/** Represents a build downloaded by `cvd fetch`.  */
class FetchedAndroidBuild : public AndroidBuild {
 public:
  FetchedAndroidBuild(const FetcherConfig&, FileSource);

  Result<std::set<std::string_view>> Images() override;

  Result<std::string_view> ImageFile(
      std::string_view name,
      std::optional<std::string_view> extract_dir = {}) override;

  Result<std::set<std::string, std::less<void>>> AbPartitions() override;

  Result<std::map<std::string, std::string>> MiscInfo() override;

 private:
  Result<ImgZip*> MemoizeImgZip();
  Result<TargetFiles*> MemoizeTargetFiles();
  std::vector<ImageProvider*> ImageProviders();
  std::ostream& Format(std::ostream&) const override;

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const FetchedAndroidBuild& provider) {
    sink.Append(absl::FormatStreamed(provider));
  }
  friend std::ostream& operator<<(std::ostream&, const FetchedAndroidBuild&);

  FetcherConfig const* fetcher_config_;
  FileSource source_;
  std::optional<ImgZip> img_;
  std::optional<TargetFiles> target_files_;
};

}  // namespace cuttlefish

namespace fmt {

template <>
struct formatter<::cuttlefish::FetchedAndroidBuild> : ostream_formatter {};

}  // namespace fmt
