//
// Copyright (C) 2019 The Android Open Source Project
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

#include "cuttlefish/host/commands/run_cvd/launch/kernel_log_monitor.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>

#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <android-base/logging.h>
#include <fruit/component.h>
#include <fruit/fruit_forward_decls.h>
#include <fruit/injector.h>
#include <fruit/macro.h>

#include "cuttlefish/common/libs/utils/result.h"
#include "cuttlefish/common/libs/utils/subprocess.h"
#include "cuttlefish/host/commands/run_cvd/reporting.h"
#include "cuttlefish/host/libs/config/cuttlefish_config.h"
#include "cuttlefish/host/libs/config/known_paths.h"
#include "cuttlefish/host/libs/feature/command_source.h"
#include "cuttlefish/host/libs/feature/feature.h"
#include "cuttlefish/host/libs/feature/inject.h"
#include "cuttlefish/host/libs/feature/kernel_log_pipe_provider.h"

namespace cuttlefish {
namespace {

class KernelLogMonitor : public CommandSource,
                         public KernelLogPipeProvider,
                         public DiagnosticInformation,
                         public LateInjected {
 public:
  INJECT(KernelLogMonitor(const CuttlefishConfig::InstanceSpecific& instance))
      : instance_(instance) {}

  // DiagnosticInformation
  std::vector<std::string> Diagnostics() const override {
    return {"Kernel log: " + instance_.PerInstancePath("kernel.log")};
  }

  Result<void> LateInject(fruit::Injector<>& injector) override {
    number_of_event_pipes_ =
        injector.getMultibindings<KernelLogPipeConsumer>().size();
    return {};
  }

  // CommandSource
  Result<std::vector<MonitorCommand>> Commands() override {
    Command command(KernelLogMonitorBinary());
    command.AddParameter("-log_pipe_fd=", fifo_);

    if (!event_pipe_write_ends_.empty()) {
      command.AddParameter("-subscriber_fds=");
      for (size_t i = 0; i < event_pipe_write_ends_.size(); i++) {
        if (i > 0) {
          command.AppendToLastParameter(",");
        }
        command.AppendToLastParameter(event_pipe_write_ends_[i]);
      }
    }
    std::vector<MonitorCommand> commands;
    commands.emplace_back(std::move(command));
    return commands;
  }

  // KernelLogPipeProvider
  SharedFD KernelLogPipe() override {
    CHECK(!event_pipe_read_ends_.empty()) << "No more kernel pipes left. Make sure you inherited "
                                             "KernelLogPipeProvider and provided multibinding "
                                             "from KernelLogPipeConsumer to your type.";
    SharedFD ret = event_pipe_read_ends_.back();
    event_pipe_read_ends_.pop_back();
    return ret;
  }

 private:
  // SetupFeature
  std::string Name() const override { return "KernelLogMonitor"; }

 private:
  std::unordered_set<SetupFeature*> Dependencies() const override { return {}; }
  Result<void> ResultSetup() override {
    auto log_name = instance_.kernel_log_pipe_name();
    // Open the pipe here (from the launcher) to ensure the pipe is not deleted
    // due to the usage counters in the kernel reaching zero. If this is not
    // done and the kernel_log_monitor crashes for some reason the VMM may get
    // SIGPIPE.
    fifo_ = CF_EXPECT(SharedFD::Fifo(log_name, 0600));

    for (unsigned int i = 0; i < number_of_event_pipes_; ++i) {
      SharedFD event_pipe_write_end, event_pipe_read_end;
      CF_EXPECT(SharedFD::Pipe(&event_pipe_read_end, &event_pipe_write_end),
                "Failed creating kernel log pipe: " << strerror(errno));
      event_pipe_write_ends_.push_back(event_pipe_write_end);
      event_pipe_read_ends_.push_back(event_pipe_read_end);
    }
    return {};
  }

  int number_of_event_pipes_ = 0;
  const CuttlefishConfig::InstanceSpecific& instance_;
  SharedFD fifo_;
  std::vector<SharedFD> event_pipe_write_ends_;
  std::vector<SharedFD> event_pipe_read_ends_;
};

}  // namespace

fruit::Component<fruit::Required<const CuttlefishConfig::InstanceSpecific>,
                 KernelLogPipeProvider>
KernelLogMonitorComponent() {
  return fruit::createComponent()
      .bind<KernelLogPipeProvider, KernelLogMonitor>()
      .addMultibinding<CommandSource, KernelLogMonitor>()
      .addMultibinding<SetupFeature, KernelLogMonitor>()
      .addMultibinding<DiagnosticInformation, KernelLogMonitor>()
      .addMultibinding<LateInjected, KernelLogMonitor>();
}

}  // namespace cuttlefish
