//
// Copyright (C) 2026 The Android Open Source Project
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

#include "cuttlefish/host/libs/zip/libzip_cc/archive.h"

#include <string>
#include <utility>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cuttlefish/host/libs/zip/libzip_cc/writable_source.h"
#include "cuttlefish/host/libs/zip/zip_string.h"
#include "cuttlefish/result/result.h"
#include "cuttlefish/result/result_matchers.h"

namespace cuttlefish {
namespace {

TEST(ReadableZipTest, ListDirectory) {
  std::string data(4096, '\0');
  auto source_result = WritableZipSource::BorrowData(data.data(), data.size());
  ASSERT_THAT(source_result, IsOk());
  WritableZipSource source = std::move(*source_result);

  auto zip_result = WritableZip::FromSource(std::move(source));
  ASSERT_THAT(zip_result, IsOk());
  WritableZip zip = std::move(*zip_result);

  ASSERT_THAT(AddStringAt(zip, "abc", "dir/file1"), IsOk());
  ASSERT_THAT(AddStringAt(zip, "def", "dir/file2"), IsOk());

  auto source_result2 = WritableZipSource::FromZip(std::move(zip));
  ASSERT_THAT(source_result2, IsOk());
  source = std::move(*source_result2);

  auto readable_zip_result = ReadableZip::FromSource(std::move(source));
  ASSERT_THAT(readable_zip_result, IsOk());
  ReadableZip readable_zip = std::move(*readable_zip_result);

  Result<std::vector<std::string>> entries = readable_zip.ListDirectory("dir");
  ASSERT_THAT(entries, IsOk());
  EXPECT_THAT(*entries, testing::UnorderedElementsAre("file1", "file2"));
}

}  // namespace
}  // namespace cuttlefish
