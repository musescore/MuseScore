/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include <string_view>

#include "finaletextconv.h"

#include "smufl_mapping.h"

#include "engraving/types/symnames.h"
#include "engraving/iengravingfont.h"

using namespace mu;
using namespace muse;
using namespace musx::dom;
using namespace mu::engraving;
namespace mu::iex::finale {

/// @todo
// MaestroTimes: import 194 as 2, 205 as 3, 202/203/193 as 2, 216 as 217 (no suitable smufl equivalent)"

static std::string calcGlyphName(char32_t c, const MusxInstance<FontInfo>& font)
{
    if (font->calcIsSMuFL()) { /// @todo use MuseScore engraver-fonts list for this? For `calcIsSMuFL` to return true, the font must be an installed SMuFL font on the user's machine
        if (const std::string_view* glyphName = smufl_mapping::getGlyphName(c)) {
            return std::string(*glyphName);
        }
    } else if (const smufl_mapping::LegacyGlyphInfo* legacyGlyphInfo = smufl_mapping::getLegacyGlyphInfo(font->getName(), c)) {
        if (legacyGlyphInfo->source == smufl_mapping::SmuflGlyphSource::Smufl) {
            return std::string(legacyGlyphInfo->name);
        }
    }
    return {};
}

engraving::SymId FinaleTextConv::symIdFromFinaleChar(char32_t c, const MusxInstance<FontInfo>& font, engraving::SymId def)
{
    const std::string glyphName = calcGlyphName(c, font);
    if (!glyphName.empty()) {
        return SymNames::symIdByName(glyphName, def);
    }
    return def;
}

std::optional<String> FinaleTextConv::symIdInsertFromFinaleChar(char32_t c, const MusxInstance<FontInfo>& font)
{
    const std::string glyphName = calcGlyphName(c, font);
    if (!glyphName.empty() && SymNames::symIdByName(glyphName) != SymId::noSym) {
        return u"<sym>" + String::fromStdString(glyphName) + u"</sym>";
    }
    return std::nullopt;
}

std::optional<String> FinaleTextConv::symIdInsertsFromStdString(const std::string& text, const MusxInstance<FontInfo>& font)
{
    std::u32string u32Text = String::fromStdString(text).toStdU32String();
    String result;
    for (char32_t c : u32Text) {
        if (std::optional<String> nextSymTag = FinaleTextConv::symIdInsertFromFinaleChar(c, font)) {
            result.append(nextSymTag.value());
        } else {
            return std::nullopt;
        }
    }
    return result;
}

std::optional<String> FinaleTextConv::smuflStringFromFinaleChar(char32_t c, const MusxInstance<FontInfo>& font)
{
    if (!font->calcIsSMuFL()) { /// @todo See note above about `calcIsSMuFL`
        if (const smufl_mapping::LegacyGlyphInfo* legacyGlyphInfo = smufl_mapping::getLegacyGlyphInfo(font->getName(), c)) {
            if (legacyGlyphInfo->source == smufl_mapping::SmuflGlyphSource::Smufl) { /// @todo do something with optional glyphs from Finale and/or Bravura?
                if (legacyGlyphInfo->codepoint) {
                    return String::fromUcs4(legacyGlyphInfo->codepoint.value());
                }
                if (const smufl_mapping::SmuflGlyphInfo* smuflGlyphInfo = getGlyphInfo(legacyGlyphInfo->name, legacyGlyphInfo->source)) {
                    return String::fromUcs4(smuflGlyphInfo->codepoint);
                }
            }
        }
        return std::nullopt;
    }
    return String::fromUcs4(c);
}

}
