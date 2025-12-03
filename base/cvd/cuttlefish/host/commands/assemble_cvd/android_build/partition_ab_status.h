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

#include <ostream>
#include <string_view>

#include "fmt/ostream.h"

namespace cuttlefish {

enum class PartitionAbStatus {
  kAbUnknown,
  kUsesAb,
  kNotAb,
};

std::string_view ToStringView(PartitionAbStatus);

std::ostream& operator<<(std::ostream&, PartitionAbStatus);

template <typename Sink>
void AbslStringify(Sink& sink, PartitionAbStatus ab_status) {
  sink.Append(ToStringView(ab_status));
}

}  // namespace cuttlefish

namespace fmt {

template <>
struct formatter<::cuttlefish::PartitionAbStatus> : ostream_formatter {};

}  // namespace fmt
