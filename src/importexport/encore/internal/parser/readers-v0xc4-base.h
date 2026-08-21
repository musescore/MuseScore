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

// Base reader shared by v0xC2 and v0xC4: element block offset and instrument metadata reads.

#pragma once

#include "readers.h"

namespace mu::iex::enc {
// Shared base for v0xC2 and v0xC4 format readers.
// Provides the element block offset, instrument encoding probe, and the full
// instrument-metadata read (MIDI programs + key transpositions) used by v0xC4.
// v0xC2 overrides readInstrumentMeta to skip MIDI/key data.
struct EncFormatReader_V0xC4Base : EncFormatReader
{
    quint32 elemBlockOffset() const override { return 0x36; }
    bool probeInstrumentEncoding() const override { return true; }
    bool clustersChordsByXoffset() const override { return true; }

    bool readInstrumentMeta(std::vector<EncInstrument>& instruments, QDataStream& ds, const EncRoot& file) const override;
};
} // namespace mu::iex::enc
