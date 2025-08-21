//===-- settings.hpp - settings interface declarations --------------------===//
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
/// Definitions of TEK Game Runtime settings types and declaration of global
///    settings object instance.
///
//===----------------------------------------------------------------------===//
#pragma once

#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace tek::game_runtime {

/// Types of supported stores that distribute games.
enum class store_type { steam };

/// Options for Steam games.
struct steam_options {
  /// Steam application ID.
  std::uint32_t app_id;
  /// Steam application ID that the runtime will use to initialize Steam
  ///    API. If zero, it will use @ref app_id and fallback to 480 if
  ///    `SteamAPI_Init` fails. After `SteamAPI_Init`, receives the effective
  ///    application ID that was used to initialize Steam API.
  std::uint32_t spoof_app_id;
  /// List of "owned" DLC app IDs and names.
  std::vector<std::pair<std::uint32_t, std::string>> dlc;
  /// List of "installed" app IDs.
  std::set<std::uint32_t> installed_dlc;
  /// Path to the `libtek-steamclient-1.dll` to load. If not specified/empty,
  ///     Windows' default DLL search behavior is used.
  std::string tek_sc_path;
  /// Value indicating whether to attempt to use tek-steamclient to update DLC
  ///    list.
  bool auto_update_dlc;
};

/// TEK Game Runtime settings structure.
struct [[gnu::visibility("internal")]] settings {
  /// Type of the store that the game is distributed on.
  store_type store;
  /// Store-specific options for `store_type::steam`.
  std::unique_ptr<steam_options> steam;

  /// Load settings from the file.
  ///
  /// @return Value indicating whether loading succeeded.
  bool load();
  /// Save current settings to the file.
  void save();
};

/// Global settings object.
[[gnu::visibility("internal")]]
inline settings g_settings;

} // namespace tek::game_runtime
