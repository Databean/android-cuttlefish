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

#include "cuttlefish/host/commands/cvd/fetch/tracing_build_api.h"

#include <string>
#include <utility>

#include "fmt/core.h"

#include "cuttlefish/common/libs/utils/files.h"
#include "cuttlefish/common/libs/utils/result.h"
#include "cuttlefish/host/commands/cvd/fetch/fetch_tracer.h"
#include "cuttlefish/host/libs/web/android_build.h"
#include "cuttlefish/host/libs/web/android_build_string.h"
#include "cuttlefish/host/libs/web/build_api.h"
#include "cuttlefish/host/libs/zip/zip_cc.h"

namespace cuttlefish {

TracingBuildApi::TracingBuildApi(BuildApi& inner, FetchTracer::Trace trace)
    : inner_(inner), trace_(std::move(trace)) {}

FetchTracer::Trace TracingBuildApi::Trace() const { return trace_; }

Result<Build> TracingBuildApi::GetBuild(const BuildString& build_string) {
  return inner_.GetBuild(build_string);
}

Result<std::string> TracingBuildApi::DownloadFile(
    const Build& build, const std::string& target_directory,
    const std::string& artifact_name) {
  std::string downloaded_path =
      CF_EXPECT(inner_.DownloadFile(build, target_directory, artifact_name));
  size_t size = FileSize(downloaded_path);
  trace_.CompletePhase("Downloading " + artifact_name, size);
  return downloaded_path;
}

Result<std::string> TracingBuildApi::DownloadFileWithBackup(
    const Build& build, const std::string& target_directory,
    const std::string& artifact_name, const std::string& backup_artifact_name) {
  std::string downloaded_path = CF_EXPECT(inner_.DownloadFileWithBackup(
      build, target_directory, artifact_name, backup_artifact_name));
  size_t size = FileSize(downloaded_path);
  trace_.CompletePhase(fmt::format("Downloading {} (fallback {})",
                                   artifact_name, backup_artifact_name),
                       size);
  return downloaded_path;
}

Result<SeekableZipSource> TracingBuildApi::FileReader(
    const Build& build, const std::string& artifact_name) {
  return inner_.FileReader(build, artifact_name);
};

}  // namespace cuttlefish
