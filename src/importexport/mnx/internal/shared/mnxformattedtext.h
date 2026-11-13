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
#pragma once

#include <variant>
#include <vector>

#ifdef MNXDOM_SYSTEM
#include <mnxdom/mnxdom.h>
#else
#include "mnxdom.h"
#endif

#include "global/types/string.h"
#include "engraving/types/symid.h"

namespace mu::engraving {
class TextBase;
}

namespace mu::iex::mnxio {
//---------------------------------------------------------
//   MnxTextStyle
//   Font styling for one chunk of MNX formatted text.
//
//   MNX does not define per-chunk styling yet, so this is empty. It exists so that the
//   chunk model, and every conversion that goes through it, already carries a style:
//   adjacent chunks merge only when their styles are equal, so adding fields here later
//   splits chunks at style changes without any change to the callers.
//---------------------------------------------------------

struct MnxTextStyle
{
    bool operator==(const MnxTextStyle&) const = default;
};

/// A run of SMuFL glyphs; never empty in a finished chunk list.
using MnxTextGlyphs = std::vector<engraving::SymId>;

//---------------------------------------------------------
//   MnxTextChunk
//   One run of MNX formatted text: either literal text (never trimmed) or a sequence of
//   SMuFL glyphs.
//---------------------------------------------------------

struct MnxTextChunk
{
    std::variant<muse::String, MnxTextGlyphs> content;
    MnxTextStyle style;
};

using MnxTextChunks = std::vector<MnxTextChunk>;

/// Appends a text run, merging it into the last chunk when kind and style match. Empty text is ignored.
void appendMnxTextChunk(MnxTextChunks& chunks, const muse::String& text, const MnxTextStyle& style = {});
/// Appends a glyph, merging it into the last chunk when kind and style match. noSym is ignored.
void appendMnxGlyphChunk(MnxTextChunks& chunks, engraving::SymId glyph, const MnxTextStyle& style = {});

/// Reads MNX formatted text. Unknown glyph names and empty chunks are skipped.
MnxTextChunks readMnxFormattedText(const mnx::FormattedText& formattedText);
/// Converts chunks to MuseScore's xml text, with glyphs as <sym> tags.
muse::String mnxTextChunksToXmlText(const MnxTextChunks& chunks);

/// Splits a MuseScore text item into chunks. Characters in the music font that do not
/// map to a SMuFL glyph are dropped.
MnxTextChunks textBaseToMnxTextChunks(const engraving::TextBase* textBase);
/// Appends chunks to an MNX formatted text array.
void writeMnxFormattedText(const MnxTextChunks& chunks, mnx::FormattedText formattedText);
} // namespace mu::iex::mnxio
