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

#ifndef MU_ENGRAVING_SYMBOLFONTS_H
#define MU_ENGRAVING_SYMBOLFONTS_H

#include <vector>

#include "types/string.h"

#include "symbolfont.h"

namespace mu::engraving {
class SymbolFonts
{
public:

    static void addFont(const String& name, const String& family, const io::path_t& filePath);
    static const std::vector<SymbolFont>& scoreFonts();
    static SymbolFont* fontByName(const String& name);

    static void setFallbackFont(const String& name);
    static SymbolFont* fallbackFont(bool load = true);
    static const char* fallbackTextFont();

private:

    struct Fallback {
        String name;
        size_t index = 0;
    };

    static Fallback s_fallback;
    static std::vector<SymbolFont> s_symbolFonts;
};
}

#endif // MU_ENGRAVING_SYMBOLFONTS_H
