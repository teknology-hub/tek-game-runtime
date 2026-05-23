//===-- api.cpp - Steam API wrapper implementation ------------------------===//
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
/// Implementation of the Steam API wrapper interface.
///
//===----------------------------------------------------------------------===//
#include "api.hpp"

#include "api/isteamapps.hpp"
#include "api/isteaminventory.hpp"
#include "api/isteammatchmaking.hpp"
#include "api/isteammatchmakingservers.hpp"
#include "api/isteamugc.hpp"
#include "api/isteamuser.hpp"
#include "api/isteamutils.hpp"
#include "common.hpp"
#include "game_cbs.hpp"
#include "settings.hpp"
#include "tsc.hpp"

#include <MinHook.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <dbghelp.h>
#include <format>
#include <iterator>
#include <locale>
#include <ranges>
#include <span>
#include <string_view>
#include <winternl.h>

//===-- NT API declarations not included in winternl.h --------------------===//
extern "C" {

#define LDR_DLL_NOTIFICATION_REASON_LOADED 1
#define LDR_DLL_NOTIFICATION_REASON_UNLOADED 2

typedef struct _LDR_DLL_LOADED_NOTIFICATION_DATA {
  ULONG Flags;
  PCUNICODE_STRING FullDllName;
  PCUNICODE_STRING BaseDllName;
  PVOID DllBase;
  ULONG SizeOfImage;
} LDR_DLL_LOADED_NOTIFICATION_DATA, *PLDR_DLL_LOADED_NOTIFICATION_DATA;

typedef struct _LDR_DLL_UNLOADED_NOTIFICATION_DATA {
  ULONG Flags;
  PCUNICODE_STRING FullDllName;
  PCUNICODE_STRING BaseDllName;
  PVOID DllBase;
  ULONG SizeOfImage;
} LDR_DLL_UNLOADED_NOTIFICATION_DATA, *PLDR_DLL_UNLOADED_NOTIFICATION_DATA;

typedef union _LDR_DLL_NOTIFICATION_DATA {
  LDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
  LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
} LDR_DLL_NOTIFICATION_DATA, *PLDR_DLL_NOTIFICATION_DATA;
typedef const LDR_DLL_NOTIFICATION_DATA *PCLDR_DLL_NOTIFICATION_DATA;

typedef VOID (*CALLBACK PLDR_DLL_NOTIFICATION_FUNCTION)(
    ULONG NotificationReason, PCLDR_DLL_NOTIFICATION_DATA NotificationData,
    PVOID Context);

NTSTATUS NTAPI LdrRegisterDllNotification(
    ULONG Flags, PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction,
    PVOID Context, PVOID *Cookie);

NTSTATUS NTAPI LdrUnregisterDllNotification(PVOID Cookie);

} // extern "C"

namespace tek::game_runtime::steam {

namespace {

//===-- Common Steam API method wrappers ----------------------------------===//

/// Wrapper for ISteamApps::BIsSubscribed, making it always return `true`.
static bool SteamApps_BIsSubscribed(void *) noexcept { return true; }

/// Pointer to the original ISteamApps::BIsSubscribedApp method.
static ISteamApps::BIsSubscribedApp_t *_Nonnull SteamApps_BIsSubscribedApp_orig;
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

/// Pointer to the original ISteamApps::BIsDlcInstalled method.
static ISteamApps::BIsDlcInstalled_t *_Nonnull SteamApps_BIsDlcInstalled_orig;
/// Wrapper for ISteamApps::BIsDlcInstalled, making it always return `true` for
///    IDs listed in the settings.
static bool SteamApps_BIsDlcInstalled(void *_Nonnull iface,
                                      std::uint32_t app_id) {
  if (g_settings.steam->installed_dlc.contains(app_id)) {
    return true;
  }
  return SteamApps_BIsDlcInstalled_orig(iface, app_id);
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
  if (idx < 0 || idx >= static_cast<int>(g_settings.steam->dlc.size())) {
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
static ISteamApps::BIsAppInstalled_t *_Nonnull SteamApps_BIsAppInstalled_orig;
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
///    `uhlfa_result::HasLicense`.
static ISteamUser::uhlfa_result
SteamUser_UserHasLicenseForApp(void *, std::uint64_t, std::uint32_t) noexcept {
  return ISteamUser::uhlfa_result::HasLicense;
}

/// Wrapper for ISteamUtils::GetAppID, making it return original app ID.
static std::uint32_t SteamUtils_GetAppID(void *) noexcept {
  return g_settings.steam->app_id;
}

//===-- SteamAPI function wrapping ----------------------------------------===//

/// `SteamAPI_Init` function type.
using SteamAPI_Init_t = bool();
/// `SteamAPI_RestartAppIfNecessary` function type.
using SteamAPI_RestartAppIfNecessary_t = bool(std::uint32_t app_id);

/// Pointer to the original `SteamAPI_Init` function.
static SteamAPI_Init_t *SteamAPI_Init_orig;
/// Wrapper for `SteamAPI_Init`.
static bool SteamAPI_Init() {
  const auto app_id{g_settings.steam->app_id};
  const auto spoof_app_id{g_settings.steam->spoof_app_id};
  std::array<WCHAR, 11> buf;
  *std::format_to_n(buf.data(), buf.size(), std::locale::classic(), L"{}",
                    spoof_app_id ? spoof_app_id : app_id)
       .out = L'\0';
  SetEnvironmentVariableW(L"SteamAppId", buf.data());
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
  constexpr auto ver_fail{[]() {
    display_error(
        L"Couldn't load steam_api64.dll file version, no changes will "
        L"be applied");
  }};
  const auto module{GetModuleHandleW(L"steam_api64.dll")};
  const auto rsrc{
      FindResourceW(module, MAKEINTRESOURCEW(VS_VERSION_INFO), RT_VERSION)};
  if (!rsrc) {
    ver_fail();
    return false;
  }
  const auto ver_res{LoadResource(module, rsrc)};
  if (!ver_res) {
    ver_fail();
    return false;
  }
  const auto ver_data{LockResource(ver_res)};
  if (!ver_data) {
    ver_fail();
    return false;
  }
  VS_FIXEDFILEINFO *file_info;
  UINT size;
  if (!VerQueryValueW(ver_data, L"\\", reinterpret_cast<LPVOID *>(&file_info),
                      &size)) {
    ver_fail();
    return false;
  }
  ver = file_info->dwFileVersionLS |
        (static_cast<std::uint64_t>(file_info->dwFileVersionMS) << 32);
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
  cpp_interface *ISteamInventory_ptr;
  cpp_interface *ISteamMatchmaking_ptr;
  cpp_interface *ISteamMatchmakingServers_ptr;
  cpp_interface *ISteamUGC_ptr;
  cpp_interface *ISteamUser_ptr;
  cpp_interface *ISteamUtils_ptr;
  if (ver >= 0x0003002A003D0042) { // 03.42.61.66
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
    const auto get_iface{[ISteamClient_ptr,
                          ISteamClient_GetISteamGenericInterface, pipe,
                          user](const char *version) {
      return ISteamClient_GetISteamGenericInterface(ISteamClient_ptr, user,
                                                    pipe, version);
    }};
    ISteamApps_ptr = get_iface(ISteamApps::get_ver_str());
    ISteamInventory_ptr = get_iface(ISteamInventory::get_ver_str());
    ISteamMatchmaking_ptr = get_iface(ISteamMatchmaking::get_ver_str());
    ISteamMatchmakingServers_ptr =
        get_iface(ISteamMatchmakingServers::get_ver_str());
    ISteamUGC_ptr = get_iface(ISteamUGC::get_ver_str());
    ISteamUser_ptr = get_iface(ISteamUser::get_ver_str());
    ISteamUtils_ptr = get_iface(ISteamUtils::get_ver_str());
  } else { // if (ver >= 0x0003002A003D0042)
    // Older Steamworks SDK versions have interface getters implemented as
    //    exported functions
    const auto get_iface{[module](const std::string_view &&symbol) {
      return reinterpret_cast<cpp_interface *(*)()>(
          GetProcAddress(module, symbol.data()))();
    }};
    ISteamApps_ptr = get_iface("SteamApps");
    // ISteamInventory first appeared in Steamworks SDK v1.32 (02.59.51.43)
    ISteamInventory_ptr =
        ver >= 0x0002003B0033002B ? get_iface("SteamInventory") : nullptr;
    ISteamMatchmaking_ptr = get_iface("SteamMatchmaking");
    ISteamMatchmakingServers_ptr = get_iface("SteamMatchmakingServers");
    // ISteamUGC first appeared in Steamworks SDK v1.26 (01.98.31.73)
    ISteamUGC_ptr = ver >= 0x00010062001F0049 ? get_iface("SteamUGC") : nullptr;
    ISteamUser_ptr = get_iface("SteamUser");
    ISteamUtils_ptr = get_iface("SteamUtils");
  } // if (ver >= 0x0003002A003D0042) else
  // Setup interface wrappers based on current version
  ISteamApps::setup(ISteamApps_ptr);
  ISteamInventory::setup(ISteamInventory_ptr);
  ISteamMatchmaking::setup(ISteamMatchmaking_ptr);
  ISteamMatchmakingServers::setup(ISteamMatchmakingServers_ptr);
  if (ISteamUGC_ptr) {
    ISteamUGC::setup(ISteamUGC_ptr);
  }
  ISteamUser::setup(ISteamUser_ptr);
  ISteamUtils::setup(ISteamUtils_ptr);
  // Get current user Steam ID
  reinterpret_cast<ISteamUser::GetSteamID_t *>(
      ISteamUser::wrapper
          .orig_vtable[ISteamUser::wrapper.vm_idxs[ISteamUser::m_GetSteamID]])(
      ISteamUser_ptr, &steam_id);
  // Setup common function wrappers
  ISteamApps::wrapper[ISteamApps::m_BIsSubscribed] = SteamApps_BIsSubscribed;
  SteamApps_BIsSubscribedApp_orig =
      ISteamApps::wrapper[ISteamApps::m_BIsSubscribedApp];
  ISteamApps::wrapper[ISteamApps::m_BIsSubscribedApp] =
      SteamApps_BIsSubscribedApp;
  SteamApps_BIsDlcInstalled_orig =
      ISteamApps::wrapper[ISteamApps::m_BIsDlcInstalled];
  ISteamApps::wrapper[ISteamApps::m_BIsDlcInstalled] =
      SteamApps_BIsDlcInstalled;
  ISteamApps::wrapper[ISteamApps::m_BIsSubscribedFromFreeWeekend] =
      SteamApps_BIsSubscribedFromFreeWeekend;
  ISteamApps::wrapper[ISteamApps::m_GetDLCCount] = SteamApps_GetDLCCount;
  ISteamApps::wrapper[ISteamApps::m_BGetDLCDataByIndex] =
      SteamApps_BGetDLCDataByIndex;
  SteamApps_BIsAppInstalled_orig =
      ISteamApps::wrapper[ISteamApps::m_BIsAppInstalled];
  ISteamApps::wrapper[ISteamApps::m_BIsAppInstalled] =
      SteamApps_BIsAppInstalled;
  ISteamApps::wrapper[ISteamApps::m_GetAppOwner] = SteamApps_GetAppOwner;
  ISteamApps::wrapper[ISteamApps::m_BIsSubscribedFromFamilySharing] =
      SteamApps_BIsSubscribedFromFamilySharing;
  ISteamApps::wrapper[ISteamApps::m_BIsTimedTrial] = SteamApps_BIsTimedTrial;
  ISteamUser::wrapper[ISteamUser::m_UserHasLicenseForApp] =
      SteamUser_UserHasLicenseForApp;
  ISteamUtils::wrapper[ISteamUtils::m_GetAppID] = SteamUtils_GetAppID;
  if (g_settings.steam->auto_update_dlc) {
    // Attempt to load tek-steamclient and update the DLC list.
    tsc::load();
    if (tsc::loaded) {
      tsc::update_dlc();
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
}

/// Wrapper for `SteamAPI_RestartAppIfNecessary`.
static bool SteamAPI_RestartAppIfNecessary(std::uint32_t) noexcept {
  return false;
}

/// Cookie value that can be used to unregister DLL notification.
static PVOID cookie;
/// DLL loaded notification handler that hooks steam_api64.dll once it gets
///    loaded.
static VOID CALLBACK dll_notification(ULONG reason,
                                      PCLDR_DLL_NOTIFICATION_DATA data, PVOID) {
  if (reason != LDR_DLL_NOTIFICATION_REASON_LOADED) {
    return;
  }
  if (std::wstring_view{data->Loaded.BaseDllName->Buffer,
                        data->Loaded.BaseDllName->Length /
                            sizeof *data->Loaded.BaseDllName->Buffer} !=
      L"steam_api64.dll") {
    return;
  }
  LdrUnregisterDllNotification(cookie);
  const auto module{reinterpret_cast<HMODULE>(data->Loaded.DllBase)};
  const auto display_mh_error{[](const std::wstring_view &&msg, MH_STATUS res) {
    std::array<WCHAR, 512> mh_msg;
    if (!MultiByteToWideChar(CP_UTF8, 0, MH_StatusToString(res), -1,
                             mh_msg.data(), mh_msg.size())) {
      std::ranges::copy(L"Unknown", mh_msg.data());
    }
    display_error(
        std::format(L"{}: ({}) {}", msg, static_cast<int>(res), mh_msg.data())
            .data());
  }};
  auto ptr{reinterpret_cast<LPVOID>(GetProcAddress(module, "SteamAPI_Init"))};
  auto mh_res{MH_CreateHook(ptr, reinterpret_cast<LPVOID>(&SteamAPI_Init),
                            reinterpret_cast<LPVOID *>(&SteamAPI_Init_orig))};
  if (mh_res != MH_OK) {
    display_mh_error(L"Failed to create hook for SteamAPI_Init", mh_res);
    return;
  }
  mh_res = MH_EnableHook(ptr);
  if (mh_res != MH_OK) {
    display_mh_error(L"Failed to enable hook for SteamAPI_Init", mh_res);
    return;
  }
  ptr = reinterpret_cast<LPVOID>(
      GetProcAddress(module, "SteamAPI_RestartAppIfNecessary"));
  mh_res = MH_CreateHook(
      ptr, reinterpret_cast<LPVOID>(&SteamAPI_RestartAppIfNecessary), nullptr);
  if (mh_res != MH_OK) {
    display_mh_error(
        L"Failed to create hook for SteamAPI_RestartAppIfNecessary", mh_res);
    return;
  }
  mh_res = MH_EnableHook(ptr);
  if (mh_res != MH_OK) {
    display_mh_error(
        L"Failed to enable hook for SteamAPI_RestartAppIfNecessary", mh_res);
    return;
  }
}

} // namespace

void wrap_funcs() {
  // Set SteamAppId env variable to original app ID early, just in case some
  //    Steam API's early checks may use it
  std::array<WCHAR, 11> buf;
  *std::format_to_n(buf.data(), buf.size(), std::locale::classic(), L"{}",
                    g_settings.steam->app_id)
       .out = L'\0';
  SetEnvironmentVariableW(L"SteamAppId", buf.data());
  const auto module{reinterpret_cast<char *>(GetModuleHandleW(nullptr))};
  const auto header{ImageNtHeader(module)};
  // Try to locate import descriptor for steam_api64.dll
  ULONG dir_size;
  const auto import_desc_base{reinterpret_cast<const IMAGE_IMPORT_DESCRIPTOR *>(
      ImageDirectoryEntryToDataEx(module, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT,
                                  &dir_size, nullptr))};
  if (import_desc_base) {
    const std::span descs{import_desc_base,
                          (dir_size / sizeof *import_desc_base) - 1};
    const auto desc{
        std::ranges::find(descs, "steam_api64.dll", [module](const auto &desc) {
          return std::string_view{&module[desc.Name]};
        })};
    if (desc != descs.end()) {
      // Make sure that the section containing IAT is writable
      const auto section{ImageRvaToSection(header, nullptr, desc->FirstThunk)};
      if (!section) {
        display_error(L"Failed to locate header for the section containing "
                      L"import address table");
        return;
      }
      const bool section_writable{
          static_cast<bool>(section->Characteristics & IMAGE_SCN_MEM_WRITE)};
      DWORD old_protect;
      if (!section_writable) {
        if (!VirtualProtect(&module[section->VirtualAddress],
                            section->Misc.VirtualSize, PAGE_READWRITE,
                            &old_protect)) {
          display_error(std::format(L"Failed to make section \"{}\" writable; "
                                    L"VirtualProtect returned error code {}",
                                    section->Name, GetLastError())
                            .data());
          return;
        }
      }
      const auto thunks{
          reinterpret_cast<IMAGE_THUNK_DATA *>(&module[desc->FirstThunk])};
      for (auto ilt_desc_base{reinterpret_cast<const IMAGE_THUNK_DATA *>(
               &module[desc->OriginalFirstThunk])},
           ilt_desc{ilt_desc_base};
           ilt_desc->u1.AddressOfData; ++ilt_desc) {
        if (ilt_desc->u1.AddressOfData & IMAGE_ORDINAL_FLAG) {
          continue;
        }
        const std::string_view name{
            reinterpret_cast<const IMAGE_IMPORT_BY_NAME *>(
                &module[ilt_desc->u1.AddressOfData])
                ->Name};
        if (name == "SteamAPI_Init") {
          SteamAPI_Init_orig = reinterpret_cast<SteamAPI_Init_t *>(
              thunks[std::distance(ilt_desc_base, ilt_desc)].u1.Function);
          thunks[std::distance(ilt_desc_base, ilt_desc)].u1.Function =
              reinterpret_cast<ULONGLONG>(SteamAPI_Init);
        } else if (name == "SteamAPI_RestartAppIfNecessary") {
          thunks[std::distance(ilt_desc_base, ilt_desc)].u1.Function =
              reinterpret_cast<ULONGLONG>(SteamAPI_RestartAppIfNecessary);
        }
      }
      if (!section_writable) {
        VirtualProtect(&module[section->VirtualAddress],
                       section->Misc.VirtualSize, old_protect, &old_protect);
      }
      return;
    } // if (import_desc != import_descs.end())
  } // if (import_desc_base)
  // Register DLL load notification and hope steam_api64.dll gets loaded some
  //    time later
  LdrRegisterDllNotification(0, dll_notification, nullptr, &cookie);
}

} // namespace tek::game_runtime::steam
