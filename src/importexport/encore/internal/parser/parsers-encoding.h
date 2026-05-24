/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// String decoding for the Encore parser: UTF-16 LE vs Latin-1 auto-detection and readers.

#pragma once

#include <QDataStream>
#include <QString>

namespace mu::iex::enc {
// Returns true if b0/b1 look like the first two bytes of a UTF-16 LE string:
// b0 is a printable ASCII byte and b1 is zero (BMP character in 0x0020..0x007E range).
bool probeUtf16LE(quint8 b0, quint8 b1);

// Reads a null-terminated string from ds, auto-detecting UTF-16 LE vs Latin-1 from the
// first two bytes.  `remaining` is decremented by every byte consumed (including the
// null terminator).  The caller is responsible for skipping any leftover bytes.
QString readEncodedStringRemaining(QDataStream& ds, int& remaining);

// Reads exactly `fixedLen` bytes from ds, auto-detecting UTF-16 LE vs Latin-1, and
// returns the decoded string up to the first null character.  All fixedLen bytes are
// consumed regardless of where the null terminator falls.
QString readEncodedStringFixed(QDataStream& ds, int fixedLen);
} // namespace mu::iex::enc
