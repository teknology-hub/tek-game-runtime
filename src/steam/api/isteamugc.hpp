//===-- isteamugc.hpp - ISteamUGC interface declarations ------------------===//
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
/// Declarations of ISteamUGC interface wrapper and related types.
///
//===----------------------------------------------------------------------===//
#pragma once
#include "steam/api.hpp"

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <ranges>
#include <tek-steamclient/cm.h>

namespace tek::game_runtime::steam::ISteamUGC {

// Virtual method enumeration.
enum {
  m_CreateQueryUserUGCRequest,
  m_CreateQueryAllUGCRequestCursor,
  m_CreateQueryAllUGCRequestPage,
  m_CreateQueryUGCDetailsRequest,
  m_SendQueryUGCRequest,
  m_GetQueryUGCResult,
  m_GetQueryUGCNumTags,
  m_GetQueryUGCTag,
  m_GetQueryUGCTagDisplayName,
  m_GetQueryUGCPreviewURL,
  m_GetQueryUGCMetadata,
  m_GetQueryUGCChildren,
  m_GetQueryUGCStatistic,
  m_GetQueryUGCNumAdditionalPreviews,
  m_GetQueryUGCAdditionalPreview,
  m_GetQueryUGCNumKeyValueTags,
  m_GetQueryFirstUGCKeyValueTag,
  m_GetQueryUGCKeyValueTag,
  m_GetNumSupportedGameVersions,
  m_GetSupportedGameVersionData,
  m_GetQueryUGCContentDescriptors,
  m_ReleaseQueryUGCRequest,
  m_AddRequiredTag,
  m_AddRequiredTagGroup,
  m_AddExcludedTag,
  m_SetReturnOnlyIDs,
  m_SetReturnKeyValueTags,
  m_SetReturnLongDescription,
  m_SetReturnMetadata,
  m_SetReturnChildren,
  m_SetReturnAdditionalPreviews,
  m_SetReturnTotalOnly,
  m_SetReturnPlaytimeStats,
  m_SetLanguage,
  m_SetAllowCachedResponse,
  m_SetAdminQuery,
  m_SetCloudFileNameFilter,
  m_SetMatchAnyTag,
  m_SetSearchText,
  m_SetRankedByTrendDays,
  m_SetTimeCreatedDateRange,
  m_SetTimeUpdatedDateRange,
  m_AddRequiredKeyValueTag,
  m_RequestUGCDetails,
  m_CreateItem,
  m_StartItemUpdate,
  m_SetItemTitle,
  m_SetItemDescription,
  m_SetItemUpdateLanguage,
  m_SetItemMetadata,
  m_SetItemVisibility,
  m_SetItemTags,
  m_SetItemContent,
  m_SetItemPreview,
  m_SetAllowLegacyUpload,
  m_RemoveAllItemKeyValueTags,
  m_RemoveItemKeyValueTags,
  m_AddItemKeyValueTag,
  m_AddItemPreviewFile,
  m_AddItemPreviewVideo,
  m_UpdateItemPreviewFile,
  m_UpdateItemPreviewVideo,
  m_RemoveItemPreview,
  m_AddContentDescriptor,
  m_RemoveContentDescriptor,
  m_SetRequiredGameVersions,
  m_SubmitItemUpdate,
  m_GetItemUpdateProgress,
  m_SetUserItemVote,
  m_GetUserItemVote,
  m_AddItemToFavorites,
  m_RemoveItemFromFavorites,
  m_SubscribeItem,
  m_UnsubscribeItem,
  m_GetNumSubscribedItems,
  m_GetSubscribedItems,
  m_GetItemState,
  m_GetItemInstallInfo,
  m_GetItemDownloadInfo,
  m_GetItemUpdateInfo = m_GetItemDownloadInfo,
  m_DownloadItem,
  m_BInitWorkshopForGameServer,
  m_SuspendDownloads,
  m_StartPlaytimeTracking,
  m_StopPlaytimeTracking,
  m_StopPlaytimeTrackingForAllItems,
  m_AddDependency,
  m_RemoveDependency,
  m_AddAppDependency,
  m_RemoveAppDependency,
  m_GetAppDependencies,
  m_DeleteItem,
  m_ShowWorkshopEULA,
  m_GetWorkshopEULAStatus,
  m_GetUserContentDescriptorPreferences,
  m_SetItemsDisabledLocally,
  m_SetSubscriptionsLoadOrder,
  num_methods
};

struct sub_result {
  tek_sc_cm_eresult result;
  std::uint64_t id;
};

/// Wrapper descriptor for ISteamUGC interface.
inline wrapper_desc<num_methods> wrapper;

/// Initialize the wrapper.
///
/// @param [in, out] iface
///    Pointer to the interface instance.
constexpr void setup(cpp_interface *_Nonnull iface) noexcept {
  if (ver >= 0x0009003C002C000A) { // 09.60.44.10
    // "STEAMUGC_INTERFACE_VERSION021", used in Steamworks SDK v1.62
    wrapper.num_methods = 96;
    std::ranges::iota(
        std::ranges::subrange(wrapper.vm_idxs, wrapper.num_methods), 0);
  } else if (ver >= 0x0008006100630046) { // 08.97.99.70
    // "STEAMUGC_INTERFACE_VERSION020", used since Steamworks SDK v1.60
    wrapper.num_methods = 94;
    std::ranges::iota(
        std::ranges::subrange(wrapper.vm_idxs, wrapper.num_methods), 0);
  } else if (ver >= 0x0008002100090017) { // 08.33.09.23
    // "STEAMUGC_INTERFACE_VERSION018", used since Steamworks SDK v1.58
    wrapper.num_methods = 90;
    wrapper.vm_idxs[m_CreateQueryUserUGCRequest] = 0;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestCursor] = 1;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestPage] = 2;
    wrapper.vm_idxs[m_CreateQueryUGCDetailsRequest] = 3;
    wrapper.vm_idxs[m_SendQueryUGCRequest] = 4;
    wrapper.vm_idxs[m_GetQueryUGCResult] = 5;
    wrapper.vm_idxs[m_GetQueryUGCNumTags] = 6;
    wrapper.vm_idxs[m_GetQueryUGCTag] = 7;
    wrapper.vm_idxs[m_GetQueryUGCTagDisplayName] = 8;
    wrapper.vm_idxs[m_GetQueryUGCPreviewURL] = 9;
    wrapper.vm_idxs[m_GetQueryUGCMetadata] = 10;
    wrapper.vm_idxs[m_GetQueryUGCChildren] = 11;
    wrapper.vm_idxs[m_GetQueryUGCStatistic] = 12;
    wrapper.vm_idxs[m_GetQueryUGCNumAdditionalPreviews] = 13;
    wrapper.vm_idxs[m_GetQueryUGCAdditionalPreview] = 14;
    wrapper.vm_idxs[m_GetQueryUGCNumKeyValueTags] = 15;
    wrapper.vm_idxs[m_GetQueryFirstUGCKeyValueTag] = 16;
    wrapper.vm_idxs[m_GetQueryUGCKeyValueTag] = 17;
    wrapper.vm_idxs[m_GetQueryUGCContentDescriptors] = 18;
    wrapper.vm_idxs[m_ReleaseQueryUGCRequest] = 19;
    wrapper.vm_idxs[m_AddRequiredTag] = 20;
    wrapper.vm_idxs[m_AddRequiredTagGroup] = 21;
    wrapper.vm_idxs[m_AddExcludedTag] = 22;
    wrapper.vm_idxs[m_SetReturnOnlyIDs] = 23;
    wrapper.vm_idxs[m_SetReturnKeyValueTags] = 24;
    wrapper.vm_idxs[m_SetReturnLongDescription] = 25;
    wrapper.vm_idxs[m_SetReturnMetadata] = 26;
    wrapper.vm_idxs[m_SetReturnChildren] = 27;
    wrapper.vm_idxs[m_SetReturnAdditionalPreviews] = 28;
    wrapper.vm_idxs[m_SetReturnTotalOnly] = 29;
    wrapper.vm_idxs[m_SetReturnPlaytimeStats] = 30;
    wrapper.vm_idxs[m_SetLanguage] = 31;
    wrapper.vm_idxs[m_SetAllowCachedResponse] = 32;
    wrapper.vm_idxs[m_SetCloudFileNameFilter] = 33;
    wrapper.vm_idxs[m_SetMatchAnyTag] = 34;
    wrapper.vm_idxs[m_SetSearchText] = 35;
    wrapper.vm_idxs[m_SetRankedByTrendDays] = 36;
    wrapper.vm_idxs[m_SetTimeCreatedDateRange] = 37;
    wrapper.vm_idxs[m_SetTimeUpdatedDateRange] = 38;
    wrapper.vm_idxs[m_AddRequiredKeyValueTag] = 39;
    wrapper.vm_idxs[m_RequestUGCDetails] = 40;
    wrapper.vm_idxs[m_CreateItem] = 41;
    wrapper.vm_idxs[m_StartItemUpdate] = 42;
    wrapper.vm_idxs[m_SetItemTitle] = 43;
    wrapper.vm_idxs[m_SetItemDescription] = 44;
    wrapper.vm_idxs[m_SetItemUpdateLanguage] = 45;
    wrapper.vm_idxs[m_SetItemMetadata] = 46;
    wrapper.vm_idxs[m_SetItemVisibility] = 47;
    wrapper.vm_idxs[m_SetItemTags] = 48;
    wrapper.vm_idxs[m_SetItemContent] = 49;
    wrapper.vm_idxs[m_SetItemPreview] = 50;
    wrapper.vm_idxs[m_SetAllowLegacyUpload] = 51;
    wrapper.vm_idxs[m_RemoveAllItemKeyValueTags] = 52;
    wrapper.vm_idxs[m_RemoveItemKeyValueTags] = 53;
    wrapper.vm_idxs[m_AddItemKeyValueTag] = 54;
    wrapper.vm_idxs[m_AddItemPreviewFile] = 55;
    wrapper.vm_idxs[m_AddItemPreviewVideo] = 56;
    wrapper.vm_idxs[m_UpdateItemPreviewFile] = 57;
    wrapper.vm_idxs[m_UpdateItemPreviewVideo] = 58;
    wrapper.vm_idxs[m_RemoveItemPreview] = 59;
    wrapper.vm_idxs[m_AddContentDescriptor] = 60;
    wrapper.vm_idxs[m_RemoveContentDescriptor] = 61;
    wrapper.vm_idxs[m_SubmitItemUpdate] = 62;
    wrapper.vm_idxs[m_GetItemUpdateProgress] = 63;
    wrapper.vm_idxs[m_SetUserItemVote] = 64;
    wrapper.vm_idxs[m_GetUserItemVote] = 65;
    wrapper.vm_idxs[m_AddItemToFavorites] = 66;
    wrapper.vm_idxs[m_RemoveItemFromFavorites] = 67;
    wrapper.vm_idxs[m_SubscribeItem] = 68;
    wrapper.vm_idxs[m_UnsubscribeItem] = 69;
    wrapper.vm_idxs[m_GetNumSubscribedItems] = 70;
    wrapper.vm_idxs[m_GetSubscribedItems] = 71;
    wrapper.vm_idxs[m_GetItemState] = 72;
    wrapper.vm_idxs[m_GetItemInstallInfo] = 73;
    wrapper.vm_idxs[m_GetItemDownloadInfo] = 74;
    wrapper.vm_idxs[m_DownloadItem] = 75;
    wrapper.vm_idxs[m_BInitWorkshopForGameServer] = 76;
    wrapper.vm_idxs[m_SuspendDownloads] = 77;
    wrapper.vm_idxs[m_StartPlaytimeTracking] = 78;
    wrapper.vm_idxs[m_StopPlaytimeTracking] = 79;
    wrapper.vm_idxs[m_StopPlaytimeTrackingForAllItems] = 80;
    wrapper.vm_idxs[m_AddDependency] = 81;
    wrapper.vm_idxs[m_RemoveDependency] = 82;
    wrapper.vm_idxs[m_AddAppDependency] = 83;
    wrapper.vm_idxs[m_RemoveAppDependency] = 84;
    wrapper.vm_idxs[m_GetAppDependencies] = 85;
    wrapper.vm_idxs[m_DeleteItem] = 86;
    wrapper.vm_idxs[m_ShowWorkshopEULA] = 87;
    wrapper.vm_idxs[m_GetWorkshopEULAStatus] = 88;
    wrapper.vm_idxs[m_GetUserContentDescriptorPreferences] = 89;
  } else if (ver >= 0x000700600000002C) { // 07.96.00.44
    // "STEAMUGC_INTERFACE_VERSION017", used since Steamworks SDK v1.56
    wrapper.num_methods = 89;
    wrapper.vm_idxs[m_CreateQueryUserUGCRequest] = 0;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestCursor] = 1;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestPage] = 2;
    wrapper.vm_idxs[m_CreateQueryUGCDetailsRequest] = 3;
    wrapper.vm_idxs[m_SendQueryUGCRequest] = 4;
    wrapper.vm_idxs[m_GetQueryUGCResult] = 5;
    wrapper.vm_idxs[m_GetQueryUGCNumTags] = 6;
    wrapper.vm_idxs[m_GetQueryUGCTag] = 7;
    wrapper.vm_idxs[m_GetQueryUGCTagDisplayName] = 8;
    wrapper.vm_idxs[m_GetQueryUGCPreviewURL] = 9;
    wrapper.vm_idxs[m_GetQueryUGCMetadata] = 10;
    wrapper.vm_idxs[m_GetQueryUGCChildren] = 11;
    wrapper.vm_idxs[m_GetQueryUGCStatistic] = 12;
    wrapper.vm_idxs[m_GetQueryUGCNumAdditionalPreviews] = 13;
    wrapper.vm_idxs[m_GetQueryUGCAdditionalPreview] = 14;
    wrapper.vm_idxs[m_GetQueryUGCNumKeyValueTags] = 15;
    wrapper.vm_idxs[m_GetQueryFirstUGCKeyValueTag] = 16;
    wrapper.vm_idxs[m_GetQueryUGCKeyValueTag] = 17;
    wrapper.vm_idxs[m_GetQueryUGCContentDescriptors] = 18;
    wrapper.vm_idxs[m_ReleaseQueryUGCRequest] = 19;
    wrapper.vm_idxs[m_AddRequiredTag] = 20;
    wrapper.vm_idxs[m_AddRequiredTagGroup] = 21;
    wrapper.vm_idxs[m_AddExcludedTag] = 22;
    wrapper.vm_idxs[m_SetReturnOnlyIDs] = 23;
    wrapper.vm_idxs[m_SetReturnKeyValueTags] = 24;
    wrapper.vm_idxs[m_SetReturnLongDescription] = 25;
    wrapper.vm_idxs[m_SetReturnMetadata] = 26;
    wrapper.vm_idxs[m_SetReturnChildren] = 27;
    wrapper.vm_idxs[m_SetReturnAdditionalPreviews] = 28;
    wrapper.vm_idxs[m_SetReturnTotalOnly] = 29;
    wrapper.vm_idxs[m_SetReturnPlaytimeStats] = 30;
    wrapper.vm_idxs[m_SetLanguage] = 31;
    wrapper.vm_idxs[m_SetAllowCachedResponse] = 32;
    wrapper.vm_idxs[m_SetCloudFileNameFilter] = 33;
    wrapper.vm_idxs[m_SetMatchAnyTag] = 34;
    wrapper.vm_idxs[m_SetSearchText] = 35;
    wrapper.vm_idxs[m_SetRankedByTrendDays] = 36;
    wrapper.vm_idxs[m_SetTimeCreatedDateRange] = 37;
    wrapper.vm_idxs[m_SetTimeUpdatedDateRange] = 38;
    wrapper.vm_idxs[m_AddRequiredKeyValueTag] = 39;
    wrapper.vm_idxs[m_RequestUGCDetails] = 40;
    wrapper.vm_idxs[m_CreateItem] = 41;
    wrapper.vm_idxs[m_StartItemUpdate] = 42;
    wrapper.vm_idxs[m_SetItemTitle] = 43;
    wrapper.vm_idxs[m_SetItemDescription] = 44;
    wrapper.vm_idxs[m_SetItemUpdateLanguage] = 45;
    wrapper.vm_idxs[m_SetItemMetadata] = 46;
    wrapper.vm_idxs[m_SetItemVisibility] = 47;
    wrapper.vm_idxs[m_SetItemTags] = 48;
    wrapper.vm_idxs[m_SetItemContent] = 49;
    wrapper.vm_idxs[m_SetItemPreview] = 50;
    wrapper.vm_idxs[m_SetAllowLegacyUpload] = 51;
    wrapper.vm_idxs[m_RemoveAllItemKeyValueTags] = 52;
    wrapper.vm_idxs[m_RemoveItemKeyValueTags] = 53;
    wrapper.vm_idxs[m_AddItemKeyValueTag] = 54;
    wrapper.vm_idxs[m_AddItemPreviewFile] = 55;
    wrapper.vm_idxs[m_AddItemPreviewVideo] = 56;
    wrapper.vm_idxs[m_UpdateItemPreviewFile] = 57;
    wrapper.vm_idxs[m_UpdateItemPreviewVideo] = 58;
    wrapper.vm_idxs[m_RemoveItemPreview] = 59;
    wrapper.vm_idxs[m_AddContentDescriptor] = 60;
    wrapper.vm_idxs[m_RemoveContentDescriptor] = 61;
    wrapper.vm_idxs[m_SubmitItemUpdate] = 62;
    wrapper.vm_idxs[m_GetItemUpdateProgress] = 63;
    wrapper.vm_idxs[m_SetUserItemVote] = 64;
    wrapper.vm_idxs[m_GetUserItemVote] = 65;
    wrapper.vm_idxs[m_AddItemToFavorites] = 66;
    wrapper.vm_idxs[m_RemoveItemFromFavorites] = 67;
    wrapper.vm_idxs[m_SubscribeItem] = 68;
    wrapper.vm_idxs[m_UnsubscribeItem] = 69;
    wrapper.vm_idxs[m_GetNumSubscribedItems] = 70;
    wrapper.vm_idxs[m_GetSubscribedItems] = 71;
    wrapper.vm_idxs[m_GetItemState] = 72;
    wrapper.vm_idxs[m_GetItemInstallInfo] = 73;
    wrapper.vm_idxs[m_GetItemDownloadInfo] = 74;
    wrapper.vm_idxs[m_DownloadItem] = 75;
    wrapper.vm_idxs[m_BInitWorkshopForGameServer] = 76;
    wrapper.vm_idxs[m_SuspendDownloads] = 77;
    wrapper.vm_idxs[m_StartPlaytimeTracking] = 78;
    wrapper.vm_idxs[m_StopPlaytimeTracking] = 79;
    wrapper.vm_idxs[m_StopPlaytimeTrackingForAllItems] = 80;
    wrapper.vm_idxs[m_AddDependency] = 81;
    wrapper.vm_idxs[m_RemoveDependency] = 82;
    wrapper.vm_idxs[m_AddAppDependency] = 83;
    wrapper.vm_idxs[m_RemoveAppDependency] = 84;
    wrapper.vm_idxs[m_GetAppDependencies] = 85;
    wrapper.vm_idxs[m_DeleteItem] = 86;
    wrapper.vm_idxs[m_ShowWorkshopEULA] = 87;
    wrapper.vm_idxs[m_GetWorkshopEULAStatus] = 88;
  } else if (ver >= 0x0006005B00150039) { // 06.91.21.57
    // "STEAMUGC_INTERFACE_VERSION016", used since Steamworks SDK v1.53
    wrapper.num_methods = 86;
    wrapper.vm_idxs[m_CreateQueryUserUGCRequest] = 0;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestCursor] = 1;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestPage] = 2;
    wrapper.vm_idxs[m_CreateQueryUGCDetailsRequest] = 3;
    wrapper.vm_idxs[m_SendQueryUGCRequest] = 4;
    wrapper.vm_idxs[m_GetQueryUGCResult] = 5;
    wrapper.vm_idxs[m_GetQueryUGCNumTags] = 6;
    wrapper.vm_idxs[m_GetQueryUGCTag] = 7;
    wrapper.vm_idxs[m_GetQueryUGCTagDisplayName] = 8;
    wrapper.vm_idxs[m_GetQueryUGCPreviewURL] = 9;
    wrapper.vm_idxs[m_GetQueryUGCMetadata] = 10;
    wrapper.vm_idxs[m_GetQueryUGCChildren] = 11;
    wrapper.vm_idxs[m_GetQueryUGCStatistic] = 12;
    wrapper.vm_idxs[m_GetQueryUGCNumAdditionalPreviews] = 13;
    wrapper.vm_idxs[m_GetQueryUGCAdditionalPreview] = 14;
    wrapper.vm_idxs[m_GetQueryUGCNumKeyValueTags] = 15;
    wrapper.vm_idxs[m_GetQueryFirstUGCKeyValueTag] = 16;
    wrapper.vm_idxs[m_GetQueryUGCKeyValueTag] = 17;
    wrapper.vm_idxs[m_ReleaseQueryUGCRequest] = 18;
    wrapper.vm_idxs[m_AddRequiredTag] = 19;
    wrapper.vm_idxs[m_AddRequiredTagGroup] = 20;
    wrapper.vm_idxs[m_AddExcludedTag] = 21;
    wrapper.vm_idxs[m_SetReturnOnlyIDs] = 22;
    wrapper.vm_idxs[m_SetReturnKeyValueTags] = 23;
    wrapper.vm_idxs[m_SetReturnLongDescription] = 24;
    wrapper.vm_idxs[m_SetReturnMetadata] = 25;
    wrapper.vm_idxs[m_SetReturnChildren] = 26;
    wrapper.vm_idxs[m_SetReturnAdditionalPreviews] = 27;
    wrapper.vm_idxs[m_SetReturnTotalOnly] = 28;
    wrapper.vm_idxs[m_SetReturnPlaytimeStats] = 29;
    wrapper.vm_idxs[m_SetLanguage] = 30;
    wrapper.vm_idxs[m_SetAllowCachedResponse] = 31;
    wrapper.vm_idxs[m_SetCloudFileNameFilter] = 32;
    wrapper.vm_idxs[m_SetMatchAnyTag] = 33;
    wrapper.vm_idxs[m_SetSearchText] = 34;
    wrapper.vm_idxs[m_SetRankedByTrendDays] = 35;
    wrapper.vm_idxs[m_SetTimeCreatedDateRange] = 36;
    wrapper.vm_idxs[m_SetTimeUpdatedDateRange] = 37;
    wrapper.vm_idxs[m_AddRequiredKeyValueTag] = 38;
    wrapper.vm_idxs[m_RequestUGCDetails] = 39;
    wrapper.vm_idxs[m_CreateItem] = 40;
    wrapper.vm_idxs[m_StartItemUpdate] = 41;
    wrapper.vm_idxs[m_SetItemTitle] = 42;
    wrapper.vm_idxs[m_SetItemDescription] = 43;
    wrapper.vm_idxs[m_SetItemUpdateLanguage] = 44;
    wrapper.vm_idxs[m_SetItemMetadata] = 45;
    wrapper.vm_idxs[m_SetItemVisibility] = 46;
    wrapper.vm_idxs[m_SetItemTags] = 47;
    wrapper.vm_idxs[m_SetItemContent] = 48;
    wrapper.vm_idxs[m_SetItemPreview] = 49;
    wrapper.vm_idxs[m_SetAllowLegacyUpload] = 50;
    wrapper.vm_idxs[m_RemoveAllItemKeyValueTags] = 51;
    wrapper.vm_idxs[m_RemoveItemKeyValueTags] = 52;
    wrapper.vm_idxs[m_AddItemKeyValueTag] = 53;
    wrapper.vm_idxs[m_AddItemPreviewFile] = 54;
    wrapper.vm_idxs[m_AddItemPreviewVideo] = 55;
    wrapper.vm_idxs[m_UpdateItemPreviewFile] = 56;
    wrapper.vm_idxs[m_UpdateItemPreviewVideo] = 57;
    wrapper.vm_idxs[m_RemoveItemPreview] = 58;
    wrapper.vm_idxs[m_SubmitItemUpdate] = 59;
    wrapper.vm_idxs[m_GetItemUpdateProgress] = 60;
    wrapper.vm_idxs[m_SetUserItemVote] = 61;
    wrapper.vm_idxs[m_GetUserItemVote] = 62;
    wrapper.vm_idxs[m_AddItemToFavorites] = 63;
    wrapper.vm_idxs[m_RemoveItemFromFavorites] = 64;
    wrapper.vm_idxs[m_SubscribeItem] = 65;
    wrapper.vm_idxs[m_UnsubscribeItem] = 66;
    wrapper.vm_idxs[m_GetNumSubscribedItems] = 67;
    wrapper.vm_idxs[m_GetSubscribedItems] = 68;
    wrapper.vm_idxs[m_GetItemState] = 69;
    wrapper.vm_idxs[m_GetItemInstallInfo] = 70;
    wrapper.vm_idxs[m_GetItemDownloadInfo] = 71;
    wrapper.vm_idxs[m_DownloadItem] = 72;
    wrapper.vm_idxs[m_BInitWorkshopForGameServer] = 73;
    wrapper.vm_idxs[m_SuspendDownloads] = 74;
    wrapper.vm_idxs[m_StartPlaytimeTracking] = 75;
    wrapper.vm_idxs[m_StopPlaytimeTracking] = 76;
    wrapper.vm_idxs[m_StopPlaytimeTrackingForAllItems] = 77;
    wrapper.vm_idxs[m_AddDependency] = 78;
    wrapper.vm_idxs[m_RemoveDependency] = 79;
    wrapper.vm_idxs[m_AddAppDependency] = 80;
    wrapper.vm_idxs[m_RemoveAppDependency] = 81;
    wrapper.vm_idxs[m_GetAppDependencies] = 82;
    wrapper.vm_idxs[m_DeleteItem] = 83;
    wrapper.vm_idxs[m_ShowWorkshopEULA] = 84;
    wrapper.vm_idxs[m_GetWorkshopEULAStatus] = 85;
  } else if (ver >= 0x0006001C00120056) { // 06.28.18.86
    // "STEAMUGC_INTERFACE_VERSION015", used since Steamworks SDK v1.51
    wrapper.num_methods = 84;
    wrapper.vm_idxs[m_CreateQueryUserUGCRequest] = 0;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestCursor] = 1;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestPage] = 2;
    wrapper.vm_idxs[m_CreateQueryUGCDetailsRequest] = 3;
    wrapper.vm_idxs[m_SendQueryUGCRequest] = 4;
    wrapper.vm_idxs[m_GetQueryUGCResult] = 5;
    wrapper.vm_idxs[m_GetQueryUGCNumTags] = 6;
    wrapper.vm_idxs[m_GetQueryUGCTag] = 7;
    wrapper.vm_idxs[m_GetQueryUGCTagDisplayName] = 8;
    wrapper.vm_idxs[m_GetQueryUGCPreviewURL] = 9;
    wrapper.vm_idxs[m_GetQueryUGCMetadata] = 10;
    wrapper.vm_idxs[m_GetQueryUGCChildren] = 11;
    wrapper.vm_idxs[m_GetQueryUGCStatistic] = 12;
    wrapper.vm_idxs[m_GetQueryUGCNumAdditionalPreviews] = 13;
    wrapper.vm_idxs[m_GetQueryUGCAdditionalPreview] = 14;
    wrapper.vm_idxs[m_GetQueryUGCNumKeyValueTags] = 15;
    wrapper.vm_idxs[m_GetQueryFirstUGCKeyValueTag] = 16;
    wrapper.vm_idxs[m_GetQueryUGCKeyValueTag] = 17;
    wrapper.vm_idxs[m_ReleaseQueryUGCRequest] = 18;
    wrapper.vm_idxs[m_AddRequiredTag] = 19;
    wrapper.vm_idxs[m_AddRequiredTagGroup] = 20;
    wrapper.vm_idxs[m_AddExcludedTag] = 21;
    wrapper.vm_idxs[m_SetReturnOnlyIDs] = 22;
    wrapper.vm_idxs[m_SetReturnKeyValueTags] = 23;
    wrapper.vm_idxs[m_SetReturnLongDescription] = 24;
    wrapper.vm_idxs[m_SetReturnMetadata] = 25;
    wrapper.vm_idxs[m_SetReturnChildren] = 26;
    wrapper.vm_idxs[m_SetReturnAdditionalPreviews] = 27;
    wrapper.vm_idxs[m_SetReturnTotalOnly] = 28;
    wrapper.vm_idxs[m_SetReturnPlaytimeStats] = 29;
    wrapper.vm_idxs[m_SetLanguage] = 30;
    wrapper.vm_idxs[m_SetAllowCachedResponse] = 31;
    wrapper.vm_idxs[m_SetCloudFileNameFilter] = 32;
    wrapper.vm_idxs[m_SetMatchAnyTag] = 33;
    wrapper.vm_idxs[m_SetSearchText] = 34;
    wrapper.vm_idxs[m_SetRankedByTrendDays] = 35;
    wrapper.vm_idxs[m_AddRequiredKeyValueTag] = 36;
    wrapper.vm_idxs[m_RequestUGCDetails] = 37;
    wrapper.vm_idxs[m_CreateItem] = 38;
    wrapper.vm_idxs[m_StartItemUpdate] = 39;
    wrapper.vm_idxs[m_SetItemTitle] = 40;
    wrapper.vm_idxs[m_SetItemDescription] = 41;
    wrapper.vm_idxs[m_SetItemUpdateLanguage] = 42;
    wrapper.vm_idxs[m_SetItemMetadata] = 43;
    wrapper.vm_idxs[m_SetItemVisibility] = 44;
    wrapper.vm_idxs[m_SetItemTags] = 45;
    wrapper.vm_idxs[m_SetItemContent] = 46;
    wrapper.vm_idxs[m_SetItemPreview] = 47;
    wrapper.vm_idxs[m_SetAllowLegacyUpload] = 48;
    wrapper.vm_idxs[m_RemoveAllItemKeyValueTags] = 49;
    wrapper.vm_idxs[m_RemoveItemKeyValueTags] = 50;
    wrapper.vm_idxs[m_AddItemKeyValueTag] = 51;
    wrapper.vm_idxs[m_AddItemPreviewFile] = 52;
    wrapper.vm_idxs[m_AddItemPreviewVideo] = 53;
    wrapper.vm_idxs[m_UpdateItemPreviewFile] = 54;
    wrapper.vm_idxs[m_UpdateItemPreviewVideo] = 55;
    wrapper.vm_idxs[m_RemoveItemPreview] = 56;
    wrapper.vm_idxs[m_SubmitItemUpdate] = 57;
    wrapper.vm_idxs[m_GetItemUpdateProgress] = 58;
    wrapper.vm_idxs[m_SetUserItemVote] = 59;
    wrapper.vm_idxs[m_GetUserItemVote] = 60;
    wrapper.vm_idxs[m_AddItemToFavorites] = 61;
    wrapper.vm_idxs[m_RemoveItemFromFavorites] = 62;
    wrapper.vm_idxs[m_SubscribeItem] = 63;
    wrapper.vm_idxs[m_UnsubscribeItem] = 64;
    wrapper.vm_idxs[m_GetNumSubscribedItems] = 65;
    wrapper.vm_idxs[m_GetSubscribedItems] = 66;
    wrapper.vm_idxs[m_GetItemState] = 67;
    wrapper.vm_idxs[m_GetItemInstallInfo] = 68;
    wrapper.vm_idxs[m_GetItemDownloadInfo] = 69;
    wrapper.vm_idxs[m_DownloadItem] = 70;
    wrapper.vm_idxs[m_BInitWorkshopForGameServer] = 71;
    wrapper.vm_idxs[m_SuspendDownloads] = 72;
    wrapper.vm_idxs[m_StartPlaytimeTracking] = 73;
    wrapper.vm_idxs[m_StopPlaytimeTracking] = 74;
    wrapper.vm_idxs[m_StopPlaytimeTrackingForAllItems] = 75;
    wrapper.vm_idxs[m_AddDependency] = 76;
    wrapper.vm_idxs[m_RemoveDependency] = 77;
    wrapper.vm_idxs[m_AddAppDependency] = 78;
    wrapper.vm_idxs[m_RemoveAppDependency] = 79;
    wrapper.vm_idxs[m_GetAppDependencies] = 80;
    wrapper.vm_idxs[m_DeleteItem] = 81;
    wrapper.vm_idxs[m_ShowWorkshopEULA] = 82;
    wrapper.vm_idxs[m_GetWorkshopEULAStatus] = 83;
  } else if (ver >= 0x000500350021004E) { // 05.53.33.78
    // "STEAMUGC_INTERFACE_VERSION014", used since Steamworks SDK v1.47
    wrapper.num_methods = 79;
    wrapper.vm_idxs[m_CreateQueryUserUGCRequest] = 0;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestCursor] = 1;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestPage] = 2;
    wrapper.vm_idxs[m_CreateQueryUGCDetailsRequest] = 3;
    wrapper.vm_idxs[m_SendQueryUGCRequest] = 4;
    wrapper.vm_idxs[m_GetQueryUGCResult] = 5;
    wrapper.vm_idxs[m_GetQueryUGCPreviewURL] = 6;
    wrapper.vm_idxs[m_GetQueryUGCMetadata] = 7;
    wrapper.vm_idxs[m_GetQueryUGCChildren] = 8;
    wrapper.vm_idxs[m_GetQueryUGCStatistic] = 9;
    wrapper.vm_idxs[m_GetQueryUGCNumAdditionalPreviews] = 10;
    wrapper.vm_idxs[m_GetQueryUGCAdditionalPreview] = 11;
    wrapper.vm_idxs[m_GetQueryUGCNumKeyValueTags] = 12;
    wrapper.vm_idxs[m_GetQueryFirstUGCKeyValueTag] = 13;
    wrapper.vm_idxs[m_GetQueryUGCKeyValueTag] = 14;
    wrapper.vm_idxs[m_ReleaseQueryUGCRequest] = 15;
    wrapper.vm_idxs[m_AddRequiredTag] = 16;
    wrapper.vm_idxs[m_AddRequiredTagGroup] = 17;
    wrapper.vm_idxs[m_AddExcludedTag] = 18;
    wrapper.vm_idxs[m_SetReturnOnlyIDs] = 19;
    wrapper.vm_idxs[m_SetReturnKeyValueTags] = 20;
    wrapper.vm_idxs[m_SetReturnLongDescription] = 21;
    wrapper.vm_idxs[m_SetReturnMetadata] = 22;
    wrapper.vm_idxs[m_SetReturnChildren] = 23;
    wrapper.vm_idxs[m_SetReturnAdditionalPreviews] = 24;
    wrapper.vm_idxs[m_SetReturnTotalOnly] = 25;
    wrapper.vm_idxs[m_SetReturnPlaytimeStats] = 26;
    wrapper.vm_idxs[m_SetLanguage] = 27;
    wrapper.vm_idxs[m_SetAllowCachedResponse] = 28;
    wrapper.vm_idxs[m_SetCloudFileNameFilter] = 29;
    wrapper.vm_idxs[m_SetMatchAnyTag] = 30;
    wrapper.vm_idxs[m_SetSearchText] = 31;
    wrapper.vm_idxs[m_SetRankedByTrendDays] = 32;
    wrapper.vm_idxs[m_AddRequiredKeyValueTag] = 33;
    wrapper.vm_idxs[m_RequestUGCDetails] = 34;
    wrapper.vm_idxs[m_CreateItem] = 35;
    wrapper.vm_idxs[m_StartItemUpdate] = 36;
    wrapper.vm_idxs[m_SetItemTitle] = 37;
    wrapper.vm_idxs[m_SetItemDescription] = 38;
    wrapper.vm_idxs[m_SetItemUpdateLanguage] = 39;
    wrapper.vm_idxs[m_SetItemMetadata] = 40;
    wrapper.vm_idxs[m_SetItemVisibility] = 41;
    wrapper.vm_idxs[m_SetItemTags] = 42;
    wrapper.vm_idxs[m_SetItemContent] = 43;
    wrapper.vm_idxs[m_SetItemPreview] = 44;
    wrapper.vm_idxs[m_SetAllowLegacyUpload] = 45;
    wrapper.vm_idxs[m_RemoveAllItemKeyValueTags] = 46;
    wrapper.vm_idxs[m_RemoveItemKeyValueTags] = 47;
    wrapper.vm_idxs[m_AddItemKeyValueTag] = 48;
    wrapper.vm_idxs[m_AddItemPreviewFile] = 49;
    wrapper.vm_idxs[m_AddItemPreviewVideo] = 50;
    wrapper.vm_idxs[m_UpdateItemPreviewFile] = 51;
    wrapper.vm_idxs[m_UpdateItemPreviewVideo] = 52;
    wrapper.vm_idxs[m_RemoveItemPreview] = 53;
    wrapper.vm_idxs[m_SubmitItemUpdate] = 54;
    wrapper.vm_idxs[m_GetItemUpdateProgress] = 55;
    wrapper.vm_idxs[m_SetUserItemVote] = 56;
    wrapper.vm_idxs[m_GetUserItemVote] = 57;
    wrapper.vm_idxs[m_AddItemToFavorites] = 58;
    wrapper.vm_idxs[m_RemoveItemFromFavorites] = 59;
    wrapper.vm_idxs[m_SubscribeItem] = 60;
    wrapper.vm_idxs[m_UnsubscribeItem] = 61;
    wrapper.vm_idxs[m_GetNumSubscribedItems] = 62;
    wrapper.vm_idxs[m_GetSubscribedItems] = 63;
    wrapper.vm_idxs[m_GetItemState] = 64;
    wrapper.vm_idxs[m_GetItemInstallInfo] = 65;
    wrapper.vm_idxs[m_GetItemDownloadInfo] = 66;
    wrapper.vm_idxs[m_DownloadItem] = 67;
    wrapper.vm_idxs[m_BInitWorkshopForGameServer] = 68;
    wrapper.vm_idxs[m_SuspendDownloads] = 69;
    wrapper.vm_idxs[m_StartPlaytimeTracking] = 70;
    wrapper.vm_idxs[m_StopPlaytimeTracking] = 71;
    wrapper.vm_idxs[m_StopPlaytimeTrackingForAllItems] = 72;
    wrapper.vm_idxs[m_AddDependency] = 73;
    wrapper.vm_idxs[m_RemoveDependency] = 74;
    wrapper.vm_idxs[m_AddAppDependency] = 75;
    wrapper.vm_idxs[m_RemoveAppDependency] = 76;
    wrapper.vm_idxs[m_GetAppDependencies] = 77;
    wrapper.vm_idxs[m_DeleteItem] = 78;
  } else if (ver >= 0x000500130026003E) { // 05.19.38.62
    // "STEAMUGC_INTERFACE_VERSION013", used since Steamworks SDK v1.45
    wrapper.num_methods = 78;
    wrapper.vm_idxs[m_CreateQueryUserUGCRequest] = 0;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestCursor] = 1;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestPage] = 2;
    wrapper.vm_idxs[m_CreateQueryUGCDetailsRequest] = 3;
    wrapper.vm_idxs[m_SendQueryUGCRequest] = 4;
    wrapper.vm_idxs[m_GetQueryUGCResult] = 5;
    wrapper.vm_idxs[m_GetQueryUGCPreviewURL] = 6;
    wrapper.vm_idxs[m_GetQueryUGCMetadata] = 7;
    wrapper.vm_idxs[m_GetQueryUGCChildren] = 8;
    wrapper.vm_idxs[m_GetQueryUGCStatistic] = 9;
    wrapper.vm_idxs[m_GetQueryUGCNumAdditionalPreviews] = 10;
    wrapper.vm_idxs[m_GetQueryUGCAdditionalPreview] = 11;
    wrapper.vm_idxs[m_GetQueryUGCNumKeyValueTags] = 12;
    wrapper.vm_idxs[m_GetQueryFirstUGCKeyValueTag] = 13;
    wrapper.vm_idxs[m_GetQueryUGCKeyValueTag] = 14;
    wrapper.vm_idxs[m_ReleaseQueryUGCRequest] = 15;
    wrapper.vm_idxs[m_AddRequiredTag] = 16;
    wrapper.vm_idxs[m_AddExcludedTag] = 17;
    wrapper.vm_idxs[m_SetReturnOnlyIDs] = 18;
    wrapper.vm_idxs[m_SetReturnKeyValueTags] = 19;
    wrapper.vm_idxs[m_SetReturnLongDescription] = 20;
    wrapper.vm_idxs[m_SetReturnMetadata] = 21;
    wrapper.vm_idxs[m_SetReturnChildren] = 22;
    wrapper.vm_idxs[m_SetReturnAdditionalPreviews] = 23;
    wrapper.vm_idxs[m_SetReturnTotalOnly] = 24;
    wrapper.vm_idxs[m_SetReturnPlaytimeStats] = 25;
    wrapper.vm_idxs[m_SetLanguage] = 26;
    wrapper.vm_idxs[m_SetAllowCachedResponse] = 27;
    wrapper.vm_idxs[m_SetCloudFileNameFilter] = 28;
    wrapper.vm_idxs[m_SetMatchAnyTag] = 29;
    wrapper.vm_idxs[m_SetSearchText] = 30;
    wrapper.vm_idxs[m_SetRankedByTrendDays] = 31;
    wrapper.vm_idxs[m_AddRequiredKeyValueTag] = 32;
    wrapper.vm_idxs[m_RequestUGCDetails] = 33;
    wrapper.vm_idxs[m_CreateItem] = 34;
    wrapper.vm_idxs[m_StartItemUpdate] = 35;
    wrapper.vm_idxs[m_SetItemTitle] = 36;
    wrapper.vm_idxs[m_SetItemDescription] = 37;
    wrapper.vm_idxs[m_SetItemUpdateLanguage] = 38;
    wrapper.vm_idxs[m_SetItemMetadata] = 39;
    wrapper.vm_idxs[m_SetItemVisibility] = 40;
    wrapper.vm_idxs[m_SetItemTags] = 41;
    wrapper.vm_idxs[m_SetItemContent] = 42;
    wrapper.vm_idxs[m_SetItemPreview] = 43;
    wrapper.vm_idxs[m_SetAllowLegacyUpload] = 44;
    wrapper.vm_idxs[m_RemoveAllItemKeyValueTags] = 45;
    wrapper.vm_idxs[m_RemoveItemKeyValueTags] = 46;
    wrapper.vm_idxs[m_AddItemKeyValueTag] = 47;
    wrapper.vm_idxs[m_AddItemPreviewFile] = 48;
    wrapper.vm_idxs[m_AddItemPreviewVideo] = 49;
    wrapper.vm_idxs[m_UpdateItemPreviewFile] = 50;
    wrapper.vm_idxs[m_UpdateItemPreviewVideo] = 51;
    wrapper.vm_idxs[m_RemoveItemPreview] = 52;
    wrapper.vm_idxs[m_SubmitItemUpdate] = 53;
    wrapper.vm_idxs[m_GetItemUpdateProgress] = 54;
    wrapper.vm_idxs[m_SetUserItemVote] = 55;
    wrapper.vm_idxs[m_GetUserItemVote] = 56;
    wrapper.vm_idxs[m_AddItemToFavorites] = 57;
    wrapper.vm_idxs[m_RemoveItemFromFavorites] = 58;
    wrapper.vm_idxs[m_SubscribeItem] = 59;
    wrapper.vm_idxs[m_UnsubscribeItem] = 60;
    wrapper.vm_idxs[m_GetNumSubscribedItems] = 61;
    wrapper.vm_idxs[m_GetSubscribedItems] = 62;
    wrapper.vm_idxs[m_GetItemState] = 63;
    wrapper.vm_idxs[m_GetItemInstallInfo] = 64;
    wrapper.vm_idxs[m_GetItemDownloadInfo] = 65;
    wrapper.vm_idxs[m_DownloadItem] = 66;
    wrapper.vm_idxs[m_BInitWorkshopForGameServer] = 67;
    wrapper.vm_idxs[m_SuspendDownloads] = 68;
    wrapper.vm_idxs[m_StartPlaytimeTracking] = 69;
    wrapper.vm_idxs[m_StopPlaytimeTracking] = 70;
    wrapper.vm_idxs[m_StopPlaytimeTrackingForAllItems] = 71;
    wrapper.vm_idxs[m_AddDependency] = 72;
    wrapper.vm_idxs[m_RemoveDependency] = 73;
    wrapper.vm_idxs[m_AddAppDependency] = 74;
    wrapper.vm_idxs[m_RemoveAppDependency] = 75;
    wrapper.vm_idxs[m_GetAppDependencies] = 76;
    wrapper.vm_idxs[m_DeleteItem] = 77;
  } else if (ver >= 0x0004005F0014001E) { // 04.95.20.30
    // "STEAMUGC_INTERFACE_VERSION012", used since Steamworks SDK v1.43
    wrapper.num_methods = 76;
    wrapper.vm_idxs[m_CreateQueryUserUGCRequest] = 0;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestCursor] = 1;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestPage] = 2;
    wrapper.vm_idxs[m_CreateQueryUGCDetailsRequest] = 3;
    wrapper.vm_idxs[m_SendQueryUGCRequest] = 4;
    wrapper.vm_idxs[m_GetQueryUGCResult] = 5;
    wrapper.vm_idxs[m_GetQueryUGCPreviewURL] = 6;
    wrapper.vm_idxs[m_GetQueryUGCMetadata] = 7;
    wrapper.vm_idxs[m_GetQueryUGCChildren] = 8;
    wrapper.vm_idxs[m_GetQueryUGCStatistic] = 9;
    wrapper.vm_idxs[m_GetQueryUGCNumAdditionalPreviews] = 10;
    wrapper.vm_idxs[m_GetQueryUGCAdditionalPreview] = 11;
    wrapper.vm_idxs[m_GetQueryUGCNumKeyValueTags] = 12;
    wrapper.vm_idxs[m_GetQueryUGCKeyValueTag] = 13;
    wrapper.vm_idxs[m_ReleaseQueryUGCRequest] = 14;
    wrapper.vm_idxs[m_AddRequiredTag] = 15;
    wrapper.vm_idxs[m_AddExcludedTag] = 16;
    wrapper.vm_idxs[m_SetReturnOnlyIDs] = 17;
    wrapper.vm_idxs[m_SetReturnKeyValueTags] = 18;
    wrapper.vm_idxs[m_SetReturnLongDescription] = 19;
    wrapper.vm_idxs[m_SetReturnMetadata] = 20;
    wrapper.vm_idxs[m_SetReturnChildren] = 21;
    wrapper.vm_idxs[m_SetReturnAdditionalPreviews] = 22;
    wrapper.vm_idxs[m_SetReturnTotalOnly] = 23;
    wrapper.vm_idxs[m_SetReturnPlaytimeStats] = 24;
    wrapper.vm_idxs[m_SetLanguage] = 25;
    wrapper.vm_idxs[m_SetAllowCachedResponse] = 26;
    wrapper.vm_idxs[m_SetCloudFileNameFilter] = 27;
    wrapper.vm_idxs[m_SetMatchAnyTag] = 28;
    wrapper.vm_idxs[m_SetSearchText] = 29;
    wrapper.vm_idxs[m_SetRankedByTrendDays] = 30;
    wrapper.vm_idxs[m_AddRequiredKeyValueTag] = 31;
    wrapper.vm_idxs[m_RequestUGCDetails] = 32;
    wrapper.vm_idxs[m_CreateItem] = 33;
    wrapper.vm_idxs[m_StartItemUpdate] = 34;
    wrapper.vm_idxs[m_SetItemTitle] = 35;
    wrapper.vm_idxs[m_SetItemDescription] = 36;
    wrapper.vm_idxs[m_SetItemUpdateLanguage] = 37;
    wrapper.vm_idxs[m_SetItemMetadata] = 38;
    wrapper.vm_idxs[m_SetItemVisibility] = 39;
    wrapper.vm_idxs[m_SetItemTags] = 40;
    wrapper.vm_idxs[m_SetItemContent] = 41;
    wrapper.vm_idxs[m_SetItemPreview] = 42;
    wrapper.vm_idxs[m_SetAllowLegacyUpload] = 43;
    wrapper.vm_idxs[m_RemoveItemKeyValueTags] = 44;
    wrapper.vm_idxs[m_AddItemKeyValueTag] = 45;
    wrapper.vm_idxs[m_AddItemPreviewFile] = 46;
    wrapper.vm_idxs[m_AddItemPreviewVideo] = 47;
    wrapper.vm_idxs[m_UpdateItemPreviewFile] = 48;
    wrapper.vm_idxs[m_UpdateItemPreviewVideo] = 49;
    wrapper.vm_idxs[m_RemoveItemPreview] = 50;
    wrapper.vm_idxs[m_SubmitItemUpdate] = 51;
    wrapper.vm_idxs[m_GetItemUpdateProgress] = 52;
    wrapper.vm_idxs[m_SetUserItemVote] = 53;
    wrapper.vm_idxs[m_GetUserItemVote] = 54;
    wrapper.vm_idxs[m_AddItemToFavorites] = 55;
    wrapper.vm_idxs[m_RemoveItemFromFavorites] = 56;
    wrapper.vm_idxs[m_SubscribeItem] = 57;
    wrapper.vm_idxs[m_UnsubscribeItem] = 58;
    wrapper.vm_idxs[m_GetNumSubscribedItems] = 59;
    wrapper.vm_idxs[m_GetSubscribedItems] = 60;
    wrapper.vm_idxs[m_GetItemState] = 61;
    wrapper.vm_idxs[m_GetItemInstallInfo] = 62;
    wrapper.vm_idxs[m_GetItemDownloadInfo] = 63;
    wrapper.vm_idxs[m_DownloadItem] = 64;
    wrapper.vm_idxs[m_BInitWorkshopForGameServer] = 65;
    wrapper.vm_idxs[m_SuspendDownloads] = 66;
    wrapper.vm_idxs[m_StartPlaytimeTracking] = 67;
    wrapper.vm_idxs[m_StopPlaytimeTracking] = 68;
    wrapper.vm_idxs[m_StopPlaytimeTrackingForAllItems] = 69;
    wrapper.vm_idxs[m_AddDependency] = 70;
    wrapper.vm_idxs[m_RemoveDependency] = 71;
    wrapper.vm_idxs[m_AddAppDependency] = 72;
    wrapper.vm_idxs[m_RemoveAppDependency] = 73;
    wrapper.vm_idxs[m_GetAppDependencies] = 74;
    wrapper.vm_idxs[m_DeleteItem] = 75;
  } else if (ver >= 0x0003005C0048003A) { // 03.92.72.58
    // "STEAMUGC_INTERFACE_VERSION010", used since Steamworks SDK v1.40
    wrapper.num_methods = 74;
    wrapper.vm_idxs[m_CreateQueryUserUGCRequest] = 0;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestPage] = 1;
    wrapper.vm_idxs[m_CreateQueryUGCDetailsRequest] = 2;
    wrapper.vm_idxs[m_SendQueryUGCRequest] = 3;
    wrapper.vm_idxs[m_GetQueryUGCResult] = 4;
    wrapper.vm_idxs[m_GetQueryUGCPreviewURL] = 5;
    wrapper.vm_idxs[m_GetQueryUGCMetadata] = 6;
    wrapper.vm_idxs[m_GetQueryUGCChildren] = 7;
    wrapper.vm_idxs[m_GetQueryUGCStatistic] = 8;
    wrapper.vm_idxs[m_GetQueryUGCNumAdditionalPreviews] = 9;
    wrapper.vm_idxs[m_GetQueryUGCAdditionalPreview] = 10;
    wrapper.vm_idxs[m_GetQueryUGCNumKeyValueTags] = 11;
    wrapper.vm_idxs[m_GetQueryUGCKeyValueTag] = 12;
    wrapper.vm_idxs[m_ReleaseQueryUGCRequest] = 13;
    wrapper.vm_idxs[m_AddRequiredTag] = 14;
    wrapper.vm_idxs[m_AddExcludedTag] = 15;
    wrapper.vm_idxs[m_SetReturnOnlyIDs] = 16;
    wrapper.vm_idxs[m_SetReturnKeyValueTags] = 17;
    wrapper.vm_idxs[m_SetReturnLongDescription] = 18;
    wrapper.vm_idxs[m_SetReturnMetadata] = 19;
    wrapper.vm_idxs[m_SetReturnChildren] = 20;
    wrapper.vm_idxs[m_SetReturnAdditionalPreviews] = 21;
    wrapper.vm_idxs[m_SetReturnTotalOnly] = 22;
    wrapper.vm_idxs[m_SetReturnPlaytimeStats] = 23;
    wrapper.vm_idxs[m_SetLanguage] = 24;
    wrapper.vm_idxs[m_SetAllowCachedResponse] = 25;
    wrapper.vm_idxs[m_SetCloudFileNameFilter] = 26;
    wrapper.vm_idxs[m_SetMatchAnyTag] = 27;
    wrapper.vm_idxs[m_SetSearchText] = 28;
    wrapper.vm_idxs[m_SetRankedByTrendDays] = 29;
    wrapper.vm_idxs[m_AddRequiredKeyValueTag] = 30;
    wrapper.vm_idxs[m_RequestUGCDetails] = 31;
    wrapper.vm_idxs[m_CreateItem] = 32;
    wrapper.vm_idxs[m_StartItemUpdate] = 33;
    wrapper.vm_idxs[m_SetItemTitle] = 34;
    wrapper.vm_idxs[m_SetItemDescription] = 35;
    wrapper.vm_idxs[m_SetItemUpdateLanguage] = 36;
    wrapper.vm_idxs[m_SetItemMetadata] = 37;
    wrapper.vm_idxs[m_SetItemVisibility] = 38;
    wrapper.vm_idxs[m_SetItemTags] = 39;
    wrapper.vm_idxs[m_SetItemContent] = 40;
    wrapper.vm_idxs[m_SetItemPreview] = 41;
    wrapper.vm_idxs[m_RemoveItemKeyValueTags] = 42;
    wrapper.vm_idxs[m_AddItemKeyValueTag] = 43;
    wrapper.vm_idxs[m_AddItemPreviewFile] = 44;
    wrapper.vm_idxs[m_AddItemPreviewVideo] = 45;
    wrapper.vm_idxs[m_UpdateItemPreviewFile] = 46;
    wrapper.vm_idxs[m_UpdateItemPreviewVideo] = 47;
    wrapper.vm_idxs[m_RemoveItemPreview] = 48;
    wrapper.vm_idxs[m_SubmitItemUpdate] = 49;
    wrapper.vm_idxs[m_GetItemUpdateProgress] = 50;
    wrapper.vm_idxs[m_SetUserItemVote] = 51;
    wrapper.vm_idxs[m_GetUserItemVote] = 52;
    wrapper.vm_idxs[m_AddItemToFavorites] = 53;
    wrapper.vm_idxs[m_RemoveItemFromFavorites] = 54;
    wrapper.vm_idxs[m_SubscribeItem] = 55;
    wrapper.vm_idxs[m_UnsubscribeItem] = 56;
    wrapper.vm_idxs[m_GetNumSubscribedItems] = 57;
    wrapper.vm_idxs[m_GetSubscribedItems] = 58;
    wrapper.vm_idxs[m_GetItemState] = 59;
    wrapper.vm_idxs[m_GetItemInstallInfo] = 60;
    wrapper.vm_idxs[m_GetItemDownloadInfo] = 61;
    wrapper.vm_idxs[m_DownloadItem] = 62;
    wrapper.vm_idxs[m_BInitWorkshopForGameServer] = 63;
    wrapper.vm_idxs[m_SuspendDownloads] = 64;
    wrapper.vm_idxs[m_StartPlaytimeTracking] = 65;
    wrapper.vm_idxs[m_StopPlaytimeTracking] = 66;
    wrapper.vm_idxs[m_StopPlaytimeTrackingForAllItems] = 67;
    wrapper.vm_idxs[m_AddDependency] = 68;
    wrapper.vm_idxs[m_RemoveDependency] = 69;
    wrapper.vm_idxs[m_AddAppDependency] = 70;
    wrapper.vm_idxs[m_RemoveAppDependency] = 71;
    wrapper.vm_idxs[m_GetAppDependencies] = 72;
    wrapper.vm_idxs[m_DeleteItem] = 73;
  } else if (ver >= 0x0003003E00520052) { // 03.62.82.82
    // "STEAMUGC_INTERFACE_VERSION009", used since Steamworks SDK v1.38
    wrapper.num_methods = 67;
    wrapper.vm_idxs[m_CreateQueryUserUGCRequest] = 0;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestPage] = 1;
    wrapper.vm_idxs[m_CreateQueryUGCDetailsRequest] = 2;
    wrapper.vm_idxs[m_SendQueryUGCRequest] = 3;
    wrapper.vm_idxs[m_GetQueryUGCResult] = 4;
    wrapper.vm_idxs[m_GetQueryUGCPreviewURL] = 5;
    wrapper.vm_idxs[m_GetQueryUGCMetadata] = 6;
    wrapper.vm_idxs[m_GetQueryUGCChildren] = 7;
    wrapper.vm_idxs[m_GetQueryUGCStatistic] = 8;
    wrapper.vm_idxs[m_GetQueryUGCNumAdditionalPreviews] = 9;
    wrapper.vm_idxs[m_GetQueryUGCAdditionalPreview] = 10;
    wrapper.vm_idxs[m_GetQueryUGCNumKeyValueTags] = 11;
    wrapper.vm_idxs[m_GetQueryUGCKeyValueTag] = 12;
    wrapper.vm_idxs[m_ReleaseQueryUGCRequest] = 13;
    wrapper.vm_idxs[m_AddRequiredTag] = 14;
    wrapper.vm_idxs[m_AddExcludedTag] = 15;
    wrapper.vm_idxs[m_SetReturnOnlyIDs] = 16;
    wrapper.vm_idxs[m_SetReturnKeyValueTags] = 17;
    wrapper.vm_idxs[m_SetReturnLongDescription] = 18;
    wrapper.vm_idxs[m_SetReturnMetadata] = 19;
    wrapper.vm_idxs[m_SetReturnChildren] = 20;
    wrapper.vm_idxs[m_SetReturnAdditionalPreviews] = 21;
    wrapper.vm_idxs[m_SetReturnTotalOnly] = 22;
    wrapper.vm_idxs[m_SetLanguage] = 23;
    wrapper.vm_idxs[m_SetAllowCachedResponse] = 24;
    wrapper.vm_idxs[m_SetCloudFileNameFilter] = 25;
    wrapper.vm_idxs[m_SetMatchAnyTag] = 26;
    wrapper.vm_idxs[m_SetSearchText] = 27;
    wrapper.vm_idxs[m_SetRankedByTrendDays] = 28;
    wrapper.vm_idxs[m_AddRequiredKeyValueTag] = 29;
    wrapper.vm_idxs[m_RequestUGCDetails] = 30;
    wrapper.vm_idxs[m_CreateItem] = 31;
    wrapper.vm_idxs[m_StartItemUpdate] = 32;
    wrapper.vm_idxs[m_SetItemTitle] = 33;
    wrapper.vm_idxs[m_SetItemDescription] = 34;
    wrapper.vm_idxs[m_SetItemUpdateLanguage] = 35;
    wrapper.vm_idxs[m_SetItemMetadata] = 36;
    wrapper.vm_idxs[m_SetItemVisibility] = 37;
    wrapper.vm_idxs[m_SetItemTags] = 38;
    wrapper.vm_idxs[m_SetItemContent] = 39;
    wrapper.vm_idxs[m_SetItemPreview] = 40;
    wrapper.vm_idxs[m_RemoveItemKeyValueTags] = 41;
    wrapper.vm_idxs[m_AddItemKeyValueTag] = 42;
    wrapper.vm_idxs[m_AddItemPreviewFile] = 43;
    wrapper.vm_idxs[m_AddItemPreviewVideo] = 44;
    wrapper.vm_idxs[m_UpdateItemPreviewFile] = 45;
    wrapper.vm_idxs[m_UpdateItemPreviewVideo] = 46;
    wrapper.vm_idxs[m_RemoveItemPreview] = 47;
    wrapper.vm_idxs[m_SubmitItemUpdate] = 48;
    wrapper.vm_idxs[m_GetItemUpdateProgress] = 49;
    wrapper.vm_idxs[m_SetUserItemVote] = 50;
    wrapper.vm_idxs[m_GetUserItemVote] = 51;
    wrapper.vm_idxs[m_AddItemToFavorites] = 52;
    wrapper.vm_idxs[m_RemoveItemFromFavorites] = 53;
    wrapper.vm_idxs[m_SubscribeItem] = 54;
    wrapper.vm_idxs[m_UnsubscribeItem] = 55;
    wrapper.vm_idxs[m_GetNumSubscribedItems] = 56;
    wrapper.vm_idxs[m_GetSubscribedItems] = 57;
    wrapper.vm_idxs[m_GetItemState] = 58;
    wrapper.vm_idxs[m_GetItemInstallInfo] = 59;
    wrapper.vm_idxs[m_GetItemDownloadInfo] = 60;
    wrapper.vm_idxs[m_DownloadItem] = 61;
    wrapper.vm_idxs[m_BInitWorkshopForGameServer] = 62;
    wrapper.vm_idxs[m_SuspendDownloads] = 63;
    wrapper.vm_idxs[m_StartPlaytimeTracking] = 64;
    wrapper.vm_idxs[m_StopPlaytimeTracking] = 65;
    wrapper.vm_idxs[m_StopPlaytimeTrackingForAllItems] = 66;
  } else if (ver >= 0x0003002A003D0042) { // 03.42.61.66
    // "STEAMUGC_INTERFACE_VERSION008", used in Steamworks SDK v1.37
    wrapper.num_methods = 63;
    wrapper.vm_idxs[m_CreateQueryUserUGCRequest] = 0;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestPage] = 1;
    wrapper.vm_idxs[m_CreateQueryUGCDetailsRequest] = 2;
    wrapper.vm_idxs[m_SendQueryUGCRequest] = 3;
    wrapper.vm_idxs[m_GetQueryUGCResult] = 4;
    wrapper.vm_idxs[m_GetQueryUGCPreviewURL] = 5;
    wrapper.vm_idxs[m_GetQueryUGCMetadata] = 6;
    wrapper.vm_idxs[m_GetQueryUGCChildren] = 7;
    wrapper.vm_idxs[m_GetQueryUGCStatistic] = 8;
    wrapper.vm_idxs[m_GetQueryUGCNumAdditionalPreviews] = 9;
    wrapper.vm_idxs[m_GetQueryUGCAdditionalPreview] = 10;
    wrapper.vm_idxs[m_GetQueryUGCNumKeyValueTags] = 11;
    wrapper.vm_idxs[m_GetQueryUGCKeyValueTag] = 12;
    wrapper.vm_idxs[m_ReleaseQueryUGCRequest] = 13;
    wrapper.vm_idxs[m_AddRequiredTag] = 14;
    wrapper.vm_idxs[m_AddExcludedTag] = 15;
    wrapper.vm_idxs[m_SetReturnKeyValueTags] = 16;
    wrapper.vm_idxs[m_SetReturnLongDescription] = 17;
    wrapper.vm_idxs[m_SetReturnMetadata] = 18;
    wrapper.vm_idxs[m_SetReturnChildren] = 19;
    wrapper.vm_idxs[m_SetReturnAdditionalPreviews] = 20;
    wrapper.vm_idxs[m_SetReturnTotalOnly] = 21;
    wrapper.vm_idxs[m_SetLanguage] = 22;
    wrapper.vm_idxs[m_SetAllowCachedResponse] = 23;
    wrapper.vm_idxs[m_SetCloudFileNameFilter] = 24;
    wrapper.vm_idxs[m_SetMatchAnyTag] = 25;
    wrapper.vm_idxs[m_SetSearchText] = 26;
    wrapper.vm_idxs[m_SetRankedByTrendDays] = 27;
    wrapper.vm_idxs[m_AddRequiredKeyValueTag] = 28;
    wrapper.vm_idxs[m_RequestUGCDetails] = 29;
    wrapper.vm_idxs[m_CreateItem] = 30;
    wrapper.vm_idxs[m_StartItemUpdate] = 31;
    wrapper.vm_idxs[m_SetItemTitle] = 32;
    wrapper.vm_idxs[m_SetItemDescription] = 33;
    wrapper.vm_idxs[m_SetItemUpdateLanguage] = 34;
    wrapper.vm_idxs[m_SetItemMetadata] = 35;
    wrapper.vm_idxs[m_SetItemVisibility] = 36;
    wrapper.vm_idxs[m_SetItemTags] = 37;
    wrapper.vm_idxs[m_SetItemContent] = 38;
    wrapper.vm_idxs[m_SetItemPreview] = 39;
    wrapper.vm_idxs[m_RemoveItemKeyValueTags] = 40;
    wrapper.vm_idxs[m_AddItemKeyValueTag] = 41;
    wrapper.vm_idxs[m_AddItemPreviewFile] = 42;
    wrapper.vm_idxs[m_AddItemPreviewVideo] = 43;
    wrapper.vm_idxs[m_UpdateItemPreviewFile] = 44;
    wrapper.vm_idxs[m_UpdateItemPreviewVideo] = 45;
    wrapper.vm_idxs[m_RemoveItemPreview] = 46;
    wrapper.vm_idxs[m_SubmitItemUpdate] = 47;
    wrapper.vm_idxs[m_GetItemUpdateProgress] = 48;
    wrapper.vm_idxs[m_SetUserItemVote] = 49;
    wrapper.vm_idxs[m_GetUserItemVote] = 50;
    wrapper.vm_idxs[m_AddItemToFavorites] = 51;
    wrapper.vm_idxs[m_RemoveItemFromFavorites] = 52;
    wrapper.vm_idxs[m_SubscribeItem] = 53;
    wrapper.vm_idxs[m_UnsubscribeItem] = 54;
    wrapper.vm_idxs[m_GetNumSubscribedItems] = 55;
    wrapper.vm_idxs[m_GetSubscribedItems] = 56;
    wrapper.vm_idxs[m_GetItemState] = 57;
    wrapper.vm_idxs[m_GetItemInstallInfo] = 58;
    wrapper.vm_idxs[m_GetItemDownloadInfo] = 59;
    wrapper.vm_idxs[m_DownloadItem] = 60;
    wrapper.vm_idxs[m_BInitWorkshopForGameServer] = 61;
    wrapper.vm_idxs[m_SuspendDownloads] = 62;
  } else if (ver >= 0x00020059002D0004) { // 02.89.45.04
    // "STEAMUGC_INTERFACE_VERSION007", used since Steamworks SDK v1.34
    wrapper.num_methods = 58;
    wrapper.vm_idxs[m_CreateQueryUserUGCRequest] = 0;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestPage] = 1;
    wrapper.vm_idxs[m_CreateQueryUGCDetailsRequest] = 2;
    wrapper.vm_idxs[m_SendQueryUGCRequest] = 3;
    wrapper.vm_idxs[m_GetQueryUGCResult] = 4;
    wrapper.vm_idxs[m_GetQueryUGCPreviewURL] = 5;
    wrapper.vm_idxs[m_GetQueryUGCMetadata] = 6;
    wrapper.vm_idxs[m_GetQueryUGCChildren] = 7;
    wrapper.vm_idxs[m_GetQueryUGCStatistic] = 8;
    wrapper.vm_idxs[m_GetQueryUGCNumAdditionalPreviews] = 9;
    wrapper.vm_idxs[m_GetQueryUGCAdditionalPreview] = 10;
    wrapper.vm_idxs[m_GetQueryUGCNumKeyValueTags] = 11;
    wrapper.vm_idxs[m_GetQueryUGCKeyValueTag] = 12;
    wrapper.vm_idxs[m_ReleaseQueryUGCRequest] = 13;
    wrapper.vm_idxs[m_AddRequiredTag] = 14;
    wrapper.vm_idxs[m_AddExcludedTag] = 15;
    wrapper.vm_idxs[m_SetReturnKeyValueTags] = 16;
    wrapper.vm_idxs[m_SetReturnLongDescription] = 17;
    wrapper.vm_idxs[m_SetReturnMetadata] = 18;
    wrapper.vm_idxs[m_SetReturnChildren] = 19;
    wrapper.vm_idxs[m_SetReturnAdditionalPreviews] = 20;
    wrapper.vm_idxs[m_SetReturnTotalOnly] = 21;
    wrapper.vm_idxs[m_SetLanguage] = 22;
    wrapper.vm_idxs[m_SetAllowCachedResponse] = 23;
    wrapper.vm_idxs[m_SetCloudFileNameFilter] = 24;
    wrapper.vm_idxs[m_SetMatchAnyTag] = 25;
    wrapper.vm_idxs[m_SetSearchText] = 26;
    wrapper.vm_idxs[m_SetRankedByTrendDays] = 27;
    wrapper.vm_idxs[m_AddRequiredKeyValueTag] = 28;
    wrapper.vm_idxs[m_RequestUGCDetails] = 29;
    wrapper.vm_idxs[m_CreateItem] = 30;
    wrapper.vm_idxs[m_StartItemUpdate] = 31;
    wrapper.vm_idxs[m_SetItemTitle] = 32;
    wrapper.vm_idxs[m_SetItemDescription] = 33;
    wrapper.vm_idxs[m_SetItemUpdateLanguage] = 34;
    wrapper.vm_idxs[m_SetItemMetadata] = 35;
    wrapper.vm_idxs[m_SetItemVisibility] = 36;
    wrapper.vm_idxs[m_SetItemTags] = 37;
    wrapper.vm_idxs[m_SetItemContent] = 38;
    wrapper.vm_idxs[m_SetItemPreview] = 39;
    wrapper.vm_idxs[m_RemoveItemKeyValueTags] = 40;
    wrapper.vm_idxs[m_AddItemKeyValueTag] = 41;
    wrapper.vm_idxs[m_SubmitItemUpdate] = 42;
    wrapper.vm_idxs[m_GetItemUpdateProgress] = 43;
    wrapper.vm_idxs[m_SetUserItemVote] = 44;
    wrapper.vm_idxs[m_GetUserItemVote] = 45;
    wrapper.vm_idxs[m_AddItemToFavorites] = 46;
    wrapper.vm_idxs[m_RemoveItemFromFavorites] = 47;
    wrapper.vm_idxs[m_SubscribeItem] = 48;
    wrapper.vm_idxs[m_UnsubscribeItem] = 49;
    wrapper.vm_idxs[m_GetNumSubscribedItems] = 50;
    wrapper.vm_idxs[m_GetSubscribedItems] = 51;
    wrapper.vm_idxs[m_GetItemState] = 52;
    wrapper.vm_idxs[m_GetItemInstallInfo] = 53;
    wrapper.vm_idxs[m_GetItemDownloadInfo] = 54;
    wrapper.vm_idxs[m_DownloadItem] = 55;
    wrapper.vm_idxs[m_BInitWorkshopForGameServer] = 56;
    wrapper.vm_idxs[m_SuspendDownloads] = 57;
  } else if (ver >= 0x0002004D00250052) { // 02.77.37.82
    // "STEAMUGC_INTERFACE_VERSION005", used in Steamworks SDK v1.33
    wrapper.num_methods = 46;
    wrapper.vm_idxs[m_CreateQueryUserUGCRequest] = 0;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestPage] = 1;
    wrapper.vm_idxs[m_CreateQueryUGCDetailsRequest] = 2;
    wrapper.vm_idxs[m_SendQueryUGCRequest] = 3;
    wrapper.vm_idxs[m_GetQueryUGCResult] = 4;
    wrapper.vm_idxs[m_GetQueryUGCPreviewURL] = 5;
    wrapper.vm_idxs[m_GetQueryUGCMetadata] = 6;
    wrapper.vm_idxs[m_GetQueryUGCChildren] = 7;
    wrapper.vm_idxs[m_GetQueryUGCStatistic] = 8;
    wrapper.vm_idxs[m_GetQueryUGCNumAdditionalPreviews] = 9;
    wrapper.vm_idxs[m_GetQueryUGCAdditionalPreview] = 10;
    wrapper.vm_idxs[m_ReleaseQueryUGCRequest] = 11;
    wrapper.vm_idxs[m_AddRequiredTag] = 12;
    wrapper.vm_idxs[m_AddExcludedTag] = 13;
    wrapper.vm_idxs[m_SetReturnLongDescription] = 14;
    wrapper.vm_idxs[m_SetReturnMetadata] = 15;
    wrapper.vm_idxs[m_SetReturnChildren] = 16;
    wrapper.vm_idxs[m_SetReturnAdditionalPreviews] = 17;
    wrapper.vm_idxs[m_SetReturnTotalOnly] = 18;
    wrapper.vm_idxs[m_SetAllowCachedResponse] = 19;
    wrapper.vm_idxs[m_SetCloudFileNameFilter] = 20;
    wrapper.vm_idxs[m_SetMatchAnyTag] = 21;
    wrapper.vm_idxs[m_SetSearchText] = 22;
    wrapper.vm_idxs[m_SetRankedByTrendDays] = 23;
    wrapper.vm_idxs[m_RequestUGCDetails] = 24;
    wrapper.vm_idxs[m_CreateItem] = 25;
    wrapper.vm_idxs[m_StartItemUpdate] = 26;
    wrapper.vm_idxs[m_SetItemTitle] = 27;
    wrapper.vm_idxs[m_SetItemDescription] = 28;
    wrapper.vm_idxs[m_SetItemMetadata] = 29;
    wrapper.vm_idxs[m_SetItemVisibility] = 30;
    wrapper.vm_idxs[m_SetItemTags] = 31;
    wrapper.vm_idxs[m_SetItemContent] = 32;
    wrapper.vm_idxs[m_SetItemPreview] = 33;
    wrapper.vm_idxs[m_SubmitItemUpdate] = 34;
    wrapper.vm_idxs[m_GetItemUpdateProgress] = 35;
    wrapper.vm_idxs[m_AddItemToFavorites] = 36;
    wrapper.vm_idxs[m_RemoveItemFromFavorites] = 37;
    wrapper.vm_idxs[m_SubscribeItem] = 38;
    wrapper.vm_idxs[m_UnsubscribeItem] = 39;
    wrapper.vm_idxs[m_GetNumSubscribedItems] = 40;
    wrapper.vm_idxs[m_GetSubscribedItems] = 41;
    wrapper.vm_idxs[m_GetItemState] = 42;
    wrapper.vm_idxs[m_GetItemInstallInfo] = 43;
    wrapper.vm_idxs[m_GetItemDownloadInfo] = 44;
    wrapper.vm_idxs[m_DownloadItem] = 45;
  } else if (ver >= 0x000200130022005D) { // 02.19.34.93
    // "STEAMUGC_INTERFACE_VERSION002" and "STEAMUGC_INTERFACE_VERSION003",
    //    used since Steamworks SDK v1.29
    wrapper.num_methods = 31;
    wrapper.vm_idxs[m_CreateQueryUserUGCRequest] = 0;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestPage] = 1;
    wrapper.vm_idxs[m_SendQueryUGCRequest] = 2;
    wrapper.vm_idxs[m_GetQueryUGCResult] = 3;
    wrapper.vm_idxs[m_ReleaseQueryUGCRequest] = 4;
    wrapper.vm_idxs[m_AddRequiredTag] = 5;
    wrapper.vm_idxs[m_AddExcludedTag] = 6;
    wrapper.vm_idxs[m_SetReturnLongDescription] = 7;
    wrapper.vm_idxs[m_SetReturnTotalOnly] = 8;
    wrapper.vm_idxs[m_SetAllowCachedResponse] = 9;
    wrapper.vm_idxs[m_SetCloudFileNameFilter] = 10;
    wrapper.vm_idxs[m_SetMatchAnyTag] = 11;
    wrapper.vm_idxs[m_SetSearchText] = 12;
    wrapper.vm_idxs[m_SetRankedByTrendDays] = 13;
    wrapper.vm_idxs[m_RequestUGCDetails] = 14;
    wrapper.vm_idxs[m_CreateItem] = 15;
    wrapper.vm_idxs[m_StartItemUpdate] = 16;
    wrapper.vm_idxs[m_SetItemTitle] = 17;
    wrapper.vm_idxs[m_SetItemDescription] = 18;
    wrapper.vm_idxs[m_SetItemVisibility] = 19;
    wrapper.vm_idxs[m_SetItemTags] = 20;
    wrapper.vm_idxs[m_SetItemContent] = 21;
    wrapper.vm_idxs[m_SetItemPreview] = 22;
    wrapper.vm_idxs[m_SubmitItemUpdate] = 23;
    wrapper.vm_idxs[m_GetItemUpdateProgress] = 24;
    wrapper.vm_idxs[m_SubscribeItem] = 25;
    wrapper.vm_idxs[m_UnsubscribeItem] = 26;
    wrapper.vm_idxs[m_GetNumSubscribedItems] = 27;
    wrapper.vm_idxs[m_GetSubscribedItems] = 28;
    wrapper.vm_idxs[m_GetItemInstallInfo] = 29;
    wrapper.vm_idxs[m_GetItemUpdateInfo] = 30;
  } else {
    // "STEAMUGC_INTERFACE_VERSION001"
    wrapper.num_methods = 14;
    wrapper.vm_idxs[m_CreateQueryUserUGCRequest] = 0;
    wrapper.vm_idxs[m_CreateQueryAllUGCRequestPage] = 1;
    wrapper.vm_idxs[m_SendQueryUGCRequest] = 2;
    wrapper.vm_idxs[m_GetQueryUGCResult] = 3;
    wrapper.vm_idxs[m_ReleaseQueryUGCRequest] = 4;
    wrapper.vm_idxs[m_AddRequiredTag] = 5;
    wrapper.vm_idxs[m_AddExcludedTag] = 6;
    wrapper.vm_idxs[m_SetReturnLongDescription] = 7;
    wrapper.vm_idxs[m_SetReturnTotalOnly] = 8;
    wrapper.vm_idxs[m_SetCloudFileNameFilter] = 9;
    wrapper.vm_idxs[m_SetMatchAnyTag] = 10;
    wrapper.vm_idxs[m_SetSearchText] = 11;
    wrapper.vm_idxs[m_SetRankedByTrendDays] = 12;
    wrapper.vm_idxs[m_RequestUGCDetails] = 13;
  }
  wrapper.orig_vtable = iface->vtable;
  wrapper.iface = iface;
  std::ranges::copy_n(iface->vtable, wrapper.num_methods,
                      wrapper.vtable.begin());
  iface->vtable = wrapper.vtable.data();
}

/// Get interface version string for Steamworks SDK v1.37+.
constexpr const char *_Nonnull get_ver_str() {
  if (ver >= 0x0009003C002C000A) { // 09.60.44.10
    // Steamworks SDK v1.62
    return "STEAMUGC_INTERFACE_VERSION021";
  } else if (ver >= 0x0008006100630046) { // 08.97.99.70
    // Steamworks SDK v1.60+
    return "STEAMUGC_INTERFACE_VERSION020";
  } else if (ver >= 0x0008002100090017) { // 08.33.09.23
    // Steamworks SDK v1.58+
    return "STEAMUGC_INTERFACE_VERSION018";
  } else if (ver >= 0x000700600000002C) { // 07.96.00.44
    // Steamworks SDK v1.56+
    return "STEAMUGC_INTERFACE_VERSION017";
  } else if (ver >= 0x0006005B00150039) { // 06.91.21.57
    // Steamworks SDK v1.53+
    return "STEAMUGC_INTERFACE_VERSION016";
  } else if (ver >= 0x0006001C00120056) { // 06.28.18.86
    // Steamworks SDK v1.51+
    return "STEAMUGC_INTERFACE_VERSION015";
  } else if (ver >= 0x000500350021004E) { // 05.53.33.78
    // Steamworks SDK v1.47+
    return "STEAMUGC_INTERFACE_VERSION014";
  } else if (ver >= 0x000500130026003E) { // 05.19.38.62
    // Steamworks SDK v1.45+
    return "STEAMUGC_INTERFACE_VERSION013";
  } else if (ver >= 0x0004005F0014001E) { // 04.95.20.30
    // Steamworks SDK v1.43+
    return "STEAMUGC_INTERFACE_VERSION012";
  } else if (ver >= 0x0003005C0048003A) { // 03.92.72.58
    // Steamworks SDK v1.40+
    return "STEAMUGC_INTERFACE_VERSION010";
  } else if (ver >= 0x0003003E00520052) { // 03.62.82.82
    // Steamworks SDK v1.38+
    return "STEAMUGC_INTERFACE_VERSION009";
  } else {
    // Steamworks SDK v1.37
    return "STEAMUGC_INTERFACE_VERSION008";
  }
}

} // namespace tek::game_runtime::steam::ISteamUGC
