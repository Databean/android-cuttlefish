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

#include "cuttlefish/host/commands/assemble_cvd/android_build/android_build_partition.h"

#include <map>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <utility>

#include "fmt/ostream.h"
#include "fmt/ranges.h"

#include "cuttlefish/common/libs/utils/result.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/partition_ab_status.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/partition_availability.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/partition_source.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/partition_type.h"

namespace cuttlefish {

AndroidBuildPartition::AndroidBuildPartition(std::string name)
    : name(std::move(name)) {}

AndroidBuildPartition::AndroidBuildPartition(
    std::string name, PartitionAbStatus is_ab,
    std::set<PartitionAvailability> availability, PartitionSource source,
    PartitionType type)
    : name(std::move(name)), is_ab(is_ab), source(source), type(type) {}

Result<void> AndroidBuildPartition::MergeIn(
    const AndroidBuildPartition& other) {
  CF_EXPECT_EQ(name, other.name);
  CF_EXPECT(MergeIn(other.is_ab));
  CF_EXPECT(MergeIn(other.availability));
  CF_EXPECT(MergeIn(other.source));
  CF_EXPECT(MergeIn(other.type));
  return {};
}

Result<void> AndroidBuildPartition::MergeIn(PartitionAbStatus is_ab) {
  if (this->is_ab == PartitionAbStatus::kAbUnknown) {
    this->is_ab = is_ab;
  } else {
    CF_EXPECT_EQ(this->is_ab, is_ab, "Mismatch for '" << name << "'");
  }
  return {};
}

Result<void> AndroidBuildPartition::MergeIn(
    PartitionAvailability availability) {
  this->availability.insert(availability);
  return {};
}

Result<void> AndroidBuildPartition::MergeIn(
    const std::set<PartitionAvailability>& availability_set) {
  for (PartitionAvailability availability : availability_set) {
    this->availability.insert(availability);
  }
  return {};
}

Result<void> AndroidBuildPartition::MergeIn(PartitionSource source) {
  if (this->source == PartitionSource::kUnknown) {
    this->source = source;
  } else {
    CF_EXPECT_EQ(this->source, source, "Mismatch for '" << name << "'");
  }
  return {};
}

Result<void> AndroidBuildPartition::MergeIn(PartitionType type) {
  if (this->type == PartitionType::kUnknown) {
    this->type = type;
  } else {
    CF_EXPECT_EQ(this->type, type, "Mismatch for '" << name << "'");
  }
  return {};
}

std::ostream& operator<<(std::ostream& out,
                         const AndroidBuildPartition& partition) {
  out << "AndroidBuildPartition { ";
  fmt::print(out, ".name = '{}', ", partition.name);
  fmt::print(out, ".is_ab = {}, ", partition.is_ab);
  fmt::print(out, ".availability = {{ {} }}",
             fmt::join(partition.availability, ", "));
  fmt::print(out, ".source = {}, ", partition.source);
  fmt::print(out, ".type = {} ", partition.type);
  return out << "}";
}

Result<std::map<std::string_view, AndroidBuildPartition>> MergePartitions(
    std::map<std::string_view, AndroidBuildPartition> left,
    std::map<std::string_view, AndroidBuildPartition> right) {
  left.merge(right);
  for (auto& [key, right_value] : right) {
    auto left_it = left.find(key);
    CF_EXPECTF(left_it != left.end(), "'{}' should have been merged in", key);
    CF_EXPECT(left_it->second.MergeIn(right_value));
  }
  return left;
}

Result<std::map<std::string_view, AndroidBuildPartition>> MergePartitionsStrict(
    std::map<std::string_view, AndroidBuildPartition> left,
    std::map<std::string_view, AndroidBuildPartition> right) {
  std::set<std::string_view> left_keys;
  for (const auto& [key, unused_value] : left) {
    left_keys.insert(key);
  }

  std::set<std::string_view> right_keys;
  for (const auto& [key, unused_value] : right) {
    right_keys.insert(key);
  }
  if (!left_keys.empty() && !right_keys.empty()) {
    CF_EXPECTF(left_keys == right_keys, "Partition mismatch: ['{}'] != ['{}']",
               fmt::join(left_keys, "', '"), fmt::join(right_keys, "', '"));
  }
  return CF_EXPECT(MergePartitions(std::move(left), std::move(right)));
}

bool HasAbInformation(
    std::map<std::string_view, AndroidBuildPartition> partitions) {
  for (auto& [unused_name, partition] : partitions) {
    if (partition.is_ab != PartitionAbStatus::kAbUnknown) {
      return true;
    }
  }
  return false;
}

bool HasTypeInformation(
    std::map<std::string_view, AndroidBuildPartition> partitions) {
  for (auto& [unused_name, partition] : partitions) {
    if (partition.type != PartitionType::kUnknown) {
      return true;
    }
  }
  return false;
}

}  // namespace cuttlefish
