//===-- isteamutils.hpp - ISteamUtils interface declarations --------------===//
//
// Copyright (c) 2026 Nuclearist <nuclearist@teknology-hub.com>
// Part of tek-game-runtime, under the GNU General Public License v3.0 or later
// See https://github.com/teknology-hub/tek-game-runtime/blob/main/COPYING for
//    license information.
// SPDX-License-Identifier: GPL-3.0-or-later
//
//===----------------------------------------------------------------------===//
///
/// @file
/// Declarations of ISteamUtils interface wrapper and related types.
///
//===----------------------------------------------------------------------===//
#pragma once
#include "steam/api.hpp"

#include "common.hpp" // IWYU pragma: keep

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <ranges>

namespace tek::game_runtime::steam::ISteamUtils {

// Virtual method enumeration.
enum {
  m_GetSecondsSinceAppActive,
  m_GetSecondsSinceComputerActive,
  m_GetConnectedUniverse,
  m_GetServerRealTime,
  m_GetIPCountry,
  m_GetImageSize,
  m_GetImageRGBA,
  m_GetCurrentBatteryPower,
  m_GetAppID,
  m_SetOverlayNotificationPosition,
  m_IsAPICallCompleted,
  m_GetAPICallFailureReason,
  m_GetAPICallResult,
  m_RunFrame,
  m_GetIPCCallCount,
  m_SetWarningMessageHook,
  m_IsOverlayEnabled,
  m_BOverlayNeedsPresent,
  m_CheckFileSignature,
  m_ShowGamepadTextInput,
  m_GetEnteredGamepadTextLength,
  m_GetEnteredGamepadTextInput,
  m_GetSteamUILanguage,
  m_IsSteamRunningInVR,
  m_SetOverlayNotificationInset,
  m_IsSteamInBigPictureMode,
  m_StartVRDashboard,
  m_IsVRHeadsetStreamingEnabled,
  m_SetVRHeadsetStreamingEnabled,
  m_IsSteamChinaLauncher,
  m_InitFilterText,
  m_FilterText,
  m_GetIPv6ConnectivityState,
  m_ShowFloatingGamepadTextInput,
  m_SetGameLauncherMode,
  m_DismissFloatingGamepadTextInput,
  m_DismissGamepadTextInput,
  m_IsRunningOnSteamHardware,
  m_GetSteamHardwareDefaultConfig,
  m_IsRunningUnderProton,
  num_methods
};

using IsAPICallCompleted_t = bool(void *_Nonnull iface, std::uint64_t call,
                                  bool *_Nonnull failed);
using GetAPICallResult_t = bool(void *_Nonnull iface, std::uint64_t call,
                                void *_Nonnull callback, int callback_size,
                                int callback_idx, bool *_Nonnull failed);

/// Wrapper descriptor for ISteamUtils interface.
inline wrapper_desc<num_methods> wrapper;

/// Initialize the wrapper.
///
/// @param [in, out] iface
///    Pointer to the interface instance.
constexpr void setup(cpp_interface *_Nonnull iface) {
  if (ver >= 0x000A0057002E0063) { // 10.87.46.99
    // "SteamUtils011", used since Steamworks SDK v1.65
    wrapper.num_methods = 40;
    std::ranges::iota(
        std::ranges::subrange(wrapper.vm_idxs, wrapper.num_methods), 0);
  } else if (ver >= 0x000600060063003B) { // 06.06.99.59
    // "SteamUtils010", used since Steamworks SDK v1.50
    wrapper.num_methods = 39;
    wrapper.vm_idxs[m_GetSecondsSinceAppActive] = 0;
    wrapper.vm_idxs[m_GetSecondsSinceComputerActive] = 1;
    wrapper.vm_idxs[m_GetConnectedUniverse] = 2;
    wrapper.vm_idxs[m_GetServerRealTime] = 3;
    wrapper.vm_idxs[m_GetIPCountry] = 4;
    wrapper.vm_idxs[m_GetImageSize] = 5;
    wrapper.vm_idxs[m_GetImageRGBA] = 6;
    wrapper.vm_idxs[m_GetCurrentBatteryPower] = 8;
    wrapper.vm_idxs[m_GetAppID] = 9;
    wrapper.vm_idxs[m_SetOverlayNotificationPosition] = 10;
    wrapper.vm_idxs[m_IsAPICallCompleted] = 11;
    wrapper.vm_idxs[m_GetAPICallFailureReason] = 12;
    wrapper.vm_idxs[m_GetAPICallResult] = 13;
    wrapper.vm_idxs[m_RunFrame] = 14;
    wrapper.vm_idxs[m_GetIPCCallCount] = 15;
    wrapper.vm_idxs[m_SetWarningMessageHook] = 16;
    wrapper.vm_idxs[m_IsOverlayEnabled] = 17;
    wrapper.vm_idxs[m_BOverlayNeedsPresent] = 18;
    wrapper.vm_idxs[m_CheckFileSignature] = 19;
    wrapper.vm_idxs[m_ShowGamepadTextInput] = 20;
    wrapper.vm_idxs[m_GetEnteredGamepadTextLength] = 21;
    wrapper.vm_idxs[m_GetEnteredGamepadTextInput] = 22;
    wrapper.vm_idxs[m_GetSteamUILanguage] = 23;
    wrapper.vm_idxs[m_IsSteamRunningInVR] = 24;
    wrapper.vm_idxs[m_SetOverlayNotificationInset] = 25;
    wrapper.vm_idxs[m_IsSteamInBigPictureMode] = 26;
    wrapper.vm_idxs[m_StartVRDashboard] = 27;
    wrapper.vm_idxs[m_IsVRHeadsetStreamingEnabled] = 28;
    wrapper.vm_idxs[m_SetVRHeadsetStreamingEnabled] = 29;
    wrapper.vm_idxs[m_IsSteamChinaLauncher] = 30;
    wrapper.vm_idxs[m_InitFilterText] = 31;
    wrapper.vm_idxs[m_FilterText] = 32;
    wrapper.vm_idxs[m_GetIPv6ConnectivityState] = 33;
    wrapper.vm_idxs[m_ShowFloatingGamepadTextInput] = 35;
    wrapper.vm_idxs[m_SetGameLauncherMode] = 36;
    wrapper.vm_idxs[m_DismissFloatingGamepadTextInput] = 37;
    wrapper.vm_idxs[m_DismissGamepadTextInput] = 38;
  } else if (ver >= 0x0003005C0048003A) { // 03.92.72.58
    // "SteamUtils009", used since Steamworks SDK v1.40
    wrapper.num_methods = 34;
    wrapper.vm_idxs[m_GetSecondsSinceAppActive] = 0;
    wrapper.vm_idxs[m_GetSecondsSinceComputerActive] = 1;
    wrapper.vm_idxs[m_GetConnectedUniverse] = 2;
    wrapper.vm_idxs[m_GetServerRealTime] = 3;
    wrapper.vm_idxs[m_GetIPCountry] = 4;
    wrapper.vm_idxs[m_GetImageSize] = 5;
    wrapper.vm_idxs[m_GetImageRGBA] = 6;
    wrapper.vm_idxs[m_GetCurrentBatteryPower] = 8;
    wrapper.vm_idxs[m_GetAppID] = 9;
    wrapper.vm_idxs[m_SetOverlayNotificationPosition] = 10;
    wrapper.vm_idxs[m_IsAPICallCompleted] = 11;
    wrapper.vm_idxs[m_GetAPICallFailureReason] = 12;
    wrapper.vm_idxs[m_GetAPICallResult] = 13;
    wrapper.vm_idxs[m_RunFrame] = 14;
    wrapper.vm_idxs[m_GetIPCCallCount] = 15;
    wrapper.vm_idxs[m_SetWarningMessageHook] = 16;
    wrapper.vm_idxs[m_IsOverlayEnabled] = 17;
    wrapper.vm_idxs[m_BOverlayNeedsPresent] = 18;
    wrapper.vm_idxs[m_CheckFileSignature] = 19;
    wrapper.vm_idxs[m_ShowGamepadTextInput] = 20;
    wrapper.vm_idxs[m_GetEnteredGamepadTextLength] = 21;
    wrapper.vm_idxs[m_GetEnteredGamepadTextInput] = 22;
    wrapper.vm_idxs[m_GetSteamUILanguage] = 23;
    wrapper.vm_idxs[m_IsSteamRunningInVR] = 24;
    wrapper.vm_idxs[m_SetOverlayNotificationInset] = 25;
    wrapper.vm_idxs[m_IsSteamInBigPictureMode] = 26;
    wrapper.vm_idxs[m_StartVRDashboard] = 27;
    wrapper.vm_idxs[m_IsVRHeadsetStreamingEnabled] = 28;
    wrapper.vm_idxs[m_SetVRHeadsetStreamingEnabled] = 29;
    wrapper.vm_idxs[m_IsSteamChinaLauncher] = 30;
    wrapper.vm_idxs[m_InitFilterText] = 31;
    wrapper.vm_idxs[m_FilterText] = 32;
    wrapper.vm_idxs[m_GetIPv6ConnectivityState] = 33;
  } else if (ver >= 0x0003002A003D0042) { // 03.42.61.66
    // "SteamUtils008", used since Steamworks SDK v1.37
    wrapper.num_methods = 28;
    wrapper.vm_idxs[m_GetSecondsSinceAppActive] = 0;
    wrapper.vm_idxs[m_GetSecondsSinceComputerActive] = 1;
    wrapper.vm_idxs[m_GetConnectedUniverse] = 2;
    wrapper.vm_idxs[m_GetServerRealTime] = 3;
    wrapper.vm_idxs[m_GetIPCountry] = 4;
    wrapper.vm_idxs[m_GetImageSize] = 5;
    wrapper.vm_idxs[m_GetImageRGBA] = 6;
    wrapper.vm_idxs[m_GetCurrentBatteryPower] = 8;
    wrapper.vm_idxs[m_GetAppID] = 9;
    wrapper.vm_idxs[m_SetOverlayNotificationPosition] = 10;
    wrapper.vm_idxs[m_IsAPICallCompleted] = 11;
    wrapper.vm_idxs[m_GetAPICallFailureReason] = 12;
    wrapper.vm_idxs[m_GetAPICallResult] = 13;
    wrapper.vm_idxs[m_RunFrame] = 14;
    wrapper.vm_idxs[m_GetIPCCallCount] = 15;
    wrapper.vm_idxs[m_SetWarningMessageHook] = 16;
    wrapper.vm_idxs[m_IsOverlayEnabled] = 17;
    wrapper.vm_idxs[m_BOverlayNeedsPresent] = 18;
    wrapper.vm_idxs[m_CheckFileSignature] = 19;
    wrapper.vm_idxs[m_ShowGamepadTextInput] = 20;
    wrapper.vm_idxs[m_GetEnteredGamepadTextLength] = 21;
    wrapper.vm_idxs[m_GetEnteredGamepadTextInput] = 22;
    wrapper.vm_idxs[m_GetSteamUILanguage] = 23;
    wrapper.vm_idxs[m_IsSteamRunningInVR] = 24;
    wrapper.vm_idxs[m_SetOverlayNotificationInset] = 25;
    wrapper.vm_idxs[m_IsSteamInBigPictureMode] = 26;
    wrapper.vm_idxs[m_StartVRDashboard] = 27;
  } else if (ver >= 0x000200130022005D) { // 02.19.34.93
    // "SteamUtils007", used since Steamworks SDK v1.29
    wrapper.num_methods = 26;
    wrapper.vm_idxs[m_GetSecondsSinceAppActive] = 0;
    wrapper.vm_idxs[m_GetSecondsSinceComputerActive] = 1;
    wrapper.vm_idxs[m_GetConnectedUniverse] = 2;
    wrapper.vm_idxs[m_GetServerRealTime] = 3;
    wrapper.vm_idxs[m_GetIPCountry] = 4;
    wrapper.vm_idxs[m_GetImageSize] = 5;
    wrapper.vm_idxs[m_GetImageRGBA] = 6;
    wrapper.vm_idxs[m_GetCurrentBatteryPower] = 8;
    wrapper.vm_idxs[m_GetAppID] = 9;
    wrapper.vm_idxs[m_SetOverlayNotificationPosition] = 10;
    wrapper.vm_idxs[m_IsAPICallCompleted] = 11;
    wrapper.vm_idxs[m_GetAPICallFailureReason] = 12;
    wrapper.vm_idxs[m_GetAPICallResult] = 13;
    wrapper.vm_idxs[m_RunFrame] = 14;
    wrapper.vm_idxs[m_GetIPCCallCount] = 15;
    wrapper.vm_idxs[m_SetWarningMessageHook] = 16;
    wrapper.vm_idxs[m_IsOverlayEnabled] = 17;
    wrapper.vm_idxs[m_BOverlayNeedsPresent] = 18;
    wrapper.vm_idxs[m_CheckFileSignature] = 19;
    wrapper.vm_idxs[m_ShowGamepadTextInput] = 20;
    wrapper.vm_idxs[m_GetEnteredGamepadTextLength] = 21;
    wrapper.vm_idxs[m_GetEnteredGamepadTextInput] = 22;
    wrapper.vm_idxs[m_GetSteamUILanguage] = 23;
    wrapper.vm_idxs[m_IsSteamRunningInVR] = 24;
    wrapper.vm_idxs[m_SetOverlayNotificationInset] = 25;
  } else if (ver >= 0x00010053001F0025) { // 01.83.31.37
    // "SteamUtils006", used since Steamworks SDK v1.25
    wrapper.num_methods = 25;
    wrapper.vm_idxs[m_GetSecondsSinceAppActive] = 0;
    wrapper.vm_idxs[m_GetSecondsSinceComputerActive] = 1;
    wrapper.vm_idxs[m_GetConnectedUniverse] = 2;
    wrapper.vm_idxs[m_GetServerRealTime] = 3;
    wrapper.vm_idxs[m_GetIPCountry] = 4;
    wrapper.vm_idxs[m_GetImageSize] = 5;
    wrapper.vm_idxs[m_GetImageRGBA] = 6;
    wrapper.vm_idxs[m_GetCurrentBatteryPower] = 8;
    wrapper.vm_idxs[m_GetAppID] = 9;
    wrapper.vm_idxs[m_SetOverlayNotificationPosition] = 10;
    wrapper.vm_idxs[m_IsAPICallCompleted] = 11;
    wrapper.vm_idxs[m_GetAPICallFailureReason] = 12;
    wrapper.vm_idxs[m_GetAPICallResult] = 13;
    wrapper.vm_idxs[m_RunFrame] = 14;
    wrapper.vm_idxs[m_GetIPCCallCount] = 15;
    wrapper.vm_idxs[m_SetWarningMessageHook] = 16;
    wrapper.vm_idxs[m_IsOverlayEnabled] = 17;
    wrapper.vm_idxs[m_BOverlayNeedsPresent] = 18;
    wrapper.vm_idxs[m_CheckFileSignature] = 19;
    wrapper.vm_idxs[m_ShowGamepadTextInput] = 20;
    wrapper.vm_idxs[m_GetEnteredGamepadTextLength] = 21;
    wrapper.vm_idxs[m_GetEnteredGamepadTextInput] = 22;
    wrapper.vm_idxs[m_GetSteamUILanguage] = 23;
    wrapper.vm_idxs[m_IsSteamRunningInVR] = 24;
  } else {
    // "SteamUtils005", used in older supported Steamworks SDK versions
    wrapper.num_methods = 23;
    wrapper.vm_idxs[m_GetSecondsSinceAppActive] = 0;
    wrapper.vm_idxs[m_GetSecondsSinceComputerActive] = 1;
    wrapper.vm_idxs[m_GetConnectedUniverse] = 2;
    wrapper.vm_idxs[m_GetServerRealTime] = 3;
    wrapper.vm_idxs[m_GetIPCountry] = 4;
    wrapper.vm_idxs[m_GetImageSize] = 5;
    wrapper.vm_idxs[m_GetImageRGBA] = 6;
    wrapper.vm_idxs[m_GetCurrentBatteryPower] = 8;
    wrapper.vm_idxs[m_GetAppID] = 9;
    wrapper.vm_idxs[m_SetOverlayNotificationPosition] = 10;
    wrapper.vm_idxs[m_IsAPICallCompleted] = 11;
    wrapper.vm_idxs[m_GetAPICallFailureReason] = 12;
    wrapper.vm_idxs[m_GetAPICallResult] = 13;
    wrapper.vm_idxs[m_RunFrame] = 14;
    wrapper.vm_idxs[m_GetIPCCallCount] = 15;
    wrapper.vm_idxs[m_SetWarningMessageHook] = 16;
    wrapper.vm_idxs[m_IsOverlayEnabled] = 17;
    wrapper.vm_idxs[m_BOverlayNeedsPresent] = 18;
    wrapper.vm_idxs[m_CheckFileSignature] = 19;
    wrapper.vm_idxs[m_ShowGamepadTextInput] = 20;
    wrapper.vm_idxs[m_GetEnteredGamepadTextLength] = 21;
    wrapper.vm_idxs[m_GetEnteredGamepadTextInput] = 22;
  }
  wrapper.orig_vtable = iface->vtable;
  wrapper.iface = iface;
  std::ranges::copy_n(iface->vtable, wrapper.num_methods,
                      wrapper.vtable.begin());
  iface->vtable = wrapper.vtable.data();
}

/// Get interface version string for Steamworks SDK v1.37+.
constexpr const char *_Nonnull get_ver_str() {
  if (ver >= 0x000A0057002E0063) { // 10.87.46.99
    // Steamworks SDK v1.65+
    return "SteamUtils011";
  } else if (ver >= 0x000600060063003B) { // 06.06.99.59
    // Steamworks SDK v1.50+
    return "SteamUtils010";
  } else if (ver >= 0x0003005C0048003A) { // 03.92.72.58
    // Steamworks SDK v1.40+
    return "SteamUtils009";
  } else {
    // All previous Steamworks SDK versions since v1.37
    return "SteamUtils008";
  }
}

} // namespace tek::game_runtime::steam::ISteamUtils
