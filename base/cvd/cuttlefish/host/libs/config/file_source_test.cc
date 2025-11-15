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
#include "cuttlefish/host/libs/config/file_source.h"

#include <gtest/gtest.h>

namespace cuttlefish {
namespace {

TEST(FileSource, StringConversion) {
  EXPECT_EQ(FileSource::DEFAULT_BUILD,
            SourceStringToEnum(SourceEnumToString(FileSource::DEFAULT_BUILD)));
  EXPECT_EQ(FileSource::SYSTEM_BUILD,
            SourceStringToEnum(SourceEnumToString(FileSource::SYSTEM_BUILD)));
  EXPECT_EQ(FileSource::KERNEL_BUILD,
            SourceStringToEnum(SourceEnumToString(FileSource::KERNEL_BUILD)));
  EXPECT_EQ(FileSource::LOCAL_FILE,
            SourceStringToEnum(SourceEnumToString(FileSource::LOCAL_FILE)));
  EXPECT_EQ(FileSource::GENERATED,
            SourceStringToEnum(SourceEnumToString(FileSource::GENERATED)));
  EXPECT_EQ(
      FileSource::BOOTLOADER_BUILD,
      SourceStringToEnum(SourceEnumToString(FileSource::BOOTLOADER_BUILD)));
  EXPECT_EQ(FileSource::ANDROID_EFI_LOADER_BUILD,
            SourceStringToEnum(
                SourceEnumToString(FileSource::ANDROID_EFI_LOADER_BUILD)));
  EXPECT_EQ(FileSource::BOOT_BUILD,
            SourceStringToEnum(SourceEnumToString(FileSource::BOOT_BUILD)));
  EXPECT_EQ(
      FileSource::HOST_PACKAGE_BUILD,
      SourceStringToEnum(SourceEnumToString(FileSource::HOST_PACKAGE_BUILD)));
  EXPECT_EQ(
      FileSource::CHROME_OS_BUILD,
      SourceStringToEnum(SourceEnumToString(FileSource::CHROME_OS_BUILD)));
  EXPECT_EQ(
      FileSource::UNKNOWN_PURPOSE,
      SourceStringToEnum(SourceEnumToString(FileSource::UNKNOWN_PURPOSE)));
}

}  // namespace
}  // namespace cuttlefish
