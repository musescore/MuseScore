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

// Encore 5.x (v0xC4) reader and its SCO5 (macOS) variant: mostly base defaults, plus artic-byte
// clearing for undersized notes and uniform page margins for SCO5.

#include "readers-v0xc4.h"
#include "readers-v0xc4-base.h"

#include <QDataStream>

#include "elem.h"

namespace mu::iex::enc {
// Encore 5.x (v0xC4) format reader.
// All defaults inherited from EncFormatReader_V0xC4Base are correct for v0xC4.
// The only v0xC4-specific post-processing is zeroing articulation bytes when
// the element is smaller than 27 bytes (bytes lie beyond the element boundary).
struct EncFormatReader_V0xC4 : EncFormatReader_V0xC4Base
{
    const char* formatName() const override { return "v0xC4"; }

    bool postProcessElement(EncMeasureElem* elem, QDataStream& /*ds*/, qint64 /*rawElemStart*/) const override
    {
        EncNote* en = dynamic_cast<EncNote*>(elem);
        if (!en) {
            return false;
        }
        // Clear artic bytes that were read beyond the element boundary for size<27.
        if (en->size < 27) {
            en->articulationUp   = 0;
            en->articulationDown = 0;
        }
        return false;
    }
};

std::unique_ptr<EncFormatReader> makeFormatReader_V0xC4()
{
    return std::make_unique<EncFormatReader_V0xC4>();
}

// macOS Encore 5 (SCO5): identical binary format to v0xC4, but it stores no importable
// document margins (WINI holds only window state, the PREC plist only printer rects), so a
// clean uniform 0.25" margin is the better default than MuseScore's A4-tuned margins.
struct EncFormatReader_SCO5 final : EncFormatReader_V0xC4
{
    const char* formatName() const override { return "SCO5"; }
    bool usesUniformPageMargins() const override { return true; }
};

std::unique_ptr<EncFormatReader> makeFormatReader_SCO5()
{
    return std::make_unique<EncFormatReader_SCO5>();
}
} // namespace mu::iex::enc
