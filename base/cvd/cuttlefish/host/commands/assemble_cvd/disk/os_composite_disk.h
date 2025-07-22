/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <optional>

#include "cuttlefish/host/commands/assemble_cvd/disk/chromeos_state.h"
#include "cuttlefish/host/commands/assemble_cvd/disk/metadata_image.h"
#include "cuttlefish/host/commands/assemble_cvd/disk/misc_image.h"
#include "cuttlefish/host/commands/assemble_cvd/disk_builder.h"
#include "cuttlefish/host/commands/assemble_cvd/flags/system_image_dir.h"
#include "cuttlefish/host/libs/config/cuttlefish_config.h"

namespace cuttlefish {

DiskBuilder OsCompositeDiskBuilder(
    const CuttlefishConfig& config,
    const CuttlefishConfig::InstanceSpecific& instance,
    const std::optional<ChromeOsStateImage>&, const MetadataImage&,
    const MiscImage&, const SystemImageDirFlag&);

}  // namespace cuttlefish
