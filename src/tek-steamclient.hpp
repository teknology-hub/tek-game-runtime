//===-- tek-steamclient.hpp - tek-steamclient library API -----------------===//
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
/// Declarations for tek-steamclient library API that uses dynamically loaded
///    `libtek-steamclient-1.dll` to operate.
///
//===----------------------------------------------------------------------===//
#pragma once

#include "common.hpp" // IWYU pragma: keep

#include <cstdint>
#include <tek-steamclient/am.h>
#include <tek-steamclient/os.h>

namespace tek::game_runtime::steamclient {

/// Value indicating whether the library is currently loaded.
inline bool loaded;

/// Attempt to load the library.
[[gnu::visibility("internal")]]
void load();

/// Free all library resources and unload it, if it's loaded.
[[gnu::visibility("internal")]]
void unload();

/// Update DLC list for current game.
[[gnu::visibility("internal")]]
void update_dlc();

/// Begin installation of specified Steam Workshop item via application manager
///    interface.
///
/// @param [in] am_dir
///    Path to the game root directory to initialize application manager
///    instance with, as a null-terminated string.
/// @param [in] ws_dir
///    Path to the base directory for Steam Workshop items, as a null-terminated
///    string.
/// @param id
///    ID of the Steam Workshop item to install.
/// @param upd_handler
///    Optional pointer to the job update handler function to use.
/// @param [out] item_desc
///    Address of variable that receives pointer to the @ref tek_sc_am_item_desc
///    for the item shortly after starting the installation job.
/// @return Value indicating whether the installation job was started, or
///    application manager API is not available.
[[gnu::visibility("internal")]]
bool install_workshop_item(const tek_sc_os_char *_Nonnull am_dir,
                           const tek_sc_os_char *_Nonnull ws_dir,
                           std::uint64_t id,
                           tek_sc_am_job_upd_func *_Nullable upd_handler,
                           tek_sc_am_item_desc *_Nullable *_Nonnull item_desc);

} // namespace tek::game_runtime::steamclient
