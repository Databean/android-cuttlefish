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

#include "uuid.h"

#include <internal_user_log.pb.h>

#include "cuttlefish/common/libs/utils/files.h"
#include "cuttlefish/common/libs/utils/flag_parser.h"
#include "cuttlefish/host/commands/cvd/metrics/cvd_metrics_api.h"
#include "cuttlefish/host/commands/cvd/metrics/utils.h"
#include "cuttlefish/host/commands/metrics/metrics_defs.h"

namespace cuttlefish {

namespace {

// 971 for atest internal events, while 934 for external events
constexpr char kToolName[] = "cvd";

constexpr char kLogSourceStr[] = "CUTTLEFISH_METRICS";
constexpr int kCppClientType =
    19;  // C++ native client type (clientanalytics.proto)

const std::string kInternalEmail = "@google.com";
const std::vector<std::string> kInternalHostname = {"google"};

std::string GenerateUUID() {
  uuid_t uuid;
  uuid_generate_random(uuid);
  std::string uuid_str = "xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx";
  uuid_unparse(uuid, uuid_str.data());
  return uuid_str;
}

std::unique_ptr<AtestLogEventInternal> BuildAtestLogEvent(
    const std::string& command_line) {
  std::unique_ptr<AtestLogEventInternal> event =
      std::make_unique<AtestLogEventInternal>();

  //  Set common fields
  std::string user_key = GenerateUUID();
  std::string run_id = GenerateUUID();
  std::string os_name = metrics::GetOsName();
  std::string dir = CurrentDirectory();
  event->set_user_key(user_key);
  event->set_run_id(run_id);
  event->set_tool_name(kToolName);
  event->set_user_type(UserType::GOOGLE);

  // Create and populate AtestStartEvent
  AtestLogEventInternal::AtestStartEvent* start_event =
      event->mutable_atest_start_event();
  start_event->set_command_line(command_line);
  start_event->set_cwd(dir);
  start_event->set_os(os_name);

  return event;
}

std::unique_ptr<LogRequest> BuildAtestLogRequest(
    uint64_t now_ms, AtestLogEventInternal* cfEvent) {
  // "log_request" is the top level LogRequest
  auto log_request = std::make_unique<LogRequest>();
  log_request->set_request_time_ms(now_ms);
  log_request->set_log_source_name(kLogSourceStr);

  ClientInfo* client_info = log_request->mutable_client_info();
  client_info->set_client_type(kCppClientType);

  std::string atest_log_event;
  if (!cfEvent->SerializeToString(&atest_log_event)) {
    LOG(ERROR) << "Serialization failed for atest event";
    return nullptr;
  }

  LogEvent* logEvent = log_request->add_log_event();
  logEvent->set_event_time_ms(now_ms);
  logEvent->set_source_extension(atest_log_event);

  return log_request;
}

std::string createCommandLine(const std::vector<std::string>& args) {
  std::string commandLine;
  for (const auto& arg : args) {
    commandLine += arg + " ";
  }
  // Remove the trailing space
  if (!commandLine.empty()) {
    commandLine.pop_back();
  }
  return commandLine;
}

std::string GetUserEmail() {
  std::string email;
  FILE* pipe = popen("git config --get user.email 2>/dev/null", "r");
  if (!pipe) {
    LOG(ERROR) << "popen() failed!";
    return "";
  }
  char buffer[128];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    email += buffer;
  }
  pclose(pipe);
  email.erase(std::remove(email.begin(), email.end(), '\n'), email.end());
  return email;
}

UserType GetUserType() {
  std::string email = GetUserEmail();
  if (email.find(kInternalEmail) != std::string::npos) {
    return UserType::GOOGLE;
  }

  char hostname[1024];
  hostname[1023] = '\0';
  gethostname(hostname, sizeof(hostname) - 1);
  std::string host_str(hostname);

  for (const auto& internal_host : kInternalHostname) {
    if (host_str.find(internal_host) != std::string::npos) {
      return UserType::GOOGLE;
    }
  }

  return UserType::EXTERNAL;
}

}  // namespace

int CvdMetrics::SendLaunchCommand(const std::string& command_line) {
  uint64_t now_ms = metrics::GetEpochTimeMs();
  auto cfEvent = BuildAtestLogEvent(command_line);

  auto logRequest = BuildAtestLogRequest(now_ms, cfEvent.get());
  if (!logRequest) {
    LOG(ERROR) << "Failed to build atest LogRequest";
    return MetricsExitCodes::kMetricsError;
  }

  std::string logRequestStr;
  if (!logRequest->SerializeToString(&logRequestStr)) {
    LOG(ERROR) << "Serialization failed for atest LogRequest";
    return MetricsExitCodes::kMetricsError;
  }
  return metrics::PostRequest(logRequestStr, metrics::kProd);
}

int CvdMetrics::SendCvdMetrics(const std::vector<std::string>& args) {
  if (GetUserType() != UserType::GOOGLE) {
    return 0;
  }
  std::string command_line = createCommandLine(args);
  return CvdMetrics::SendLaunchCommand(command_line);
}

}  // namespace cuttlefish
