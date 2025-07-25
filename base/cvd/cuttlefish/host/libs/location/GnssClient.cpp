/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include "cuttlefish/host/libs/location/GnssClient.h"

#include <memory>

#include <android-base/logging.h>
#include <grpcpp/channel.h>
#include <grpcpp/support/status.h>

#include "cuttlefish/common/libs/utils/result.h"

using gnss_grpc_proxy::GnssGrpcProxy;
using gnss_grpc_proxy::GpsCoordinates;
using gnss_grpc_proxy::SendGpsCoordinatesReply;
using gnss_grpc_proxy::SendGpsCoordinatesRequest;
using grpc::ClientContext;

namespace cuttlefish {

GnssClient::GnssClient(const std::shared_ptr<grpc::Channel>& channel)
    : stub_(GnssGrpcProxy::NewStub(channel)) {}

Result<void> GnssClient::SendGpsLocations(int delay,
                                          const GpsFixArray& coordinates) {
  // Data we are sending to the server.
  SendGpsCoordinatesRequest request;
  request.set_delay(delay);
  for (const auto& loc : coordinates) {
    GpsCoordinates* curr = request.add_coordinates();
    curr->set_longitude(loc.longitude);
    curr->set_latitude(loc.latitude);
    curr->set_elevation(loc.elevation);
  }

  // Container for the data we expect from the server.
  SendGpsCoordinatesReply reply;
  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  ClientContext context;
  // The actual RPC.
  grpc::Status status = stub_->SendGpsVector(&context, request, &reply);
  // Act upon its status.
  CF_EXPECTF(status.ok(), "GPS data sending failed: {} ({})",
             status.error_message(),
             static_cast<std::uint32_t>(status.error_code()));

  LOG(DEBUG) << reply.status();

  return {};
}

}  // namespace cuttlefish
