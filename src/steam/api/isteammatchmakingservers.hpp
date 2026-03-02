//===-- isteammatchmakingservers.hpp - ISteamMatchmakingServers interface -===//
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
/// Declarations of ISteamMatchmakingServers interface wrapper and related
///    types.
///
//===----------------------------------------------------------------------===//
#pragma once
#include "steam/api.hpp"

#include "common.hpp" // IWYU pragma: keep

#include <algorithm>
#include <array>
#include <cstdint>
#include <numeric>
#include <ranges>

namespace tek::game_runtime::steam::ISteamMatchmakingServers {

// Virtual method enumeration.
enum {
  m_RequestInternetServerList,
  m_RequestLANServerList,
  m_RequestFriendsServerList,
  m_RequestFavoritesServerList,
  m_RequestHistoryServerList,
  m_RequestSpectatorServerList,
  m_ReleaseRequest,
  m_GetServerDetails,
  m_CancelQuery,
  m_RefreshQuery,
  m_IsRefreshing,
  m_GetServerCount,
  m_RefreshServer,
  m_PingServer,
  m_PlayerDetails,
  m_ServerRules,
  m_CancelServerQuery,
  num_methods
};

struct kv_pair {
  std::array<char, 256> key;
  std::array<char, 256> value;
};

struct ISteamMatchmakingRulesResponse {
  virtual void RulesResponded(const char *_Nonnull key,
                              const char *_Nonnull value) = 0;
  virtual void RulesFailedToRespond() = 0;
  virtual void RulesRefreshComplete() = 0;
};

using RequestInternetServerList_t =
    void *_Nonnull(void *_Nonnull iface, std::uint32_t app_id,
                   const kv_pair *const _Nonnull *_Nullable filters,
                   std::uint32_t num_filters, void *_Nonnull response_handler);
using ServerRules_t =
    int(void *_Nonnull iface, std::uint32_t ip, std::uint16_t port,
        ISteamMatchmakingRulesResponse *_Nonnull response_handler);
using CancelServerQuery_t = void(void *_Nonnull iface, int query);

/// Wrapper descriptor for ISteamMatchmakingServers interface.
inline wrapper_desc<num_methods> wrapper;

/// Initialize the wrapper.
///
/// @param [in, out] iface
///    Pointer to the interface instance.
constexpr void setup(cpp_interface *_Nonnull iface) noexcept {
  wrapper.num_methods = 17;
  std::ranges::iota(std::ranges::subrange(wrapper.vm_idxs, wrapper.num_methods),
                    0);
  wrapper.orig_vtable = iface->vtable;
  wrapper.iface = iface;
  std::ranges::copy_n(iface->vtable, wrapper.num_methods,
                      wrapper.vtable.begin());
  iface->vtable = wrapper.vtable.data();
}

/// Get interface version string for Steamworks SDK v1.37+.
constexpr const char *_Nonnull get_ver_str() {
  return "SteamMatchMakingServers002";
}

} // namespace tek::game_runtime::steam::ISteamMatchmakingServers
