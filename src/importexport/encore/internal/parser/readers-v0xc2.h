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

// Factory for the Encore 3.x/4.x (v0xC2) format reader; the class itself lives in readers-v0xc2.cpp.

#pragma once

#include <memory>
#include "readers.h"

namespace mu::iex::enc {
// formatVersion is the file format version at header 0x28. Below 3.07 the file predates the Encore
// 4.0 element layout change and every element body field from +8 onward sits two bytes earlier.
std::unique_ptr<EncFormatReader> makeFormatReader_V0xC2(quint16 formatVersion);
} // namespace mu::iex::enc
