//
// Copyright (C) 2023 The Android Open Source Project
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

#include "cuttlefish/host/commands/run_cvd/launch/control_env_proxy_server.h"

#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <fruit/component.h>
#include <fruit/fruit_forward_decls.h>
#include <fruit/macro.h>

#include "cuttlefish/common/libs/utils/result.h"
#include "cuttlefish/common/libs/utils/subprocess.h"
#include "cuttlefish/host/commands/run_cvd/launch/grpc_socket_creator.h"
#include "cuttlefish/host/libs/config/cuttlefish_config.h"
#include "cuttlefish/host/libs/config/known_paths.h"
#include "cuttlefish/host/libs/feature/command_source.h"
#include "cuttlefish/host/libs/feature/feature.h"

namespace cuttlefish {
namespace {

class ControlEnvProxyServer : public CommandSource {
 public:
  INJECT(
      ControlEnvProxyServer(const CuttlefishConfig::InstanceSpecific& instance,
                            GrpcSocketCreator& grpc_socket))
      : instance_(instance), grpc_socket_(grpc_socket) {}

  // CommandSource
  Result<std::vector<MonitorCommand>> Commands() override {
    Command command(ControlEnvProxyServerBinary());
    command.AddParameter("--grpc_uds_path=",
                         grpc_socket_.CreateGrpcSocket(Name()));
    command.AddParameter("--grpc_socket_path=", instance_.grpc_socket_path());
    std::vector<MonitorCommand> commands;
    commands.emplace_back(std::move(command));
    return commands;
  }

  // SetupFeature
  std::string Name() const override { return "ControlEnvProxyServer"; }

 private:
  std::unordered_set<SetupFeature*> Dependencies() const override { return {}; }
  Result<void> ResultSetup() override { return {}; }

  const CuttlefishConfig::InstanceSpecific& instance_;
  GrpcSocketCreator& grpc_socket_;
};

}  // namespace

fruit::Component<fruit::Required<const CuttlefishConfig::InstanceSpecific,
                                 GrpcSocketCreator>>
ControlEnvProxyServerComponent() {
  return fruit::createComponent()
      .addMultibinding<CommandSource, ControlEnvProxyServer>()
      .addMultibinding<SetupFeature, ControlEnvProxyServer>();
}

}  // namespace cuttlefish
