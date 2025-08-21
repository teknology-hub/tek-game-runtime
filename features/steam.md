# Steam

## Features

- All Steam API versions up until 1.62 are supported
- Wrappers for the following interfaces are provided (more can be added if needed):
  + ISteamApps
  + ISteamMatchmaking
  + ISteamMatchmakingServers
  + ISteamUGC
  + ISteamUser
  + ISteamUtils
- Method availability and their vtable indices are computed at runtime based on detected version
- Common features enabled for all games:
  + `ISteamApps::BIsSubscribed` will always return `true`
  + `ISteamApps::BIsSubscribedApp` will always return `true` for current app ID and DLC listed under `dlc` settings option
  + `ISteamApps::BIsDlcInstalled` will always return `true` for DLC listed under `installed_dlc` settings option
  + `ISteamApps::BIsSubscribedFromFreeWeekend` will always return `false`
  + `ISteamApps::GetDLCCount` will return the number of elements in `dlc` settings option
  + `ISteamApps::BGetDLCDataByIndex` will return data listed under `dlc` settings option
  + `ISteamApps::BIsAppInstalled` will always return `true` for current app ID and DLC listed under `installed_dlc` settings option
  + `ISteamApps::BIsSubscribedFromFamilySharing` will always return current user's Steam ID, even if the game is family-shared
  + `ISteamApps::BIsTimedTrial` will always return `false`
  + `ISteamApps::UserHasLicenseForApp` will always return `k_EUserHasLicenseResultHasLicense`
  + `ISteamUtils::GetAppID` will always return actual app ID regardless of which one was used to initialize Steam API

## Settings options

|Option|Type|Description|
|-|-|-|
|`app_id` (Required)|Number|(Real) Steam app ID of the game|
|`spoof_app_id`|Number|Steam app ID to impersonate, i.e. to use to initialize Steam API. If omitted, tek-game-runtime tries `app_id` first, and if it fails, falls back to 480 (Spacewar)|
|`dlc`|Dictionary of strings|List of "owned" DLC. Keys are DLC app IDs, values are display names, both can be found on SteamDB's DLC tab for the game|
|`installed_dlc`|Array of numbers|List of DLC app IDs that should be considered installed. If omitted and `dlc` is not empty, all IDs from `dlc` are copied|
|`tek_sc_path`|String|Path to [tek-steamclient](https://github.com/teknology-hub/tek-steamclient) DLL to load. If ommitted, `libtek-steamclient-1.dll` is assumed and [Windows' standard DLL search order](https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-search-order#standard-search-order-for-unpackaged-apps) is used. DLL presence is not mandatory, if it's missing, only the features that require it won't work|
|`auto_update_dlc`|Boolean|If `true`, tek-game-runtime will attempt to use tek-steamclient to get game's current DLC list at startup. DLC entries that are not listed in `dlc` yet will be added to there and `installed_dlc`. If settings are loaded from a file path, that file will be updated|

## Game-specific features
- [346110 (ARK: Survival Evolved)](https://github.com/teknology-hub/tek-game-runtime/blob/main/features/steam/346110.md)
- [2399830 (ARK: Survival Ascended)](https://github.com/teknology-hub/tek-game-runtime/blob/main/features/steam/2399830.md)
