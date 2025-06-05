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

#include <memory>

#include "cuttlefish/common/libs/utils/result.h"
#include "cuttlefish/host/commands/cvd/fetch/fetch_cvd_parser.h"
#include "cuttlefish/host/libs/web/build_api.h"
#include "cuttlefish/host/libs/web/luci_build_api.h"

namespace cuttlefish {

class Downloaders {
 public:
  static Result<Downloaders> Create(const BuildApiFlags&);

  Downloaders(Downloaders&&);
  ~Downloaders();

  BuildApi& AndroidBuild();
  LuciBuildApi& Luci();

 private:
  struct Impl;  // for pimpl

  Downloaders(std::unique_ptr<Impl>);

  std::unique_ptr<Impl> impl_;
};

}  // namespace cuttlefish
