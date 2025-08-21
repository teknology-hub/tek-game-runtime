//===-- settings.cpp - settings load/save implementation ------------------===//
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
/// Implementation of `settings::load` and `settings::save`.
///
//===----------------------------------------------------------------------===//
#include "settings.hpp"

#include "common.hpp" // IWYU pragma: keep
#include "game_cbs.hpp"

#include <array>
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <format>
#include <memory>
#include <ranges>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/reader.h>
#include <string>
#include <string_view>
#include <system_error>

namespace tek::game_runtime {

namespace {

/// Supported methods for loading the settings.
enum class load_type {
  /// Settings file path is received over the pipe, tek-game-runtime reads that
  ///    file.
  file_path,
  /// Settings JSON content is received over the pipe directly.
  pipe
};

/// The first message received over the pipe.
struct pipe_header {
  /// Settings loading method.
  load_type type;
  /// Size of the remaining data to read from the pipe, in bytes. When @ref type
  ///    is `load_type::file_path`, the data is path to the file, or
  ///    "tek-gr-settings.json" in current directory if the size is zero. When
  ///    @ref type is `load_type::pipe`, the data is actual settings JSON
  ///    content.
  std::uint32_t size;
};

/// RAII wrapper for file handles.
class [[gnu::visibility("internal")]] unique_file {
  HANDLE value;

public:
  constexpr unique_file(HANDLE handle) noexcept : value{handle} {}
  ~unique_file() noexcept { close(); }
  constexpr operator bool() const noexcept {
    return value != INVALID_HANDLE_VALUE;
  }
  constexpr operator HANDLE() const noexcept { return value; }
  void close() noexcept {
    if (value != INVALID_HANDLE_VALUE) {
      CloseHandle(value);
      value = INVALID_HANDLE_VALUE;
    }
  }
};

/// Path to the settings file, if file-based settings loading is used.
std::wstring file_path;

} // namespace

bool settings::load() {
  unique_file pipe{CreateFileW(L"\\\\.\\pipe\\tek-game-runtime", GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                               OPEN_EXISTING, 0, nullptr)};
  if (!pipe) {
    display_error(
        std::format(L"Failed to connect to the pipe, got error code {}",
                    GetLastError())
            .data());
    return false;
  }
  pipe_header hdr;
  if (DWORD bytes_read;
      !ReadFile(pipe, &hdr, sizeof hdr, &bytes_read, nullptr)) {
    display_error(
        std::format(L"Failed to read header from the pipe, got error code {}",
                    GetLastError())
            .data());
    return false;
  }
  std::string data(hdr.size, '\0');
  for (auto next{data.data()}; hdr.size;) {
    DWORD bytes_read;
    if (!ReadFile(pipe, next, hdr.size, &bytes_read, nullptr)) {
      display_error(
          std::format(L"Failed to read data from the pipe, got error code {}",
                      GetLastError())
              .data());
      return false;
    }
    next += bytes_read;
    hdr.size -= bytes_read;
  }
  pipe.close();
  rapidjson::Document doc;
  switch (hdr.type) {
  case load_type::file_path: {
    if (data.empty()) {
      file_path = L"tek-gr-settings.json";
    } else {
      file_path.resize(MultiByteToWideChar(CP_UTF8, 0, data.data(), data.size(),
                                           nullptr, 0));
      MultiByteToWideChar(CP_UTF8, 0, data.data(), data.size(),
                          file_path.data(), file_path.length());
    }
    const std::unique_ptr<std::FILE, decltype(&std::fclose)> file{
        _wfopen(file_path.data(), L"rbNS"), std::fclose};
    if (!file) {
      display_error(L"Failed to load settings: unable to open settings file");
      return false;
    }
    std::array<char, 2048> read_buf;
    rapidjson::FileReadStream stream{file.get(), read_buf.data(),
                                     read_buf.size()};
    doc.ParseStream<rapidjson::kParseStopWhenDoneFlag>(stream);
    break;
  }
  case load_type::pipe:
    doc.ParseInsitu<rapidjson::kParseStopWhenDoneFlag>(data.data());
    break;
  default:
    display_error(L"Unknown load type specified in the header");
    return false;
  } // switch (hdr.type)
  if (doc.HasParseError() || !doc.IsObject()) {
    display_error(L"Failed to load settings: JSON parsing error");
    return false;
  }
  const auto m_store{doc.FindMember("store")};
  if (m_store == doc.MemberEnd() || !m_store->value.IsString()) {
    display_error(L"Failed to load settings: \"store\" field not found or is "
                  L"not a string");
    return false;
  }
  std::string_view view{m_store->value.GetString(),
                        m_store->value.GetStringLength()};
  if (view == "steam") {
    store = store_type::steam;
    // Load Steam-specific options
    const auto app_id{doc.FindMember("app_id")};
    if (app_id == doc.MemberEnd() || !app_id->value.IsUint()) {
      display_error(L"Failed to load settings: \"app_id\" field not found or "
                    L"is not a number");
      return false;
    }
    steam = std::make_unique<steam_options>();
    steam->app_id = app_id->value.GetUint();
    const auto spoof_app_id{doc.FindMember("spoof_app_id")};
    if (spoof_app_id != doc.MemberEnd() && spoof_app_id->value.IsUint()) {
      steam->spoof_app_id = spoof_app_id->value.GetUint();
    } else {
      steam->spoof_app_id = 0;
    }
    const auto dlc{doc.FindMember("dlc")};
    if (dlc != doc.MemberEnd() && dlc->value.IsObject()) {
      steam->dlc.reserve(dlc->value.MemberCount());
      for (const auto &dlc_item : dlc->value.GetObject()) {
        std::uint32_t id;
        view = {dlc_item.name.GetString(), dlc_item.name.GetStringLength()};
        if (std::from_chars(view.begin(), view.end(), id).ec != std::errc{} ||
            !dlc_item.value.IsString()) {
          continue;
        }
        steam->dlc.emplace_back(id,
                                std::string{dlc_item.value.GetString(),
                                            dlc_item.value.GetStringLength()});
      }
    }
    const auto installed_dlc{doc.FindMember("installed_dlc")};
    if (installed_dlc != doc.MemberEnd() && installed_dlc->value.IsArray()) {
      for (const auto &elem : installed_dlc->value.GetArray()) {
        if (elem.IsUint()) {
          steam->installed_dlc.emplace(elem.GetUint());
        }
      }
    } else {
      for (auto id : steam->dlc | std::views::keys) {
        steam->installed_dlc.emplace(id);
      }
    }
    const auto tek_sc_path{doc.FindMember("tek_sc_path")};
    if (tek_sc_path != doc.MemberEnd() && tek_sc_path->value.IsString()) {
      steam->tek_sc_path = {tek_sc_path->value.GetString(),
                            tek_sc_path->value.GetStringLength()};
    }
    const auto auto_update_dlc{doc.FindMember("auto_update_dlc")};
    if (auto_update_dlc != doc.MemberEnd() && auto_update_dlc->value.IsBool()) {
      steam->auto_update_dlc = auto_update_dlc->value.GetBool();
    }
  } else { // if (view == "steam")
    display_error(
        std::format(L"Failed to load settings: unknown store \"{}\"", view)
            .data());
    return false;
  } // if (view == "steam") else
  // Load game-specific options
  const auto cb{get_settings_load_cb()};
  if (cb) {
    cb(doc);
  }
  return true;
}

void settings::save() {
  if (file_path.empty()) {
    return;
  }
  const std::unique_ptr<std::FILE, decltype(&std::fclose)> file{
      _wfopen(file_path.data(), L"wbNS"), std::fclose};
  if (!file) {
    return;
  }
  std::array<char, 2048> write_buf;
  rapidjson::FileWriteStream stream{file.get(), write_buf.data(),
                                    write_buf.size()};
  rapidjson::PrettyWriter writer{stream};
  writer.SetIndent(' ', 2);
  writer.StartObject();
  std::string_view str{"store"};
  writer.Key(str.data(), str.length());
  switch (store) {
  case store_type::steam:
    str = "steam";
    break;
  default:
    str = "unknown";
  }
  writer.String(str.data(), str.length());
  switch (store) {
  case store_type::steam: {
    str = "app_id";
    writer.Key(str.data(), str.length());
    writer.Uint(steam->app_id);
    if (steam->spoof_app_id && steam->spoof_app_id != steam->app_id) {
      str = "spoof_app_id";
      writer.Key(str.data(), str.length());
      writer.Uint(steam->spoof_app_id);
    }
    if (!steam->dlc.empty()) {
      str = "dlc";
      writer.Key(str.data(), str.length());
      writer.StartObject();
      for (const auto &[id, name] : steam->dlc) {
        std::array<char, 10> buf;
        const auto res = std::to_chars(buf.begin(), buf.end(), id);
        if (res.ec != std::errc{}) {
          continue;
        }
        str = {buf.begin(), res.ptr};
        writer.Key(str.data(), str.length());
        writer.String(name.data(), name.length());
      }
      writer.EndObject();
    }
    if (!steam->installed_dlc.empty()) {
      str = "installed_dlc";
      writer.Key(str.data(), str.length());
      writer.StartArray();
      for (const auto id : steam->installed_dlc) {
        writer.Uint(id);
      }
      writer.EndArray();
    }
    if (!steam->tek_sc_path.empty()) {
      str = "tek_sc_path";
      writer.Key(str.data(), str.length());
      writer.String(steam->tek_sc_path.data(), steam->tek_sc_path.length());
    }
    str = "auto_update_dlc";
    writer.Key(str.data(), str.length());
    writer.Bool(steam->auto_update_dlc);
    break;
  } // case store_type::steam
  } // switch (store)
  // Save game-specific options
  const auto cb{get_settings_save_cb()};
  if (cb) {
    cb(writer);
  }
  writer.EndObject();
}

} // namespace tek::game_runtime
