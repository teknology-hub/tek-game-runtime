//===-- steam_api.cpp - Steam API wrapper implementation ------------------===//
//
// Copyright (c) 2025 Nuclearist <nuclearist@teknology-hub.com>
// Part of tek-game-runtime, under the GNU General Public License v3.0 or later
// See https://github.com/teknology-hub/tek-game-runtime/blob/main/COPYING for
//    license information.
// SPDX-License-Identifier: GPL-3.0-or-later
//
//===----------------------------------------------------------------------===//
///
/// @file
/// Implementation of the Steam API wrapper interface.
///
//===----------------------------------------------------------------------===//
#include "steam_api.hpp"

#include "common.hpp"
#include "game_cbs.hpp"
#include "settings.hpp"
#include "tek-steamclient.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <dbghelp.h>
#include <format>
#include <iterator>
#include <locale>
#include <ranges>
#include <span>
#include <string_view>

namespace tek::game_runtime::steam_api {

namespace {

//===-- Common Steam API method wrappers ----------------------------------===//

/// Wrapper for ISteamApps::BIsSubscribed, making it always return `true`.
static bool SteamApps_BIsSubscribed(void *) noexcept { return true; }

/// Pointer to the original ISteamApps::BIsSubscribedApp method.
static ISteamApps_BIsSubscribedApp_t *_Nonnull SteamApps_BIsSubscribedApp_orig;
/// Wrapper for ISteamApps::BIsSubscribedApp, making it always return `true` for
///    current application ID and DLC listed in settings.
static bool SteamApps_BIsSubscribedApp(void *_Nonnull iface,
                                       std::uint32_t app_id) {
  if (app_id == g_settings.steam->app_id ||
      std::ranges::contains(g_settings.steam->dlc | std::views::elements<0>,
                            app_id)) {
    return true;
  }
  return SteamApps_BIsSubscribedApp_orig(iface, app_id);
}

/// Wrapper for ISteamApps::BIsDlcInstalled, making it always return `true` for
///    IDs listed in the settings.
static bool SteamApps_BIsDlcInstalled(void *, std::uint32_t app_id) {
  return g_settings.steam->installed_dlc.contains(app_id);
}

/// Wrapper for ISteamApps::BIsSubscribedFromFreeWeekend, making it always
///    return `false`.
static bool SteamApps_BIsSubscribedFromFreeWeekend(void *) noexcept {
  return false;
}

/// Wrapper for ISteamApps::GetDLCCount, making it return the number of DLC
///    entries in settings.
static int SteamApps_GetDLCCount(void *) noexcept {
  return g_settings.steam->dlc.size();
}

/// Wrapper for ISteamApps::BGetDLCDataByIndex, making it return data for the
///    corresponding DLC entry in settings.
static bool SteamApps_BGetDLCDataByIndex(void *, int idx,
                                         std::uint32_t *_Nonnull app_id,
                                         bool *_Nonnull available,
                                         char *_Nullable name_buf,
                                         int name_buf_size) {
  if (idx < static_cast<int>(g_settings.steam->dlc.size())) {
    return false;
  }
  auto &[id, name]{g_settings.steam->dlc[idx]};
  *app_id = id;
  *available = true;
  if (name_buf_size > 0) {
    name_buf[name.copy(name_buf, name_buf_size - 1)] = '\0';
  }
  return true;
}

/// Pointer to the original ISteamApps::BIsAppInstalled method.
static ISteamApps_BIsAppInstalled_t *_Nonnull SteamApps_BIsAppInstalled_orig;
/// Wrapper for ISteamApps::BIsAppInstalled, making it always return `true` for
///    current application ID and installed DLC listed in the settings.
static bool SteamApps_BIsAppInstalled(void *_Nonnull iface,
                                      std::uint32_t app_id) {
  if (app_id == g_settings.steam->app_id ||
      g_settings.steam->installed_dlc.contains(app_id)) {
    return true;
  }
  return SteamApps_BIsAppInstalled_orig(iface, app_id);
}

/// Wrapper for ISteamApps::GetAppOwner, making it return current user's Steam
///    ID.
static std::uint64_t *_Nonnull SteamApps_GetAppOwner(
    void *, std::uint64_t *_Nonnull id) noexcept {
  *id = steam_id;
  return id;
}

/// Wrapper for ISteamApps::BIsSubscribedFromFamilySharing, making it always
///    return `false`.
static bool SteamApps_BIsSubscribedFromFamilySharing(void *) noexcept {
  return false;
}

/// Wrapper for ISteamApps::BIsTimedTrial, making it always return `false`.
static bool SteamApps_BIsTimedTrial(void *, std::uint32_t *,
                                    std::uint32_t *) noexcept {
  return false;
}

/// Wrapper for ISteamUser::UserHasLicenseForApp, making it always return
///    `UserHasLicenseForAppResult::HasLicense`.
static UserHasLicenseForAppResult
SteamUser_UserHasLicenseForApp(void *, std::uint64_t, std::uint32_t) noexcept {
  return UserHasLicenseForAppResult::HasLicense;
}

/// Wrapper for ISteamUtils::GetAppID, making it return original app ID.
static std::uint32_t SteamUtils_GetAppID(void *) noexcept {
  return g_settings.steam->app_id;
}

//===-- SteamAPI_Init wrapping --------------------------------------------===//

/// Primitive C++ interface representation.
struct cpp_interface {
  /// Pointer to the virtual method table.
  void *const _Nonnull *_Nonnull vtable;
};

/// `SteamAPI_Init` function type.
using SteamAPI_Init_t = bool();

/// Wrapper for `SteamAPI_Init`.
static bool SteamAPI_Init() {
  const auto app_id{g_settings.steam->app_id};
  const auto spoof_app_id{g_settings.steam->spoof_app_id};
  std::array<WCHAR, 11> buf;
  *std::format_to_n(buf.data(), buf.size(), std::locale::classic(), L"{}",
                    spoof_app_id ? spoof_app_id : app_id)
       .out = L'\0';
  SetEnvironmentVariableW(L"SteamAppId", buf.data());
  const auto SteamAPI_Init_orig{reinterpret_cast<SteamAPI_Init_t *>(
      GetProcAddress(GetModuleHandleW(L"steam_api64.dll"), "SteamAPI_Init"))};
  bool res{SteamAPI_Init_orig()};
  if (!spoof_app_id) {
    if (res) {
      g_settings.steam->spoof_app_id = app_id;
    } else {
      // User probably doesn't have license for app_id, try again with 480
      SetEnvironmentVariableW(L"SteamAppId", L"480");
      res = SteamAPI_Init_orig();
      if (res) {
        g_settings.steam->spoof_app_id = 480;
      }
    }
  }
  if (!res) {
    display_error(
        L"SteamAPI_Init() returned false. Make sure that Steam is running; if "
        L"it is, try signing out of your account then signing back in.");
    return false;
  }
  // Get Steam API file version
  const auto module{GetModuleHandleW(L"steam_api64.dll")};
  {
    const auto rsrc{
        FindResourceW(module, MAKEINTRESOURCEW(VS_VERSION_INFO), RT_VERSION)};
    if (!rsrc) {
      goto version_fail;
    }
    const auto ver_res{LoadResource(module, rsrc)};
    if (!ver_res) {
      goto version_fail;
    }
    const auto ver_data{LockResource(ver_res)};
    if (!ver_data) {
      goto version_fail;
    }
    VS_FIXEDFILEINFO *file_info;
    UINT size;
    if (!VerQueryValueW(ver_data, L"\\", reinterpret_cast<LPVOID *>(&file_info),
                        &size)) {
      goto version_fail;
    }
    ver = file_info->dwFileVersionLS |
          (static_cast<std::uint64_t>(file_info->dwFileVersionMS) << 32);
  }
  if (ver > max_supported_ver) {
    const auto ver_words{reinterpret_cast<const WORD *>(&ver)};
    display_error(
        std::format(
            std::locale::classic(),
            L"Unsupported steam_api64.dll file version {:02}.{:02}.{:02}.{:02}",
            ver_words[3], ver_words[2], ver_words[1], ver_words[0])
            .data());
    return false;
  }
  // Obtain interface pointers
  cpp_interface *ISteamApps_ptr;
  cpp_interface *ISteamMatchmaking_ptr;
  cpp_interface *ISteamMatchmakingServers_ptr;
  cpp_interface *ISteamUGC_ptr;
  cpp_interface *ISteamUser_ptr;
  cpp_interface *ISteamUtils_ptr;
  if (ver >= 0x0003002A003D0042) {
    // Steamworks SDK v1.37+ use SteamInternal_CreateInterface to create an
    //    ISteamClient instance, which is used to obtain other interfaces;
    //    getters are inlined to use cached pointers from this setup
    using SteamInternal_CreateInterface_t = cpp_interface *(const char *);
    using SteamAPI_GetHSteam_t = std::int32_t();
    using ISteamClient_GetISteamGenericInterface_t = cpp_interface *(
        cpp_interface *, std::int32_t, std::int32_t, const char *);
    const char *interface_ver;
    // Get ISteamClient
    if (ver >= 0x0008003F000B0054) { // 08.63.11.84
      // Steamworks SDK v1.59+
      interface_ver = "SteamClient021";
    } else if (ver >= 0x000500350021004E) { // 05.53.33.78
      // Steamworks SDK v1.47+
      interface_ver = "SteamClient020";
    } else if (ver >= 0x0005001900410015) { // 05.25.65.21
      // Steamworks SDK v1.46
      interface_ver = "SteamClient019";
    } else if (ver >= 0x0004005F0014001E) { // 04.95.20.30
      // Steamworks SDK v1.43+
      interface_ver = "SteamClient018";
    } else {
      // All previous Steamworks SDK versions since v1.37
      interface_ver = "SteamClient017";
    }
    const auto ISteamClient_ptr{
        reinterpret_cast<SteamInternal_CreateInterface_t *>(GetProcAddress(
            module, "SteamInternal_CreateInterface"))(interface_ver)};
    const auto ISteamClient_GetISteamGenericInterface{
        reinterpret_cast<ISteamClient_GetISteamGenericInterface_t *>(
            ISteamClient_ptr->vtable[12])};
    const auto pipe{reinterpret_cast<SteamAPI_GetHSteam_t *>(
        GetProcAddress(module, "SteamAPI_GetHSteamPipe"))()};
    const auto user{reinterpret_cast<SteamAPI_GetHSteam_t *>(
        GetProcAddress(module, "SteamAPI_GetHSteamUser"))()};
    // Get ISteamApps
    ISteamApps_ptr = ISteamClient_GetISteamGenericInterface(
        ISteamClient_ptr, user, pipe, "STEAMAPPS_INTERFACE_VERSION008");
    // Get ISteamMatchmaking
    ISteamMatchmaking_ptr = ISteamClient_GetISteamGenericInterface(
        ISteamClient_ptr, user, pipe, "SteamMatchMaking009");
    // Get ISteamMatchmakingServers
    ISteamMatchmakingServers_ptr = ISteamClient_GetISteamGenericInterface(
        ISteamClient_ptr, user, pipe, "SteamMatchMakingServers002");
    // Get ISteamUGC
    ISteamUGC_ptr = ISteamClient_GetISteamGenericInterface(
        ISteamClient_ptr, user, pipe, interface_ver);
    if (ver >= 0x0009003C002C000A) { // 09.60.44.10
      // Steamworks SDK v1.62
      interface_ver = "STEAMUGC_INTERFACE_VERSION021";
    } else if (ver >= 0x0008006100630046) { // 08.97.99.70
      // Steamworks SDK v1.60+
      interface_ver = "STEAMUGC_INTERFACE_VERSION020";
    } else if (ver >= 0x0008002100090017) { // 08.33.09.23
      // Steamworks SDK v1.58+
      interface_ver = "STEAMUGC_INTERFACE_VERSION018";
    } else if (ver >= 0x000700600000002C) { // 07.96.00.44
      // Steamworks SDK v1.56+
      interface_ver = "STEAMUGC_INTERFACE_VERSION017";
    } else if (ver >= 0x0006005B00150039) { // 06.91.21.57
      // Steamworks SDK v1.53+
      interface_ver = "STEAMUGC_INTERFACE_VERSION016";
    } else if (ver >= 0x0006001C00120056) { // 06.28.18.86
      // Steamworks SDK v1.51+
      interface_ver = "STEAMUGC_INTERFACE_VERSION015";
    } else if (ver >= 0x000500350021004E) { // 05.53.33.78
      // Steamworks SDK v1.47+
      interface_ver = "STEAMUGC_INTERFACE_VERSION014";
    } else if (ver >= 0x000500130026003E) { // 05.19.38.62
      // Steamworks SDK v1.45+
      interface_ver = "STEAMUGC_INTERFACE_VERSION013";
    } else if (ver >= 0x0004005F0014001E) { // 04.95.20.30
      // Steamworks SDK v1.43+
      interface_ver = "STEAMUGC_INTERFACE_VERSION012";
    } else if (ver >= 0x0003005C0048003A) { // 03.92.72.58
      // Steamworks SDK v1.40+
      interface_ver = "STEAMUGC_INTERFACE_VERSION010";
    } else if (ver >= 0x0003003E00520052) { // 03.62.82.82
      // Steamworks SDK v1.38+
      interface_ver = "STEAMUGC_INTERFACE_VERSION009";
    } else {
      // Steamworks SDK v1.37
      interface_ver = "STEAMUGC_INTERFACE_VERSION008";
    }
    // Get ISteamUser
    if (ver >= 0x000800020015005F) { // 08.02.21.95
      // Steamworks SDK v1.57+
      interface_ver = "SteamUser023";
    } else if (ver >= 0x000700600000002C) { // 07.96.00.44
      // Steamworks SDK v1.56
      interface_ver = "SteamUser022";
    } else if (ver >= 0x0005005C0024004B) { // 05.92.36.75
      // Steamworks SDK v1.49+
      interface_ver = "SteamUser021";
    } else if (ver >= 0x0004005F0014001E) { // 04.95.20.30
      // Steamworks SDK v1.43+
      interface_ver = "SteamUser020";
    } else {
      // All previous Steamworks SDK versions since v1.37
      interface_ver = "SteamUser019";
    }
    ISteamUser_ptr = ISteamClient_GetISteamGenericInterface(
        ISteamClient_ptr, user, pipe, interface_ver);
    // Get ISteamUtils
    if (ver >= 0x000600060063003B) { // 06.06.99.59
      // Steamworks SDK v1.50+
      interface_ver = "SteamUtils010";
    } else if (ver >= 0x0003005C0048003A) { // 03.92.72.58
      // Steamworks SDK v1.40+
      interface_ver = "SteamUtils009";
    } else {
      // All previous Steamworks SDK versions since v1.37
      interface_ver = "SteamUtils008";
    }
    ISteamUtils_ptr = ISteamClient_GetISteamGenericInterface(
        ISteamClient_ptr, user, pipe, interface_ver);
  } else { // if (ver >= 0x0003002A003D0042)
    // Older Steamworks SDK versions have interface getters implemented as
    // exported functions
    using getter_t = cpp_interface *();
    ISteamApps_ptr =
        reinterpret_cast<getter_t *>(GetProcAddress(module, "SteamApps"))();
    ISteamMatchmaking_ptr = reinterpret_cast<getter_t *>(
        GetProcAddress(module, "SteamMatchmaking"))();
    ISteamMatchmakingServers_ptr = reinterpret_cast<getter_t *>(
        GetProcAddress(module, "SteamMatchmakingServers"))();
    if (ver >= 0x00010062001F0049) { // 01.98.31.73
      /// ISteamUGC appeared only in Steamworks SDK v1.26
      ISteamUGC_ptr =
          reinterpret_cast<getter_t *>(GetProcAddress(module, "SteamUGC"))();
    } else {
      ISteamUGC_ptr = nullptr;
    }
    ISteamUser_ptr =
        reinterpret_cast<getter_t *>(GetProcAddress(module, "SteamUser"))();
    ISteamUtils_ptr =
        reinterpret_cast<getter_t *>(GetProcAddress(module, "SteamUtils"))();
  } // if (ver >= 0x0003002A003D0042) else
  // Setup interface wrappers based on current version
  // ISteamApps
  if (ver >= 0x0003002A003D0042) { // 03.42.61.66
    // "STEAMAPPS_INTERFACE_VERSION008", used since Steamworks SDK v1.37
    ISteamApps_desc.num_methods = 33;
  } else if (ver >= 0x0002003B0033002B) { // 02.59.51.43
    // "STEAMAPPS_INTERFACE_VERSION007", used since Steamworks SDK v1.32
    ISteamApps_desc.num_methods = 24;
  } else if (ver >= 0x00010062001F0049) { // 01.98.31.73
    // "STEAMAPPS_INTERFACE_VERSION006", used since Steamworks SDK v1.26
    ISteamApps_desc.num_methods = 22;
  } else if (ver >= 0x0001001E0032002E) { // 01.30.50.46
    // "STEAMAPPS_INTERFACE_VERSION005", used since Steamworks SDK v1.18
    ISteamApps_desc.num_methods = 20;
  } else if (ver >= 0x0000006000210030) { // 00.96.33.48
    // "STEAMAPPS_INTERFACE_VERSION004", used since Steamworks SDK v1.12
    ISteamApps_desc.num_methods = 14;
  } else {
    // "STEAMAPPS_INTERFACE_VERSION003", used in Steamworks SDK v1.11
    ISteamApps_desc.num_methods = 8;
  }
  ISteamApps_desc.orig_vtable = ISteamApps_ptr->vtable;
  ISteamApps_desc.iface = ISteamApps_ptr;
  std::ranges::copy_n(ISteamApps_ptr->vtable, ISteamApps_desc.num_methods,
                      ISteamApps_desc.vtable.begin());
  ISteamApps_ptr->vtable = ISteamApps_desc.vtable.data();
  for (std::size_t i{}; i < ISteamApps_desc.num_methods; ++i) {
    ISteamApps_desc.vm_idxs[i] = i;
  }
  // ISteamMatchamking
  if (ver >= 0x00010017002D005D) { // 01.23.45.93
    // "SteamMatchMaking009", used since Steamworks SDK v1.17
    ISteamMatchmaking_desc.num_methods = 38;
    for (std::size_t i{}; i < ISteamMatchmaking_desc.num_methods; ++i) {
      ISteamMatchmaking_desc.vm_idxs[i] = i;
    }
  } else {
    // "SteamMatchMaking008", used in older supported Steamworks SDK versions
    ISteamMatchmaking_desc.num_methods = 36;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_GetFavoriteGameCount] =
        0;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_GetFavoriteGame] = 1;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_AddFavoriteGame] = 2;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_RemoveFavoriteGame] = 3;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_RequestLobbyList] = 4;
    ISteamMatchmaking_desc
        .vm_idxs[ISteamMatchmaking_m_AddRequestLobbyListStringFilter] = 5;
    ISteamMatchmaking_desc
        .vm_idxs[ISteamMatchmaking_m_AddRequestLobbyListNumericalFilter] = 6;
    ISteamMatchmaking_desc
        .vm_idxs[ISteamMatchmaking_m_AddRequestLobbyListNearValueFilter] = 7;
    ISteamMatchmaking_desc
        .vm_idxs[ISteamMatchmaking_m_AddRequestLobbyListFilterSlotsAvailable] =
        8;
    ISteamMatchmaking_desc
        .vm_idxs[ISteamMatchmaking_m_AddRequestLobbyListDistanceFilter] = 9;
    ISteamMatchmaking_desc
        .vm_idxs[ISteamMatchmaking_m_AddRequestLobbyListResultCountFilter] = 10;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_GetLobbyByIndex] = 11;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_CreateLobby] = 12;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_JoinLobby] = 13;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_LeaveLobby] = 14;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_InviteUserToLobby] = 15;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_GetNumLobbyMembers] = 16;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_GetLobbyMemberByIndex] =
        17;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_GetLobbyData] = 18;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_SetLobbyData] = 19;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_GetLobbyDataCount] = 20;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_GetLobbyDataByIndex] =
        21;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_DeleteLobbyData] = 22;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_GetLobbyMemberData] = 23;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_SetLobbyMemberData] = 24;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_SendLobbyChatMsg] = 25;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_GetLobbyChatEntry] = 26;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_RequestLobbyData] = 27;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_SetLobbyGameServer] = 28;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_GetLobbyGameServer] = 29;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_SetLobbyMemberLimit] =
        30;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_GetLobbyMemberLimit] =
        31;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_SetLobbyType] = 32;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_SetLobbyJoinable] = 33;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_GetLobbyOwner] = 34;
    ISteamMatchmaking_desc.vm_idxs[ISteamMatchmaking_m_SetLobbyOwner] = 35;
  }
  ISteamMatchmaking_desc.orig_vtable = ISteamMatchmaking_ptr->vtable;
  ISteamMatchmaking_desc.iface = ISteamMatchmaking_ptr;
  std::ranges::copy_n(ISteamMatchmaking_ptr->vtable,
                      ISteamMatchmaking_desc.num_methods,
                      ISteamMatchmaking_desc.vtable.begin());
  ISteamMatchmaking_ptr->vtable = ISteamMatchmaking_desc.vtable.data();
  // ISteamMatchmakingServers
  ISteamMatchmakingServers_desc.num_methods = 17;
  ISteamMatchmakingServers_desc.orig_vtable =
      ISteamMatchmakingServers_ptr->vtable;
  ISteamMatchmakingServers_desc.iface = ISteamMatchmakingServers_ptr;
  std::ranges::copy_n(ISteamMatchmakingServers_ptr->vtable,
                      ISteamMatchmakingServers_desc.num_methods,
                      ISteamMatchmakingServers_desc.vtable.begin());
  ISteamMatchmakingServers_ptr->vtable =
      ISteamMatchmakingServers_desc.vtable.data();
  for (std::size_t i = 0; i < ISteamMatchmakingServers_desc.num_methods; ++i) {
    ISteamMatchmakingServers_desc.vm_idxs[i] = i;
  }
  // ISteamUGC
  if (ISteamUGC_ptr) {
    if (ver >= 0x0009003C002C000A) { // 09.60.44.10
      // "STEAMUGC_INTERFACE_VERSION021", used in Steamworks SDK v1.62
      ISteamUGC_desc.num_methods = 96;
      for (std::size_t i = 0; i < ISteamUGC_desc.num_methods; ++i) {
        ISteamUGC_desc.vm_idxs[i] = i;
      }
    } else if (ver >= 0x0008006100630046) { // 08.97.99.70
      // "STEAMUGC_INTERFACE_VERSION020", used since Steamworks SDK v1.60
      ISteamUGC_desc.num_methods = 94;
      for (std::size_t i = 0; i < ISteamUGC_desc.num_methods; ++i) {
        ISteamUGC_desc.vm_idxs[i] = i;
      }
    } else if (ver >= 0x0008002100090017) { // 08.33.09.23
      // "STEAMUGC_INTERFACE_VERSION018", used since Steamworks SDK v1.58
      ISteamUGC_desc.num_methods = 90;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUserUGCRequest] = 0;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestCursor] = 1;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestPage] = 2;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUGCDetailsRequest] = 3;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SendQueryUGCRequest] = 4;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCResult] = 5;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumTags] = 6;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCTag] = 7;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCTagDisplayName] = 8;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCPreviewURL] = 9;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCMetadata] = 10;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCChildren] = 11;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCStatistic] = 12;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumAdditionalPreviews] = 13;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCAdditionalPreview] = 14;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumKeyValueTags] = 15;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryFirstUGCKeyValueTag] = 16;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCKeyValueTag] = 17;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCContentDescriptors] = 18;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ReleaseQueryUGCRequest] = 19;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTag] = 20;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTagGroup] = 21;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddExcludedTag] = 22;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnOnlyIDs] = 23;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnKeyValueTags] = 24;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnLongDescription] = 25;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnMetadata] = 26;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnChildren] = 27;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnAdditionalPreviews] = 28;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnTotalOnly] = 29;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnPlaytimeStats] = 30;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetLanguage] = 31;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowCachedResponse] = 32;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetCloudFileNameFilter] = 33;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetMatchAnyTag] = 34;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetSearchText] = 35;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetRankedByTrendDays] = 36;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetTimeCreatedDateRange] = 37;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetTimeUpdatedDateRange] = 38;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredKeyValueTag] = 39;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RequestUGCDetails] = 40;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateItem] = 41;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartItemUpdate] = 42;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTitle] = 43;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemDescription] = 44;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemUpdateLanguage] = 45;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemMetadata] = 46;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemVisibility] = 47;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTags] = 48;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemContent] = 49;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemPreview] = 50;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowLegacyUpload] = 51;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveAllItemKeyValueTags] = 52;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemKeyValueTags] = 53;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemKeyValueTag] = 54;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewFile] = 55;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewVideo] = 56;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewFile] = 57;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewVideo] = 58;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemPreview] = 59;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddContentDescriptor] = 60;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveContentDescriptor] = 61;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubmitItemUpdate] = 62;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemUpdateProgress] = 63;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetUserItemVote] = 64;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetUserItemVote] = 65;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemToFavorites] = 66;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemFromFavorites] = 67;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubscribeItem] = 68;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UnsubscribeItem] = 69;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetNumSubscribedItems] = 70;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetSubscribedItems] = 71;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemState] = 72;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemInstallInfo] = 73;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemDownloadInfo] = 74;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DownloadItem] = 75;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_BInitWorkshopForGameServer] = 76;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SuspendDownloads] = 77;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartPlaytimeTracking] = 78;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTracking] = 79;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTrackingForAllItems] = 80;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddDependency] = 81;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveDependency] = 82;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddAppDependency] = 83;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveAppDependency] = 84;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetAppDependencies] = 85;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DeleteItem] = 86;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ShowWorkshopEULA] = 87;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetWorkshopEULAStatus] = 88;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetUserContentDescriptorPreferences] =
          89;
    } else if (ver >= 0x000700600000002C) { // 07.96.00.44
      // "STEAMUGC_INTERFACE_VERSION017", used since Steamworks SDK v1.56
      ISteamUGC_desc.num_methods = 89;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUserUGCRequest] = 0;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestCursor] = 1;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestPage] = 2;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUGCDetailsRequest] = 3;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SendQueryUGCRequest] = 4;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCResult] = 5;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumTags] = 6;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCTag] = 7;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCTagDisplayName] = 8;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCPreviewURL] = 9;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCMetadata] = 10;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCChildren] = 11;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCStatistic] = 12;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumAdditionalPreviews] = 13;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCAdditionalPreview] = 14;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumKeyValueTags] = 15;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryFirstUGCKeyValueTag] = 16;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCKeyValueTag] = 17;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCContentDescriptors] = 18;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ReleaseQueryUGCRequest] = 19;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTag] = 20;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTagGroup] = 21;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddExcludedTag] = 22;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnOnlyIDs] = 23;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnKeyValueTags] = 24;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnLongDescription] = 25;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnMetadata] = 26;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnChildren] = 27;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnAdditionalPreviews] = 28;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnTotalOnly] = 29;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnPlaytimeStats] = 30;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetLanguage] = 31;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowCachedResponse] = 32;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetCloudFileNameFilter] = 33;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetMatchAnyTag] = 34;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetSearchText] = 35;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetRankedByTrendDays] = 36;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetTimeCreatedDateRange] = 37;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetTimeUpdatedDateRange] = 38;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredKeyValueTag] = 39;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RequestUGCDetails] = 40;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateItem] = 41;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartItemUpdate] = 42;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTitle] = 43;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemDescription] = 44;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemUpdateLanguage] = 45;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemMetadata] = 46;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemVisibility] = 47;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTags] = 48;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemContent] = 49;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemPreview] = 50;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowLegacyUpload] = 51;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveAllItemKeyValueTags] = 52;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemKeyValueTags] = 53;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemKeyValueTag] = 54;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewFile] = 55;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewVideo] = 56;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewFile] = 57;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewVideo] = 58;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemPreview] = 59;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddContentDescriptor] = 60;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveContentDescriptor] = 61;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubmitItemUpdate] = 62;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemUpdateProgress] = 63;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetUserItemVote] = 64;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetUserItemVote] = 65;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemToFavorites] = 66;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemFromFavorites] = 67;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubscribeItem] = 68;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UnsubscribeItem] = 69;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetNumSubscribedItems] = 70;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetSubscribedItems] = 71;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemState] = 72;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemInstallInfo] = 73;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemDownloadInfo] = 74;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DownloadItem] = 75;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_BInitWorkshopForGameServer] = 76;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SuspendDownloads] = 77;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartPlaytimeTracking] = 78;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTracking] = 79;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTrackingForAllItems] = 80;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddDependency] = 81;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveDependency] = 82;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddAppDependency] = 83;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveAppDependency] = 84;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetAppDependencies] = 85;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DeleteItem] = 86;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ShowWorkshopEULA] = 87;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetWorkshopEULAStatus] = 88;
    } else if (ver >= 0x0006005B00150039) { // 06.91.21.57
      // "STEAMUGC_INTERFACE_VERSION016", used since Steamworks SDK v1.53
      ISteamUGC_desc.num_methods = 86;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUserUGCRequest] = 0;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestCursor] = 1;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestPage] = 2;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUGCDetailsRequest] = 3;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SendQueryUGCRequest] = 4;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCResult] = 5;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumTags] = 6;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCTag] = 7;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCTagDisplayName] = 8;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCPreviewURL] = 9;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCMetadata] = 10;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCChildren] = 11;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCStatistic] = 12;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumAdditionalPreviews] = 13;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCAdditionalPreview] = 14;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumKeyValueTags] = 15;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryFirstUGCKeyValueTag] = 16;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCKeyValueTag] = 17;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ReleaseQueryUGCRequest] = 18;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTag] = 19;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTagGroup] = 20;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddExcludedTag] = 21;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnOnlyIDs] = 22;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnKeyValueTags] = 23;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnLongDescription] = 24;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnMetadata] = 25;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnChildren] = 26;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnAdditionalPreviews] = 27;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnTotalOnly] = 28;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnPlaytimeStats] = 29;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetLanguage] = 30;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowCachedResponse] = 31;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetCloudFileNameFilter] = 32;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetMatchAnyTag] = 33;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetSearchText] = 34;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetRankedByTrendDays] = 35;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetTimeCreatedDateRange] = 36;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetTimeUpdatedDateRange] = 37;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredKeyValueTag] = 38;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RequestUGCDetails] = 39;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateItem] = 40;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartItemUpdate] = 41;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTitle] = 42;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemDescription] = 43;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemUpdateLanguage] = 44;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemMetadata] = 45;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemVisibility] = 46;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTags] = 47;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemContent] = 48;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemPreview] = 49;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowLegacyUpload] = 50;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveAllItemKeyValueTags] = 51;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemKeyValueTags] = 52;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemKeyValueTag] = 53;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewFile] = 54;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewVideo] = 55;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewFile] = 56;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewVideo] = 57;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemPreview] = 58;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubmitItemUpdate] = 59;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemUpdateProgress] = 60;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetUserItemVote] = 61;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetUserItemVote] = 62;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemToFavorites] = 63;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemFromFavorites] = 64;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubscribeItem] = 65;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UnsubscribeItem] = 66;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetNumSubscribedItems] = 67;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetSubscribedItems] = 68;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemState] = 69;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemInstallInfo] = 70;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemDownloadInfo] = 71;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DownloadItem] = 72;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_BInitWorkshopForGameServer] = 73;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SuspendDownloads] = 74;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartPlaytimeTracking] = 75;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTracking] = 76;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTrackingForAllItems] = 77;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddDependency] = 78;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveDependency] = 79;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddAppDependency] = 80;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveAppDependency] = 81;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetAppDependencies] = 82;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DeleteItem] = 83;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ShowWorkshopEULA] = 84;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetWorkshopEULAStatus] = 85;
    } else if (ver >= 0x0006001C00120056) { // 06.28.18.86
      // "STEAMUGC_INTERFACE_VERSION015", used since Steamworks SDK v1.51
      ISteamUGC_desc.num_methods = 84;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUserUGCRequest] = 0;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestCursor] = 1;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestPage] = 2;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUGCDetailsRequest] = 3;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SendQueryUGCRequest] = 4;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCResult] = 5;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumTags] = 6;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCTag] = 7;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCTagDisplayName] = 8;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCPreviewURL] = 9;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCMetadata] = 10;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCChildren] = 11;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCStatistic] = 12;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumAdditionalPreviews] = 13;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCAdditionalPreview] = 14;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumKeyValueTags] = 15;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryFirstUGCKeyValueTag] = 16;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCKeyValueTag] = 17;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ReleaseQueryUGCRequest] = 18;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTag] = 19;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTagGroup] = 20;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddExcludedTag] = 21;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnOnlyIDs] = 22;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnKeyValueTags] = 23;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnLongDescription] = 24;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnMetadata] = 25;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnChildren] = 26;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnAdditionalPreviews] = 27;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnTotalOnly] = 28;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnPlaytimeStats] = 29;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetLanguage] = 30;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowCachedResponse] = 31;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetCloudFileNameFilter] = 32;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetMatchAnyTag] = 33;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetSearchText] = 34;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetRankedByTrendDays] = 35;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredKeyValueTag] = 36;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RequestUGCDetails] = 37;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateItem] = 38;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartItemUpdate] = 39;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTitle] = 40;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemDescription] = 41;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemUpdateLanguage] = 42;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemMetadata] = 43;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemVisibility] = 44;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTags] = 45;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemContent] = 46;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemPreview] = 47;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowLegacyUpload] = 48;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveAllItemKeyValueTags] = 49;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemKeyValueTags] = 50;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemKeyValueTag] = 51;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewFile] = 52;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewVideo] = 53;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewFile] = 54;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewVideo] = 55;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemPreview] = 56;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubmitItemUpdate] = 57;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemUpdateProgress] = 58;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetUserItemVote] = 59;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetUserItemVote] = 60;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemToFavorites] = 61;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemFromFavorites] = 62;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubscribeItem] = 63;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UnsubscribeItem] = 64;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetNumSubscribedItems] = 65;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetSubscribedItems] = 66;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemState] = 67;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemInstallInfo] = 68;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemDownloadInfo] = 69;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DownloadItem] = 70;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_BInitWorkshopForGameServer] = 71;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SuspendDownloads] = 72;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartPlaytimeTracking] = 73;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTracking] = 74;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTrackingForAllItems] = 75;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddDependency] = 76;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveDependency] = 77;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddAppDependency] = 78;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveAppDependency] = 79;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetAppDependencies] = 80;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DeleteItem] = 81;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ShowWorkshopEULA] = 82;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetWorkshopEULAStatus] = 83;
    } else if (ver >= 0x000500350021004E) { // 05.53.33.78
      // "STEAMUGC_INTERFACE_VERSION014", used since Steamworks SDK v1.47
      ISteamUGC_desc.num_methods = 79;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUserUGCRequest] = 0;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestCursor] = 1;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestPage] = 2;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUGCDetailsRequest] = 3;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SendQueryUGCRequest] = 4;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCResult] = 5;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCPreviewURL] = 6;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCMetadata] = 7;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCChildren] = 8;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCStatistic] = 9;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumAdditionalPreviews] = 10;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCAdditionalPreview] = 11;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumKeyValueTags] = 12;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryFirstUGCKeyValueTag] = 13;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCKeyValueTag] = 14;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ReleaseQueryUGCRequest] = 15;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTag] = 16;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTagGroup] = 17;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddExcludedTag] = 18;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnOnlyIDs] = 19;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnKeyValueTags] = 20;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnLongDescription] = 21;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnMetadata] = 22;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnChildren] = 23;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnAdditionalPreviews] = 24;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnTotalOnly] = 25;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnPlaytimeStats] = 26;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetLanguage] = 27;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowCachedResponse] = 28;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetCloudFileNameFilter] = 29;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetMatchAnyTag] = 30;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetSearchText] = 31;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetRankedByTrendDays] = 32;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredKeyValueTag] = 33;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RequestUGCDetails] = 34;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateItem] = 35;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartItemUpdate] = 36;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTitle] = 37;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemDescription] = 38;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemUpdateLanguage] = 39;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemMetadata] = 40;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemVisibility] = 41;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTags] = 42;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemContent] = 43;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemPreview] = 44;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowLegacyUpload] = 45;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveAllItemKeyValueTags] = 46;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemKeyValueTags] = 47;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemKeyValueTag] = 48;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewFile] = 49;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewVideo] = 50;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewFile] = 51;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewVideo] = 52;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemPreview] = 53;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubmitItemUpdate] = 54;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemUpdateProgress] = 55;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetUserItemVote] = 56;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetUserItemVote] = 57;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemToFavorites] = 58;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemFromFavorites] = 59;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubscribeItem] = 60;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UnsubscribeItem] = 61;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetNumSubscribedItems] = 62;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetSubscribedItems] = 63;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemState] = 64;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemInstallInfo] = 65;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemDownloadInfo] = 66;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DownloadItem] = 67;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_BInitWorkshopForGameServer] = 68;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SuspendDownloads] = 69;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartPlaytimeTracking] = 70;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTracking] = 71;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTrackingForAllItems] = 72;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddDependency] = 73;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveDependency] = 74;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddAppDependency] = 75;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveAppDependency] = 76;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetAppDependencies] = 77;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DeleteItem] = 78;
    } else if (ver >= 0x000500130026003E) { // 05.19.38.62
      // "STEAMUGC_INTERFACE_VERSION013", used since Steamworks SDK v1.45
      ISteamUGC_desc.num_methods = 78;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUserUGCRequest] = 0;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestCursor] = 1;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestPage] = 2;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUGCDetailsRequest] = 3;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SendQueryUGCRequest] = 4;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCResult] = 5;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCPreviewURL] = 6;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCMetadata] = 7;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCChildren] = 8;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCStatistic] = 9;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumAdditionalPreviews] = 10;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCAdditionalPreview] = 11;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumKeyValueTags] = 12;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryFirstUGCKeyValueTag] = 13;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCKeyValueTag] = 14;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ReleaseQueryUGCRequest] = 15;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTag] = 16;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddExcludedTag] = 17;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnOnlyIDs] = 18;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnKeyValueTags] = 19;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnLongDescription] = 20;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnMetadata] = 21;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnChildren] = 22;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnAdditionalPreviews] = 23;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnTotalOnly] = 24;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnPlaytimeStats] = 25;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetLanguage] = 26;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowCachedResponse] = 27;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetCloudFileNameFilter] = 28;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetMatchAnyTag] = 29;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetSearchText] = 30;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetRankedByTrendDays] = 31;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredKeyValueTag] = 32;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RequestUGCDetails] = 33;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateItem] = 34;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartItemUpdate] = 35;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTitle] = 36;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemDescription] = 37;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemUpdateLanguage] = 38;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemMetadata] = 39;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemVisibility] = 40;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTags] = 41;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemContent] = 42;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemPreview] = 43;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowLegacyUpload] = 44;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveAllItemKeyValueTags] = 45;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemKeyValueTags] = 46;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemKeyValueTag] = 47;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewFile] = 48;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewVideo] = 49;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewFile] = 50;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewVideo] = 51;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemPreview] = 52;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubmitItemUpdate] = 53;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemUpdateProgress] = 54;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetUserItemVote] = 55;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetUserItemVote] = 56;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemToFavorites] = 57;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemFromFavorites] = 58;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubscribeItem] = 59;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UnsubscribeItem] = 60;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetNumSubscribedItems] = 61;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetSubscribedItems] = 62;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemState] = 63;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemInstallInfo] = 64;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemDownloadInfo] = 65;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DownloadItem] = 66;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_BInitWorkshopForGameServer] = 67;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SuspendDownloads] = 68;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartPlaytimeTracking] = 69;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTracking] = 70;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTrackingForAllItems] = 71;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddDependency] = 72;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveDependency] = 73;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddAppDependency] = 74;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveAppDependency] = 75;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetAppDependencies] = 76;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DeleteItem] = 77;
    } else if (ver >= 0x0004005F0014001E) { // 04.95.20.30
      // "STEAMUGC_INTERFACE_VERSION012", used since Steamworks SDK v1.43
      ISteamUGC_desc.num_methods = 76;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUserUGCRequest] = 0;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestCursor] = 1;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestPage] = 2;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUGCDetailsRequest] = 3;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SendQueryUGCRequest] = 4;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCResult] = 5;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCPreviewURL] = 6;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCMetadata] = 7;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCChildren] = 8;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCStatistic] = 9;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumAdditionalPreviews] = 10;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCAdditionalPreview] = 11;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumKeyValueTags] = 12;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCKeyValueTag] = 13;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ReleaseQueryUGCRequest] = 14;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTag] = 15;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddExcludedTag] = 16;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnOnlyIDs] = 17;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnKeyValueTags] = 18;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnLongDescription] = 19;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnMetadata] = 20;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnChildren] = 21;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnAdditionalPreviews] = 22;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnTotalOnly] = 23;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnPlaytimeStats] = 24;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetLanguage] = 25;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowCachedResponse] = 26;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetCloudFileNameFilter] = 27;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetMatchAnyTag] = 28;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetSearchText] = 29;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetRankedByTrendDays] = 30;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredKeyValueTag] = 31;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RequestUGCDetails] = 32;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateItem] = 33;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartItemUpdate] = 34;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTitle] = 35;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemDescription] = 36;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemUpdateLanguage] = 37;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemMetadata] = 38;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemVisibility] = 39;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTags] = 40;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemContent] = 41;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemPreview] = 42;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowLegacyUpload] = 43;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemKeyValueTags] = 44;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemKeyValueTag] = 45;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewFile] = 46;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewVideo] = 47;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewFile] = 48;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewVideo] = 49;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemPreview] = 50;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubmitItemUpdate] = 51;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemUpdateProgress] = 52;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetUserItemVote] = 53;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetUserItemVote] = 54;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemToFavorites] = 55;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemFromFavorites] = 56;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubscribeItem] = 57;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UnsubscribeItem] = 58;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetNumSubscribedItems] = 59;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetSubscribedItems] = 60;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemState] = 61;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemInstallInfo] = 62;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemDownloadInfo] = 63;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DownloadItem] = 64;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_BInitWorkshopForGameServer] = 65;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SuspendDownloads] = 66;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartPlaytimeTracking] = 67;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTracking] = 68;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTrackingForAllItems] = 69;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddDependency] = 70;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveDependency] = 71;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddAppDependency] = 72;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveAppDependency] = 73;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetAppDependencies] = 74;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DeleteItem] = 75;
    } else if (ver >= 0x0003005C0048003A) { // 03.92.72.58
      // "STEAMUGC_INTERFACE_VERSION010", used since Steamworks SDK v1.40
      ISteamUGC_desc.num_methods = 74;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUserUGCRequest] = 0;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestPage] = 1;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUGCDetailsRequest] = 2;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SendQueryUGCRequest] = 3;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCResult] = 4;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCPreviewURL] = 5;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCMetadata] = 6;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCChildren] = 7;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCStatistic] = 8;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumAdditionalPreviews] = 9;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCAdditionalPreview] = 10;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumKeyValueTags] = 11;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCKeyValueTag] = 12;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ReleaseQueryUGCRequest] = 13;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTag] = 14;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddExcludedTag] = 15;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnOnlyIDs] = 16;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnKeyValueTags] = 17;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnLongDescription] = 18;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnMetadata] = 19;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnChildren] = 20;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnAdditionalPreviews] = 21;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnTotalOnly] = 22;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnPlaytimeStats] = 23;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetLanguage] = 24;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowCachedResponse] = 25;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetCloudFileNameFilter] = 26;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetMatchAnyTag] = 27;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetSearchText] = 28;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetRankedByTrendDays] = 29;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredKeyValueTag] = 30;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RequestUGCDetails] = 31;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateItem] = 32;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartItemUpdate] = 33;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTitle] = 34;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemDescription] = 35;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemUpdateLanguage] = 36;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemMetadata] = 37;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemVisibility] = 38;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTags] = 39;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemContent] = 40;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemPreview] = 41;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemKeyValueTags] = 42;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemKeyValueTag] = 43;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewFile] = 44;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewVideo] = 45;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewFile] = 46;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewVideo] = 47;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemPreview] = 48;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubmitItemUpdate] = 49;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemUpdateProgress] = 50;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetUserItemVote] = 51;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetUserItemVote] = 52;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemToFavorites] = 53;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemFromFavorites] = 54;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubscribeItem] = 55;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UnsubscribeItem] = 56;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetNumSubscribedItems] = 57;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetSubscribedItems] = 58;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemState] = 59;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemInstallInfo] = 60;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemDownloadInfo] = 61;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DownloadItem] = 62;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_BInitWorkshopForGameServer] = 63;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SuspendDownloads] = 64;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartPlaytimeTracking] = 65;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTracking] = 66;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTrackingForAllItems] = 67;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddDependency] = 68;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveDependency] = 69;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddAppDependency] = 70;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveAppDependency] = 71;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetAppDependencies] = 72;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DeleteItem] = 73;
    } else if (ver >= 0x0003003E00520052) { // 03.62.82.82
      // "STEAMUGC_INTERFACE_VERSION009", used since Steamworks SDK v1.38
      ISteamUGC_desc.num_methods = 67;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUserUGCRequest] = 0;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestPage] = 1;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUGCDetailsRequest] = 2;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SendQueryUGCRequest] = 3;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCResult] = 4;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCPreviewURL] = 5;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCMetadata] = 6;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCChildren] = 7;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCStatistic] = 8;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumAdditionalPreviews] = 9;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCAdditionalPreview] = 10;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumKeyValueTags] = 11;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCKeyValueTag] = 12;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ReleaseQueryUGCRequest] = 13;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTag] = 14;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddExcludedTag] = 15;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnOnlyIDs] = 16;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnKeyValueTags] = 17;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnLongDescription] = 18;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnMetadata] = 19;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnChildren] = 20;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnAdditionalPreviews] = 21;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnTotalOnly] = 22;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetLanguage] = 23;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowCachedResponse] = 24;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetCloudFileNameFilter] = 25;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetMatchAnyTag] = 26;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetSearchText] = 27;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetRankedByTrendDays] = 28;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredKeyValueTag] = 29;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RequestUGCDetails] = 30;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateItem] = 31;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartItemUpdate] = 32;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTitle] = 33;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemDescription] = 34;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemUpdateLanguage] = 35;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemMetadata] = 36;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemVisibility] = 37;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTags] = 38;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemContent] = 39;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemPreview] = 40;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemKeyValueTags] = 41;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemKeyValueTag] = 42;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewFile] = 43;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewVideo] = 44;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewFile] = 45;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewVideo] = 46;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemPreview] = 47;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubmitItemUpdate] = 48;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemUpdateProgress] = 49;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetUserItemVote] = 50;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetUserItemVote] = 51;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemToFavorites] = 52;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemFromFavorites] = 53;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubscribeItem] = 54;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UnsubscribeItem] = 55;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetNumSubscribedItems] = 56;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetSubscribedItems] = 57;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemState] = 58;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemInstallInfo] = 59;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemDownloadInfo] = 60;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DownloadItem] = 61;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_BInitWorkshopForGameServer] = 62;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SuspendDownloads] = 63;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartPlaytimeTracking] = 64;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTracking] = 65;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StopPlaytimeTrackingForAllItems] = 66;
    } else if (ver >= 0x0003002A003D0042) { // 03.42.61.66
      // "STEAMUGC_INTERFACE_VERSION008", used in Steamworks SDK v1.37
      ISteamUGC_desc.num_methods = 63;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUserUGCRequest] = 0;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestPage] = 1;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUGCDetailsRequest] = 2;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SendQueryUGCRequest] = 3;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCResult] = 4;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCPreviewURL] = 5;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCMetadata] = 6;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCChildren] = 7;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCStatistic] = 8;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumAdditionalPreviews] = 9;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCAdditionalPreview] = 10;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumKeyValueTags] = 11;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCKeyValueTag] = 12;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ReleaseQueryUGCRequest] = 13;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTag] = 14;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddExcludedTag] = 15;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnKeyValueTags] = 16;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnLongDescription] = 17;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnMetadata] = 18;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnChildren] = 19;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnAdditionalPreviews] = 20;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnTotalOnly] = 21;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetLanguage] = 22;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowCachedResponse] = 23;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetCloudFileNameFilter] = 24;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetMatchAnyTag] = 25;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetSearchText] = 26;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetRankedByTrendDays] = 27;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredKeyValueTag] = 28;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RequestUGCDetails] = 29;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateItem] = 30;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartItemUpdate] = 31;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTitle] = 32;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemDescription] = 33;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemUpdateLanguage] = 34;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemMetadata] = 35;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemVisibility] = 36;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTags] = 37;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemContent] = 38;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemPreview] = 39;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemKeyValueTags] = 40;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemKeyValueTag] = 41;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewFile] = 42;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemPreviewVideo] = 43;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewFile] = 44;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UpdateItemPreviewVideo] = 45;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemPreview] = 46;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubmitItemUpdate] = 47;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemUpdateProgress] = 48;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetUserItemVote] = 49;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetUserItemVote] = 50;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemToFavorites] = 51;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemFromFavorites] = 52;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubscribeItem] = 53;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UnsubscribeItem] = 54;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetNumSubscribedItems] = 55;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetSubscribedItems] = 56;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemState] = 57;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemInstallInfo] = 58;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemDownloadInfo] = 59;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DownloadItem] = 60;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_BInitWorkshopForGameServer] = 61;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SuspendDownloads] = 62;
    } else if (ver >= 0x00020059002D0004) { // 02.89.45.04
      // "STEAMUGC_INTERFACE_VERSION007", used since Steamworks SDK v1.34
      ISteamUGC_desc.num_methods = 58;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUserUGCRequest] = 0;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestPage] = 1;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUGCDetailsRequest] = 2;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SendQueryUGCRequest] = 3;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCResult] = 4;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCPreviewURL] = 5;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCMetadata] = 6;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCChildren] = 7;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCStatistic] = 8;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumAdditionalPreviews] = 9;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCAdditionalPreview] = 10;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumKeyValueTags] = 11;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCKeyValueTag] = 12;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ReleaseQueryUGCRequest] = 13;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTag] = 14;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddExcludedTag] = 15;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnKeyValueTags] = 16;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnLongDescription] = 17;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnMetadata] = 18;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnChildren] = 19;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnAdditionalPreviews] = 20;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnTotalOnly] = 21;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetLanguage] = 22;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowCachedResponse] = 23;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetCloudFileNameFilter] = 24;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetMatchAnyTag] = 25;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetSearchText] = 26;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetRankedByTrendDays] = 27;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredKeyValueTag] = 28;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RequestUGCDetails] = 29;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateItem] = 30;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartItemUpdate] = 31;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTitle] = 32;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemDescription] = 33;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemUpdateLanguage] = 34;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemMetadata] = 35;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemVisibility] = 36;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTags] = 37;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemContent] = 38;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemPreview] = 39;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemKeyValueTags] = 40;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemKeyValueTag] = 41;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubmitItemUpdate] = 42;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemUpdateProgress] = 43;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetUserItemVote] = 44;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetUserItemVote] = 45;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemToFavorites] = 46;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemFromFavorites] = 47;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubscribeItem] = 48;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UnsubscribeItem] = 49;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetNumSubscribedItems] = 50;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetSubscribedItems] = 51;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemState] = 52;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemInstallInfo] = 53;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemDownloadInfo] = 54;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DownloadItem] = 55;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_BInitWorkshopForGameServer] = 56;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SuspendDownloads] = 57;
    } else if (ver >= 0x0002004D00250052) { // 02.77.37.82
      // "STEAMUGC_INTERFACE_VERSION005", used in Steamworks SDK v1.33
      ISteamUGC_desc.num_methods = 46;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUserUGCRequest] = 0;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestPage] = 1;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUGCDetailsRequest] = 2;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SendQueryUGCRequest] = 3;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCResult] = 4;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCPreviewURL] = 5;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCMetadata] = 6;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCChildren] = 7;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCStatistic] = 8;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCNumAdditionalPreviews] = 9;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCAdditionalPreview] = 10;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ReleaseQueryUGCRequest] = 11;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTag] = 12;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddExcludedTag] = 13;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnLongDescription] = 14;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnMetadata] = 15;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnChildren] = 16;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnAdditionalPreviews] = 17;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnTotalOnly] = 18;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowCachedResponse] = 19;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetCloudFileNameFilter] = 20;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetMatchAnyTag] = 21;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetSearchText] = 22;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetRankedByTrendDays] = 23;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RequestUGCDetails] = 24;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateItem] = 25;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartItemUpdate] = 26;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTitle] = 27;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemDescription] = 28;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemMetadata] = 29;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemVisibility] = 30;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTags] = 31;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemContent] = 32;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemPreview] = 33;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubmitItemUpdate] = 34;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemUpdateProgress] = 35;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddItemToFavorites] = 36;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RemoveItemFromFavorites] = 37;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubscribeItem] = 38;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UnsubscribeItem] = 39;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetNumSubscribedItems] = 40;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetSubscribedItems] = 41;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemState] = 42;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemInstallInfo] = 43;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemDownloadInfo] = 44;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_DownloadItem] = 45;
    } else if (ver >= 0x000200130022005D) { // 02.19.34.93
      // "STEAMUGC_INTERFACE_VERSION002" and "STEAMUGC_INTERFACE_VERSION003",
      //    used since Steamworks SDK v1.29
      ISteamUGC_desc.num_methods = 31;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUserUGCRequest] = 0;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestPage] = 1;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SendQueryUGCRequest] = 2;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCResult] = 3;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ReleaseQueryUGCRequest] = 4;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTag] = 5;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddExcludedTag] = 6;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnLongDescription] = 7;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnTotalOnly] = 8;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetAllowCachedResponse] = 9;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetCloudFileNameFilter] = 10;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetMatchAnyTag] = 11;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetSearchText] = 12;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetRankedByTrendDays] = 13;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RequestUGCDetails] = 14;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateItem] = 15;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_StartItemUpdate] = 16;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTitle] = 17;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemDescription] = 18;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemVisibility] = 19;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemTags] = 20;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemContent] = 21;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetItemPreview] = 22;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubmitItemUpdate] = 23;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemUpdateProgress] = 24;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SubscribeItem] = 25;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_UnsubscribeItem] = 26;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetNumSubscribedItems] = 27;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetSubscribedItems] = 28;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemInstallInfo] = 29;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetItemUpdateInfo] = 30;
    } else {
      // "STEAMUGC_INTERFACE_VERSION001"
      ISteamUGC_desc.num_methods = 14;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryUserUGCRequest] = 0;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_CreateQueryAllUGCRequestPage] = 1;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SendQueryUGCRequest] = 2;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_GetQueryUGCResult] = 3;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_ReleaseQueryUGCRequest] = 4;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddRequiredTag] = 5;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_AddExcludedTag] = 6;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnLongDescription] = 7;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetReturnTotalOnly] = 8;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetCloudFileNameFilter] = 9;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetMatchAnyTag] = 10;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetSearchText] = 11;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_SetRankedByTrendDays] = 12;
      ISteamUGC_desc.vm_idxs[ISteamUGC_m_RequestUGCDetails] = 13;
    }
    ISteamUGC_desc.orig_vtable = ISteamUGC_ptr->vtable;
    ISteamUGC_desc.iface = ISteamUGC_ptr;
    std::ranges::copy_n(ISteamUGC_ptr->vtable, ISteamUGC_desc.num_methods,
                        ISteamUGC_desc.vtable.begin());
    ISteamUGC_ptr->vtable = ISteamUGC_desc.vtable.data();
  } // if (ISteamUGC_ptr)
  // ISteamUser
  if (ver >= 0x000800020015005F) { // 08.02.21.95
    // "SteamUser023", used since Steamworks SDK v1.57
    ISteamUser_desc.num_methods = 33;
    for (std::size_t i{}; i < ISteamUser_desc.num_methods; ++i) {
      ISteamUser_desc.vm_idxs[i] = i;
    }
  } else if (ver >= 0x0005005C0024004B) { // 05.92.36.75
    // "SteamUser021" & "SteamUser022", used since Steamworks SDK v1.49
    ISteamUser_desc.num_methods = 32;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetHSteamUser] = 0;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BLoggedOn] = 1;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetSteamID] = 2;
    ISteamUser_desc.vm_idxs[ISteamUser_m_InitiateGameConnection] = 3;
    ISteamUser_desc.vm_idxs[ISteamUser_m_TerminateGameConnection] = 4;
    ISteamUser_desc.vm_idxs[ISteamUser_m_TrackAppUsageEvent] = 5;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetUserDataFolder] = 6;
    ISteamUser_desc.vm_idxs[ISteamUser_m_StartVoiceRecording] = 7;
    ISteamUser_desc.vm_idxs[ISteamUser_m_StopVoiceRecording] = 8;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetAvailableVoice] = 9;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetVoice] = 10;
    ISteamUser_desc.vm_idxs[ISteamUser_m_DecompressVoice] = 11;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetVoiceOptimalSampleRate] = 12;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetAuthSessionTicket] = 13;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BeginAuthSession] = 14;
    ISteamUser_desc.vm_idxs[ISteamUser_m_EndAuthSession] = 15;
    ISteamUser_desc.vm_idxs[ISteamUser_m_CancelAuthTicket] = 16;
    ISteamUser_desc.vm_idxs[ISteamUser_m_UserHasLicenseForApp] = 17;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsBehindNAT] = 18;
    ISteamUser_desc.vm_idxs[ISteamUser_m_AdvertiseGame] = 19;
    ISteamUser_desc.vm_idxs[ISteamUser_m_RequestEncryptedAppTicket] = 20;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetEncryptedAppTicket] = 21;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetGameBadgeLevel] = 22;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetPlayerSteamLevel] = 23;
    ISteamUser_desc.vm_idxs[ISteamUser_m_RequestStoreAuthURL] = 24;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsPhoneVerified] = 25;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsTwoFactorEnabled] = 26;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsPhoneIdentifying] = 27;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsPhoneRequiringVerification] = 28;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetMarketEligibility] = 29;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetDurationControl] = 30;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BSetDurationControlOnlineState] = 31;
  } else if (ver >= 0x0004005F0014001E) { // 04.95.20.30
    // "SteamUser020", used since Steamworks SDK v1.43
    ISteamUser_desc.num_methods = 31;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetHSteamUser] = 0;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BLoggedOn] = 1;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetSteamID] = 2;
    ISteamUser_desc.vm_idxs[ISteamUser_m_InitiateGameConnection] = 3;
    ISteamUser_desc.vm_idxs[ISteamUser_m_TerminateGameConnection] = 4;
    ISteamUser_desc.vm_idxs[ISteamUser_m_TrackAppUsageEvent] = 5;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetUserDataFolder] = 6;
    ISteamUser_desc.vm_idxs[ISteamUser_m_StartVoiceRecording] = 7;
    ISteamUser_desc.vm_idxs[ISteamUser_m_StopVoiceRecording] = 8;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetAvailableVoice] = 9;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetVoice] = 10;
    ISteamUser_desc.vm_idxs[ISteamUser_m_DecompressVoice] = 11;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetVoiceOptimalSampleRate] = 12;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetAuthSessionTicket] = 13;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BeginAuthSession] = 14;
    ISteamUser_desc.vm_idxs[ISteamUser_m_EndAuthSession] = 15;
    ISteamUser_desc.vm_idxs[ISteamUser_m_CancelAuthTicket] = 16;
    ISteamUser_desc.vm_idxs[ISteamUser_m_UserHasLicenseForApp] = 17;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsBehindNAT] = 18;
    ISteamUser_desc.vm_idxs[ISteamUser_m_AdvertiseGame] = 19;
    ISteamUser_desc.vm_idxs[ISteamUser_m_RequestEncryptedAppTicket] = 20;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetEncryptedAppTicket] = 21;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetGameBadgeLevel] = 22;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetPlayerSteamLevel] = 23;
    ISteamUser_desc.vm_idxs[ISteamUser_m_RequestStoreAuthURL] = 24;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsPhoneVerified] = 25;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsTwoFactorEnabled] = 26;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsPhoneIdentifying] = 27;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsPhoneRequiringVerification] = 28;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetMarketEligibility] = 29;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetDurationControl] = 30;
  } else if (ver >= 0x0003002A003D0042) { // 03.42.61.66
    // "SteamUser019", used since Steamworks SDK v1.37
    ISteamUser_desc.num_methods = 29;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetHSteamUser] = 0;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BLoggedOn] = 1;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetSteamID] = 2;
    ISteamUser_desc.vm_idxs[ISteamUser_m_InitiateGameConnection] = 3;
    ISteamUser_desc.vm_idxs[ISteamUser_m_TerminateGameConnection] = 4;
    ISteamUser_desc.vm_idxs[ISteamUser_m_TrackAppUsageEvent] = 5;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetUserDataFolder] = 6;
    ISteamUser_desc.vm_idxs[ISteamUser_m_StartVoiceRecording] = 7;
    ISteamUser_desc.vm_idxs[ISteamUser_m_StopVoiceRecording] = 8;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetAvailableVoice] = 9;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetVoice] = 10;
    ISteamUser_desc.vm_idxs[ISteamUser_m_DecompressVoice] = 11;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetVoiceOptimalSampleRate] = 12;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetAuthSessionTicket] = 13;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BeginAuthSession] = 14;
    ISteamUser_desc.vm_idxs[ISteamUser_m_EndAuthSession] = 15;
    ISteamUser_desc.vm_idxs[ISteamUser_m_CancelAuthTicket] = 16;
    ISteamUser_desc.vm_idxs[ISteamUser_m_UserHasLicenseForApp] = 17;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsBehindNAT] = 18;
    ISteamUser_desc.vm_idxs[ISteamUser_m_AdvertiseGame] = 19;
    ISteamUser_desc.vm_idxs[ISteamUser_m_RequestEncryptedAppTicket] = 20;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetEncryptedAppTicket] = 21;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetGameBadgeLevel] = 22;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetPlayerSteamLevel] = 23;
    ISteamUser_desc.vm_idxs[ISteamUser_m_RequestStoreAuthURL] = 24;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsPhoneVerified] = 25;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsTwoFactorEnabled] = 26;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsPhoneIdentifying] = 27;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsPhoneRequiringVerification] = 28;
  } else if (ver >= 0x0002003B0033002B) { // 02.59.51.43
    // "SteamUser018", used since Steamworks SDK v1.32
    ISteamUser_desc.num_methods = 25;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetHSteamUser] = 0;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BLoggedOn] = 1;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetSteamID] = 2;
    ISteamUser_desc.vm_idxs[ISteamUser_m_InitiateGameConnection] = 3;
    ISteamUser_desc.vm_idxs[ISteamUser_m_TerminateGameConnection] = 4;
    ISteamUser_desc.vm_idxs[ISteamUser_m_TrackAppUsageEvent] = 5;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetUserDataFolder] = 6;
    ISteamUser_desc.vm_idxs[ISteamUser_m_StartVoiceRecording] = 7;
    ISteamUser_desc.vm_idxs[ISteamUser_m_StopVoiceRecording] = 8;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetAvailableVoice] = 9;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetVoice] = 10;
    ISteamUser_desc.vm_idxs[ISteamUser_m_DecompressVoice] = 11;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetVoiceOptimalSampleRate] = 12;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetAuthSessionTicket] = 13;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BeginAuthSession] = 14;
    ISteamUser_desc.vm_idxs[ISteamUser_m_EndAuthSession] = 15;
    ISteamUser_desc.vm_idxs[ISteamUser_m_CancelAuthTicket] = 16;
    ISteamUser_desc.vm_idxs[ISteamUser_m_UserHasLicenseForApp] = 17;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsBehindNAT] = 18;
    ISteamUser_desc.vm_idxs[ISteamUser_m_AdvertiseGame] = 19;
    ISteamUser_desc.vm_idxs[ISteamUser_m_RequestEncryptedAppTicket] = 20;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetEncryptedAppTicket] = 21;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetGameBadgeLevel] = 22;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetPlayerSteamLevel] = 23;
    ISteamUser_desc.vm_idxs[ISteamUser_m_RequestStoreAuthURL] = 24;
  } else if (ver >= 0x00010053001F0025) { // 01.83.31.37
    // "SteamUser017", used since Steamworks SDK v1.25
    ISteamUser_desc.num_methods = 24;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetHSteamUser] = 0;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BLoggedOn] = 1;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetSteamID] = 2;
    ISteamUser_desc.vm_idxs[ISteamUser_m_InitiateGameConnection] = 3;
    ISteamUser_desc.vm_idxs[ISteamUser_m_TerminateGameConnection] = 4;
    ISteamUser_desc.vm_idxs[ISteamUser_m_TrackAppUsageEvent] = 5;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetUserDataFolder] = 6;
    ISteamUser_desc.vm_idxs[ISteamUser_m_StartVoiceRecording] = 7;
    ISteamUser_desc.vm_idxs[ISteamUser_m_StopVoiceRecording] = 8;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetAvailableVoice] = 9;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetVoice] = 10;
    ISteamUser_desc.vm_idxs[ISteamUser_m_DecompressVoice] = 11;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetVoiceOptimalSampleRate] = 12;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetAuthSessionTicket] = 13;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BeginAuthSession] = 14;
    ISteamUser_desc.vm_idxs[ISteamUser_m_EndAuthSession] = 15;
    ISteamUser_desc.vm_idxs[ISteamUser_m_CancelAuthTicket] = 16;
    ISteamUser_desc.vm_idxs[ISteamUser_m_UserHasLicenseForApp] = 17;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsBehindNAT] = 18;
    ISteamUser_desc.vm_idxs[ISteamUser_m_AdvertiseGame] = 19;
    ISteamUser_desc.vm_idxs[ISteamUser_m_RequestEncryptedAppTicket] = 20;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetEncryptedAppTicket] = 21;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetGameBadgeLevel] = 22;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetPlayerSteamLevel] = 23;
  } else if (ver >= 0x000100060063003D) { // 01.06.99.61
    // "SteamUser016", used since Steamworks SDK v1.13
    ISteamUser_desc.num_methods = 22;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetHSteamUser] = 0;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BLoggedOn] = 1;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetSteamID] = 2;
    ISteamUser_desc.vm_idxs[ISteamUser_m_InitiateGameConnection] = 3;
    ISteamUser_desc.vm_idxs[ISteamUser_m_TerminateGameConnection] = 4;
    ISteamUser_desc.vm_idxs[ISteamUser_m_TrackAppUsageEvent] = 5;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetUserDataFolder] = 6;
    ISteamUser_desc.vm_idxs[ISteamUser_m_StartVoiceRecording] = 7;
    ISteamUser_desc.vm_idxs[ISteamUser_m_StopVoiceRecording] = 8;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetAvailableVoice] = 9;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetVoice] = 10;
    ISteamUser_desc.vm_idxs[ISteamUser_m_DecompressVoice] = 11;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetVoiceOptimalSampleRate] = 12;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetAuthSessionTicket] = 13;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BeginAuthSession] = 14;
    ISteamUser_desc.vm_idxs[ISteamUser_m_EndAuthSession] = 15;
    ISteamUser_desc.vm_idxs[ISteamUser_m_CancelAuthTicket] = 16;
    ISteamUser_desc.vm_idxs[ISteamUser_m_UserHasLicenseForApp] = 17;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsBehindNAT] = 18;
    ISteamUser_desc.vm_idxs[ISteamUser_m_AdvertiseGame] = 19;
    ISteamUser_desc.vm_idxs[ISteamUser_m_RequestEncryptedAppTicket] = 20;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetEncryptedAppTicket] = 21;
  } else {
    // "SteamUser014", used in older supported Steamworks SDK versions
    ISteamUser_desc.num_methods = 21;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetHSteamUser] = 0;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BLoggedOn] = 1;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetSteamID] = 2;
    ISteamUser_desc.vm_idxs[ISteamUser_m_InitiateGameConnection] = 3;
    ISteamUser_desc.vm_idxs[ISteamUser_m_TerminateGameConnection] = 4;
    ISteamUser_desc.vm_idxs[ISteamUser_m_TrackAppUsageEvent] = 5;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetUserDataFolder] = 6;
    ISteamUser_desc.vm_idxs[ISteamUser_m_StartVoiceRecording] = 7;
    ISteamUser_desc.vm_idxs[ISteamUser_m_StopVoiceRecording] = 8;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetAvailableVoice] = 9;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetVoice] = 10;
    ISteamUser_desc.vm_idxs[ISteamUser_m_DecompressVoice] = 11;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetAuthSessionTicket] = 12;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BeginAuthSession] = 13;
    ISteamUser_desc.vm_idxs[ISteamUser_m_EndAuthSession] = 14;
    ISteamUser_desc.vm_idxs[ISteamUser_m_CancelAuthTicket] = 15;
    ISteamUser_desc.vm_idxs[ISteamUser_m_UserHasLicenseForApp] = 16;
    ISteamUser_desc.vm_idxs[ISteamUser_m_BIsBehindNAT] = 17;
    ISteamUser_desc.vm_idxs[ISteamUser_m_AdvertiseGame] = 18;
    ISteamUser_desc.vm_idxs[ISteamUser_m_RequestEncryptedAppTicket] = 19;
    ISteamUser_desc.vm_idxs[ISteamUser_m_GetEncryptedAppTicket] = 20;
  }
  ISteamUser_desc.orig_vtable = ISteamUser_ptr->vtable;
  ISteamUser_desc.iface = ISteamUser_ptr;
  std::ranges::copy_n(ISteamUser_ptr->vtable, ISteamUser_desc.num_methods,
                      ISteamUser_desc.vtable.begin());
  ISteamUser_ptr->vtable = ISteamUser_desc.vtable.data();
  // ISteamUtils
  if (ver >= 0x000600060063003B) { // 06.06.99.59
    // "SteamUtils010", used since Steamworks SDK v1.50
    ISteamUtils_desc.num_methods = 39;
  } else if (ver >= 0x0003005C0048003A) { // 03.92.72.58
    // "SteamUtils009", used since Steamworks SDK v1.40
    ISteamUtils_desc.num_methods = 34;
  } else if (ver >= 0x0003002A003D0042) { // 03.42.61.66
    // "SteamUtils008", used since Steamworks SDK v1.37
    ISteamUtils_desc.num_methods = 28;
  } else if (ver >= 0x000200130022005D) { // 02.19.34.93
    // "SteamUtils007", used since Steamworks SDK v1.29
    ISteamUtils_desc.num_methods = 26;
  } else if (ver >= 0x00010053001F0025) { // 01.83.31.37
    // "SteamUtils006", used since Steamworks SDK v1.25
    ISteamUtils_desc.num_methods = 25;
  } else {
    // "SteamUtils005", used in older supported Steamworks SDK versions
    ISteamUtils_desc.num_methods = 23;
  }
  ISteamUtils_desc.orig_vtable = ISteamUtils_ptr->vtable;
  ISteamUtils_desc.iface = ISteamUtils_ptr;
  std::ranges::copy_n(ISteamUtils_ptr->vtable, ISteamUtils_desc.num_methods,
                      ISteamUtils_desc.vtable.begin());
  ISteamUtils_ptr->vtable = ISteamUtils_desc.vtable.data();
  for (std::size_t i{}; i < ISteamUtils_desc.num_methods; ++i) {
    ISteamUtils_desc.vm_idxs[i] = i;
  }
  // Get current user Steam ID
  reinterpret_cast<ISteamUser_GetSteamID_t *>(
      ISteamUser_desc
          .orig_vtable[ISteamUser_desc.vm_idxs[ISteamUser_m_GetSteamID]])(
      ISteamUser_ptr, &steam_id);
  // Setup common function wrappers
  ISteamApps_desc.vtable[ISteamApps_desc.vm_idxs[ISteamApps_m_BIsSubscribed]] =
      reinterpret_cast<void *>(SteamApps_BIsSubscribed);
  SteamApps_BIsSubscribedApp_orig = reinterpret_cast<
      ISteamApps_BIsSubscribedApp_t *>(
      ISteamApps_desc
          .orig_vtable[ISteamApps_desc.vm_idxs[ISteamApps_m_BIsSubscribedApp]]);
  ISteamApps_desc
      .vtable[ISteamApps_desc.vm_idxs[ISteamApps_m_BIsSubscribedApp]] =
      reinterpret_cast<void *>(SteamApps_BIsSubscribedApp);
  ISteamApps_desc
      .vtable[ISteamApps_desc.vm_idxs[ISteamApps_m_BIsDlcInstalled]] =
      reinterpret_cast<void *>(SteamApps_BIsDlcInstalled);
  int idx;
  idx = ISteamApps_desc.vm_idxs[ISteamApps_m_BIsSubscribedFromFreeWeekend];
  if (idx >= 0) {
    ISteamApps_desc.vtable[idx] =
        reinterpret_cast<void *>(SteamApps_BIsSubscribedFromFreeWeekend);
  }
  idx = ISteamApps_desc.vm_idxs[ISteamApps_m_GetDLCCount];
  if (idx >= 0) {
    ISteamApps_desc.vtable[idx] =
        reinterpret_cast<void *>(SteamApps_GetDLCCount);
  }
  idx = ISteamApps_desc.vm_idxs[ISteamApps_m_BGetDLCDataByIndex];
  if (idx >= 0) {
    ISteamApps_desc.vtable[idx] =
        reinterpret_cast<void *>(SteamApps_BGetDLCDataByIndex);
  }
  idx = ISteamApps_desc.vm_idxs[ISteamApps_m_BIsAppInstalled];
  if (idx >= 0) {
    SteamApps_BIsAppInstalled_orig =
        reinterpret_cast<ISteamApps_BIsAppInstalled_t *>(
            ISteamApps_desc.orig_vtable[idx]);
    ISteamApps_desc.vtable[idx] =
        reinterpret_cast<void *>(SteamApps_BIsAppInstalled);
  }
  idx = ISteamApps_desc.vm_idxs[ISteamApps_m_GetAppOwner];
  if (idx >= 0) {
    ISteamApps_desc.vtable[idx] =
        reinterpret_cast<void *>(SteamApps_GetAppOwner);
  }
  idx = ISteamApps_desc.vm_idxs[ISteamApps_m_BIsSubscribedFromFamilySharing];
  if (idx >= 0) {
    ISteamApps_desc.vtable[idx] =
        reinterpret_cast<void *>(SteamApps_BIsSubscribedFromFamilySharing);
  }
  idx = ISteamApps_desc.vm_idxs[ISteamApps_m_BIsTimedTrial];
  if (idx >= 0) {
    ISteamApps_desc.vtable[idx] =
        reinterpret_cast<void *>(SteamApps_BIsTimedTrial);
  }
  ISteamUser_desc
      .vtable[ISteamUser_desc.vm_idxs[ISteamUser_m_UserHasLicenseForApp]] =
      reinterpret_cast<void *>(SteamUser_UserHasLicenseForApp);
  ISteamUtils_desc.vtable[ISteamUtils_desc.vm_idxs[ISteamUtils_m_GetAppID]] =
      reinterpret_cast<void *>(SteamUtils_GetAppID);
  if (g_settings.steam->auto_update_dlc) {
    // Attempt to load tek-steamclient and update the DLC list.
    steamclient::load();
    if (steamclient::loaded) {
      steamclient::update_dlc();
    }
  }
  // Perform game-specific setup
  {
    const auto cb{get_steam_api_init_cb()};
    if (cb) {
      cb();
    }
  }
  return true;
version_fail:
  display_error(L"Couldn't load steam_api64.dll file version, no changes will "
                L"be applied");
  return false;
}

} // namespace

void wrap_init() {
  const auto module{reinterpret_cast<char *>(GetModuleHandleW(nullptr))};
  SteamAPI_Init_t **thunk_ptr{};
  // First, try to locate regular import descriptor for steam_api64.dll
  ULONG dir_size;
  const auto import_desc_base{reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR *>(
      ImageDirectoryEntryToDataEx(module, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT,
                                  &dir_size, nullptr))};
  if (import_desc_base) {
    const std::span import_descs{import_desc_base,
                                 (dir_size / sizeof *import_desc_base) - 1};
    const auto import_desc{std::ranges::find(
        import_descs, "steam_api64.dll", [module](const auto &desc) {
          return std::string_view{&module[desc.Name]};
        })};
    if (import_desc != import_descs.end()) {
      for (auto ilt_desc_base{reinterpret_cast<const IMAGE_THUNK_DATA *>(
               &module[import_desc->OriginalFirstThunk])},
           ilt_desc{ilt_desc_base};
           ilt_desc->u1.AddressOfData; ++ilt_desc) {
        if (!(ilt_desc->u1.AddressOfData & IMAGE_ORDINAL_FLAG) &&
            std::string_view{reinterpret_cast<const IMAGE_IMPORT_BY_NAME *>(
                                 &module[ilt_desc->u1.AddressOfData])
                                 ->Name} == "SteamAPI_Init") {
          thunk_ptr = reinterpret_cast<SteamAPI_Init_t **>(&(
              reinterpret_cast<IMAGE_THUNK_DATA *>(
                  &module[import_desc->FirstThunk])[std::distance(ilt_desc_base,
                                                                  ilt_desc)]
                  .u1.Function));
          break;
        }
      }
    } // if (import_desc != import_descs.end())
  } // if (import_desc_base)
  if (!thunk_ptr) {
    // Try to locate delay load descriptor for steam_api64.dll
    const auto delay_load_desc_base{
        reinterpret_cast<const IMAGE_DELAYLOAD_DESCRIPTOR *>(
            ImageDirectoryEntryToDataEx(const_cast<char *>(module), TRUE,
                                        IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT,
                                        &dir_size, nullptr))};
    if (delay_load_desc_base) {
      const std::span delay_load_descs{
          delay_load_desc_base, (dir_size / sizeof *delay_load_desc_base) - 1};
      const auto delay_desc{std::ranges::find(
          delay_load_descs, "steam_api64.dll", [module](const auto &desc) {
            return std::string_view{&module[desc.DllNameRVA]};
          })};
      if (delay_desc != delay_load_descs.end()) {
        for (auto int_desc_base{reinterpret_cast<const IMAGE_THUNK_DATA *>(
                 &module[delay_desc->ImportNameTableRVA])},
             int_desc{int_desc_base};
             int_desc->u1.AddressOfData; ++int_desc) {
          if (std::string_view{reinterpret_cast<const IMAGE_IMPORT_BY_NAME *>(
                                   &module[int_desc->u1.AddressOfData])
                                   ->Name} == "SteamAPI_Init") {
            thunk_ptr = reinterpret_cast<SteamAPI_Init_t **>(
                &(reinterpret_cast<IMAGE_THUNK_DATA *>(
                      &module[delay_desc->ImportAddressTableRVA])
                      [std::distance(int_desc_base, int_desc)]
                          .u1.Function));
            break;
          }
        }
      } // if (delay_desc != delay_load_descs.end())
    } // if (delay_load_desc_base)
  } // if (!thunk_ptr)
  if (thunk_ptr) {
    *thunk_ptr = SteamAPI_Init;
  }
}

} // namespace tek::game_runtime::steam_api
