/*
 * Copyright (C) 2017 The Android Open Source Project
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
#include "cuttlefish/host/commands/assemble_cvd/guest_config.h"

#include <sys/types.h>
#include <unistd.h>

#include <optional>
#include <string>
#include <vector>

#include <android-base/parseint.h>
#include <android-base/strings.h>

#include "cuttlefish/common/libs/key_equals_value/key_equals_value.h"
#include "cuttlefish/common/libs/utils/architecture.h"
#include "cuttlefish/common/libs/utils/environment.h"
#include "cuttlefish/common/libs/utils/files.h"
#include "cuttlefish/common/libs/utils/in_sandbox.h"
#include "cuttlefish/host/commands/assemble_cvd/assemble_cvd_flags.h"
#include "cuttlefish/host/commands/assemble_cvd/boot_image_utils.h"
#include "cuttlefish/host/commands/assemble_cvd/flags/boot_image.h"
#include "cuttlefish/host/commands/assemble_cvd/flags/kernel_path.h"
#include "cuttlefish/host/commands/assemble_cvd/flags/system_image_dir.h"
#include "cuttlefish/host/libs/config/display.h"
#include "cuttlefish/host/libs/config/instance_nums.h"

namespace cuttlefish {
namespace {

std::optional<std::string> MapGetOptional(
    const std::map<std::string, std::string>& map, const std::string& key) {
  std::map<std::string, std::string>::const_iterator it = map.find(key);
  return it == map.end() ? std::nullopt : std::make_optional(it->second);
}

}  // namespace

Result<std::vector<GuestConfig>> ReadGuestConfig(
    const BootImageFlag& boot_image, const KernelPathFlag& kernel_path,
    const SystemImageDirFlag& system_image_dir) {
  std::vector<GuestConfig> guest_configs;
  std::string kernel_image_path = "";
  std::string cur_boot_image;

  std::string current_path = StringFromEnv("PATH", "");
  std::string bin_folder = DefaultHostArtifactsPath("bin");
  std::string new_path = "PATH=";
  new_path += current_path;
  new_path += ":";
  new_path += bin_folder;
  auto instance_nums =
      CF_EXPECT(InstanceNumsCalculator().FromGlobalGflags().Calculate());
  for (int instance_index = 0; instance_index < instance_nums.size();
       instance_index++) {
    // extract-ikconfig can be called directly on the boot image since it looks
    // for the ikconfig header in the image before extracting the config list.
    // This code is liable to break if the boot image ever includes the
    // ikconfig header outside the kernel.
    std::string cur_boot_image = boot_image.BootImageForIndex(instance_index);

    if (!kernel_path.KernelPathForIndex(instance_index).empty()) {
      kernel_image_path = kernel_path.KernelPathForIndex(instance_index);
    } else if (!cur_boot_image.empty()) {
      kernel_image_path = cur_boot_image;
    }

    GuestConfig guest_config;
    guest_config.android_version_number =
        CF_EXPECT(ReadAndroidVersionFromBootImage(cur_boot_image),
                  "Failed to read guest's android version");

    if (InSandbox()) {
      // TODO: b/359309462 - real sandboxing for extract-ikconfig
      guest_config.target_arch = HostArch();
      guest_config.bootconfig_supported = true;
      guest_config.hctr2_supported = true;
    } else {
      Command ikconfig_cmd(HostBinaryPath("extract-ikconfig"));
      ikconfig_cmd.AddParameter(kernel_image_path);
      ikconfig_cmd.UnsetFromEnvironment("PATH").AddEnvironmentVariable(
          "PATH", new_path);
      std::string ikconfig_path = FLAGS_early_tmp_dir + "/ikconfig.XXXXXX";
      auto ikconfig_fd = SharedFD::Mkstemp(&ikconfig_path);
      CF_EXPECT(ikconfig_fd->IsOpen(),
                "Unable to create ikconfig file: " << ikconfig_fd->StrError());
      ikconfig_cmd.RedirectStdIO(Subprocess::StdIOChannel::kStdOut,
                                 ikconfig_fd);

      auto ikconfig_proc = ikconfig_cmd.Start();
      CF_EXPECT(ikconfig_proc.Started() && ikconfig_proc.Wait() == 0,
                "Failed to extract ikconfig from " << kernel_image_path);

      std::string config = ReadFile(ikconfig_path);

      if (config.find("\nCONFIG_ARM=y") != std::string::npos) {
        guest_config.target_arch = Arch::Arm;
      } else if (config.find("\nCONFIG_ARM64=y") != std::string::npos) {
        guest_config.target_arch = Arch::Arm64;
      } else if (config.find("\nCONFIG_ARCH_RV64I=y") != std::string::npos) {
        guest_config.target_arch = Arch::RiscV64;
      } else if (config.find("\nCONFIG_X86_64=y") != std::string::npos) {
        guest_config.target_arch = Arch::X86_64;
      } else if (config.find("\nCONFIG_X86=y") != std::string::npos) {
        guest_config.target_arch = Arch::X86;
      } else {
        return CF_ERR("Unknown target architecture");
      }
      guest_config.bootconfig_supported =
          config.find("\nCONFIG_BOOT_CONFIG=y") != std::string::npos;
      // Once all Cuttlefish kernel versions are at least 5.15, this code can be
      // removed. CONFIG_CRYPTO_HCTR2=y will always be set.
      // Note there's also a platform dep for hctr2 introduced in Android 14.
      // Hence the version check.
      guest_config.hctr2_supported =
          (config.find("\nCONFIG_CRYPTO_HCTR2=y") != std::string::npos) &&
          (guest_config.android_version_number != "11.0.0") &&
          (guest_config.android_version_number != "13.0.0") &&
          (guest_config.android_version_number != "11") &&
          (guest_config.android_version_number != "13");

      unlink(ikconfig_path.c_str());
    }

    std::string instance_android_info_txt =
        system_image_dir.ForIndex(instance_index) + "/android-info.txt";
    CF_EXPECT(FileExists(instance_android_info_txt));

    std::string android_info_contents = ReadFile(instance_android_info_txt);

    std::map<std::string, std::string> android_info_map =
        CF_EXPECT(ParseKeyEqualsValue(android_info_contents));

    // If "device_type" is not explicitly set, fall back to parse "config".
    guest_config.device_type = ParseDeviceType(
        MapGetOptional(android_info_map, "device_type")
            .value_or(MapGetOptional(android_info_map, "config").value_or("")));

    guest_config.gfxstream_supported =
        MapGetOptional(android_info_map, "gfxstream").value_or("") ==
        "supported";

    guest_config.gfxstream_gl_program_binary_link_status_supported =
        MapGetOptional(android_info_map,
                       "gfxstream_gl_program_binary_link_status")
            .value_or("") == "supported";

    guest_config.mouse_supported =
        MapGetOptional(android_info_map, "mouse").value_or("") == "supported";

    guest_config.gamepad_supported =
        MapGetOptional(android_info_map, "gamepad").value_or("") == "supported";

    std::optional<std::string> custom_keyboard_config_opt =
        MapGetOptional(android_info_map, "custom_keyboard");
    if (custom_keyboard_config_opt) {
      guest_config.custom_keyboard_config =
          DefaultHostArtifactsPath(*custom_keyboard_config_opt);
    }

    std::optional<std::string> domkey_mapping_config_opt =
        MapGetOptional(android_info_map, "domkey_mapping");
    if (domkey_mapping_config_opt) {
      guest_config.domkey_mapping_config =
          DefaultHostArtifactsPath(*domkey_mapping_config_opt);
    }

    guest_config.supports_bgra_framebuffers =
        MapGetOptional(android_info_map, "supports_bgra_framebuffers")
            .value_or("") == "true";

    guest_config.vhost_user_vsock =
        MapGetOptional(android_info_map, "vhost_user_vsock").value_or("") ==
        "true";

    guest_config.prefer_drm_virgl_when_supported =
        MapGetOptional(android_info_map, "prefer_drm_virgl_when_supported")
            .value_or("") == "true";

    guest_config.ti50_emulator =
        MapGetOptional(android_info_map, "ti50_emulator").value_or("");

    std::optional<std::string> output_audio_streams_count_opt =
        MapGetOptional(android_info_map, "output_audio_streams_count");
    if (output_audio_streams_count_opt) {
      CF_EXPECTF(
          android::base::ParseInt(*output_audio_streams_count_opt,
                                  &guest_config.output_audio_streams_count),
          "Failed to parse value '{}' for output audio stream count",
          *output_audio_streams_count_opt);
    }

    guest_config.enforce_mac80211_hwsim =
        MapGetOptional(android_info_map, "enforce_mac80211_hwsim")
            .value_or("true") == "true";

    std::optional<std::string> blank_data_image_mb_opt =
        MapGetOptional(android_info_map, "blank_data_image_mb");
    if (blank_data_image_mb_opt) {
      CF_EXPECTF(android::base::ParseInt(*blank_data_image_mb_opt,
                                         &guest_config.blank_data_image_mb),
                 "Failed to parse value '{}' for blank data image size",
                 *blank_data_image_mb_opt);
    }

    guest_configs.push_back(guest_config);
  }
  return guest_configs;
}

}  // namespace cuttlefish

