//===-- main.cpp - DLL entry point ----------------------------------------===//
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
/// Implementation of the `DllMain` function.
///
//===----------------------------------------------------------------------===//
#include "common.hpp" // IWYU pragma: keep

#include "game_cbs.hpp"
#include "settings.hpp"
#include "steam/api.hpp"
#include "steam/tsc.hpp"

#include <MinHook.h>
#include <algorithm>
#include <array>
#include <format>

namespace tek::game_runtime {

extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
  switch (reason) {
  case DLL_PROCESS_ATTACH: {
    if (!g_settings.load()) {
      return FALSE;
    }
    const auto mh_res{MH_Initialize()};
    if (mh_res != MH_OK) {
      std::array<WCHAR, 512> msg;
      if (!MultiByteToWideChar(CP_UTF8, 0, MH_StatusToString(mh_res), -1,
                               msg.data(), msg.size())) {
        std::ranges::copy(L"Unknown", msg.data());
      }
      display_error(std::format(L"Failed to initialize MinHook: ({}) {}",
                                static_cast<int>(mh_res), msg.data())
                        .data());
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
      steam::wrap_funcs();
      break;
    }
    return TRUE;
  }
  case DLL_PROCESS_DETACH:
    switch (g_settings.store) {
    case store_type::steam:
      steam::tsc::unload();
      break;
    }
    MH_Uninitialize();
    return TRUE;
  default:
    return TRUE;
  }
}

} // namespace tek::game_runtime
