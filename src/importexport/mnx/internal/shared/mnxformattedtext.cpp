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
#include "mnxformattedtext.h"

#include <string>
#include <type_traits>

#include "global/modularity/ioc.h"
#include "log.h"

#include "engraving/dom/score.h"
#include "engraving/dom/textbase.h"
#include "engraving/iengravingfont.h"
#include "engraving/iengravingfontsprovider.h"
#include "engraving/types/symnames.h"

using namespace mu::engraving;
using namespace muse;

namespace mu::iex::mnxio {
/// Returns the last chunk's content if a new run of type T with this style can merge into it.
template<typename T>
static T* mergeTarget(MnxTextChunks& chunks, const MnxTextStyle& style)
{
    if (chunks.empty() || !(chunks.back().style == style)) {
        return nullptr;
    }
    return std::get_if<T>(&chunks.back().content);
}

void appendMnxTextChunk(MnxTextChunks& chunks, const String& text, const MnxTextStyle& style)
{
    if (text.isEmpty()) {
        return;
    }
    if (String* target = mergeTarget<String>(chunks, style)) {
        *target += text;
    } else {
        chunks.push_back({ text, style });
    }
}

void appendMnxGlyphChunk(MnxTextChunks& chunks, SymId glyph, const MnxTextStyle& style)
{
    if (glyph == SymId::noSym) {
        return;
    }
    if (MnxTextGlyphs* target = mergeTarget<MnxTextGlyphs>(chunks, style)) {
        target->push_back(glyph);
    } else {
        chunks.push_back({ MnxTextGlyphs { glyph }, style });
    }
}

//---------------------------------------------------------
//   import side
//---------------------------------------------------------

MnxTextChunks readMnxFormattedText(const mnx::FormattedText& formattedText)
{
    MnxTextChunks chunks;
    for (const auto& mnxChunk : formattedText) {
        /// @todo Read chunk styling into MnxTextStyle once MNX defines it.
        const MnxTextStyle style;
        if (mnxChunk.type() == mnx::text::Text::ContentTypeValue) {
            appendMnxTextChunk(chunks, String::fromStdString(mnxChunk.get<mnx::text::Text>().text()), style);
        } else if (mnxChunk.type() == mnx::text::Smufl::ContentTypeValue) {
            // Schema 49 requires at least one glyph, but an empty array from an older file
            // simply contributes nothing.
            for (const auto& glyphName : mnxChunk.get<mnx::text::Smufl>().glyphs()) {
                const SymId symId = SymNames::symIdByName(String::fromStdString(glyphName));
                if (symId == SymId::noSym) {
                    LOGI() << "Skipping unknown SMuFL glyph \"" << glyphName << "\" in " << mnxChunk.pointer().to_string();
                    continue;
                }
                appendMnxGlyphChunk(chunks, symId, style);
            }
        } else {
            LOGW() << "Skipping formatted text chunk of unknown type \"" << mnxChunk.type() << "\" in "
                   << mnxChunk.pointer().to_string();
        }
    }
    return chunks;
}

String mnxTextChunksToXmlText(const MnxTextChunks& chunks)
{
    String result;
    for (const MnxTextChunk& chunk : chunks) {
        /// @todo Wrap the chunk in <b>, <i>, <u>, <s> and <font face/size> tags from
        /// chunk.style once MNX defines chunk styling.
        std::visit([&](const auto& content) {
            using T = std::decay_t<decltype(content)>;
            if constexpr (std::is_same_v<T, String>) {
                result += String::toXmlEscaped(content);
            } else if constexpr (std::is_same_v<T, MnxTextGlyphs>) {
                for (const SymId symId : content) {
                    result += u"<sym>" + String::fromAscii(SymNames::nameForSymId(symId).ascii()) + u"</sym>";
                }
            } else {
                static_assert(!sizeof(T), "Unhandled MnxTextChunk content in mnxTextChunksToXmlText");
            }
        }, chunk.content);
    }
    return result;
}

//---------------------------------------------------------
//   export side
//---------------------------------------------------------

/// Maps a character drawn in the music font to its SMuFL glyph, as TextBlock does when it
/// writes <sym> tags: the score's font first, then the fallback font.
static SymId symIdForMusicFontChar(const TextBase* textBase, char32_t code)
{
    SymId symId = SymId::noSym;
    if (textBase->score() && textBase->score()->engravingFont()) {
        symId = textBase->score()->engravingFont()->fromCode(code);
    }
    if (symId == SymId::noSym) {
        static const auto engravingFonts = muse::modularity::globalIoc()->resolve<IEngravingFontsProvider>("engraving");
        if (engravingFonts && engravingFonts->fallbackFont()) {
            symId = engravingFonts->fallbackFont()->fromCode(code);
        }
    }
    return symId;
}

MnxTextChunks textBaseToMnxTextChunks(const TextBase* textBase)
{
    MnxTextChunks chunks;
    IF_ASSERT_FAILED(textBase) {
        return chunks;
    }

    // Build the blocks into a local layout so the item's own layout data is untouched.
    // TextBase::fragmentList() is not used because it appends a line break to every
    // fragment of a line rather than once per line.
    TextBase::LayoutData ldata;
    textBase->createBlocks(&ldata);

    for (const TextBlock& block : ldata.blocks) {
        for (const TextFragment& fragment : block.fragments()) {
            /// @todo Map fragment.format (bold, italic, underline, strike, font family and
            /// size) to MnxTextStyle once MNX defines chunk styling. Until then it is dropped.
            const MnxTextStyle style;
            if (fragment.format.fontFamily() == u"ScoreText") {
                for (const char32_t code : fragment.text.toStdU32String()) {
                    const SymId symId = symIdForMusicFontChar(textBase, code);
                    if (symId == SymId::noSym) {
                        LOGI() << "Dropping music font character U+" << String::number(int(code), 16).toUpper()
                               << " with no SMuFL name from \"" << textBase->plainText() << "\".";
                        continue;
                    }
                    appendMnxGlyphChunk(chunks, symId, style);
                }
            } else {
                appendMnxTextChunk(chunks, fragment.text, style);
            }
        }
        if (block.eol()) {
            appendMnxTextChunk(chunks, u"\n");
        }
    }
    return chunks;
}

void writeMnxFormattedText(const MnxTextChunks& chunks, mnx::FormattedText formattedText)
{
    for (const MnxTextChunk& chunk : chunks) {
        /// @todo Write chunk.style once MNX defines chunk styling.
        std::visit([&](const auto& content) {
            using T = std::decay_t<decltype(content)>;
            if constexpr (std::is_same_v<T, String>) {
                if (!content.isEmpty()) {
                    formattedText.appendText(content.toStdString());
                }
            } else if constexpr (std::is_same_v<T, MnxTextGlyphs>) {
                // MNX requires at least one glyph in a SMuFL chunk.
                if (!content.empty()) {
                    std::vector<std::string> glyphNames;
                    glyphNames.reserve(content.size());
                    for (const SymId symId : content) {
                        glyphNames.emplace_back(SymNames::nameForSymId(symId).ascii());
                    }
                    formattedText.appendSmufl(glyphNames);
                }
            } else {
                static_assert(!sizeof(T), "Unhandled MnxTextChunk content in writeMnxFormattedText");
            }
        }, chunk.content);
    }
}
} // namespace mu::iex::mnxio
