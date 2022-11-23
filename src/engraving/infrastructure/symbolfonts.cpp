/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "symbolfonts.h"

#include "containers.h"

#include "log.h"

using namespace mu::engraving;

std::vector<SymbolFont> SymbolFonts::s_symbolFonts {};
SymbolFonts::Fallback SymbolFonts::s_fallback = {};

void SymbolFonts::addFont(const String& name, const String& family, const io::path_t& filePath)
{
    s_symbolFonts.push_back(SymbolFont(name, family, filePath));
}

const std::vector<SymbolFont>& SymbolFonts::scoreFonts()
{
    return s_symbolFonts;
}

SymbolFont* SymbolFonts::fontByName(const String& name)
{
    SymbolFont* font = nullptr;
    for (SymbolFont& f : s_symbolFonts) {
        if (f.name().toLower() == name.toLower()) { // case insensitive
            font = &f;
            break;
        }
    }

    if (!font) {
        return fallbackFont();
    }

    if (!font->m_loaded) {
        font->load();
    }

    return font;
}

void SymbolFonts::setFallbackFont(const String& name)
{
    s_fallback.name = name;

    size_t idx = mu::nidx;
    for (size_t i = 0; i < s_symbolFonts.size(); ++i) {
        if (s_symbolFonts.at(i).name() == name) {
            idx = i;
            break;
        }
    }

    if (idx != mu::nidx) {
        s_fallback.index = idx;
    } else {
        s_fallback.index = 0;
        LOGE() << "not found font with name: " << name;
    }
}

SymbolFont* SymbolFonts::fallbackFont(bool load)
{
    SymbolFont* font = &s_symbolFonts[s_fallback.index];

    if (load && !font->m_loaded) {
        font->load();
    }

    return font;
}

const char* SymbolFonts::fallbackTextFont()
{
    return "Bravura Text";
}
