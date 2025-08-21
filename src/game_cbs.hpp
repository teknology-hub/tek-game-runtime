//===-- game_cbs.hpp - game callback getters ------------------------------===//
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
/// Definitions of functions that get game callbacks for various TEK Game
///    Runtime lifetime stages.
///
//===----------------------------------------------------------------------===//
#pragma once

#include "common.hpp" // IWYU pragma: keep
#include "settings.hpp"

#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>

namespace tek::game_runtime {

/// The callback that may be used to load game-specific settings.
///
/// @param [in] doc
///    RapidJSON document describing settings file contents.
using settings_load_cb_t = void(const rapidjson::Document &doc);

/// The callback that may be used to save game-specific settings.
///
/// @param [in, out] writer
///    RapidJSON writer that can be used to write data to the settings file.
using settings_save_cb_t =
    void(rapidjson::Writer<rapidjson::FileWriteStream> &writer);

/// The earliest callback that runs in `DllMain` right after loading settings.
///
/// @return Value indicating whether the callback succeeded. If `false` is
///    returned, DLL loading fails.
using dllmain_cb_t = bool();

/// The callback that runs in SteamAPI_Init wrapper after setting up all
///    interface wrappers. May be used to setup game-specific Steam API method
///    wrappers.
using steam_api_init_cb_t = void();

namespace cbs {

namespace steam {

settings_load_cb_t settings_load_346110;
settings_save_cb_t settings_save_346110;
steam_api_init_cb_t steam_api_init_346110;

settings_load_cb_t settings_load_2399830;
settings_save_cb_t settings_save_2399830;
dllmain_cb_t dllmain_2399830;
steam_api_init_cb_t steam_api_init_2399830;

} // namespace steam

} // namespace cbs

/// Get pointer to the settings load callback for current game, if it exists.
///
/// @return Pointer to the callback function for current game, or `nullptr` if
///    it doesn't exist.
static inline settings_load_cb_t *_Nullable get_settings_load_cb() noexcept {
  switch (g_settings.store) {
  case store_type::steam:
    switch (g_settings.steam->app_id) {
    case 346110:
      return cbs::steam::settings_load_346110;
    case 2399830:
      return cbs::steam::settings_load_2399830;
    }
    break;
  }
  return nullptr;
}

/// Get pointer to the settings save callback for current game, if it exists.
///
/// @return Pointer to the callback function for current game, or `nullptr` if
///    it doesn't exist.
static inline settings_save_cb_t *_Nullable get_settings_save_cb() noexcept {
  switch (g_settings.store) {
  case store_type::steam:
    switch (g_settings.steam->app_id) {
    case 346110:
      return cbs::steam::settings_save_346110;
    case 2399830:
      return cbs::steam::settings_save_2399830;
    }
    break;
  }
  return nullptr;
}

/// Get pointer to the `DllMain` callback for current game, if it exists.
///
/// @return Pointer to the callback function for current game, or `nullptr` if
///    it doesn't exist.
static inline dllmain_cb_t *_Nullable get_dllmain_cb() noexcept {
  switch (g_settings.store) {
  case store_type::steam:
    switch (g_settings.steam->app_id) {
    case 2399830:
      return cbs::steam::dllmain_2399830;
    }
    break;
  }
  return nullptr;
}

/// Get pointer to the `SteamAPI_Init` callback for current game, if it exists.
///
/// @return Pointer to the callback function for current game, or `nullptr` if
///    it doesn't exist.
static inline steam_api_init_cb_t *_Nullable get_steam_api_init_cb() noexcept {
  switch (g_settings.store) {
  case store_type::steam:
    switch (g_settings.steam->app_id) {
    case 346110:
      return cbs::steam::steam_api_init_346110;
    case 2399830:
      return cbs::steam::steam_api_init_2399830;
    }
    break;
  }
  return nullptr;
}

} // namespace tek::game_runtime
