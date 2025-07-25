/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include <map>
#include <memory>

#include "cuttlefish/common/libs/fs/shared_fd.h"
#include "cuttlefish/host/frontend/webrtc/display_handler.h"
#include "cuttlefish/host/frontend/webrtc/kernel_log_events_handler.h"
#include "cuttlefish/host/frontend/webrtc/libdevice/camera_controller.h"
#include "cuttlefish/host/frontend/webrtc/libdevice/connection_observer.h"
#include "cuttlefish/host/frontend/webrtc/libdevice/lights_observer.h"
#include "cuttlefish/host/frontend/webrtc/sensors_handler.h"
#include "cuttlefish/host/libs/confui/host_virtual_input.h"
#include "cuttlefish/host/libs/input_connector/input_connector.h"

namespace cuttlefish {

class CfConnectionObserverFactory
    : public webrtc_streaming::ConnectionObserverFactory {
 public:
  CfConnectionObserverFactory(
      InputConnector& input_connector,
      KernelLogEventsHandler& kernel_log_events_handler,
      webrtc_streaming::SensorsHandler& sensors_handler,
      std::shared_ptr<webrtc_streaming::LightsObserver> lights_observer);
  ~CfConnectionObserverFactory() override = default;

  std::shared_ptr<webrtc_streaming::ConnectionObserver> CreateObserver()
      override;

  void AddCustomActionServer(SharedFD custom_action_server_fd,
                             const std::vector<std::string>& commands);

  void SetDisplayHandler(std::weak_ptr<DisplayHandler> display_handler);

  void SetCameraHandler(CameraController* controller);

 private:
  InputConnector& input_connector_;
  KernelLogEventsHandler& kernel_log_events_handler_;
  std::map<std::string, SharedFD>
      commands_to_custom_action_servers_;
  std::weak_ptr<DisplayHandler> weak_display_handler_;
  cuttlefish::CameraController* camera_controller_ = nullptr;
  webrtc_streaming::SensorsHandler& sensors_handler_;
  std::shared_ptr<webrtc_streaming::LightsObserver> lights_observer_;
};

}  // namespace cuttlefish
