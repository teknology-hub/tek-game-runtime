//===-- 346110.cpp - game-specific code for Steam app 346110 --------------===//
//
// Copyright (c) 2025-2026 Nuclearist <nuclearist@teknology-hub.com>
// Part of tek-game-runtime, under the GNU General Public License v3.0 or later
// See https://github.com/teknology-hub/tek-game-runtime/blob/main/COPYING for
//    license information.
// SPDX-License-Identifier: GPL-3.0-or-later
//
//===----------------------------------------------------------------------===//
///
/// @file
/// Game-specific code for Steam app 346110 (ARK: Survival Evolved).
///
//===----------------------------------------------------------------------===//
#include "game_cbs.hpp"

#include "common.hpp" // IWYU pragma: keep
#include "steam/api/isteamapps.hpp"
#include "steam/api/isteammatchmakingservers.hpp"
#include "steam/api/isteamugc.hpp"
#include "steam/api/isteamutils.hpp"
#include "steam/tsc.hpp"

#include <algorithm>
#include <atomic>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cwchar>
#include <filesystem>
#include <format>
#include <locale>
#include <memory>
#include <mutex>
#include <ranges>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <string>
#include <string_view>
#include <tek-steamclient/am.h>
#include <tek-steamclient/cm.h>
#include <unordered_map>
#include <vector>

namespace tek::game_runtime {

using namespace steam;

namespace {

//===-- Settings variables ------------------------------------------------===//

/// Value indicating whether BattlEye-protected servers are allowed to appear in
///    search results.
static bool show_be_servers;
/// Value indicating whether servers that the user cannot join are allowed to
///    appear in search results. For users with effective app ID 346110,
///    unavailable servers are servers that have a DLC map that user doesn't own
///    *and* don't have TEK Wrapper. For users with effective app ID different
///    from 346110, unavailable servers are all servers that don't have TEK
///    Wrapper.
static bool show_unavailable_servers;
/// Path to the base directory for Steam Workshop items for the game.
static std::string ws_dir_path;
/// Path to the game root directory that will be used to initialize application
///    manager instance for Steam Workshop items.
static std::string ws_am_path;

//===-- Internal variables ------------------------------------------------===//

/// List of DLC maps not owned by the user.
static std::vector<std::string> unavailable_dlc;
/// List of installed/"subscribed" Steam Workshop item IDs.
static std::vector<std::uint64_t> mods;
/// Mutex for locking concurrent access to @ref mods.
static std::mutex mods_mtx;
/// Pointers to active Steam Workshop item job descriptors.
static std::unordered_map<std::uint64_t, tek_sc_am_item_desc *> ws_descs;
/// Mutex for locking concurrent access to @ref ws_descs.
static std::mutex ws_descs_mtx;

//===-- ISteamMatchmakingServers method wrappers --------------------------===//

/// Pointer to the original ISteamMatchmakingServers::CancelServerQuery method.
static ISteamMatchmakingServers::CancelServerQuery_t
    *_Nullable SteamMatchmakingServers_CancelServerQuery_orig;
/// Wrapper for game's ISteamMatchmakingRulesResponse handler.
class rules_response_wrapper final
    : public ISteamMatchmakingServers::ISteamMatchmakingRulesResponse {
  /// Pointer to the underlying handler instance.
  ISteamMatchmakingRulesResponse *const _Nonnull base;

public:
  /// Server query handle.
  int query;

  constexpr rules_response_wrapper(
      ISteamMatchmakingRulesResponse *_Nonnull base) noexcept
      : base{base} {}

  void RulesResponded(const char *_Nonnull key,
                      const char *_Nonnull value) override {
    if (const std::string_view key_view{key};
        (!show_be_servers && key_view == "SERVERUSESBATTLEYE_b" &&
         std::string_view{value} != "false") ||
        (!show_unavailable_servers &&
         g_settings.steam->spoof_app_id != 346110 &&
         key_view == "SEARCHKEYWORDS_s" &&
         !std::string_view{value}.starts_with("TEKWrapper"))) {
      SteamMatchmakingServers_CancelServerQuery_orig(
          ISteamMatchmakingServers::wrapper.iface, query);
      base->RulesFailedToRespond();
      delete this;
    } else {
      base->RulesResponded(key, value);
    }
  }
  void RulesFailedToRespond() override {
    base->RulesFailedToRespond();
    delete this;
  }
  void RulesRefreshComplete() override {
    base->RulesRefreshComplete();
    delete this;
  }
};

/// Pointer to the original ISteamMatchmakingServers::RequestInternetServerList
///    method.
static ISteamMatchmakingServers::RequestInternetServerList_t
    *_Nullable SteamMatchmakingServers_RequestInternetServerList_orig;
/// Wrapper for ISteamMatchmakingServers::RequestInternetServerList, making it
///    apply extra search filters based on settings.
static void *_Nonnull SteamMatchmakingServers_RequestInternetServerList(
    void *_Nonnull iface, std::uint32_t app_id,
    const ISteamMatchmakingServers::kv_pair *const _Nonnull *_Nullable filters,
    std::uint32_t num_filters, void *_Nonnull response_handler) {
  auto num_new_filters{num_filters};
  if (!show_be_servers) {
    ++num_new_filters;
  }
  if (!show_unavailable_servers) {
    num_new_filters +=
        unavailable_dlc.empty()
            ? ((show_be_servers && g_settings.steam->spoof_app_id != 346110)
                   ? 1
                   : 0)
            : (3 + unavailable_dlc.size());
  }
  const auto new_filters{
      std::make_unique_for_overwrite<ISteamMatchmakingServers::kv_pair[]>(
          num_new_filters)};
  std::ranges::copy_n(*filters, num_filters, new_filters.get());
  auto cur_filter{&new_filters[num_filters]};
  if (!show_be_servers) {
    std::ranges::copy("gamedataand", cur_filter->key.data());
    if (!show_unavailable_servers && g_settings.steam->spoof_app_id != 346110) {
      std::ranges::copy("SERVERUSESBATTLEYE_b:false,TEKWrapper:1",
                        cur_filter->value.data());
    } else {
      std::ranges::copy("SERVERUSESBATTLEYE_b:false", cur_filter->value.data());
    }
    ++cur_filter;
  }
  if (!show_unavailable_servers) {
    if (unavailable_dlc.empty()) {
      if (show_be_servers && g_settings.steam->spoof_app_id != 346110) {
        std::ranges::copy("gamedataand", cur_filter->key.data());
        std::ranges::copy("TEKWrapper:1", cur_filter->value.data());
      }
    } else {
      std::ranges::copy("or", cur_filter->key.data());
      *std::to_chars(cur_filter->value.begin(), cur_filter->value.end() - 1,
                     unavailable_dlc.size() + 2)
           .ptr = '\0';
      ++cur_filter;
      std::ranges::copy("gamedataand", cur_filter->key.data());
      std::ranges::copy("TEKWrapper:1", cur_filter->value.data());
      ++cur_filter;
      std::ranges::copy("nor", cur_filter->key.data());
      *std::to_chars(cur_filter->value.begin(), cur_filter->value.end() - 1,
                     unavailable_dlc.size())
           .ptr = '\0';
      for (const auto &dlc : unavailable_dlc) {
        ++cur_filter;
        std::ranges::copy("map", cur_filter->key.data());
        std::ranges::copy(dlc.cbegin(), dlc.cend() + 1,
                          cur_filter->value.data());
      }
    }
  }
  const auto ptr = new_filters.get();
  return SteamMatchmakingServers_RequestInternetServerList_orig(
      iface, app_id, &ptr, num_new_filters, response_handler);
}

/// Pointer to the original ISteamMatchmakingServers::SevrerRules method.
static ISteamMatchmakingServers::ServerRules_t
    *_Nullable SteamMatchmakingServers_ServerRules_orig;
/// Wrapper for ISteamMatchmakingServers::ServerRules, making it create a
///     wrapper for response handler.
static int SteamMatchmakingServers_ServerRules(
    void *_Nonnull iface, std::uint32_t ip, std::uint16_t port,
    ISteamMatchmakingServers::ISteamMatchmakingRulesResponse
        *_Nonnull response_handler) {
  const auto wrapper{new rules_response_wrapper{response_handler}};
  wrapper->query =
      SteamMatchmakingServers_ServerRules_orig(iface, ip, port, wrapper);
  return wrapper->query;
}

//===-- ISteamUGC method wrappers -----------------------------------------===//

/// Update handler for tek-steamclient jobs.
static void job_upd_handler(tek_sc_am_item_desc *_Nonnull desc,
                            tek_sc_am_upd_type upd_mask) {
  if (upd_mask & TEK_SC_AM_UPD_TYPE_state &&
      desc->job.state.load(std::memory_order::relaxed) ==
          TEK_SC_AM_JOB_STATE_stopped) {
    if (desc->current_manifest_id) {
      const std::scoped_lock lock{mods_mtx};
      mods.emplace_back(desc->id.ws_item_id);
    }
    const std::scoped_lock lock{ws_descs_mtx};
    ws_descs.erase(desc->id.ws_item_id);
  }
}

/// Wrapper for ISteamUGC::SubscribeItem, making it start a tek-steamclient
///    application manager job.
static std::uint64_t SteamUGC_SubscribeItem(void *, std::uint64_t id) {
  std::wstring am_path(MultiByteToWideChar(CP_UTF8, 0, ws_am_path.data(),
                                           ws_am_path.size(), nullptr, 0),
                       L'\0');
  MultiByteToWideChar(CP_UTF8, 0, ws_am_path.data(), ws_am_path.size(),
                      am_path.data(), am_path.length());
  std::wstring dir_path(MultiByteToWideChar(CP_UTF8, 0, ws_dir_path.data(),
                                            ws_dir_path.size(), nullptr, 0),
                        L'\0');
  MultiByteToWideChar(CP_UTF8, 0, ws_dir_path.data(), ws_dir_path.size(),
                      dir_path.data(), dir_path.length());
  ws_descs_mtx.lock();
  const auto [it, emplaced]{ws_descs.try_emplace(id)};
  ws_descs_mtx.unlock();
  if (emplaced) {
    steam::tsc::install_workshop_item(am_path.data(), dir_path.data(), id,
                                      job_upd_handler, &it->second);
  }
  return id;
}

/// Wrapper for ISteamUGC::GetNumSubscribedItems, making it return the number of
///    elements in @ref mods and @ref ws_descs.
static std::uint32_t SteamUGC_GetNumSubscribedItems(void *) {
  const std::scoped_lock lock{mods_mtx, ws_descs_mtx};
  return mods.size() + ws_descs.size();
}

/// Wrapper for ISteamUGC::GetSubscribedItems, making it return the elements
///    from @ref mods and IDs from @ref ws_descs.
static std::uint32_t SteamUGC_GetSubscribedItems(void *,
                                                 std::uint64_t *_Nonnull ids,
                                                 std::uint32_t max_entries) {
  std::uint32_t total_copied{};
  {
    const std::scoped_lock lock{mods_mtx};
    const auto n{std::min<std::size_t>(mods.size(), max_entries)};
    ids = std::ranges::copy_n(mods.cbegin(), n, ids).out;
    max_entries -= n;
    total_copied += n;
  }
  {
    const std::scoped_lock lock{ws_descs_mtx};
    const auto n{std::min<std::size_t>(ws_descs.size(), max_entries)};
    std::ranges::copy_n((ws_descs | std::views::keys).begin(), n, ids);
    total_copied += n;
  }
  return total_copied;
}

/// Wrapper for ISteamUGC::GetItemInstallInfo, making it return information
///    based on @ref mods, @ref ws_descs, and @ref ws_dir_path.
static bool SteamUGC_GetItemInstallInfo(void *, std::uint64_t id,
                                        std::uint64_t *_Nonnull size_on_disk,
                                        char *_Nullable folder,
                                        std::uint32_t folder_size,
                                        bool *_Nonnull legacy_item) {
  *size_on_disk = 0;
  if (const std::scoped_lock lock{mods_mtx}; std::ranges::contains(mods, id)) {
    *legacy_item = false;
    *std::format_to_n(folder, folder_size - 1, std::locale::classic(), "{}\\{}",
                      ws_dir_path, id)
         .out = '\0';

    return true;
  } else {
    if (folder_size) {
      *folder = '\0';
    }
    return false;
  }
}

/// Wrapper for ISteamUGC::GetItemUpdateInfo, making it return information
///    based on @ref ws_descs.
static bool SteamUGC_GetItemUpdateInfo(void *, std::uint64_t id,
                                       bool *_Nonnull need_update,
                                       bool *_Nonnull is_downloading,
                                       std::uint64_t *_Nonnull bytes_downloaded,
                                       std::uint64_t *_Nonnull bytes_total) {
  const std::scoped_lock lock{ws_descs_mtx};
  const auto it{ws_descs.find(id)};
  if (it == ws_descs.end()) {
    return false;
  }
  *need_update = true;
  *is_downloading = true;
  const auto desc{it->second};
  if (desc && desc->job.stage == TEK_SC_AM_JOB_STAGE_downloading) {
    *bytes_downloaded = desc->job.progress_current;
    *bytes_total = desc->job.progress_total;
  } else {
    *bytes_downloaded = 0;
    *bytes_total = 0;
  }
  return true;
}

//===-- ISteamUtils method wrappers ---------------------------------------===//

/// Pointer to the original ISteamUtils::IsAPICallCompleted method.
static ISteamUtils::IsAPICallCompleted_t
    *_Nullable SteamUtils_IsAPICallCompleted_orig;
/// Wrapper for ISteamUtils::IsAPICallCompleted, making it return status for
///    @ref ws_decs.
bool SteamUtils_IsAPICallCompleted(void *_Nonnull iface, std::uint64_t call,
                                   bool *_Nonnull failed) {
  {
    const std::scoped_lock lock{ws_descs_mtx};
    if (const auto it{ws_descs.find(call)}; it != ws_descs.end()) {
      *failed = !it->second;
      return true;
    }
  }
  return SteamUtils_IsAPICallCompleted_orig(iface, call, failed);
}

/// Pointer to the original ISteamUtils::GetAPICallResult method.
static ISteamUtils::GetAPICallResult_t
    *_Nullable SteamUtils_GetAPICallResult_orig;
/// Wrapper for ISteamUtils::GetAPICallResult, making it return results for
///    @ref ws_decs.
bool SteamUtils_GetAPICallResult(void *_Nonnull iface, std::uint64_t call,
                                 void *_Nonnull callback, int callback_size,
                                 int callback_idx, bool *_Nonnull failed) {
  if (callback_idx == 1313) {
    const std::scoped_lock lock{ws_descs_mtx};
    if (const auto it{ws_descs.find(call)}; it != ws_descs.end()) {
      if (callback_size >= static_cast<int>(sizeof(ISteamUGC::sub_result))) {
        *reinterpret_cast<ISteamUGC::sub_result *>(callback) = {
            .result = TEK_SC_CM_ERESULT_ok, .id = it->first};
      }
      *failed = !it->second;
      return true;
    }
  }
  return SteamUtils_GetAPICallResult_orig(iface, call, callback, callback_size,
                                          callback_idx, failed);
}

} // namespace

namespace cbs::steam {

//===-- Game callbacks ----------------------------------------------------===//

void settings_load_346110(const rapidjson::Document &doc) {
  const auto show_be_servers_m{doc.FindMember("show_be_servers")};
  if (show_be_servers_m != doc.MemberEnd() &&
      show_be_servers_m->value.IsBool()) {
    show_be_servers = show_be_servers_m->value.GetBool();
  }
  const auto show_unavailable_servers_m{
      doc.FindMember("show_unavailable_servers")};
  if (show_unavailable_servers_m != doc.MemberEnd() &&
      show_unavailable_servers_m->value.IsBool()) {
    show_unavailable_servers = show_unavailable_servers_m->value.GetBool();
  }
  const auto workshop_dir_path{doc.FindMember("workshop_dir_path")};
  if (workshop_dir_path != doc.MemberEnd() &&
      workshop_dir_path->value.IsString()) {
    ws_dir_path = {workshop_dir_path->value.GetString(),
                   workshop_dir_path->value.GetStringLength()};
  }
  const auto workshop_am_path{doc.FindMember("workshop_am_path")};
  if (workshop_am_path != doc.MemberEnd() &&
      workshop_am_path->value.IsString()) {
    ws_am_path = {workshop_am_path->value.GetString(),
                  workshop_am_path->value.GetStringLength()};
  } else {
    ws_am_path = ws_dir_path;
  }
}

void settings_save_346110(
    rapidjson::Writer<rapidjson::FileWriteStream> &writer) {
  std::string_view str{"show_be_servers"};
  writer.Key(str.data(), str.length());
  writer.Bool(show_be_servers);
  str = "show_unavailable_servers";
  writer.Key(str.data(), str.length());
  writer.Bool(show_unavailable_servers);
  if (!ws_dir_path.empty()) {
    str = "workshop_dir_path";
    writer.Key(str.data(), str.length());
    writer.String(ws_dir_path.data(), ws_dir_path.length());
  }
  if (!ws_am_path.empty()) {
    str = "workshop_am_path";
    writer.Key(str.data(), str.length());
    writer.String(ws_am_path.data(), ws_am_path.length());
  }
}

void steam_api_init_346110() {
  if (!show_be_servers || !show_unavailable_servers) {
    if (!show_unavailable_servers && g_settings.steam->spoof_app_id == 346110) {
      // Get the list of unowned DLC maps
      ISteamApps::BIsSubscribedApp_t *const ISteamApps_BIsSubscribedApp{
          ISteamApps::wrapper[ISteamApps::m_BIsSubscribedApp]};
      const auto ISteamApps_ptr{ISteamApps::wrapper.iface};
      const auto check_dlc{[ISteamApps_BIsSubscribedApp,
                            ISteamApps_ptr](std::uint32_t app_id,
                                            const std::string_view &&map_name) {
        if (!ISteamApps_BIsSubscribedApp(ISteamApps_ptr, app_id)) {
          unavailable_dlc.emplace_back(map_name);
        }
      }};
      check_dlc(473850, "TheCenter");
      check_dlc(512540, "ScorchedEarth");
      check_dlc(642250, "Ragnarok");
      check_dlc(708770, "Aberration");
      check_dlc(887380, "Extinction");
      check_dlc(1100810, "Valguero_P");
      check_dlc(1113410, "Genesis");
      check_dlc(1113410, "Gen2");
      check_dlc(1270830, "CrystalIsles");
      check_dlc(1691800, "LostIsland");
      check_dlc(3537070, "Aquatica");
    } // if (!show_unavailable_servers && spoof_app_id == 346110)
    // Setup wrappers for ISteamMatchmakingServers
    SteamMatchmakingServers_RequestInternetServerList_orig =
        ISteamMatchmakingServers::wrapper
            [ISteamMatchmakingServers::m_RequestInternetServerList];
    ISteamMatchmakingServers::wrapper
        [ISteamMatchmakingServers::m_RequestInternetServerList] =
            SteamMatchmakingServers_RequestInternetServerList;
    SteamMatchmakingServers_ServerRules_orig = ISteamMatchmakingServers::wrapper
        [ISteamMatchmakingServers::m_ServerRules];
    ISteamMatchmakingServers::wrapper[ISteamMatchmakingServers::m_ServerRules] =
        SteamMatchmakingServers_ServerRules;
    SteamMatchmakingServers_CancelServerQuery_orig = ISteamMatchmakingServers::
        wrapper[ISteamMatchmakingServers::m_CancelServerQuery];
  } // if (!show_be_servers || !show_unavailable_servers)
  if (g_settings.steam->spoof_app_id != 346110) {
    if (!ws_dir_path.empty()) {
      const std::filesystem::path path{
          std::u8string{ws_dir_path.cbegin(), ws_dir_path.cend()}};
      if (std::filesystem::exists(path)) {
        // Get initial mod list
        for (const auto &child : std::filesystem::directory_iterator{path}) {
          if (!child.is_directory()) {
            continue;
          }
          const auto &name{child.path().filename().native()};
          wchar_t *endptr;
          const auto id{std::wcstoull(name.data(), &endptr, 10)};
          if (id && endptr == std::to_address(name.end())) {
            mods.emplace_back(id);
          }
        }
        tsc::load();
      }
    }
    // Setup wrappers for ISteamUGC
    ISteamUGC::wrapper[ISteamUGC::m_GetNumSubscribedItems] =
        SteamUGC_GetNumSubscribedItems;
    ISteamUGC::wrapper[ISteamUGC::m_GetSubscribedItems] =
        SteamUGC_GetSubscribedItems;
    ISteamUGC::wrapper[ISteamUGC::m_GetItemInstallInfo] =
        SteamUGC_GetItemInstallInfo;
    if (!ws_dir_path.empty() && tsc::loaded) {
      // Setup wrappers for making mod downloads work
      ISteamUGC::wrapper[ISteamUGC::m_SubscribeItem] = SteamUGC_SubscribeItem;
      ISteamUGC::wrapper[ISteamUGC::m_GetItemUpdateInfo] =
          SteamUGC_GetItemUpdateInfo;
      SteamUtils_IsAPICallCompleted_orig =
          ISteamUtils::wrapper[ISteamUtils::m_IsAPICallCompleted];
      ISteamUtils::wrapper[ISteamUtils::m_IsAPICallCompleted] =
          SteamUtils_IsAPICallCompleted;
      SteamUtils_GetAPICallResult_orig =
          ISteamUtils::wrapper[ISteamUtils::m_GetAPICallResult];
      ISteamUtils::wrapper[ISteamUtils::m_GetAPICallResult] =
          reinterpret_cast<void *>(SteamUtils_GetAPICallResult);
    }
  }
}

} // namespace cbs::steam
} // namespace tek::game_runtime
