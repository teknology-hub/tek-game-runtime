//===-- main.cpp - DLL entry point ----------------------------------------===//
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
/// Implementation of the `DllMain` function.
///
//===----------------------------------------------------------------------===//
#include "common.hpp" // IWYU pragma: keep

#include "game_cbs.hpp"
#include "settings.hpp"
#include "steam_api.hpp"
#include "tek-steamclient.hpp"

namespace tek::game_runtime {

extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
  switch (reason) {
  case DLL_PROCESS_ATTACH: {
    if (!g_settings.load()) {
      return FALSE;
    }
    const auto cb{get_dllmain_cb()};
    if (cb) {
      if (!cb()) {
        return FALSE;
      }
    }
    switch (g_settings.store) {
    case store_type::steam:
      steam_api::wrap_init();
      break;
    }
    return TRUE;
  }
  case DLL_PROCESS_DETACH:
    steamclient::unload();
    return TRUE;
  default:
    return TRUE;
  }
}

} // namespace tek::game_runtime
