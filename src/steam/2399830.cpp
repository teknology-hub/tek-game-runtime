//===-- 2399830.cpp - game-specific code for Steam app 2399830 ------------===//
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
/// Game-specific code for Steam app 2399830 (ARK: Survival Ascended).
///
//===----------------------------------------------------------------------===//
#include "game_cbs.hpp"

#include "common.hpp"
#include "steam_api.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <dbghelp.h>
#include <format>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <span>
#include <string>
#include <string_view>

namespace tek::game_runtime {

namespace {

//===-- EOS SDK types -----------------------------------------------------===//

enum class [[clang::flag_enum]] EOS_EAuthScopeFlags {
  no_flags,
  basic_profile = 1 << 0,
  friends_list = 1 << 1,
  presence = 1 << 2,
  friends_management = 1 << 3,
  email = 1 << 4,
  country = 1 << 5
};

enum class EOS_EExternalAccountType {
  epic,
  steam,
  psn,
  xbl,
  discord,
  gog,
  nintendo,
  uplay,
  openid,
  apple,
  google,
  oculus,
  itchio,
  amazon,
  viveport
};

enum class EOS_EExternalCredentialType {
  epic,
  steam_app_ticket,
  psn_id_token,
  xbl_xsts_token,
  discord_access_token,
  gog_session_ticket,
  nintendo_id_token,
  nintendo_nsa_id_token,
  uplay_access_token,
  openid_access_token,
  deviceid_access_token,
  apple_id_token,
  google_id_token,
  oculus_userid_nonce,
  itchio_jwt,
  itchio_key,
  epic_id_token,
  amazon_access_token,
  steam_session_ticket,
  viveport_user_token
};

enum class EOS_ELoginCredentialType {
  password,
  exchange_code,
  persistent_auth,
  device_code,
  developer,
  refresh_token,
  account_portal,
  external_auth
};

struct EOS_Auth_IdToken {
  std::int32_t api_version;
  void *_Nonnull account_id;
  const char *_Nonnull jwt;
};

struct EOS_Auth_CopyIdTokenOptions {
  std::int32_t api_version;
  void *_Nonnull account_id;
};

struct EOS_Auth_Credentials {
  std::int32_t api_version;
  const char *_Nullable id;
  const char *_Nullable token;
  EOS_ELoginCredentialType type;
  void *_Nullable system_auth_credentials_options;
  EOS_EExternalCredentialType external_type;
};

struct EOS_Auth_LoginOptions {
  std::int32_t api_version;
  const EOS_Auth_Credentials *_Nonnull credentials;
  EOS_EAuthScopeFlags scope_flags;
  std::uint64_t login_flags;
};

struct EOS_Auth_LoginCallbackInfo {
  std::int32_t result_code;
  void *_Nullable client_data;
  void *_Nonnull local_user_id;
  const void *_Nullable pin_grant_info;
  void *_Nullable continuance_token;
  const void *_Nullable deprecated;
  void *_Nonnull selected_account_id;
};

struct EOS_Connect_Credentials {
  std::int32_t api_version;
  const char *_Nonnull token;
  EOS_EExternalCredentialType type;
};

struct EOS_Connect_ExternalAccountInfo {
  std::int32_t api_version;
  /// tek-game-runtime specific, used to identify whether the instance is
  ///    original or tek-game-runtime's copy.
  std::uint32_t magic;
  void *_Nonnull product_user_id;
  const char *_Nullable display_name;
  const char *_Nonnull account_id;
  EOS_EExternalAccountType account_id_type;
  int64_t last_login_time;
  /// tek-game-runtime specific, points to EOS SDK's instance.
  EOS_Connect_ExternalAccountInfo *_Nullable orig;

  /// Value for @ref magic that indicates that the instance is
  ///    tek-game-runtime's copy.
  constexpr static std::uint32_t tgr_magic = 0x4B4554;
};

struct EOS_Connect_LoginOptions {
  std::int32_t api_version;
  const EOS_Connect_Credentials *_Nonnull credentials;
  const void *_Nullable user_login_info;
};

struct EOS_Connect_LoginCallbackInfo {
  std::int32_t result_code;
  void *_Nullable client_data;
  void *_Nonnull local_user_id;
  void *_Nullable continuance_token;
};

using EOS_Auth_OnLoginCallback =
    void(const EOS_Auth_LoginCallbackInfo *_Nonnull data);
using EOS_Connect_OnLoginCallback =
    void(const EOS_Connect_LoginCallbackInfo *_Nonnull data);
using EOS_Auth_CopyIdToken_t = std::int32_t(
    void *_Nonnull handle, const EOS_Auth_CopyIdTokenOptions *_Nonnull options,
    EOS_Auth_IdToken *_Nullable *_Nonnull id_token);
using EOS_Auth_IdToken_Release_t = void(EOS_Auth_IdToken *_Nonnull id_token);
using EOS_Auth_Login_t =
    void(void *_Nonnull handle, const EOS_Auth_LoginOptions *_Nonnull options,
         void *_Nullable client_data,
         EOS_Auth_OnLoginCallback *_Nonnull completion_delegate);
using EOS_Connect_CopyProductUserInfo_t = std::int32_t(
    void *_Nonnull handle, const void *_Nonnull options,
    EOS_Connect_ExternalAccountInfo *_Nullable *_Nonnull external_account_info);
using EOS_Connect_ExternalAccountInfo_Release_t =
    void(EOS_Connect_ExternalAccountInfo *_Nonnull info);
using EOS_Connect_Login_t = void(
    void *_Nonnull handle, const EOS_Connect_LoginOptions *_Nonnull options,
    void *_Nullable client_data,
    EOS_Connect_OnLoginCallback *_Nonnull completion_delegate);
using EOS_Platform_Create_t = void *_Nonnull(const void *_Nonnull options);
using EOS_Platform_GetAuthInterface_t = void *_Nonnull(void *_Nonnull handle);

struct login_ctx {
  void *_Nonnull handle;
  const EOS_Connect_LoginOptions *_Nonnull options;
  void *_Nullable client_data;
  EOS_Connect_OnLoginCallback *_Nonnull completion_delegate;
  EOS_Auth_IdToken *_Nullable token;
  EOS_Auth_Credentials auth_creds;
  EOS_Connect_Credentials connect_creds;
};

//===-- Settings variables ------------------------------------------------===//

/// Value indicating whether Epic Games authentication should be used for
///    EOS_Connect even when effective app ID is 2399830 and Steam
///    authentication is available.
static bool force_egs_auth;
/// If non-empty, domain name of CurseForge API wrapper that will be used
///    instead of `api.curseforge.com`. Due to technical limitations, its length
///    cannot exceed the length of `api.curseforge.com`.
static std::string cf_api_wrapper;

//===-- Internal variables ------------------------------------------------===//

/// String form of current user's Steam ID.
static std::array<char, 21> steam_id_str;
/// Handle for the EOS Auth interface.
static void *_Nonnull eos_auth_iface;

//===-- EOS SDK function wrappers -----------------------------------------===//

/// Pointer to the original `EOS_Auth_CopyIdToken_t` function.
static EOS_Auth_CopyIdToken_t *_Nullable EOS_Auth_CopyIdToken_orig;
/// Pointer to the original `EOS_Auth_IdToken_Release` function.
static EOS_Auth_IdToken_Release_t *_Nullable EOS_Auth_IdToken_Release_orig;
/// Pointer to the original `EOS_Auth_Login` function.
static EOS_Auth_Login_t *_Nullable EOS_Auth_Login_orig;
/// Pointer to the original `EOS_Connect_Login` function.
static EOS_Connect_Login_t *_Nullable EOS_Connect_Login_orig;

/// Pointer to the original `EOS_Connect_CopyProductUserInfo` function.
static EOS_Connect_CopyProductUserInfo_t
    *_Nullable EOS_Connect_CopyProductUserInfo_orig;
/// Wrapper for `EOS_Connect_CopyProductUserInfo`, that changes account type
///    back to Steam.
static std::int32_t
EOS_Connect_CopyProductUserInfo(void *_Nonnull handle,
                                const void *_Nonnull options,
                                EOS_Connect_ExternalAccountInfo *_Nullable
                                    *_Nonnull external_account_info) {
  const auto res{EOS_Connect_CopyProductUserInfo_orig(handle, options,
                                                      external_account_info)};
  if (!res) {
    auto &info = **external_account_info;
    if (info.account_id_type != EOS_EExternalAccountType::steam) {
      const auto copy{new EOS_Connect_ExternalAccountInfo{info}};
      copy->magic = EOS_Connect_ExternalAccountInfo::tgr_magic;
      copy->account_id = steam_id_str.data();
      copy->account_id_type = EOS_EExternalAccountType::steam;
      copy->orig = &info;
      *external_account_info = copy;
    }
  }
  return res;
}

/// Pointer to the original `EOS_Connect_ExternalAccountInfo_Release` function.
static EOS_Connect_ExternalAccountInfo_Release_t
    *_Nullable EOS_Connect_ExternalAccountInfo_Release_orig;
/// Wrapper for `EOS_Connect_ExternalAccountInfo_Release`, that correctly
///    handles tek-game-runtime's copies.
static void EOS_Connect_ExternalAccountInfo_Release(
    EOS_Connect_ExternalAccountInfo *_Nonnull info) {
  if (info && info->magic == EOS_Connect_ExternalAccountInfo::tgr_magic) {
    const auto copy{info};
    info = info->orig;
    delete copy;
  }
  EOS_Connect_ExternalAccountInfo_Release_orig(info);
}

/// Completion handler for `EOS_Connect_Login`.
static void
connect_login_complete(const EOS_Connect_LoginCallbackInfo *_Nonnull data) {
  auto &ctx{*reinterpret_cast<login_ctx *>(data->client_data)};
  EOS_Auth_IdToken_Release_orig(ctx.token);
  EOS_Connect_LoginCallbackInfo data_copy{*data};
  data_copy.client_data = ctx.client_data;
  ctx.completion_delegate(&data_copy);
  delete &ctx;
}

/// Completion handler for `EOS_Auth_Login`.
static void
auth_login_complete(const EOS_Auth_LoginCallbackInfo *_Nonnull data) {
  auto &ctx{*reinterpret_cast<login_ctx *>(data->client_data)};
  if (data->result_code) {
    // Login failed
    if (ctx.auth_creds.type == EOS_ELoginCredentialType::persistent_auth) {
      MessageBoxW(nullptr,
                  L"After you press OK, a browser prompt will open for Epic "
                  L"Games account authorization. You must finish it for online "
                  L"funcionality to work.",
                  L"TEK Game Runtime", MB_OK | MB_ICONINFORMATION);
      ctx.auth_creds.type = EOS_ELoginCredentialType::account_portal;
      const EOS_Auth_LoginOptions options{.api_version = 3,
                                          .credentials = &ctx.auth_creds,
                                          .scope_flags =
                                              EOS_EAuthScopeFlags::no_flags,
                                          .login_flags = 0};
      EOS_Auth_Login_orig(eos_auth_iface, &options, &ctx, auth_login_complete);
      return;
    }
  } else {
    // Login succeeded
    const EOS_Auth_CopyIdTokenOptions options{
        .api_version = 1, .account_id = data->local_user_id};
    if (!EOS_Auth_CopyIdToken_orig(eos_auth_iface, &options, &ctx.token)) {
      ctx.connect_creds.token = ctx.token->jwt;
      ctx.connect_creds.type = EOS_EExternalCredentialType::epic_id_token;
    }
  }
  if (ctx.token) {
    EOS_Connect_LoginOptions options{*ctx.options};
    options.credentials = &ctx.connect_creds;
    EOS_Connect_Login_orig(ctx.handle, &options, &ctx, connect_login_complete);
  } else {
    EOS_Connect_Login_orig(ctx.handle, ctx.options, ctx.client_data,
                           ctx.completion_delegate);
    delete &ctx;
  }
}

/// Wrapper for `EOS_Connect_Login`, that forces Epic Account Service
///    authentication instead of Steam when requested.
static void
EOS_Connect_Login(void *_Nonnull handle,
                  const EOS_Connect_LoginOptions *_Nonnull options,
                  void *_Nullable client_data,
                  EOS_Connect_OnLoginCallback *_Nonnull completion_delegate) {
  if (force_egs_auth || g_settings.steam->spoof_app_id != 2399830) {
    const auto ctx{new login_ctx{
        .handle = handle,
        .options = options,
        .client_data = client_data,
        .completion_delegate = completion_delegate,
        .token = nullptr,
        .auth_creds{.api_version = 4,
                    .id = nullptr,
                    .token = nullptr,
                    .type = EOS_ELoginCredentialType::persistent_auth,
                    .system_auth_credentials_options = nullptr,
                    .external_type = EOS_EExternalCredentialType::epic},
        .connect_creds{*options->credentials}}};
    const EOS_Auth_LoginOptions login_options{.api_version = 3,
                                              .credentials = &ctx->auth_creds,
                                              .scope_flags =
                                                  EOS_EAuthScopeFlags::no_flags,
                                              .login_flags = 0};
    EOS_Auth_Login_orig(eos_auth_iface, &login_options, ctx,
                        auth_login_complete);
  } else {
    EOS_Connect_Login_orig(handle, options, client_data, completion_delegate);
  }
}

/// Wrapper for `EOS_Platform_Create`, that gets original pointers for other
///    functions and caches the auth interface handle.
static void *_Nonnull EOS_Platform_Create(const void *_Nonnull options) {
  const auto module{GetModuleHandleW(L"EOSSDK-Win64-Shipping.dll")};
  EOS_Auth_CopyIdToken_orig = reinterpret_cast<EOS_Auth_CopyIdToken_t *>(
      GetProcAddress(module, "EOS_Auth_CopyIdToken"));
  EOS_Auth_IdToken_Release_orig =
      reinterpret_cast<EOS_Auth_IdToken_Release_t *>(
          GetProcAddress(module, "EOS_Auth_IdToken_Release"));
  EOS_Auth_Login_orig = reinterpret_cast<EOS_Auth_Login_t *>(
      GetProcAddress(module, "EOS_Auth_Login"));
  EOS_Connect_CopyProductUserInfo_orig =
      reinterpret_cast<EOS_Connect_CopyProductUserInfo_t *>(
          GetProcAddress(module, "EOS_Connect_CopyProductUserInfo"));
  EOS_Connect_ExternalAccountInfo_Release_orig =
      reinterpret_cast<EOS_Connect_ExternalAccountInfo_Release_t *>(
          GetProcAddress(module, "EOS_Connect_ExternalAccountInfo_Release"));
  EOS_Connect_Login_orig = reinterpret_cast<EOS_Connect_Login_t *>(
      GetProcAddress(module, "EOS_Connect_Login"));
  const auto EOS_Platform_Create_orig{reinterpret_cast<EOS_Platform_Create_t *>(
      GetProcAddress(module, "EOS_Platform_Create"))};
  const auto EOS_Platform_GetAuthInterface_orig{
      reinterpret_cast<EOS_Platform_GetAuthInterface_t *>(
          GetProcAddress(module, "EOS_Platform_GetAuthInterface"))};
  const auto platform{EOS_Platform_Create_orig(options)};
  eos_auth_iface = EOS_Platform_GetAuthInterface_orig(platform);
  return platform;
}

} // namespace

namespace cbs::steam {

//===-- Game callbacks ----------------------------------------------------===//

void settings_load_2399830(const rapidjson::Document &doc) {
  const auto force_egs_auth_m{doc.FindMember("force_egs_auth")};
  if (force_egs_auth_m != doc.MemberEnd() && force_egs_auth_m->value.IsBool()) {
    force_egs_auth = force_egs_auth_m->value.GetBool();
  }
  const auto cf_api_wrapper_m{doc.FindMember("cf_api_wrapper")};
  if (cf_api_wrapper_m != doc.MemberEnd() &&
      cf_api_wrapper_m->value.IsString()) {
    cf_api_wrapper = {cf_api_wrapper_m->value.GetString(),
                      cf_api_wrapper_m->value.GetStringLength()};
  }
}

void settings_save_2399830(
    rapidjson::Writer<rapidjson::FileWriteStream> &writer) {
  std::string_view str{"force_egs_auth"};
  writer.Key(str.data(), str.length());
  writer.Bool(force_egs_auth);
  if (!cf_api_wrapper.empty()) {
    str = "cf_api_wrapper";
    writer.Key(str.data(), str.length());
    writer.String(cf_api_wrapper.data(), cf_api_wrapper.length());
  }
}

bool dllmain_2399830() {
  const auto module{reinterpret_cast<char *>(GetModuleHandleW(nullptr))};
  if (!cf_api_wrapper.empty()) {
    constexpr std::wstring_view cf_api_domain{L"api.curseforge.com"};
    if (cf_api_wrapper.length() > cf_api_domain.length()) {
      display_error(std::format(L"The length of cf_api_wrapper string cannot "
                                L"exceed the number of characters in \"{}\"",
                                cf_api_domain)
                        .data());
    }
    const auto hdr{ImageNtHeader(module)};
    const std::span sections{
        reinterpret_cast<const IMAGE_SECTION_HEADER *>(hdr + 1),
        hdr->FileHeader.NumberOfSections};
    constexpr std::string_view section_name{".rdata"};
    const auto rdata{std::ranges::find(
        sections, section_name, [&section_name](const auto &sec) {
          return std::string_view{reinterpret_cast<const char *>(sec.Name),
                                  section_name.length()};
        })};
    if (rdata == sections.end()) {
      display_error(L"Unable to apply CF API wrapper: .rdata section not found "
                    L"in the executable");
      return false;
    }
    constexpr auto cf_api_domain_raw_size{
        cf_api_domain.length() * sizeof(decltype(cf_api_domain)::value_type)};
    const auto idx{std::string_view{&module[rdata->VirtualAddress],
                                    rdata->Misc.VirtualSize}
                       .find(std::string_view{
                           reinterpret_cast<const char *>(cf_api_domain.data()),
                           cf_api_domain_raw_size})};
    if (idx == std::string_view::npos) {
      display_error(std::format(L"Unable to apply CF API wrapper: The string "
                                L"\"{}\" is not found in .rdata",
                                cf_api_domain)
                        .data());
      return false;
    }
    const auto str{
        reinterpret_cast<LPWSTR>(&module[rdata->VirtualAddress + idx])};
    DWORD prev_protect;
    if (!VirtualProtect(str, cf_api_domain_raw_size, PAGE_READWRITE,
                        &prev_protect)) {
      display_error(
          std::format(L"Unable to apply CF API wrapper: VirtualProtect call "
                      L"failed with error code {}",
                      GetLastError())
              .data());
    }
    MultiByteToWideChar(CP_UTF8, 0, cf_api_wrapper.data(), -1, str,
                        cf_api_domain.length() + 1);
    VirtualProtect(str, cf_api_domain_raw_size, prev_protect, &prev_protect);
  } // if (!cf_api_wrapper.empty())
  ULONG dir_size;
  // Locate delay load descriptor for EOSSDK-Win64-Shipping.dll
  const auto delay_load_desc_base{
      reinterpret_cast<const IMAGE_DELAYLOAD_DESCRIPTOR *>(
          ImageDirectoryEntryToDataEx(const_cast<char *>(module), TRUE,
                                      IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT,
                                      &dir_size, nullptr))};
  if (!delay_load_desc_base) {
    display_error(L"Delay load descriptor not found");
    return false;
  }
  const std::span delay_load_descs{
      delay_load_desc_base, (dir_size / sizeof *delay_load_desc_base) - 1};
  const auto delay_desc{
      std::ranges::find(delay_load_descs, "EOSSDK-Win64-Shipping.dll",
                        [module](const auto &desc) {
                          return std::string_view{&module[desc.DllNameRVA]};
                        })};
  if (delay_desc == delay_load_descs.end()) {
    display_error(
        L"Delay load descriptor for EOSSDK-Win64-Shipping.dll not found");
    return false;
  }
  const auto iat{reinterpret_cast<IMAGE_THUNK_DATA *>(
      &module[delay_desc->ImportAddressTableRVA])};
  // Wrap EOS SDK functions
  for (auto int_desc_base{reinterpret_cast<const IMAGE_THUNK_DATA *>(
           &module[delay_desc->ImportNameTableRVA])},
       int_desc{int_desc_base};
       int_desc->u1.AddressOfData; ++int_desc) {
    const std::string_view name{reinterpret_cast<const IMAGE_IMPORT_BY_NAME *>(
                                    &module[int_desc->u1.AddressOfData])
                                    ->Name};
    if (name == "EOS_Connect_Login") {
      *reinterpret_cast<EOS_Connect_Login_t **>(
          &(iat[std::distance(int_desc_base, int_desc)].u1.Function)) =
          EOS_Connect_Login;
    } else if (name == "EOS_Connect_CopyProductUserInfo") {
      *reinterpret_cast<EOS_Connect_CopyProductUserInfo_t **>(
          &(iat[std::distance(int_desc_base, int_desc)].u1.Function)) =
          EOS_Connect_CopyProductUserInfo;
    } else if (name == "EOS_Connect_ExternalAccountInfo_Release") {
      *reinterpret_cast<EOS_Connect_ExternalAccountInfo_Release_t **>(
          &(iat[std::distance(int_desc_base, int_desc)].u1.Function)) =
          EOS_Connect_ExternalAccountInfo_Release;
    } else if (name == "EOS_Platform_Create") {
      *reinterpret_cast<EOS_Platform_Create_t **>(
          &(iat[std::distance(int_desc_base, int_desc)].u1.Function)) =
          EOS_Platform_Create;
    }
  }
  return true;
}

void steam_api_init_2399830() {
  *std::to_chars(steam_id_str.begin(), steam_id_str.end(), steam_api::steam_id)
       .ptr = '\0';
}

} // namespace cbs::steam
} // namespace tek::game_runtime
