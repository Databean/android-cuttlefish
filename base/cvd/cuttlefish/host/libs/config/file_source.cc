/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "cuttlefish/host/libs/config/file_source.h"

#include <cctype>
#include <string>

namespace cuttlefish {

FileSource SourceStringToEnum(std::string source) {
  for (auto& c : source) {
    c = std::tolower(c);
  }
  if (source == "default_build") {
    return FileSource::DEFAULT_BUILD;
  } else if (source == "system_build") {
    return FileSource::SYSTEM_BUILD;
  } else if (source == "kernel_build") {
    return FileSource::KERNEL_BUILD;
  } else if (source == "local_file") {
    return FileSource::LOCAL_FILE;
  } else if (source == "generated") {
    return FileSource::GENERATED;
  } else if (source == "bootloader_build") {
    return FileSource::BOOTLOADER_BUILD;
  } else if (source == "android_efi_loader_build") {
    return FileSource::ANDROID_EFI_LOADER_BUILD;
  } else if (source == "boot_build") {
    return FileSource::BOOT_BUILD;
  } else if (source == "host_package_build") {
    return FileSource::HOST_PACKAGE_BUILD;
  } else if (source == "chrome_os_build") {
    return FileSource::CHROME_OS_BUILD;
  } else {
    return FileSource::UNKNOWN_PURPOSE;
  }
}

std::string SourceEnumToString(FileSource source) {
  switch (source) {
    case FileSource::DEFAULT_BUILD:
      return "default_build";
    case FileSource::SYSTEM_BUILD:
      return "system_build";
    case FileSource::KERNEL_BUILD:
      return "kernel_build";
    case FileSource::LOCAL_FILE:
      return "local_file";
    case FileSource::GENERATED:
      return "generated";
    case FileSource::BOOTLOADER_BUILD:
      return "bootloader_build";
    case FileSource::ANDROID_EFI_LOADER_BUILD:
      return "android_efi_loader_build";
    case FileSource::BOOT_BUILD:
      return "boot_build";
    case FileSource::HOST_PACKAGE_BUILD:
      return "host_package_build";
    case FileSource::CHROME_OS_BUILD:
      return "chrome_os_build";
    default:
      return "unknown";
  }
}

}  // namespace cuttlefish
