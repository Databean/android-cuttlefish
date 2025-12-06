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

#include <optional>
#include <set>
#include <string_view>
#include <vector>

#include "absl/strings/str_format.h"
#include "fmt/ostream.h"

#include "cuttlefish/common/libs/utils/result.h"

namespace cuttlefish {

class ImageProvider {
 public:
  virtual ~ImageProvider() = default;
  /**
   * Image information, as reported by the Android build system.
   *
   * An image may be one of three different categories:
   * - A partition in the top-level GPT, such as the the `super` partition.
   * - A logical partition stored inside the GPT `super` partition.
   * - A `super_empty` pseudo-partition file that reports what should be in the
   *   `super` partition, but without the logical partition contents.
   */
  virtual Result<std::set<std::string_view>> Images() = 0;
  /*
   * A file on the host that represents an image. If the file is not already
   * stored in a distinct file on the host, it is first saved to `extract_dir`
   * and returned from there.
   * */
  virtual Result<std::string_view> ImageFile(
      std::string_view name,
      std::optional<std::string_view> extract_dir = {}) = 0;

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const ImageProvider& provider) {
    sink.Append(absl::FormatStreamed(provider));
  }

  friend std::ostream& operator<<(std::ostream&, const ImageProvider&);

 private:
  virtual std::ostream& Format(std::ostream&) const = 0;
};

Result<std::set<std::string_view>> Images(const std::vector<ImageProvider*>&);

Result<std::string_view> ImageFile(
    const std::vector<ImageProvider*>&, std::string_view name,
    std::optional<std::string_view> extract_dir = {});

}  // namespace cuttlefish

namespace fmt {

template <>
struct formatter<::cuttlefish::ImageProvider> : ostream_formatter {};

}  // namespace fmt
