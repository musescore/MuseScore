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

// Factories for the Encore 5.x (v0xC4) reader and its macOS SCO5 variant; classes live in the cpp.

#ifndef MU_IMPORTEXPORT_ENC_PARSER_READER_V0XC4_H
#define MU_IMPORTEXPORT_ENC_PARSER_READER_V0XC4_H

#include <memory>
#include "readers.h"

namespace mu::iex::enc {
// SCO5 is the macOS Encore 5 variant: same binary format as v0xC4 (SCOW) but with no importable
// document margins.
std::unique_ptr<EncFormatReader> makeFormatReader_V0xC4();
std::unique_ptr<EncFormatReader> makeFormatReader_SCO5();
} // namespace mu::iex::enc

#endif // MU_IMPORTEXPORT_ENC_PARSER_READER_V0XC4_H
