//===-- common.hpp - common declarations shared across all modules --------===//
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
/// Definitions of Clang's `_Nullable`, `_Nonnull`, and `_Null_unspecified`
///    attributes for other compilers, and inclusion of windows.h.
///
//===----------------------------------------------------------------------===//
#pragma once

#ifndef __clang__

#ifndef _Nullable
#define _Nullable
#endif // ndef _Nullable
#ifndef _Nonnull
#define _Nonnull
#endif // ndef _Nonnull
#ifndef _Null_unspecified
#define _Null_unspecified
#endif // ndef _Null_unspecified

#endif // ndef __clang__

// Include windows.h with API set reduced as much as possible
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0xA000
#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#include <windows.h>

namespace tek::game_runtime {
/// Display a message box with specified error message.
static inline void display_error(LPCWSTR msg) {
  MessageBoxW(nullptr, msg, L"TEK Game Runtime", MB_OK | MB_ICONERROR);
}
} // namespace tek::game_runtime
