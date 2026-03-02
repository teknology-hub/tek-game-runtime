//===-- isteamapps.hpp - ISteamApps interface declarations ----------------===//
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
/// Declarations of ISteamApps interface wrapper and related types.
///
//===----------------------------------------------------------------------===//
#pragma once
#include "steam/api.hpp"

#include "common.hpp" // IWYU pragma: keep

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <ranges>

namespace tek::game_runtime::steam::ISteamApps {

// Virtual method enumeration.
enum {
  m_BIsSubscribed,
  m_BIsLowViolence,
  m_BIsCybercafe,
  m_BIsVACBanned,
  m_GetCurrentGameLanguage,
  m_GetAvailableGameLanguages,
  m_BIsSubscribedApp,
  m_BIsDlcInstalled,
  m_GetEarliestPurchaseUnixTime,
  m_BIsSubscribedFromFreeWeekend,
  m_GetDLCCount,
  m_BGetDLCDataByIndex,
  m_InstallDLC,
  m_UninstallDLC,
  m_RequestAppProofOfPurchaseKey,
  m_GetCurrentBetaName,
  m_MarkContentCorrupt,
  m_GetInstalledDepots,
  m_GetAppInstallDir,
  m_BIsAppInstalled,
  m_GetAppOwner,
  m_GetLaunchQueryParam,
  m_GetDlcDownloadProgress,
  m_GetAppBuildId,
  m_RequestAllProofOfPurchaseKeys,
  m_GetFileDetails,
  m_GetLaunchCommandLine,
  m_BIsSubscribedFromFamilySharing,
  m_BIsTimedTrial,
  m_SetDlcContext,
  m_GetNumBetas,
  m_GetBetaInfo,
  m_SetActiveBeta,
  num_methods
};

using BIsSubscribedApp_t = bool(void *_Nonnull iface, std::uint32_t app_id);
using BIsAppInstalled_t = bool(void *_Nonnull iface, std::uint32_t app_id);

/// Wrapper descriptor for ISteamApps interface.
inline wrapper_desc<num_methods> wrapper;

/// Initialize the wrapper.
///
/// @param [in, out] iface
///    Pointer to the interface instance.
constexpr void setup(cpp_interface *_Nonnull iface) noexcept {
  if (ver >= 0x0003002A003D0042) { // 03.42.61.66
    // "STEAMAPPS_INTERFACE_VERSION008", used since Steamworks SDK v1.37
    wrapper.num_methods = 33;
  } else if (ver >= 0x0002003B0033002B) { // 02.59.51.43
    // "STEAMAPPS_INTERFACE_VERSION007", used since Steamworks SDK v1.32
    wrapper.num_methods = 24;
  } else if (ver >= 0x00010062001F0049) { // 01.98.31.73
    // "STEAMAPPS_INTERFACE_VERSION006", used since Steamworks SDK v1.26
    wrapper.num_methods = 22;
  } else if (ver >= 0x0001001E0032002E) { // 01.30.50.46
    // "STEAMAPPS_INTERFACE_VERSION005", used since Steamworks SDK v1.18
    wrapper.num_methods = 20;
  } else if (ver >= 0x0000006000210030) { // 00.96.33.48
    // "STEAMAPPS_INTERFACE_VERSION004", used since Steamworks SDK v1.12
    wrapper.num_methods = 14;
  } else {
    // "STEAMAPPS_INTERFACE_VERSION003", used in Steamworks SDK v1.11
    wrapper.num_methods = 8;
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
  return "STEAMAPPS_INTERFACE_VERSION008";
}

} // namespace tek::game_runtime::steam::ISteamApps
