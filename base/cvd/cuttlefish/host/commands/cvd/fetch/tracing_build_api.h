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

#include <string>

#include "cuttlefish/common/libs/utils/result.h"
#include "cuttlefish/host/commands/cvd/fetch/fetch_tracer.h"
#include "cuttlefish/host/libs/web/android_build.h"
#include "cuttlefish/host/libs/web/android_build_string.h"
#include "cuttlefish/host/libs/web/build_api.h"
#include "cuttlefish/host/libs/zip/zip_cc.h"

namespace cuttlefish {

class TracingBuildApi : public BuildApi {
 public:
  TracingBuildApi(BuildApi& inner, FetchTracer::Trace trace);

  FetchTracer::Trace Trace() const;

  Result<Build> GetBuild(const BuildString& build_string) override;

  Result<std::string> DownloadFile(const Build& build,
                                   const std::string& target_directory,
                                   const std::string& artifact_name) override;

  Result<std::string> DownloadFileWithBackup(
      const Build& build, const std::string& target_directory,
      const std::string& artifact_name,
      const std::string& backup_artifact_name) override;

  Result<SeekableZipSource> FileReader(
      const Build&, const std::string& artifact_name) override;

 private:
  BuildApi& inner_;
  FetchTracer::Trace trace_;
};

}  // namespace cuttlefish
