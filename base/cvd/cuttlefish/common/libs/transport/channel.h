/*
 * Copyright 2023 The Android Open Source Project
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

#pragma once

#include <memory>

#include "cuttlefish/common/libs/utils/result.h"

namespace cuttlefish {
namespace transport {

/**
 * RawMessage - Header and raw byte payload for a serialized
 * secure env message.
 *
 * @command: the command.
 * @is_response: flag to mark message as a request/response.
 * @payload_size: amount of payload data we're going to transfer.
 * @payload: start of the serialized command specific payload.
 */
struct RawMessage {
  uint32_t command : 31;
  bool is_response : 1;
  uint32_t payload_size;
  uint8_t payload[0];
};

/**
 * A destroyer for RawMessage instances created with
 * CreateMessage. Wipes memory from the RawMessage
 * instances.
 */
class MessageDestroyer {
 public:
  void operator()(RawMessage* ptr);
};

/** An owning pointer for a RawMessage instance. */
using ManagedMessage = std::unique_ptr<RawMessage, MessageDestroyer>;

/**
 * Allocates memory for a RawMessage carrying a message of size
 * `payload_size`.
 */
Result<ManagedMessage> CreateMessage(uint32_t command, bool is_response,
                                     size_t payload_size);
Result<ManagedMessage> CreateMessage(uint32_t command, size_t payload_size);

/*
 * Interface for communication channels that synchronously communicate
 * HAL IPC/RPC calls.
 */
class Channel {
 public:
  virtual Result<void> SendRequest(RawMessage& message) = 0;
  virtual Result<void> SendResponse(RawMessage& message) = 0;
  virtual Result<ManagedMessage> ReceiveMessage() = 0;
  virtual Result<int> WaitForMessage() = 0;
  virtual ~Channel() {}
};

}  // namespace transport
}  // namespace cuttlefish