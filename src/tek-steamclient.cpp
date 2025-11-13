//===-- tek-steamclient.cpp - tek-steamclient library API implementation --===//
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
/// tek-steamclient library loader and API bindings implementation.
///
//===----------------------------------------------------------------------===//
#include "tek-steamclient.hpp"

#include "common.hpp" // IWYU pragma: keep
#include "settings.hpp"

#include <algorithm>
#include <atomic>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <process.h>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <tek-steamclient/am.h>
#include <tek-steamclient/base.h>
#include <tek-steamclient/cm.h>
#include <tek-steamclient/error.h>
#include <tek-steamclient/os.h>
#include <utility>
#include <vdf_parser.hpp>
#include <vector>

namespace tek::game_runtime::steamclient {

namespace {

//===-- Private variables -------------------------------------------------===//

/// libtek-steamclient-1.dll module handle.
static HMODULE module;
/// Pointer to the tek-steamclient library context.
static tek_sc_lib_ctx *_Nullable lib_ctx;
/// Pointer to the application manager instance.
static tek_sc_am *_Nullable am;

//===-- tek-steamclient function pointers ---------------------------------===//

static decltype(&tek_sc_lib_init) lib_init;
static decltype(&tek_sc_lib_cleanup) lib_cleanup;

static decltype(&tek_sc_cm_client_create) cm_client_create;
static decltype(&tek_sc_cm_client_destroy) cm_client_destroy;
static decltype(&tek_sc_cm_connect) cm_connect;
static decltype(&tek_sc_cm_disconnect) cm_disconnect;
static decltype(&tek_sc_cm_sign_in_anon) cm_sign_in_anon;
static decltype(&tek_sc_cm_get_access_token) cm_get_access_token;
static decltype(&tek_sc_cm_get_product_info) cm_get_product_info;

static decltype(&tek_sc_am_create) am_create;
static decltype(&tek_sc_am_destroy) am_destroy;
static decltype(&tek_sc_am_set_ws_dir) am_set_ws_dir;
static decltype(&tek_sc_am_get_item_desc) am_get_item_desc;
static decltype(&tek_sc_am_create_job) am_create_job;
static decltype(&tek_sc_am_run_job) am_run_job;

//===-- CM client callbacks -----------------------------------------------===//

/// The callback for CM client PICS DLC info received event.
///
/// @param [in, out] client
///    Pointer to the CM client instance that emitted the callback.
/// @param [in, out] data
///    Pointer to `tek_sc_cm_data_pics` associated with the request.
static void cb_dlc_info(tek_sc_cm_client *_Nonnull client, void *_Nonnull data,
                        void *) {
  auto &data_pics{*reinterpret_cast<tek_sc_cm_data_pics *>(data)};
  if (!tek_sc_err_success(&data_pics.result)) {
    delete[] data_pics.app_entries;
    delete &data_pics;
    cm_disconnect(client);
    return;
  }
  bool save_settings{};
  for (const auto &entry :
       std::span{data_pics.app_entries,
                 static_cast<std::size_t>(data_pics.num_app_entries)}) {
    if (!tek_sc_err_success(&entry.result)) {
      continue;
    }
    const std::string_view view{reinterpret_cast<const char *>(entry.data),
                                static_cast<std::size_t>(entry.data_size)};
    std::error_code ec;
    const auto vdf{tyti::vdf::read(view.cbegin(), view.cend(), ec)};
    if (ec != std::error_code{}) {
      continue;
    }
    const auto common{vdf.childs.find("common")};
    if (common != vdf.childs.end()) {
      const auto &common_m{common->second};
      const auto name{common_m->attribs.find("name")};
      if (name != common_m->attribs.end()) {
        g_settings.steam->dlc.emplace_back(entry.id, std::move(name->second));
        g_settings.steam->installed_dlc.emplace(entry.id);
        save_settings = true;
      }
    }
  }
  if (save_settings) {
    g_settings.save();
  }
  delete[] data_pics.app_entries;
  delete &data_pics;
  cm_disconnect(client);
}

/// The callback for CM client DLC PICS access token received event.
///
/// @param [in, out] client
///    Pointer to the CM client instance that emitted the callback.
/// @param [in, out] data
///    Pointer to `tek_sc_cm_data_pics` associated with the request.
static void cb_dlc_access_token(tek_sc_cm_client *_Nonnull client,
                                void *_Nonnull data, void *) {
  auto &data_pics{*reinterpret_cast<tek_sc_cm_data_pics *>(data)};
  if (!tek_sc_err_success(&data_pics.result)) {
    delete[] data_pics.app_entries;
    delete &data_pics;
    cm_disconnect(client);
    return;
  }
  cm_get_product_info(client, &data_pics, cb_dlc_info, 2500);
}

/// The callback for CM client PICS app info received event.
///
/// @param [in, out] client
///    Pointer to the CM client instance that emitted the callback.
/// @param [in, out] data
///    Pointer to `tek_sc_cm_data_pics` associated with the request.
static void cb_app_info(tek_sc_cm_client *_Nonnull client, void *_Nonnull data,
                        void *) {
  auto &data_pics{*reinterpret_cast<tek_sc_cm_data_pics *>(data)};
  if (!tek_sc_err_success(&data_pics.result) ||
      !tek_sc_err_success(&data_pics.app_entries->result)) {
    delete data_pics.app_entries;
    delete &data_pics;
    cm_disconnect(client);
    return;
  }
  const std::string_view view{
      reinterpret_cast<const char *>(data_pics.app_entries->data),
      static_cast<std::size_t>(data_pics.app_entries->data_size)};
  std::error_code ec;
  const auto vdf{tyti::vdf::read(view.cbegin(), view.cend(), ec)};
  if (ec != std::error_code{}) {
    delete data_pics.app_entries;
    delete &data_pics;
    cm_disconnect(client);
    return;
  }
  std::vector<std::uint32_t> new_dlc;
  const auto extended{vdf.childs.find("extended")};
  if (extended != vdf.childs.end()) {
    const auto &extended_m{extended->second};
    const auto listofdlc{extended_m->attribs.find("listofdlc")};
    if (listofdlc != extended_m->attribs.end()) {
      const auto &dlc{g_settings.steam->dlc};
      for (const auto &&id_view : listofdlc->second | std::views::split(',') |
                                      std::views::transform([](auto &&segment) {
                                        return std::string_view{segment};
                                      })) {
        if (std::uint32_t id;
            std::from_chars(id_view.cbegin(), id_view.cend(), id).ec ==
                std::errc{} &&
            !std::ranges::contains(dlc | std::views::keys, id)) {
          new_dlc.emplace_back(id);
        }
      }
    }
  }
  if (new_dlc.empty()) {
    delete data_pics.app_entries;
    delete &data_pics;
    cm_disconnect(client);
    return;
  }
  delete data_pics.app_entries;
  data_pics.app_entries = new tek_sc_cm_pics_entry[new_dlc.size()]();
  for (auto &&[id, entry] : std::views::zip(
           new_dlc, std::span{data_pics.app_entries, new_dlc.size()})) {
    entry.id = id;
  }
  data_pics.num_app_entries = new_dlc.size();
  cm_get_access_token(client, &data_pics, cb_dlc_access_token, 2500);
}

/// The callback for CM client PICS access token received event.
///
/// @param [in, out] client
///    Pointer to the CM client instance that emitted the callback.
/// @param [in, out] data
///    Pointer to `tek_sc_cm_data_pics` associated with the request.
static void cb_access_token(tek_sc_cm_client *_Nonnull client,
                            void *_Nonnull data, void *) {
  auto &data_pics{*reinterpret_cast<tek_sc_cm_data_pics *>(data)};
  if (!tek_sc_err_success(&data_pics.result) ||
      !tek_sc_err_success(&data_pics.app_entries->result)) {
    delete data_pics.app_entries;
    delete &data_pics;
    cm_disconnect(client);
    return;
  }
  cm_get_product_info(client, &data_pics, cb_app_info, 2500);
}

/// The callback for CM client signed in event.
///
/// @param [in, out] client
///    Pointer to the CM client instance that emitted the callback.
/// @param [in] data
///    Pointer to `tek_sc_err` indicating the result of the sign-in attempt.
static void cb_signed_in(tek_sc_cm_client *_Nonnull client, void *_Nonnull data,
                         void *) {
  if (!tek_sc_err_success(reinterpret_cast<const tek_sc_err *>(data))) {
    cm_disconnect(client);
    return;
  }
  auto &data_pics{*new tek_sc_cm_data_pics{}};
  data_pics.app_entries = new tek_sc_cm_pics_entry();
  data_pics.app_entries->id = g_settings.steam->app_id;
  data_pics.num_app_entries = 1;
  data_pics.timeout_ms = 2500;
  cm_get_access_token(client, &data_pics, cb_access_token, 2500);
}

/// The callback for CM client connected event.
///
/// @param [in, out] client
///    Pointer to the CM client instance that emitted the callback.
/// @param [in] data
///    Pointer to `tek_sc_err` indicating the result of connection.
/// @param [in, out] user_data
///    Pointer to the futex associated with @p client.
static void cb_connected(tek_sc_cm_client *_Nonnull client, void *_Nonnull data,
                         void *_Nonnull user_data) {
  if (tek_sc_err_success(reinterpret_cast<const tek_sc_err *>(data))) {
    cm_sign_in_anon(client, cb_signed_in, 2500);
  } else {
    reinterpret_cast<std::atomic_bool *>(user_data)->store(
        true, std::memory_order::relaxed);
    WakeByAddressSingle(user_data);
  }
}

/// The callback for CM client disconnected event.
///
/// @param [in, out] user_data
///    Pointer to the futex associated with the client.
static void cb_disconnected(tek_sc_cm_client *, void *,
                            void *_Nonnull user_data) {
  reinterpret_cast<std::atomic_bool *>(user_data)->store(
      true, std::memory_order::relaxed);
  WakeByAddressSingle(user_data);
}

//===-- Steam Workshop item install processing ----------------------------===//

/// Temporary storage for passing arguments to the job thread.
struct ws_job_args {
  std::uint64_t id;
  tek_sc_am_job_upd_func *_Nullable upd_handler;
  tek_sc_am_item_desc *_Nullable *_Nonnull item_desc;
};

/// Steam Workshop item download job procedure.
static unsigned ws_job_proc(void *_Nonnull arg) {
  const auto args_ptr{reinterpret_cast<ws_job_args *>(arg)};
  const auto args{*args_ptr};
  delete args_ptr;
  const tek_sc_item_id item_id{.app_id = g_settings.steam->app_id,
                               .depot_id = g_settings.steam->app_id,
                               .ws_item_id = args.id};
  auto &desc = *args.item_desc;
  desc = am_get_item_desc(am, &item_id);
  if (!desc || !(desc->status & TEK_SC_AM_ITEM_STATUS_job)) {
    auto const res = am_create_job(am, &item_id, 0, false, &desc).primary;
    if (res) {
      return res;
    }
  }
  return static_cast<unsigned>(am_run_job(am, desc, args.upd_handler).primary);
}

} // namespace

//===-- Internal functions ------------------------------------------------===//

void load() {
  if (loaded) {
    return;
  }
  std::wstring path;
  const auto &tek_sc_path{g_settings.steam->tek_sc_path};
  if (!tek_sc_path.empty()) {
    path.resize(MultiByteToWideChar(CP_UTF8, 0, tek_sc_path.data(),
                                    tek_sc_path.size(), nullptr, 0));
    MultiByteToWideChar(CP_UTF8, 0, tek_sc_path.data(), tek_sc_path.size(),
                        path.data(), path.length());
  }
  module =
      LoadLibraryW(path.empty() ? L"libtek-steamclient-1.dll" : path.data());
  if (!module) {
    return;
  }
  lib_init = reinterpret_cast<decltype(lib_init)>(
      GetProcAddress(module, "tek_sc_lib_init"));
  if (!lib_init) {
    goto free_lib;
  }
  lib_cleanup = reinterpret_cast<decltype(lib_cleanup)>(
      GetProcAddress(module, "tek_sc_lib_cleanup"));
  if (!lib_cleanup) {
    goto free_lib;
  }
  cm_client_create = reinterpret_cast<decltype(cm_client_create)>(
      GetProcAddress(module, "tek_sc_cm_client_create"));
  if (!cm_client_create) {
    goto free_lib;
  }
  cm_client_destroy = reinterpret_cast<decltype(cm_client_destroy)>(
      GetProcAddress(module, "tek_sc_cm_client_destroy"));
  if (!cm_client_destroy) {
    goto free_lib;
  }
  cm_connect = reinterpret_cast<decltype(cm_connect)>(
      GetProcAddress(module, "tek_sc_cm_connect"));
  if (!cm_connect) {
    goto free_lib;
  }
  cm_disconnect = reinterpret_cast<decltype(cm_disconnect)>(
      GetProcAddress(module, "tek_sc_cm_disconnect"));
  if (!cm_disconnect) {
    goto free_lib;
  }
  cm_sign_in_anon = reinterpret_cast<decltype(cm_sign_in_anon)>(
      GetProcAddress(module, "tek_sc_cm_sign_in_anon"));
  if (!cm_sign_in_anon) {
    goto free_lib;
  }
  cm_get_access_token = reinterpret_cast<decltype(cm_get_access_token)>(
      GetProcAddress(module, "tek_sc_cm_get_access_token"));
  if (!cm_get_access_token) {
    goto free_lib;
  }
  cm_get_product_info = reinterpret_cast<decltype(cm_get_product_info)>(
      GetProcAddress(module, "tek_sc_cm_get_product_info"));
  if (!cm_get_product_info) {
    goto free_lib;
  }
  am_create = reinterpret_cast<decltype(am_create)>(
      GetProcAddress(module, "tek_sc_am_create"));
  if (!am_create) {
    goto free_lib;
  }
  am_destroy = reinterpret_cast<decltype(am_destroy)>(
      GetProcAddress(module, "tek_sc_am_destroy"));
  if (!am_destroy) {
    goto free_lib;
  }
  am_set_ws_dir = reinterpret_cast<decltype(am_set_ws_dir)>(
      GetProcAddress(module, "tek_sc_am_set_ws_dir"));
  if (!am_set_ws_dir) {
    goto free_lib;
  }
  am_get_item_desc = reinterpret_cast<decltype(am_get_item_desc)>(
      GetProcAddress(module, "tek_sc_am_get_item_desc"));
  if (!am_get_item_desc) {
    goto free_lib;
  }
  am_create_job = reinterpret_cast<decltype(am_create_job)>(
      GetProcAddress(module, "tek_sc_am_create_job"));
  if (!am_create_job) {
    goto free_lib;
  }
  am_run_job = reinterpret_cast<decltype(am_run_job)>(
      GetProcAddress(module, "tek_sc_am_run_job"));
  if (!am_run_job) {
    goto free_lib;
  }
  lib_ctx = lib_init(true, true);
  if (!lib_ctx) {
    goto free_lib;
  }
  loaded = true;
  return;
free_lib:
  FreeLibrary(module);
  module = nullptr;
}

void unload() {
  loaded = false;
  if (!module) {
    return;
  }
  if (am) {
    am_destroy(am);
  }
  if (!lib_ctx) {
    goto free_lib;
  }
  lib_cleanup(lib_ctx);
  lib_ctx = nullptr;
free_lib:
  FreeLibrary(module);
  module = nullptr;
}

void update_dlc() {
  std::atomic_bool done{};
  const auto client{cm_client_create(lib_ctx, &done)};
  if (!client) {
    return;
  }
  cm_connect(client, cb_connected, 2500, cb_disconnected);
  do {
    bool cmp{};
    if (!WaitOnAddress(&done, &cmp, sizeof cmp, 10000) &&
        GetLastError() == ERROR_TIMEOUT) {
      break;
    }
  } while (!done.load(std::memory_order::relaxed));
  cm_client_destroy(client);
}

bool install_workshop_item(const tek_sc_os_char *ws_dir, std::uint64_t id,
                           tek_sc_am_job_upd_func *upd_handler,
                           tek_sc_am_item_desc **item_desc) {
  if (!am) {
    tek_sc_err err;
    am = am_create(lib_ctx, ws_dir, &err);
    if (!am) {
      return false;
    }
    am_set_ws_dir(am, ws_dir);
  }
  const auto args{new ws_job_args{
      .id = id, .upd_handler = upd_handler, .item_desc = item_desc}};
  const auto thread{_beginthreadex(nullptr, 0, ws_job_proc, args, 0, nullptr)};
  if (thread) {
    CloseHandle(reinterpret_cast<HANDLE>(thread));
  } else {
    delete args;
  }
  return true;
}

} // namespace tek::game_runtime::steamclient
