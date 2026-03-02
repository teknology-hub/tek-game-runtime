//===-- isteammatchmaking.hpp - ISteamMatchmaking interface declarations --===//
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
/// Declarations of ISteamMatchmaking interface wrapper and related types.
///
//===----------------------------------------------------------------------===//
#pragma once
#include "steam/api.hpp"

#include <algorithm>
#include <numeric>
#include <ranges>

namespace tek::game_runtime::steam::ISteamMatchmaking {

// Virtual method enumeration.
enum {
  m_GetFavoriteGameCount,
  m_GetFavoriteGame,
  m_AddFavoriteGame,
  m_RemoveFavoriteGame,
  m_RequestLobbyList,
  m_AddRequestLobbyListStringFilter,
  m_AddRequestLobbyListNumericalFilter,
  m_AddRequestLobbyListNearValueFilter,
  m_AddRequestLobbyListFilterSlotsAvailable,
  m_AddRequestLobbyListDistanceFilter,
  m_AddRequestLobbyListResultCountFilter,
  m_AddRequestLobbyListCompatibleMembersFilter,
  m_GetLobbyByIndex,
  m_CreateLobby,
  m_JoinLobby,
  m_LeaveLobby,
  m_InviteUserToLobby,
  m_GetNumLobbyMembers,
  m_GetLobbyMemberByIndex,
  m_GetLobbyData,
  m_SetLobbyData,
  m_GetLobbyDataCount,
  m_GetLobbyDataByIndex,
  m_DeleteLobbyData,
  m_GetLobbyMemberData,
  m_SetLobbyMemberData,
  m_SendLobbyChatMsg,
  m_GetLobbyChatEntry,
  m_RequestLobbyData,
  m_SetLobbyGameServer,
  m_GetLobbyGameServer,
  m_SetLobbyMemberLimit,
  m_GetLobbyMemberLimit,
  m_SetLobbyType,
  m_SetLobbyJoinable,
  m_GetLobbyOwner,
  m_SetLobbyOwner,
  m_SetLinkedLobby,
  num_methods
};

/// Wrapper descriptor for ISteamMatchmaking interface.
inline wrapper_desc<num_methods> wrapper;

/// Initialize the wrapper.
///
/// @param [in, out] iface
///    Pointer to the interface instance.
constexpr void setup(cpp_interface *_Nonnull iface) noexcept {
  if (ver >= 0x00010017002D005D) { // 01.23.45.93
    // "SteamMatchMaking009", used since Steamworks SDK v1.17
    wrapper.num_methods = 38;
    std::ranges::iota(
        std::ranges::subrange(wrapper.vm_idxs, wrapper.num_methods), 0);
  } else {
    // "SteamMatchMaking008", used in older supported Steamworks SDK versions
    wrapper.num_methods = 36;
    wrapper.vm_idxs[m_GetFavoriteGameCount] = 0;
    wrapper.vm_idxs[m_GetFavoriteGame] = 1;
    wrapper.vm_idxs[m_AddFavoriteGame] = 2;
    wrapper.vm_idxs[m_RemoveFavoriteGame] = 3;
    wrapper.vm_idxs[m_RequestLobbyList] = 4;
    wrapper.vm_idxs[m_AddRequestLobbyListStringFilter] = 5;
    wrapper.vm_idxs[m_AddRequestLobbyListNumericalFilter] = 6;
    wrapper.vm_idxs[m_AddRequestLobbyListNearValueFilter] = 7;
    wrapper.vm_idxs[m_AddRequestLobbyListFilterSlotsAvailable] = 8;
    wrapper.vm_idxs[m_AddRequestLobbyListDistanceFilter] = 9;
    wrapper.vm_idxs[m_AddRequestLobbyListResultCountFilter] = 10;
    wrapper.vm_idxs[m_GetLobbyByIndex] = 11;
    wrapper.vm_idxs[m_CreateLobby] = 12;
    wrapper.vm_idxs[m_JoinLobby] = 13;
    wrapper.vm_idxs[m_LeaveLobby] = 14;
    wrapper.vm_idxs[m_InviteUserToLobby] = 15;
    wrapper.vm_idxs[m_GetNumLobbyMembers] = 16;
    wrapper.vm_idxs[m_GetLobbyMemberByIndex] = 17;
    wrapper.vm_idxs[m_GetLobbyData] = 18;
    wrapper.vm_idxs[m_SetLobbyData] = 19;
    wrapper.vm_idxs[m_GetLobbyDataCount] = 20;
    wrapper.vm_idxs[m_GetLobbyDataByIndex] = 21;
    wrapper.vm_idxs[m_DeleteLobbyData] = 22;
    wrapper.vm_idxs[m_GetLobbyMemberData] = 23;
    wrapper.vm_idxs[m_SetLobbyMemberData] = 24;
    wrapper.vm_idxs[m_SendLobbyChatMsg] = 25;
    wrapper.vm_idxs[m_GetLobbyChatEntry] = 26;
    wrapper.vm_idxs[m_RequestLobbyData] = 27;
    wrapper.vm_idxs[m_SetLobbyGameServer] = 28;
    wrapper.vm_idxs[m_GetLobbyGameServer] = 29;
    wrapper.vm_idxs[m_SetLobbyMemberLimit] = 30;
    wrapper.vm_idxs[m_GetLobbyMemberLimit] = 31;
    wrapper.vm_idxs[m_SetLobbyType] = 32;
    wrapper.vm_idxs[m_SetLobbyJoinable] = 33;
    wrapper.vm_idxs[m_GetLobbyOwner] = 34;
    wrapper.vm_idxs[m_SetLobbyOwner] = 35;
  }
  wrapper.orig_vtable = iface->vtable;
  wrapper.iface = iface;
  std::ranges::copy_n(iface->vtable, wrapper.num_methods,
                      wrapper.vtable.begin());
  iface->vtable = wrapper.vtable.data();
}

/// Get interface version string for Steamworks SDK v1.37+.
constexpr const char *_Nonnull get_ver_str() { return "SteamMatchMaking009"; }

} // namespace tek::game_runtime::steam::ISteamMatchmaking
