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
  m_GetCSERIPPort,
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
  m_IsSteamRunningOnSteamDeck,
  m_ShowFloatingGamepadTextInput,
  m_SetGameLauncherMode,
  m_DismissFloatingGamepadTextInput,
  m_DismissGamepadTextInput,
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
  if (ver >= 0x000600060063003B) { // 06.06.99.59
    // "SteamUtils010", used since Steamworks SDK v1.50
    wrapper.num_methods = 39;
  } else if (ver >= 0x0003005C0048003A) { // 03.92.72.58
    // "SteamUtils009", used since Steamworks SDK v1.40
    wrapper.num_methods = 34;
  } else if (ver >= 0x0003002A003D0042) { // 03.42.61.66
    // "SteamUtils008", used since Steamworks SDK v1.37
    wrapper.num_methods = 28;
  } else if (ver >= 0x000200130022005D) { // 02.19.34.93
    // "SteamUtils007", used since Steamworks SDK v1.29
    wrapper.num_methods = 26;
  } else if (ver >= 0x00010053001F0025) { // 01.83.31.37
    // "SteamUtils006", used since Steamworks SDK v1.25
    wrapper.num_methods = 25;
  } else {
    // "SteamUtils005", used in older supported Steamworks SDK versions
    wrapper.num_methods = 23;
  }
  std::ranges::iota(std::ranges::subrange(wrapper.vm_idxs, wrapper.num_methods),
                    0);
  wrapper.orig_vtable = iface->vtable;
  wrapper.iface = iface;
  std::ranges::copy_n(iface->vtable, wrapper.num_methods,
                      wrapper.vtable.begin());
  iface->vtable = wrapper.vtable.data();
}

/// Get interface version string for Steamworks SDK v1.37+.
constexpr const char *_Nonnull get_ver_str() {
  if (ver >= 0x000600060063003B) { // 06.06.99.59
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
