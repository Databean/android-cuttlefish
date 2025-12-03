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

#include <map>
#include <ostream>
#include <set>
#include <string>
#include <string_view>

#include "absl/strings/str_format.h"
#include "fmt/ostream.h"

#include "cuttlefish/common/libs/utils/result.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/partition_ab_status.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/partition_availability.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/partition_source.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/partition_type.h"

namespace cuttlefish {

struct AndroidBuildPartition {
  explicit AndroidBuildPartition(std::string);
  AndroidBuildPartition(std::string, PartitionAbStatus,
                        std::set<PartitionAvailability>, PartitionSource,
                        PartitionType);

  // Performs additive transformations only. Fields can go from "unknown" to
  // "known" or sets can gain additional members.
  Result<void> MergeIn(const AndroidBuildPartition&);
  Result<void> MergeIn(PartitionAbStatus);
  Result<void> MergeIn(PartitionAvailability);
  Result<void> MergeIn(const std::set<PartitionAvailability>&);
  Result<void> MergeIn(PartitionSource);
  Result<void> MergeIn(PartitionType);

  template <typename Sink>
  friend void AbslStringify(Sink& sink,
                            const AndroidBuildPartition& partition) {
    sink.Append(absl::FormatStreamed(partition));
  }

  std::string name;
  PartitionAbStatus is_ab = PartitionAbStatus::kAbUnknown;
  std::set<PartitionAvailability> availability;
  PartitionSource source = PartitionSource::kUnknown;
  PartitionType type = PartitionType::kUnknown;
};

std::ostream& operator<<(std::ostream&, const AndroidBuildPartition&);

Result<std::map<std::string_view, AndroidBuildPartition>> MergePartitions(
    std::map<std::string_view, AndroidBuildPartition>,
    std::map<std::string_view, AndroidBuildPartition>);

Result<std::map<std::string_view, AndroidBuildPartition>> MergePartitionsStrict(
    std::map<std::string_view, AndroidBuildPartition>,
    std::map<std::string_view, AndroidBuildPartition>);

bool HasAbInformation(
    std::map<std::string_view, AndroidBuildPartition> partitions);

bool HasTypeInformation(
    std::map<std::string_view, AndroidBuildPartition> partitions);

}  // namespace cuttlefish

namespace fmt {

template <>
struct formatter<::cuttlefish::AndroidBuildPartition> : ostream_formatter {};

}  // namespace fmt
