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
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "absl/strings/str_format.h"
#include "fmt/ostream.h"

#include "cuttlefish/common/libs/utils/result.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/android_build.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/android_product_dir.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/img_zip.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/target_files.h"

namespace cuttlefish {

class AndroidDistBuild : public AndroidBuild {
 public:
  static Result<AndroidDistBuild> Create(AndroidProductDir product_dir,
                                         const std::string& dist_dir);

  Result<std::set<std::string_view>> Images() override;

  Result<std::string_view> ImageFile(
      std::string_view name,
      std::optional<std::string_view> extract_dir = {}) override;

  Result<std::set<std::string, std::less<void>>> AbPartitions() override;

  Result<std::map<std::string, std::string>> MiscInfo() override;

 private:
  AndroidDistBuild(AndroidProductDir, ImgZip, TargetFiles);

  std::vector<ImageProvider*> Providers();
  std::ostream& Format(std::ostream&) const override;

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const AndroidDistBuild& provider) {
    sink.Append(absl::FormatStreamed(provider));
  }

  friend std::ostream& operator<<(std::ostream&, const AndroidDistBuild&);

  AndroidProductDir product_dir_;
  ImgZip img_zip_;
  TargetFiles target_files_;
};

}  // namespace cuttlefish

namespace fmt {

template <>
struct formatter<::cuttlefish::AndroidDistBuild> : ostream_formatter {};

}  // namespace fmt
