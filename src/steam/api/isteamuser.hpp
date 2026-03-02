//===-- isteamuser.hpp - ISteamUser interface declarations ----------------===//
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
/// Declarations of ISteamUser interface wrapper and related types.
///
//===----------------------------------------------------------------------===//
#pragma once
#include "steam/api.hpp"

#include "common.hpp" // IWYU pragma: keep

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <ranges>

namespace tek::game_runtime::steam::ISteamUser {

// Virtual method enumeration.
enum {
  m_GetHSteamUser,
  m_BLoggedOn,
  m_GetSteamID,
  m_InitiateGameConnection,
  m_TerminateGameConnection,
  m_TrackAppUsageEvent,
  m_GetUserDataFolder,
  m_StartVoiceRecording,
  m_StopVoiceRecording,
  m_GetAvailableVoice,
  m_GetVoice,
  m_DecompressVoice,
  m_GetVoiceOptimalSampleRate,
  m_GetAuthSessionTicket,
  m_GetAuthTicketForWebApi,
  m_BeginAuthSession,
  m_EndAuthSession,
  m_CancelAuthTicket,
  m_UserHasLicenseForApp,
  m_BIsBehindNAT,
  m_AdvertiseGame,
  m_RequestEncryptedAppTicket,
  m_GetEncryptedAppTicket,
  m_GetGameBadgeLevel,
  m_GetPlayerSteamLevel,
  m_RequestStoreAuthURL,
  m_BIsPhoneVerified,
  m_BIsTwoFactorEnabled,
  m_BIsPhoneIdentifying,
  m_BIsPhoneRequiringVerification,
  m_GetMarketEligibility,
  m_GetDurationControl,
  m_BSetDurationControlOnlineState,
  num_methods
};

enum class uhlfa_result { HasLicense, DoesNotHaveLicense, NoAuth };

using GetSteamID_t = std::uint64_t *_Nonnull(void *_Nonnull iface,
                                             std::uint64_t *_Nonnull id);

/// Wrapper descriptor for ISteamUser interface.
inline wrapper_desc<num_methods> wrapper;

/// Initialize the wrapper.
///
/// @param [in, out] iface
///    Pointer to the interface instance.
constexpr void setup(cpp_interface *_Nonnull iface) {
  if (ver >= 0x000800020015005F) { // 08.02.21.95
    // "SteamUser023", used since Steamworks SDK v1.57
    wrapper.num_methods = 33;
    std::ranges::iota(
        std::ranges::subrange(wrapper.vm_idxs, wrapper.num_methods), 0);
  } else if (ver >= 0x0005005C0024004B) { // 05.92.36.75
    // "SteamUser021" & "SteamUser022", used since Steamworks SDK v1.49
    wrapper.num_methods = 32;
    wrapper.vm_idxs[m_GetHSteamUser] = 0;
    wrapper.vm_idxs[m_BLoggedOn] = 1;
    wrapper.vm_idxs[m_GetSteamID] = 2;
    wrapper.vm_idxs[m_InitiateGameConnection] = 3;
    wrapper.vm_idxs[m_TerminateGameConnection] = 4;
    wrapper.vm_idxs[m_TrackAppUsageEvent] = 5;
    wrapper.vm_idxs[m_GetUserDataFolder] = 6;
    wrapper.vm_idxs[m_StartVoiceRecording] = 7;
    wrapper.vm_idxs[m_StopVoiceRecording] = 8;
    wrapper.vm_idxs[m_GetAvailableVoice] = 9;
    wrapper.vm_idxs[m_GetVoice] = 10;
    wrapper.vm_idxs[m_DecompressVoice] = 11;
    wrapper.vm_idxs[m_GetVoiceOptimalSampleRate] = 12;
    wrapper.vm_idxs[m_GetAuthSessionTicket] = 13;
    wrapper.vm_idxs[m_BeginAuthSession] = 14;
    wrapper.vm_idxs[m_EndAuthSession] = 15;
    wrapper.vm_idxs[m_CancelAuthTicket] = 16;
    wrapper.vm_idxs[m_UserHasLicenseForApp] = 17;
    wrapper.vm_idxs[m_BIsBehindNAT] = 18;
    wrapper.vm_idxs[m_AdvertiseGame] = 19;
    wrapper.vm_idxs[m_RequestEncryptedAppTicket] = 20;
    wrapper.vm_idxs[m_GetEncryptedAppTicket] = 21;
    wrapper.vm_idxs[m_GetGameBadgeLevel] = 22;
    wrapper.vm_idxs[m_GetPlayerSteamLevel] = 23;
    wrapper.vm_idxs[m_RequestStoreAuthURL] = 24;
    wrapper.vm_idxs[m_BIsPhoneVerified] = 25;
    wrapper.vm_idxs[m_BIsTwoFactorEnabled] = 26;
    wrapper.vm_idxs[m_BIsPhoneIdentifying] = 27;
    wrapper.vm_idxs[m_BIsPhoneRequiringVerification] = 28;
    wrapper.vm_idxs[m_GetMarketEligibility] = 29;
    wrapper.vm_idxs[m_GetDurationControl] = 30;
    wrapper.vm_idxs[m_BSetDurationControlOnlineState] = 31;
  } else if (ver >= 0x0004005F0014001E) { // 04.95.20.30
    // "SteamUser020", used since Steamworks SDK v1.43
    wrapper.num_methods = 31;
    wrapper.vm_idxs[m_GetHSteamUser] = 0;
    wrapper.vm_idxs[m_BLoggedOn] = 1;
    wrapper.vm_idxs[m_GetSteamID] = 2;
    wrapper.vm_idxs[m_InitiateGameConnection] = 3;
    wrapper.vm_idxs[m_TerminateGameConnection] = 4;
    wrapper.vm_idxs[m_TrackAppUsageEvent] = 5;
    wrapper.vm_idxs[m_GetUserDataFolder] = 6;
    wrapper.vm_idxs[m_StartVoiceRecording] = 7;
    wrapper.vm_idxs[m_StopVoiceRecording] = 8;
    wrapper.vm_idxs[m_GetAvailableVoice] = 9;
    wrapper.vm_idxs[m_GetVoice] = 10;
    wrapper.vm_idxs[m_DecompressVoice] = 11;
    wrapper.vm_idxs[m_GetVoiceOptimalSampleRate] = 12;
    wrapper.vm_idxs[m_GetAuthSessionTicket] = 13;
    wrapper.vm_idxs[m_BeginAuthSession] = 14;
    wrapper.vm_idxs[m_EndAuthSession] = 15;
    wrapper.vm_idxs[m_CancelAuthTicket] = 16;
    wrapper.vm_idxs[m_UserHasLicenseForApp] = 17;
    wrapper.vm_idxs[m_BIsBehindNAT] = 18;
    wrapper.vm_idxs[m_AdvertiseGame] = 19;
    wrapper.vm_idxs[m_RequestEncryptedAppTicket] = 20;
    wrapper.vm_idxs[m_GetEncryptedAppTicket] = 21;
    wrapper.vm_idxs[m_GetGameBadgeLevel] = 22;
    wrapper.vm_idxs[m_GetPlayerSteamLevel] = 23;
    wrapper.vm_idxs[m_RequestStoreAuthURL] = 24;
    wrapper.vm_idxs[m_BIsPhoneVerified] = 25;
    wrapper.vm_idxs[m_BIsTwoFactorEnabled] = 26;
    wrapper.vm_idxs[m_BIsPhoneIdentifying] = 27;
    wrapper.vm_idxs[m_BIsPhoneRequiringVerification] = 28;
    wrapper.vm_idxs[m_GetMarketEligibility] = 29;
    wrapper.vm_idxs[m_GetDurationControl] = 30;
  } else if (ver >= 0x0003002A003D0042) { // 03.42.61.66
    // "SteamUser019", used since Steamworks SDK v1.37
    wrapper.num_methods = 29;
    wrapper.vm_idxs[m_GetHSteamUser] = 0;
    wrapper.vm_idxs[m_BLoggedOn] = 1;
    wrapper.vm_idxs[m_GetSteamID] = 2;
    wrapper.vm_idxs[m_InitiateGameConnection] = 3;
    wrapper.vm_idxs[m_TerminateGameConnection] = 4;
    wrapper.vm_idxs[m_TrackAppUsageEvent] = 5;
    wrapper.vm_idxs[m_GetUserDataFolder] = 6;
    wrapper.vm_idxs[m_StartVoiceRecording] = 7;
    wrapper.vm_idxs[m_StopVoiceRecording] = 8;
    wrapper.vm_idxs[m_GetAvailableVoice] = 9;
    wrapper.vm_idxs[m_GetVoice] = 10;
    wrapper.vm_idxs[m_DecompressVoice] = 11;
    wrapper.vm_idxs[m_GetVoiceOptimalSampleRate] = 12;
    wrapper.vm_idxs[m_GetAuthSessionTicket] = 13;
    wrapper.vm_idxs[m_BeginAuthSession] = 14;
    wrapper.vm_idxs[m_EndAuthSession] = 15;
    wrapper.vm_idxs[m_CancelAuthTicket] = 16;
    wrapper.vm_idxs[m_UserHasLicenseForApp] = 17;
    wrapper.vm_idxs[m_BIsBehindNAT] = 18;
    wrapper.vm_idxs[m_AdvertiseGame] = 19;
    wrapper.vm_idxs[m_RequestEncryptedAppTicket] = 20;
    wrapper.vm_idxs[m_GetEncryptedAppTicket] = 21;
    wrapper.vm_idxs[m_GetGameBadgeLevel] = 22;
    wrapper.vm_idxs[m_GetPlayerSteamLevel] = 23;
    wrapper.vm_idxs[m_RequestStoreAuthURL] = 24;
    wrapper.vm_idxs[m_BIsPhoneVerified] = 25;
    wrapper.vm_idxs[m_BIsTwoFactorEnabled] = 26;
    wrapper.vm_idxs[m_BIsPhoneIdentifying] = 27;
    wrapper.vm_idxs[m_BIsPhoneRequiringVerification] = 28;
  } else if (ver >= 0x0002003B0033002B) { // 02.59.51.43
    // "SteamUser018", used since Steamworks SDK v1.32
    wrapper.num_methods = 25;
    wrapper.vm_idxs[m_GetHSteamUser] = 0;
    wrapper.vm_idxs[m_BLoggedOn] = 1;
    wrapper.vm_idxs[m_GetSteamID] = 2;
    wrapper.vm_idxs[m_InitiateGameConnection] = 3;
    wrapper.vm_idxs[m_TerminateGameConnection] = 4;
    wrapper.vm_idxs[m_TrackAppUsageEvent] = 5;
    wrapper.vm_idxs[m_GetUserDataFolder] = 6;
    wrapper.vm_idxs[m_StartVoiceRecording] = 7;
    wrapper.vm_idxs[m_StopVoiceRecording] = 8;
    wrapper.vm_idxs[m_GetAvailableVoice] = 9;
    wrapper.vm_idxs[m_GetVoice] = 10;
    wrapper.vm_idxs[m_DecompressVoice] = 11;
    wrapper.vm_idxs[m_GetVoiceOptimalSampleRate] = 12;
    wrapper.vm_idxs[m_GetAuthSessionTicket] = 13;
    wrapper.vm_idxs[m_BeginAuthSession] = 14;
    wrapper.vm_idxs[m_EndAuthSession] = 15;
    wrapper.vm_idxs[m_CancelAuthTicket] = 16;
    wrapper.vm_idxs[m_UserHasLicenseForApp] = 17;
    wrapper.vm_idxs[m_BIsBehindNAT] = 18;
    wrapper.vm_idxs[m_AdvertiseGame] = 19;
    wrapper.vm_idxs[m_RequestEncryptedAppTicket] = 20;
    wrapper.vm_idxs[m_GetEncryptedAppTicket] = 21;
    wrapper.vm_idxs[m_GetGameBadgeLevel] = 22;
    wrapper.vm_idxs[m_GetPlayerSteamLevel] = 23;
    wrapper.vm_idxs[m_RequestStoreAuthURL] = 24;
  } else if (ver >= 0x00010053001F0025) { // 01.83.31.37
    // "SteamUser017", used since Steamworks SDK v1.25
    wrapper.num_methods = 24;
    wrapper.vm_idxs[m_GetHSteamUser] = 0;
    wrapper.vm_idxs[m_BLoggedOn] = 1;
    wrapper.vm_idxs[m_GetSteamID] = 2;
    wrapper.vm_idxs[m_InitiateGameConnection] = 3;
    wrapper.vm_idxs[m_TerminateGameConnection] = 4;
    wrapper.vm_idxs[m_TrackAppUsageEvent] = 5;
    wrapper.vm_idxs[m_GetUserDataFolder] = 6;
    wrapper.vm_idxs[m_StartVoiceRecording] = 7;
    wrapper.vm_idxs[m_StopVoiceRecording] = 8;
    wrapper.vm_idxs[m_GetAvailableVoice] = 9;
    wrapper.vm_idxs[m_GetVoice] = 10;
    wrapper.vm_idxs[m_DecompressVoice] = 11;
    wrapper.vm_idxs[m_GetVoiceOptimalSampleRate] = 12;
    wrapper.vm_idxs[m_GetAuthSessionTicket] = 13;
    wrapper.vm_idxs[m_BeginAuthSession] = 14;
    wrapper.vm_idxs[m_EndAuthSession] = 15;
    wrapper.vm_idxs[m_CancelAuthTicket] = 16;
    wrapper.vm_idxs[m_UserHasLicenseForApp] = 17;
    wrapper.vm_idxs[m_BIsBehindNAT] = 18;
    wrapper.vm_idxs[m_AdvertiseGame] = 19;
    wrapper.vm_idxs[m_RequestEncryptedAppTicket] = 20;
    wrapper.vm_idxs[m_GetEncryptedAppTicket] = 21;
    wrapper.vm_idxs[m_GetGameBadgeLevel] = 22;
    wrapper.vm_idxs[m_GetPlayerSteamLevel] = 23;
  } else if (ver >= 0x000100060063003D) { // 01.06.99.61
    // "SteamUser016", used since Steamworks SDK v1.13
    wrapper.num_methods = 22;
    wrapper.vm_idxs[m_GetHSteamUser] = 0;
    wrapper.vm_idxs[m_BLoggedOn] = 1;
    wrapper.vm_idxs[m_GetSteamID] = 2;
    wrapper.vm_idxs[m_InitiateGameConnection] = 3;
    wrapper.vm_idxs[m_TerminateGameConnection] = 4;
    wrapper.vm_idxs[m_TrackAppUsageEvent] = 5;
    wrapper.vm_idxs[m_GetUserDataFolder] = 6;
    wrapper.vm_idxs[m_StartVoiceRecording] = 7;
    wrapper.vm_idxs[m_StopVoiceRecording] = 8;
    wrapper.vm_idxs[m_GetAvailableVoice] = 9;
    wrapper.vm_idxs[m_GetVoice] = 10;
    wrapper.vm_idxs[m_DecompressVoice] = 11;
    wrapper.vm_idxs[m_GetVoiceOptimalSampleRate] = 12;
    wrapper.vm_idxs[m_GetAuthSessionTicket] = 13;
    wrapper.vm_idxs[m_BeginAuthSession] = 14;
    wrapper.vm_idxs[m_EndAuthSession] = 15;
    wrapper.vm_idxs[m_CancelAuthTicket] = 16;
    wrapper.vm_idxs[m_UserHasLicenseForApp] = 17;
    wrapper.vm_idxs[m_BIsBehindNAT] = 18;
    wrapper.vm_idxs[m_AdvertiseGame] = 19;
    wrapper.vm_idxs[m_RequestEncryptedAppTicket] = 20;
    wrapper.vm_idxs[m_GetEncryptedAppTicket] = 21;
  } else {
    // "SteamUser014", used in older supported Steamworks SDK versions
    wrapper.num_methods = 21;
    wrapper.vm_idxs[m_GetHSteamUser] = 0;
    wrapper.vm_idxs[m_BLoggedOn] = 1;
    wrapper.vm_idxs[m_GetSteamID] = 2;
    wrapper.vm_idxs[m_InitiateGameConnection] = 3;
    wrapper.vm_idxs[m_TerminateGameConnection] = 4;
    wrapper.vm_idxs[m_TrackAppUsageEvent] = 5;
    wrapper.vm_idxs[m_GetUserDataFolder] = 6;
    wrapper.vm_idxs[m_StartVoiceRecording] = 7;
    wrapper.vm_idxs[m_StopVoiceRecording] = 8;
    wrapper.vm_idxs[m_GetAvailableVoice] = 9;
    wrapper.vm_idxs[m_GetVoice] = 10;
    wrapper.vm_idxs[m_DecompressVoice] = 11;
    wrapper.vm_idxs[m_GetAuthSessionTicket] = 12;
    wrapper.vm_idxs[m_BeginAuthSession] = 13;
    wrapper.vm_idxs[m_EndAuthSession] = 14;
    wrapper.vm_idxs[m_CancelAuthTicket] = 15;
    wrapper.vm_idxs[m_UserHasLicenseForApp] = 16;
    wrapper.vm_idxs[m_BIsBehindNAT] = 17;
    wrapper.vm_idxs[m_AdvertiseGame] = 18;
    wrapper.vm_idxs[m_RequestEncryptedAppTicket] = 19;
    wrapper.vm_idxs[m_GetEncryptedAppTicket] = 20;
  }
  wrapper.orig_vtable = iface->vtable;
  wrapper.iface = iface;
  std::ranges::copy_n(iface->vtable, wrapper.num_methods,
                      wrapper.vtable.begin());
  iface->vtable = wrapper.vtable.data();
}

/// Get interface version string for Steamworks SDK v1.37+.
constexpr const char *_Nonnull get_ver_str() {
  if (ver >= 0x000800020015005F) { // 08.02.21.95
    // Steamworks SDK v1.57+
    return "SteamUser023";
  } else if (ver >= 0x000700600000002C) { // 07.96.00.44
    // Steamworks SDK v1.56
    return "SteamUser022";
  } else if (ver >= 0x0005005C0024004B) { // 05.92.36.75
    // Steamworks SDK v1.49+
    return "SteamUser021";
  } else if (ver >= 0x0004005F0014001E) { // 04.95.20.30
    // Steamworks SDK v1.43+
    return "SteamUser020";
  } else {
    // All previous Steamworks SDK versions since v1.37
    return "SteamUser019";
  }
}

} // namespace tek::game_runtime::steam::ISteamUser
