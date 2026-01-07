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

#include "cuttlefish/host/commands/assemble_cvd/disk/boot_image.h"

#include <stddef.h>

#include <string>

#include "cuttlefish/host/libs/config/cuttlefish_config.h"

namespace cuttlefish {

BootImage::BootImage(const CuttlefishConfig::InstanceSpecific& ins) {
  path_ = ins.new_boot_image();
}

std::string BootImage::Name() const { return std::string(kName); }

Result<std::string> BootImage::Path() const { return path_; }

Result<std::string> BootImage::Generate() {
  // TODO: schuffelen - import creation logic
  return path_;
}

}  // namespace cuttlefish
