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

#include "cuttlefish/host/commands/assemble_cvd/android_build/identify_partitions.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/partition_availability.h"
#include "fmt/ranges.h"
#include "liblp/liblp.h"
#include "liblp/metadata_format.h"

#include "cuttlefish/common/libs/utils/result.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/android_build.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/android_build_partition.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/partition_ab_status.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/partition_source.h"
#include "cuttlefish/host/commands/assemble_cvd/android_build/partition_type.h"

namespace cuttlefish {
namespace {

constexpr std::string_view kMiscInfoPartitionGroups = "super_partition_groups";
constexpr std::string_view kSuperEmpty = "super_empty";
constexpr std::string_view kSuper = "super";

Result<std::map<std::string_view, AndroidBuildPartition>> ImagesAsPartitions(
    AndroidBuild& build) {
  std::set<std::string_view> images = CF_EXPECT(build.Images());
  CF_EXPECT(!images.empty());

  std::map<std::string_view, AndroidBuildPartition> ret;
  for (std::string_view image : images) {
    AndroidBuildPartition partition{std::string(image)};

    if (build.ImageFile(image).has_value()) {
      CF_EXPECT(partition.MergeIn(PartitionAvailability::kOnFilesystem));
    } else {
      CF_EXPECT(partition.MergeIn(PartitionAvailability::kInZipArchive));
    }

    ret.emplace(partition.name, std::move(partition));
  }

  return ret;
}

bool IsSystem(std::string_view group) {
  return absl::StrContains(group, "system");
}

bool IsVendor(std::string_view group) {
  return absl::StrContains(group, "vendor");
}

std::string GroupPartitionKey(std::string_view group_name) {
  return absl::StrCat("super_", group_name, "_partition_list");
}

// A successful Result with an empty unique_ptr is possible, and means there was
// no file named `super.img` or `super_empty.img`
Result<std::unique_ptr<android::fs_mgr::LpMetadata>> SuperImageMetadata(
    AndroidBuild& build, std::string_view extract_dir) {
  // Prefer an already extracted file, then prefer extracting super_empty since
  // it is smaller.
  Result<std::string_view> path;
  if (path = build.ImageFile(kSuperEmpty); path.ok()) {
  } else if (path = build.ImageFile(kSuper); path.ok()) {
  } else if (path = build.ImageFile(kSuperEmpty, extract_dir); path.ok()) {
  } else if (path = build.ImageFile(kSuper, extract_dir); path.ok()) {
  } else {
    return {};
  }
  std::unique_ptr<android::fs_mgr::LpMetadata> metadata =
      android::fs_mgr::ReadFromImageFile(std::string(*path));
  CF_EXPECTF(metadata.get(), "Failed to parse super image '{}'", *path);
  return metadata;
}

Result<std::map<std::string_view, AndroidBuildPartition>>
LogicalPartitionsFromSuperMetadata(
    AndroidBuild& build, const android::fs_mgr::LpMetadata& metadata) {
  std::set<std::string_view> images = CF_EXPECT(build.Images());

  std::map<std::string_view, AndroidBuildPartition> ret;
  for (const LpMetadataPartition& partition : metadata.partitions) {
    AndroidBuildPartition out(android::fs_mgr::GetPartitionName(partition));

    CF_EXPECT(out.MergeIn(partition.attributes & LP_PARTITION_ATTR_SLOT_SUFFIXED
                              ? PartitionAbStatus::kUsesAb
                              : PartitionAbStatus::kNotAb));

    CF_EXPECT_LE(partition.group_index, metadata.groups.size());
    std::string group_name = android::fs_mgr::GetPartitionGroupName(
        metadata.groups[partition.group_index]);
    if (IsSystem(group_name)) {
      CF_EXPECT(out.MergeIn(PartitionSource::kSystem));
    } else if (IsVendor(group_name)) {
      CF_EXPECT(out.MergeIn(PartitionSource::kVendor));
    }

    if (build.ImageFile(kSuper).ok()) {
      CF_EXPECT(out.MergeIn(PartitionAvailability::kInSuperImageOnFilesystem));
    } else if (images.count(kSuper)) {
      CF_EXPECT(out.MergeIn(PartitionAvailability::kInSuperImageInZipArchive));
    }

    ret.emplace(out.name, std::move(out));
  }
  return ret;
}

Result<std::map<std::string_view, AndroidBuildPartition>>
LogicalPartitionsFromMiscInfo(
    const std::map<std::string, std::string>& misc_info) {
  auto groups_it = misc_info.find(std::string(kMiscInfoPartitionGroups));
  CF_EXPECTF(groups_it != misc_info.end(), "Could not find entry for key '{}'",
             kMiscInfoPartitionGroups);

  std::set<std::string_view> groups =
      absl::StrSplit(groups_it->second, " ", absl::SkipEmpty());
  CF_EXPECT(!groups.empty());

  std::string_view system_group, vendor_group;
  for (std::string_view group : groups) {
    if (IsSystem(group)) {
      system_group = group;
    } else if (IsVendor(group)) {
      vendor_group = group;
    }
  }
  CF_EXPECTF(!system_group.empty(), "No system group in ['{}']",
             fmt::join(groups, "', '"));
  CF_EXPECTF(!vendor_group.empty(), "No vendor group in ['{}']",
             fmt::join(groups, "', '"));

  std::map<std::string_view, AndroidBuildPartition> ret;

  std::string system_key = GroupPartitionKey(system_group);
  auto system_it = misc_info.find(system_key);
  CF_EXPECTF(system_it != misc_info.end(), "Could not find entry for key '{}'",
             system_key);

  std::set<std::string_view> system_partitions =
      absl::StrSplit(system_it->second, " ", absl::SkipEmpty());
  for (std::string_view system_partition : system_partitions) {
    AndroidBuildPartition partition{std::string(system_partition)};
    CF_EXPECT(partition.MergeIn(PartitionSource::kSystem));
    CF_EXPECT(partition.MergeIn(PartitionType::kLogical));

    bool inserted = ret.emplace(partition.name, std::move(partition)).second;
    CF_EXPECTF(!!inserted, "Duplicate partition '{}'", system_partition);
  }

  std::string vendor_key = GroupPartitionKey(vendor_group);
  auto vendor_it = misc_info.find(vendor_key);
  CF_EXPECTF(vendor_it != misc_info.end(), "Could not find entry for key '{}'",
             vendor_key);

  std::set<std::string_view> vendor_partitions =
      absl::StrSplit(system_it->second, " ", absl::SkipEmpty());
  for (std::string_view vendor_partition : vendor_partitions) {
    AndroidBuildPartition partition{std::string(vendor_partition)};
    CF_EXPECT(partition.MergeIn(PartitionSource::kVendor));
    CF_EXPECT(partition.MergeIn(PartitionType::kLogical));

    bool inserted = ret.emplace(partition.name, std::move(partition)).second;
    CF_EXPECTF(!!inserted, "Duplicate partition '{}'", vendor_partition);
  }

  return ret;
}

Result<std::map<std::string_view, AndroidBuildPartition>> AbInfoAsPartitions(
    AndroidBuild& build) {
  std::map<std::string_view, AndroidBuildPartition> ret;
  for (std::string partition_name : CF_EXPECT(build.AbPartitions())) {
    AndroidBuildPartition partition(std::move(partition_name));
    CF_EXPECT(partition.MergeIn(PartitionAbStatus::kUsesAb));

    ret.emplace(partition.name, std::move(partition));
  }
  return ret;
}

}  // namespace

Result<std::map<std::string_view, AndroidBuildPartition>> IdentifyPartitions(
    AndroidBuild& build, std::string_view extract_dir) {
  std::map<std::string_view, AndroidBuildPartition> partitions =
      CF_EXPECT(ImagesAsPartitions(build));

  std::map<std::string_view, AndroidBuildPartition> logical_from_super;
  if (std::unique_ptr<android::fs_mgr::LpMetadata> super_metadata =
          CF_EXPECT(SuperImageMetadata(build, extract_dir));
      super_metadata.get()) {
    logical_from_super =
        CF_EXPECT(LogicalPartitionsFromSuperMetadata(build, *super_metadata));
  }

  std::map<std::string_view, AndroidBuildPartition> logical_from_misc;
  if (Result<std::map<std::string, std::string>> misc_info = build.MiscInfo();
      misc_info.ok()) {
    logical_from_misc =
        CF_EXPECT(LogicalPartitionsFromMiscInfo(misc_info.value()));
  }

  std::map<std::string_view, AndroidBuildPartition> logical_partitions =
      CF_EXPECT(MergePartitionsStrict(std::move(logical_from_super),
                                      std::move(logical_from_misc)));

  partitions =
      CF_EXPECT(MergePartitions(std::move(partitions), logical_partitions));

  if (Result<std::map<std::string_view, AndroidBuildPartition>> ab_partitions =
          AbInfoAsPartitions(build);
      ab_partitions.ok()) {
    partitions = CF_EXPECT(
        MergePartitions(std::move(partitions), std::move(*ab_partitions)));
  }

  return partitions;
}

}  // namespace cuttlefish
