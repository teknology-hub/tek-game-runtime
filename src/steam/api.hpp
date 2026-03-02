//===-- api.hpp - Steam API wrapper interface declarations ----------------===//
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
/// Common declarations for Steam API wrapper interface.
///
//===----------------------------------------------------------------------===//
#pragma once
#include "common.hpp" // IWYU pragma: keep

#include <array>
#include <cstddef>
#include <cstdint>

namespace tek::game_runtime::steam {

/// Primitive C++ interface representation.
struct cpp_interface {
  /// Pointer to the virtual method table.
  void *const _Nonnull *_Nonnull vtable;
};

/// Steam interface wrapper descriptor.
///
/// @tparam max_num_methods
///    The maximum number of methods that the interface may contain, i.e. the
///    number of methods in the highest supported version.
template <std::size_t max_num_methods> struct wrapper_desc {
  /// Type that holds reference to a memory address, can read/write function
  ///    pointers with implicit casting, while doing nothing if it holds null
  ///    reference.
  class fptr {
    void *_Nonnull *_Nullable ptr;

  public:
    constexpr fptr() noexcept : ptr{} {}
    constexpr fptr(void *_Nonnull &ptr) noexcept : ptr{&ptr} {}
    template <typename T> constexpr operator T *_Nullable() const noexcept {
      return ptr ? reinterpret_cast<T *>(*ptr) : nullptr;
    }
    template <typename T>
    constexpr void operator=(T *_Nonnull func) const noexcept {
      if (ptr) {
        *ptr = reinterpret_cast<void *>(func);
      }
    }
  };

  /// Actual number of available interface methods, determined at runtime via
  ///    interface version detection.
  std::size_t num_methods;
  /// Pointer to the original virtual method table for the interface.
  void *const _Nonnull *_Nullable orig_vtable;
  /// Pointer to the interface instance.
  void *_Nullable iface;
  /// Wrapper's virtual method table
  std::array<void *_Nullable, max_num_methods> vtable;
  /// Table for mapping interface's corresponding m_ enum values to vtable
  ///    indices. Values of `-1` indicate that method is unavailable.
  std::array<int, max_num_methods> vm_idxs;

  [[gnu::visibility("internal")]]
  constexpr wrapper_desc() noexcept {
    vm_idxs.fill(-1);
  }

  constexpr const fptr operator[](int method) noexcept {
    const int idx{vm_idxs[method]};
    return idx < 0 ? fptr{} : fptr{vtable[idx]};
  }
};

/// Highest supported steam_api64.dll file version.
/// Current is `09.60.44.10` from Steamworks SDK v1.62
constexpr std::uint64_t max_supported_ver{0x0009003C002C000A};

/// Current detected steam_api64.dll file version.
inline std::uint64_t ver;
/// Current user's Steam ID.
inline std::uint64_t steam_id;

/// Install initial IAT hooks for steam_api64.dll functions.
[[gnu::visibility("internal")]]
void wrap_funcs();

} // namespace tek::game_runtime::steam
