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
#include <ostream>
#include <set>
#include <string>
#include <string_view>

#include "absl/strings/str_format.h"
#include "fmt/ostream.h"

#include "cuttlefish/host/commands/assemble_cvd/android_build/image_provider.h"
#include "cuttlefish/host/libs/config/build_archive.h"
#include "cuttlefish/host/libs/config/file_source.h"

namespace cuttlefish {

class TargetFiles : public ImageProvider {
 public:
  static Result<TargetFiles> FromFetcherConfig(const FetcherConfig&,
                                               FileSource);
  static Result<TargetFiles> FromDirectory(const std::string& path);
  static Result<TargetFiles> FromBuildArchive(BuildArchive);

  Result<std::set<std::string_view>> Images() override;
  /**
   * Returns a filesystem path where the image `name` can be read.
   *
   * If the image file needs to be extracted from an archive, it's saved at
   * `extract_dir`.
   */
  Result<std::string_view> ImageFile(
      std::string_view name,
      std::optional<std::string_view> extract_dir) override;

  Result<std::set<std::string, std::less<void>>> AbPartitionsTxt();

  Result<std::map<std::string, std::string>> MiscInfoTxt();

 private:
  TargetFiles(BuildArchive);

  template <typename Sink>
  friend void AbseilStringify(Sink& sink, const TargetFiles& img_zip) {
    sink.Append(absl::FormatStreamed(img_zip));
  }
  std::ostream& Format(std::ostream&) const override;
  friend std::ostream& operator<<(std::ostream&, const TargetFiles&);

  BuildArchive archive_;
};

}  // namespace cuttlefish

namespace fmt {

template <>
struct formatter<::cuttlefish::TargetFiles> : ostream_formatter {};

}  // namespace fmt
