//===-- isteaminventory.hpp - ISteamInventory interface declarations ------===//
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
/// Declarations of ISteamInventory interface wrapper and related types.
///
//===----------------------------------------------------------------------===//
#pragma once
#include "steam/api.hpp"

#include "common.hpp" // IWYU pragma: keep

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <ranges>

namespace tek::game_runtime::steam::ISteamInventory {

// Virtual method enumeration.
enum {
  m_GetResultStatus,
  m_GetResultItems,
  m_GetResultItemProperty,
  m_GetResultTimestamp,
  m_CheckResultSteamID,
  m_DestroyResult,
  m_GetAllItems,
  m_GetItemsByID,
  m_SerializeResult,
  m_DeserializeResult,
  m_GenerateItems,
  m_GrantPromoItems,
  m_AddPromoItem,
  m_AddPromoItems,
  m_ConsumeItem,
  m_ExchangeItems,
  m_TransferItemQuantity,
  m_SendItemDropHeartbeat,
  m_TriggerItemDrop,
  m_TradeItems,
  m_LoadItemDefinitions,
  m_GetItemDefinitionIDs,
  m_GetItemDefinitionProperty,
  m_RequestEligiblePromoItemDefinitionsIDs,
  m_GetEligiblePromoItemDefinitionIDs,
  m_StartPurchase,
  m_RequestPrices,
  m_GetNumItemsWithPrices,
  m_GetItemsWithPrices,
  m_GetItemPrice,
  m_StartUpdateProperties,
  m_RemoveProperty,
  m_SetPropertyFloat,
  m_SetPropertyInt64,
  m_SetPropertyBool,
  m_SetPropertyString,
  m_SubmitUpdateProperties,
  m_InspectItem,
  num_methods
};

using SteamInventoryResult_t = std::int32_t;

using GetAllItems_t = bool(void *_Nonnull iface,
                           SteamInventoryResult_t *_Nonnull result_handle);

/// Wrapper descriptor for ISteamApps interface.
inline wrapper_desc<num_methods> wrapper;

/// Initialize the wrapper.
///
/// @param [in, out] iface
///    Pointer to the interface instance.
constexpr void setup(cpp_interface *_Nonnull iface) noexcept {
  if (ver >= 0x0004005F0014001E) { // 04.95.20.30
    // "STEAMINVENTORY_INTERFACE_V003", used since Steamworks SDK v1.43
    wrapper.num_methods = 38;
    std::ranges::iota(
        std::ranges::subrange(wrapper.vm_idxs, wrapper.num_methods), 0);
  } else if (ver >= 0x0003005C0048003A) { // 03.92.72.58
    // "STEAMINVENTORY_INTERFACE_V002", used since Steamworks SDK v1.40
    wrapper.num_methods = 37;
    std::ranges::iota(
        std::ranges::subrange(wrapper.vm_idxs, wrapper.num_methods), 0);
  } else {
    // "STEAMINVENTORY_INTERFACE_V001", used in older supported Steamworks SDK
    // versions
    wrapper.num_methods = 24;
    wrapper.vm_idxs[m_GetResultStatus] = 0;
    wrapper.vm_idxs[m_GetResultItems] = 1;
    wrapper.vm_idxs[m_GetResultTimestamp] = 2;
    wrapper.vm_idxs[m_CheckResultSteamID] = 3;
    wrapper.vm_idxs[m_DestroyResult] = 4;
    wrapper.vm_idxs[m_GetAllItems] = 5;
    wrapper.vm_idxs[m_GetItemsByID] = 6;
    wrapper.vm_idxs[m_SerializeResult] = 7;
    wrapper.vm_idxs[m_DeserializeResult] = 8;
    wrapper.vm_idxs[m_GenerateItems] = 9;
    wrapper.vm_idxs[m_GrantPromoItems] = 10;
    wrapper.vm_idxs[m_AddPromoItem] = 11;
    wrapper.vm_idxs[m_AddPromoItems] = 12;
    wrapper.vm_idxs[m_ConsumeItem] = 13;
    wrapper.vm_idxs[m_ExchangeItems] = 14;
    wrapper.vm_idxs[m_TransferItemQuantity] = 15;
    wrapper.vm_idxs[m_SendItemDropHeartbeat] = 16;
    wrapper.vm_idxs[m_TriggerItemDrop] = 17;
    wrapper.vm_idxs[m_TradeItems] = 18;
    wrapper.vm_idxs[m_LoadItemDefinitions] = 19;
    wrapper.vm_idxs[m_GetItemDefinitionIDs] = 20;
    wrapper.vm_idxs[m_GetItemDefinitionProperty] = 21;
    wrapper.vm_idxs[m_RequestEligiblePromoItemDefinitionsIDs] = 22;
    wrapper.vm_idxs[m_GetEligiblePromoItemDefinitionIDs] = 23;
  }
  wrapper.orig_vtable = iface->vtable;
  wrapper.iface = iface;
  std::ranges::copy_n(iface->vtable, wrapper.num_methods,
                      wrapper.vtable.begin());
  iface->vtable = wrapper.vtable.data();
}

/// Get interface version string for Steamworks SDK v1.37+.
constexpr const char *_Nonnull get_ver_str() {
  if (ver >= 0x0004005F0014001E) { // 04.95.20.30
    // Steamworks SDK v1.43+
    return "STEAMINVENTORY_INTERFACE_V003";
  } else if (ver >= 0x0003005C0048003A) { // 03.92.72.58
    // Steamworks SDK v1.40+
    return "STEAMINVENTORY_INTERFACE_V002";
  } else {
    // All previous Steamworks SDK versions since v1.37
    return "STEAMINVENTORY_INTERFACE_V001";
  }
}

} // namespace tek::game_runtime::steam::ISteamInventory
