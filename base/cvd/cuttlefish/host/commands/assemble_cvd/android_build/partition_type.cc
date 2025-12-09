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

#include "cuttlefish/host/commands/assemble_cvd/android_build/partition_type.h"

#include <ostream>
#include <string_view>

namespace cuttlefish {

std::string_view ToStringView(PartitionType ab_status) {
  switch (ab_status) {
    case PartitionType::kUnknown:
      return "PartitionType::kUnknown";
    case PartitionType::kPhysical:
      return "PartitionType::kPhysical";
    case PartitionType::kLogical:
      return "PartitionType::kLogical";
  }
}

std::ostream& operator<<(std::ostream& out, PartitionType ab_status) {
  return out << ToStringView(ab_status);
}

}  // namespace cuttlefish
