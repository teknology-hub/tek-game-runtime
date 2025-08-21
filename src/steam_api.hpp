//===-- steam_api.hpp - Steam API wrapper interface internal declarations -===//
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
/// Declarations of Steam API wrapper interface functions that may be used by
///    other modules.
///
//===----------------------------------------------------------------------===//
#pragma once
#include "common.hpp" // IWYU pragma: keep

#include <array>
#include <cstddef>
#include <cstdint>
#include <tek-steamclient/cm.h>

namespace tek::game_runtime::steam_api {

//===-- Types -------------------------------------------------------------===//

/// Steam interface wrapper descriptor.
///
/// @tparam max_num_methods
///    The maximum number of methods that the interface may contain, i.e. the
///    number of methods in the highest supported version.
template <std::size_t max_num_methods> struct wrapper_desc {
  /// Actual number of available interface methods, determined at runtime via
  ///    interface version detection.
  std::size_t num_methods;
  /// Pointer to the original virtual method table for the interface.
  void *const _Nonnull *_Nullable orig_vtable;
  /// Pointer to the interface instance.
  void *_Nullable iface;
  /// Wrapper's virtual method table
  std::array<void *_Nullable, max_num_methods> vtable;
  /// Table for mapping interface's corresponding _m enum values to vtable
  ///    indices. Values of `-1` indicate that method is unavailable.
  std::array<int, max_num_methods> vm_idxs;

  [[gnu::visibility("internal")]]
  constexpr wrapper_desc() {
    vm_idxs.fill(-1);
  }
};

/// Virtual method enumeration for ISteamApps interface.
enum ISteamApps_m {
  ISteamApps_m_BIsSubscribed,
  ISteamApps_m_BIsLowViolence,
  ISteamApps_m_BIsCybercafe,
  ISteamApps_m_BIsVACBanned,
  ISteamApps_m_GetCurrentGameLanguage,
  ISteamApps_m_GetAvailableGameLanguages,
  ISteamApps_m_BIsSubscribedApp,
  ISteamApps_m_BIsDlcInstalled,
  ISteamApps_m_GetEarliestPurchaseUnixTime,
  ISteamApps_m_BIsSubscribedFromFreeWeekend,
  ISteamApps_m_GetDLCCount,
  ISteamApps_m_BGetDLCDataByIndex,
  ISteamApps_m_InstallDLC,
  ISteamApps_m_UninstallDLC,
  ISteamApps_m_RequestAppProofOfPurchaseKey,
  ISteamApps_m_GetCurrentBetaName,
  ISteamApps_m_MarkContentCorrupt,
  ISteamApps_m_GetInstalledDepots,
  ISteamApps_m_GetAppInstallDir,
  ISteamApps_m_BIsAppInstalled,
  ISteamApps_m_GetAppOwner,
  ISteamApps_m_GetLaunchQueryParam,
  ISteamApps_m_GetDlcDownloadProgress,
  ISteamApps_m_GetAppBuildId,
  ISteamApps_m_RequestAllProofOfPurchaseKeys,
  ISteamApps_m_GetFileDetails,
  ISteamApps_m_GetLaunchCommandLine,
  ISteamApps_m_BIsSubscribedFromFamilySharing,
  ISteamApps_m_BIsTimedTrial,
  ISteamApps_m_SetDlcContext,
  ISteamApps_m_GetNumBetas,
  ISteamApps_m_GetBetaInfo,
  ISteamApps_m_SetActiveBeta,
  ISteamApps_num_methods
};

/// Virtual method enumeration for ISteamMatchmaking interface.
enum ISteamMatchmaking_m {
  ISteamMatchmaking_m_GetFavoriteGameCount,
  ISteamMatchmaking_m_GetFavoriteGame,
  ISteamMatchmaking_m_AddFavoriteGame,
  ISteamMatchmaking_m_RemoveFavoriteGame,
  ISteamMatchmaking_m_RequestLobbyList,
  ISteamMatchmaking_m_AddRequestLobbyListStringFilter,
  ISteamMatchmaking_m_AddRequestLobbyListNumericalFilter,
  ISteamMatchmaking_m_AddRequestLobbyListNearValueFilter,
  ISteamMatchmaking_m_AddRequestLobbyListFilterSlotsAvailable,
  ISteamMatchmaking_m_AddRequestLobbyListDistanceFilter,
  ISteamMatchmaking_m_AddRequestLobbyListResultCountFilter,
  ISteamMatchmaking_m_AddRequestLobbyListCompatibleMembersFilter,
  ISteamMatchmaking_m_GetLobbyByIndex,
  ISteamMatchmaking_m_CreateLobby,
  ISteamMatchmaking_m_JoinLobby,
  ISteamMatchmaking_m_LeaveLobby,
  ISteamMatchmaking_m_InviteUserToLobby,
  ISteamMatchmaking_m_GetNumLobbyMembers,
  ISteamMatchmaking_m_GetLobbyMemberByIndex,
  ISteamMatchmaking_m_GetLobbyData,
  ISteamMatchmaking_m_SetLobbyData,
  ISteamMatchmaking_m_GetLobbyDataCount,
  ISteamMatchmaking_m_GetLobbyDataByIndex,
  ISteamMatchmaking_m_DeleteLobbyData,
  ISteamMatchmaking_m_GetLobbyMemberData,
  ISteamMatchmaking_m_SetLobbyMemberData,
  ISteamMatchmaking_m_SendLobbyChatMsg,
  ISteamMatchmaking_m_GetLobbyChatEntry,
  ISteamMatchmaking_m_RequestLobbyData,
  ISteamMatchmaking_m_SetLobbyGameServer,
  ISteamMatchmaking_m_GetLobbyGameServer,
  ISteamMatchmaking_m_SetLobbyMemberLimit,
  ISteamMatchmaking_m_GetLobbyMemberLimit,
  ISteamMatchmaking_m_SetLobbyType,
  ISteamMatchmaking_m_SetLobbyJoinable,
  ISteamMatchmaking_m_GetLobbyOwner,
  ISteamMatchmaking_m_SetLobbyOwner,
  ISteamMatchmaking_m_SetLinkedLobby,
  ISteamMatchmaking_num_methods
};

/// Virtual method enumeration for ISteamMatchmakingServers interface.
enum ISteamMatchmakingServers_m {
  ISteamMatchmakingServers_m_RequestInternetServerList,
  ISteamMatchmakingServers_m_RequestLANServerList,
  ISteamMatchmakingServers_m_RequestFriendsServerList,
  ISteamMatchmakingServers_m_RequestFavoritesServerList,
  ISteamMatchmakingServers_m_RequestHistoryServerList,
  ISteamMatchmakingServers_m_RequestSpectatorServerList,
  ISteamMatchmakingServers_m_ReleaseRequest,
  ISteamMatchmakingServers_m_GetServerDetails,
  ISteamMatchmakingServers_m_CancelQuery,
  ISteamMatchmakingServers_m_RefreshQuery,
  ISteamMatchmakingServers_m_IsRefreshing,
  ISteamMatchmakingServers_m_GetServerCount,
  ISteamMatchmakingServers_m_RefreshServer,
  ISteamMatchmakingServers_m_PingServer,
  ISteamMatchmakingServers_m_PlayerDetails,
  ISteamMatchmakingServers_m_ServerRules,
  ISteamMatchmakingServers_m_CancelServerQuery,
  ISteamMatchmakingServers_num_methods
};

/// Virtual method enumeration for ISteamUGC interface.
enum ISteamUGC_m {
  ISteamUGC_m_CreateQueryUserUGCRequest,
  ISteamUGC_m_CreateQueryAllUGCRequestCursor,
  ISteamUGC_m_CreateQueryAllUGCRequestPage,
  ISteamUGC_m_CreateQueryUGCDetailsRequest,
  ISteamUGC_m_SendQueryUGCRequest,
  ISteamUGC_m_GetQueryUGCResult,
  ISteamUGC_m_GetQueryUGCNumTags,
  ISteamUGC_m_GetQueryUGCTag,
  ISteamUGC_m_GetQueryUGCTagDisplayName,
  ISteamUGC_m_GetQueryUGCPreviewURL,
  ISteamUGC_m_GetQueryUGCMetadata,
  ISteamUGC_m_GetQueryUGCChildren,
  ISteamUGC_m_GetQueryUGCStatistic,
  ISteamUGC_m_GetQueryUGCNumAdditionalPreviews,
  ISteamUGC_m_GetQueryUGCAdditionalPreview,
  ISteamUGC_m_GetQueryUGCNumKeyValueTags,
  ISteamUGC_m_GetQueryFirstUGCKeyValueTag,
  ISteamUGC_m_GetQueryUGCKeyValueTag,
  ISteamUGC_m_GetNumSupportedGameVersions,
  ISteamUGC_m_GetSupportedGameVersionData,
  ISteamUGC_m_GetQueryUGCContentDescriptors,
  ISteamUGC_m_ReleaseQueryUGCRequest,
  ISteamUGC_m_AddRequiredTag,
  ISteamUGC_m_AddRequiredTagGroup,
  ISteamUGC_m_AddExcludedTag,
  ISteamUGC_m_SetReturnOnlyIDs,
  ISteamUGC_m_SetReturnKeyValueTags,
  ISteamUGC_m_SetReturnLongDescription,
  ISteamUGC_m_SetReturnMetadata,
  ISteamUGC_m_SetReturnChildren,
  ISteamUGC_m_SetReturnAdditionalPreviews,
  ISteamUGC_m_SetReturnTotalOnly,
  ISteamUGC_m_SetReturnPlaytimeStats,
  ISteamUGC_m_SetLanguage,
  ISteamUGC_m_SetAllowCachedResponse,
  ISteamUGC_m_SetAdminQuery,
  ISteamUGC_m_SetCloudFileNameFilter,
  ISteamUGC_m_SetMatchAnyTag,
  ISteamUGC_m_SetSearchText,
  ISteamUGC_m_SetRankedByTrendDays,
  ISteamUGC_m_SetTimeCreatedDateRange,
  ISteamUGC_m_SetTimeUpdatedDateRange,
  ISteamUGC_m_AddRequiredKeyValueTag,
  ISteamUGC_m_RequestUGCDetails,
  ISteamUGC_m_CreateItem,
  ISteamUGC_m_StartItemUpdate,
  ISteamUGC_m_SetItemTitle,
  ISteamUGC_m_SetItemDescription,
  ISteamUGC_m_SetItemUpdateLanguage,
  ISteamUGC_m_SetItemMetadata,
  ISteamUGC_m_SetItemVisibility,
  ISteamUGC_m_SetItemTags,
  ISteamUGC_m_SetItemContent,
  ISteamUGC_m_SetItemPreview,
  ISteamUGC_m_SetAllowLegacyUpload,
  ISteamUGC_m_RemoveAllItemKeyValueTags,
  ISteamUGC_m_RemoveItemKeyValueTags,
  ISteamUGC_m_AddItemKeyValueTag,
  ISteamUGC_m_AddItemPreviewFile,
  ISteamUGC_m_AddItemPreviewVideo,
  ISteamUGC_m_UpdateItemPreviewFile,
  ISteamUGC_m_UpdateItemPreviewVideo,
  ISteamUGC_m_RemoveItemPreview,
  ISteamUGC_m_AddContentDescriptor,
  ISteamUGC_m_RemoveContentDescriptor,
  ISteamUGC_m_SetRequiredGameVersions,
  ISteamUGC_m_SubmitItemUpdate,
  ISteamUGC_m_GetItemUpdateProgress,
  ISteamUGC_m_SetUserItemVote,
  ISteamUGC_m_GetUserItemVote,
  ISteamUGC_m_AddItemToFavorites,
  ISteamUGC_m_RemoveItemFromFavorites,
  ISteamUGC_m_SubscribeItem,
  ISteamUGC_m_UnsubscribeItem,
  ISteamUGC_m_GetNumSubscribedItems,
  ISteamUGC_m_GetSubscribedItems,
  ISteamUGC_m_GetItemState,
  ISteamUGC_m_GetItemInstallInfo,
  ISteamUGC_m_GetItemDownloadInfo,
  ISteamUGC_m_GetItemUpdateInfo = ISteamUGC_m_GetItemDownloadInfo,
  ISteamUGC_m_DownloadItem,
  ISteamUGC_m_BInitWorkshopForGameServer,
  ISteamUGC_m_SuspendDownloads,
  ISteamUGC_m_StartPlaytimeTracking,
  ISteamUGC_m_StopPlaytimeTracking,
  ISteamUGC_m_StopPlaytimeTrackingForAllItems,
  ISteamUGC_m_AddDependency,
  ISteamUGC_m_RemoveDependency,
  ISteamUGC_m_AddAppDependency,
  ISteamUGC_m_RemoveAppDependency,
  ISteamUGC_m_GetAppDependencies,
  ISteamUGC_m_DeleteItem,
  ISteamUGC_m_ShowWorkshopEULA,
  ISteamUGC_m_GetWorkshopEULAStatus,
  ISteamUGC_m_GetUserContentDescriptorPreferences,
  ISteamUGC_m_SetItemsDisabledLocally,
  ISteamUGC_m_SetSubscriptionsLoadOrder,
  ISteamUGC_num_methods
};

/// Virtual method enumeration for ISteamUser interface.
enum ISteamUser_m {
  ISteamUser_m_GetHSteamUser,
  ISteamUser_m_BLoggedOn,
  ISteamUser_m_GetSteamID,
  ISteamUser_m_InitiateGameConnection,
  ISteamUser_m_TerminateGameConnection,
  ISteamUser_m_TrackAppUsageEvent,
  ISteamUser_m_GetUserDataFolder,
  ISteamUser_m_StartVoiceRecording,
  ISteamUser_m_StopVoiceRecording,
  ISteamUser_m_GetAvailableVoice,
  ISteamUser_m_GetVoice,
  ISteamUser_m_DecompressVoice,
  ISteamUser_m_GetVoiceOptimalSampleRate,
  ISteamUser_m_GetAuthSessionTicket,
  ISteamUser_m_GetAuthTicketForWebApi,
  ISteamUser_m_BeginAuthSession,
  ISteamUser_m_EndAuthSession,
  ISteamUser_m_CancelAuthTicket,
  ISteamUser_m_UserHasLicenseForApp,
  ISteamUser_m_BIsBehindNAT,
  ISteamUser_m_AdvertiseGame,
  ISteamUser_m_RequestEncryptedAppTicket,
  ISteamUser_m_GetEncryptedAppTicket,
  ISteamUser_m_GetGameBadgeLevel,
  ISteamUser_m_GetPlayerSteamLevel,
  ISteamUser_m_RequestStoreAuthURL,
  ISteamUser_m_BIsPhoneVerified,
  ISteamUser_m_BIsTwoFactorEnabled,
  ISteamUser_m_BIsPhoneIdentifying,
  ISteamUser_m_BIsPhoneRequiringVerification,
  ISteamUser_m_GetMarketEligibility,
  ISteamUser_m_GetDurationControl,
  ISteamUser_m_BSetDurationControlOnlineState,
  ISteamUser_num_methods
};

/// Virtual method enumeration for ISteamUtils interface.
enum ISteamUtils_m {
  ISteamUtils_m_GetSecondsSinceAppActive,
  ISteamUtils_m_GetSecondsSinceComputerActive,
  ISteamUtils_m_GetConnectedUniverse,
  ISteamUtils_m_GetServerRealTime,
  ISteamUtils_m_GetIPCountry,
  ISteamUtils_m_GetImageSize,
  ISteamUtils_m_GetImageRGBA,
  ISteamUtils_m_GetCSERIPPort,
  ISteamUtils_m_GetCurrentBatteryPower,
  ISteamUtils_m_GetAppID,
  ISteamUtils_m_SetOverlayNotificationPosition,
  ISteamUtils_m_IsAPICallCompleted,
  ISteamUtils_m_GetAPICallFailureReason,
  ISteamUtils_m_GetAPICallResult,
  ISteamUtils_m_RunFrame,
  ISteamUtils_m_GetIPCCallCount,
  ISteamUtils_m_SetWarningMessageHook,
  ISteamUtils_m_IsOverlayEnabled,
  ISteamUtils_m_BOverlayNeedsPresent,
  ISteamUtils_m_CheckFileSignature,
  ISteamUtils_m_ShowGamepadTextInput,
  ISteamUtils_m_GetEnteredGamepadTextLength,
  ISteamUtils_m_GetEnteredGamepadTextInput,
  ISteamUtils_m_GetSteamUILanguage,
  ISteamUtils_m_IsSteamRunningInVR,
  ISteamUtils_m_SetOverlayNotificationInset,
  ISteamUtils_m_IsSteamInBigPictureMode,
  ISteamUtils_m_StartVRDashboard,
  ISteamUtils_m_IsVRHeadsetStreamingEnabled,
  ISteamUtils_m_SetVRHeadsetStreamingEnabled,
  ISteamUtils_m_IsSteamChinaLauncher,
  ISteamUtils_m_InitFilterText,
  ISteamUtils_m_FilterText,
  ISteamUtils_m_GetIPv6ConnectivityState,
  ISteamUtils_m_IsSteamRunningOnSteamDeck,
  ISteamUtils_m_ShowFloatingGamepadTextInput,
  ISteamUtils_m_SetGameLauncherMode,
  ISteamUtils_m_DismissFloatingGamepadTextInput,
  ISteamUtils_m_DismissGamepadTextInput,
  ISteamUtils_num_methods
};

enum class UserHasLicenseForAppResult {
  HasLicense,
  DoesNotHaveLicense,
  NoAuth
};

struct matchmaking_kv_pair {
  std::array<char, 256> key;
  std::array<char, 256> value;
};

struct remote_storage_sub_result {
  tek_sc_cm_eresult result;
  std::uint64_t id;
};

struct ISteamMatchmakingRulesResponse {
  virtual void RulesResponded(const char *_Nonnull key,
                              const char *_Nonnull value) = 0;
  virtual void RulesFailedToRespond() = 0;
  virtual void RulesRefreshComplete() = 0;
};

using ISteamApps_BIsSubscribedApp_t = bool(void *_Nonnull iface,
                                           std::uint32_t app_id);
using ISteamApps_BIsAppInstalled_t = bool(void *_Nonnull iface,
                                          std::uint32_t app_id);

using ISteamMatchmakingServers_RequestInternetServerList_t =
    void *_Nonnull(void *_Nonnull iface, std::uint32_t app_id,
                   const matchmaking_kv_pair *const _Nonnull *_Nullable filters,
                   std::uint32_t num_filters, void *_Nonnull response_handler);
using ISteamMatchmakingServers_ServerRules_t =
    int(void *_Nonnull iface, std::uint32_t ip, std::uint16_t port,
        ISteamMatchmakingRulesResponse *_Nonnull response_handler);
using ISteamMatchmakingServers_CancelServerQuery_t = void(void *_Nonnull iface,
                                                          int query);

using ISteamUser_GetSteamID_t =
    std::uint64_t *_Nonnull(void *_Nonnull iface, std::uint64_t *_Nonnull id);

using ISteamUtils_IsAPICallCompleted_t = bool(void *_Nonnull iface,
                                              std::uint64_t call,
                                              bool *_Nonnull failed);
using ISteamUtils_GetAPICallResult_t = bool(void *_Nonnull iface,
                                            std::uint64_t call,
                                            void *_Nonnull callback,
                                            int callback_size, int callback_idx,
                                            bool *_Nonnull failed);

//===-- Variables ---------------------------------------------------------===//

/// Highest supported steam_api64.dll file version.
/// Current is `09.60.44.10` from Steamworks SDK v1.62
constexpr std::uint64_t max_supported_ver = 0x0009003C002C000A;

/// Current detected steam_api64.dll file version.
inline std::uint64_t ver;
/// Current user's Steam ID.
inline std::uint64_t steam_id;

/// Wrapper descriptor for ISteamApps interface.
inline wrapper_desc<ISteamApps_num_methods> ISteamApps_desc;
/// Wrapper descriptor for ISteamMatchmaking interface.
inline wrapper_desc<ISteamMatchmaking_num_methods> ISteamMatchmaking_desc;
/// Wrapper descriptor for ISteamMatchmakingServers interface.
inline wrapper_desc<ISteamMatchmakingServers_num_methods>
    ISteamMatchmakingServers_desc;
/// Wrapper descriptor for ISteamUGC interface.
inline wrapper_desc<ISteamUGC_num_methods> ISteamUGC_desc;
/// Wrapper descriptor for ISteamUser interface.
inline wrapper_desc<ISteamUser_num_methods> ISteamUser_desc;
/// Wrapper descriptor for ISteamUtils interface.
inline wrapper_desc<ISteamUtils_num_methods> ISteamUtils_desc;

//===-- Function ----------------------------------------------------------===//

/// Install IAT hook for SteamAPI_Init to setup vtable wrappers.
[[gnu::visibility("internal")]]
void wrap_init();

} // namespace tek::game_runtime::steam_api
