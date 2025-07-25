//
// Copyright (C) 2020 The Android Open Source Project
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

#include "tee_logging.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cinttypes>
#include <cstring>
#include <ctime>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <android-base/logging.h>
#include <android-base/macros.h>
#include <android-base/parseint.h>
#include <android-base/stringprintf.h>
#include <android-base/strings.h>
#include <android-base/threads.h>

#include "cuttlefish/common/libs/fs/shared_buf.h"
#include "cuttlefish/common/libs/utils/contains.h"
#include "cuttlefish/common/libs/utils/environment.h"
#include "cuttlefish/common/libs/utils/result.h"

using android::base::FATAL;
using android::base::GetThreadId;
using android::base::LogSeverity;
using android::base::StringPrintf;

namespace cuttlefish {
namespace {

std::string ToUpper(const std::string& input) {
  std::string output = input;
  std::transform(output.begin(), output.end(), output.begin(),
                 [](unsigned char ch) { return std::toupper(ch); });
  return output;
}

}  // namespace

std::string FromSeverity(const android::base::LogSeverity severity) {
  switch (severity) {
    case android::base::VERBOSE:
      return "VERBOSE";
    case android::base::DEBUG:
      return "DEBUG";
    case android::base::INFO:
      return "INFO";
    case android::base::WARNING:
      return "WARNING";
    case android::base::ERROR:
      return "ERROR";
    case android::base::FATAL_WITHOUT_ABORT:
      return "FATAL_WITHOUT_ABORT";
    case android::base::FATAL:
      return "FATAL";
  }
  return "Unexpected severity";
}

Result<LogSeverity> ToSeverity(const std::string& value) {
  const std::unordered_map<std::string, android::base::LogSeverity>
      string_to_severity{
          {"VERBOSE", android::base::VERBOSE},
          {"DEBUG", android::base::DEBUG},
          {"INFO", android::base::INFO},
          {"WARNING", android::base::WARNING},
          {"ERROR", android::base::ERROR},
          {"FATAL_WITHOUT_ABORT", android::base::FATAL_WITHOUT_ABORT},
          {"FATAL", android::base::FATAL},
      };

  const auto upper_value = ToUpper(value);
  if (Contains(string_to_severity, upper_value)) {
    return string_to_severity.at(value);
  } else {
    int value_int;
    CF_EXPECT(android::base::ParseInt(value, &value_int),
              "Unable to determine severity from \"" << value << "\"");
    const auto iter = std::find_if(
        string_to_severity.begin(), string_to_severity.end(),
        [&value_int](
            const std::pair<std::string, android::base::LogSeverity>& entry) {
          return static_cast<int>(entry.second) == value_int;
        });
    CF_EXPECT(iter != string_to_severity.end(),
              "Unable to determine severity from \"" << value << "\"");
    return iter->second;
  }
}

static LogSeverity GuessSeverity(const std::string& env_var,
                                 LogSeverity default_value) {
  std::string env_value = StringFromEnv(env_var, "");
  auto severity_result = ToSeverity(env_value);
  if (!severity_result.ok()) {
    return default_value;
  }
  return severity_result.value();
}

LogSeverity ConsoleSeverity() {
  return GuessSeverity("CF_CONSOLE_SEVERITY", android::base::INFO);
}

LogSeverity LogFileSeverity() {
  return GuessSeverity("CF_FILE_SEVERITY", android::base::DEBUG);
}

TeeLogger::TeeLogger(const std::vector<SeverityTarget>& destinations,
                     const std::string& prefix)
    : destinations_(destinations), prefix_(prefix) {}

ScopedTeeLogger::ScopedTeeLogger(TeeLogger tee_logger)
    // Set the android logger to full verbosity, the tee_logger will choose
    // whether to write each line.
    : scoped_severity_(android::base::VERBOSE) {
  old_logger_ = android::base::SetLogger(tee_logger);
}

ScopedTeeLogger::~ScopedTeeLogger() {
  // restore the previous logger
  android::base::SetLogger(std::move(old_logger_));
}

// Copied from system/libbase/logging_splitters.h
static std::pair<int, int> CountSizeAndNewLines(const char* message) {
  int size = 0;
  int new_lines = 0;
  while (*message != '\0') {
    size++;
    if (*message == '\n') {
      ++new_lines;
    }
    ++message;
  }
  return {size, new_lines};
}

// Copied from system/libbase/logging_splitters.h
// This splits the message up line by line, by calling log_function with a
// pointer to the start of each line and the size up to the newline character.
// It sends size = -1 for the final line.
template <typename F, typename... Args>
static void SplitByLines(const char* msg, const F& log_function,
                         Args&&... args) {
  const char* newline = strchr(msg, '\n');
  while (newline != nullptr) {
    log_function(msg, newline - msg, args...);
    msg = newline + 1;
    newline = strchr(msg, '\n');
  }

  log_function(msg, -1, args...);
}

// Copied from system/libbase/logging_splitters.h
// This adds the log header to each line of message and returns it as a string
// intended to be written to stderr.
std::string StderrOutputGenerator(const struct tm& now, int pid, uint64_t tid,
                                  LogSeverity severity, const char* tag,
                                  const char* file, unsigned int line,
                                  const char* message) {
  char timestamp[32];
  strftime(timestamp, sizeof(timestamp), "%m-%d %H:%M:%S", &now);

  static const char log_characters[] = "VDIWEFF";
  static_assert(arraysize(log_characters) - 1 == FATAL + 1,
                "Mismatch in size of log_characters and values in LogSeverity");
  char severity_char = log_characters[severity];
  std::string line_prefix;
  if (file != nullptr) {
    line_prefix =
        StringPrintf("%s %c %s %5d %5" PRIu64 " %s:%u] ", tag ? tag : "nullptr",
                     severity_char, timestamp, pid, tid, file, line);
  } else {
    line_prefix =
        StringPrintf("%s %c %s %5d %5" PRIu64 " ", tag ? tag : "nullptr",
                     severity_char, timestamp, pid, tid);
  }

  auto [size, new_lines] = CountSizeAndNewLines(message);
  std::string output_string;
  output_string.reserve(size + new_lines * line_prefix.size() + 1);

  auto concat_lines = [&](const char* message, int size) {
    output_string.append(line_prefix);
    if (size == -1) {
      output_string.append(message);
    } else {
      output_string.append(message, size);
    }
    output_string.append("\n");
  };
  SplitByLines(message, concat_lines);
  return output_string;
}

// TODO(schuffelen): Do something less primitive.
std::string StripColorCodes(const std::string& str) {
  std::stringstream sstream;
  bool in_color_code = false;
  for (char c : str) {
    if (c == '\033') {
      in_color_code = true;
    }
    if (!in_color_code) {
      sstream << c;
    }
    if (c == 'm') {
      in_color_code = false;
    }
  }
  return sstream.str();
}

void TeeLogger::operator()(android::base::LogId,
                           android::base::LogSeverity severity, const char* tag,
                           const char* file, unsigned int line,
                           const char* message) {
  for (const auto& destination : destinations_) {
    std::string msg_with_prefix = prefix_ + message;
    std::string output_string;
    switch (destination.metadata_level) {
      case MetadataLevel::ONLY_MESSAGE:
        output_string = msg_with_prefix + std::string("\n");
        break;
      case MetadataLevel::TAG_AND_MESSAGE:
        output_string = fmt::format("{}] {}{}", tag, msg_with_prefix, "\n");
        break;
      default:
        struct tm now;
        time_t t = time(nullptr);
        localtime_r(&t, &now);
        output_string =
            StderrOutputGenerator(now, getpid(), GetThreadId(), severity, tag,
                                  file, line, msg_with_prefix.c_str());
        break;
    }
    if (severity >= destination.severity) {
      if (destination.target->IsATTY()) {
        WriteAll(destination.target, output_string);
      } else {
        WriteAll(destination.target, StripColorCodes(output_string));
      }
    }
  }
}

static std::vector<SeverityTarget> SeverityTargetsForFiles(
    const std::vector<std::string>& files) {
  std::vector<SeverityTarget> log_severities;
  for (const auto& file : files) {
    auto log_file_fd =
        SharedFD::Open(file, O_CREAT | O_WRONLY | O_APPEND,
                       S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (!log_file_fd->IsOpen()) {
      LOG(FATAL) << "Failed to create log file: " << log_file_fd->StrError();
    }
    log_severities.push_back(
        SeverityTarget{LogFileSeverity(), log_file_fd, MetadataLevel::FULL});
  }
  return log_severities;
}

TeeLogger LogToStderr(
    const std::string& log_prefix, MetadataLevel stderr_level,
    std::optional<android::base::LogSeverity> stderr_severity) {
  std::vector<SeverityTarget> log_severities{
      SeverityTarget{stderr_severity ? *stderr_severity : ConsoleSeverity(),
                     SharedFD::Dup(/* stderr */ 2), stderr_level}};
  return TeeLogger(log_severities, log_prefix);
}

TeeLogger LogToFiles(const std::vector<std::string>& files,
                     const std::string& log_prefix) {
  return TeeLogger(SeverityTargetsForFiles(files), log_prefix);
}

TeeLogger LogToStderrAndFiles(
    const std::vector<std::string>& files, const std::string& log_prefix,
    MetadataLevel stderr_level,
    std::optional<android::base::LogSeverity> stderr_severity) {
  std::vector<SeverityTarget> log_severities = SeverityTargetsForFiles(files);
  log_severities.push_back(
      SeverityTarget{stderr_severity ? *stderr_severity : ConsoleSeverity(),
                     SharedFD::Dup(/* stderr */ 2), stderr_level});
  return TeeLogger(log_severities, log_prefix);
}

}  // namespace cuttlefish
