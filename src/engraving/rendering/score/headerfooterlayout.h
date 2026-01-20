/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "layoutcontext.h"

namespace mu::engraving {
class Page;
class System;
class Text;
class TextBlock;
class TextFragment;
class CharFormat;

enum class TextStyleType : unsigned char;
}

namespace mu::engraving::rendering::score {
class HeaderFooterLayout
{
public:
    static void layoutHeaderFooter(const LayoutContext& ctx, Page* page);

    /// How much the header extends into the page (i.e., not in the margins)
    static double headerExtension(const LayoutContext& ctx, const Page* page);

    /// How much the footer extends into the page (i.e., not in the margins)
    static double footerExtension(const LayoutContext& ctx, const Page* page);

private:
    static void createUpdateHeaderText(const LayoutContext& ctx, Page* page, int area, const String& s);
    static void createUpdateFooterText(const LayoutContext& ctx, Page* page, int area, const String& s);

    /// Returns false if text is empty and should be removed
    static bool updateHeaderFooterText(const LayoutContext& ctx, Page* page, Text* text, const String& s);

    static void removeHeaderText(Page* page, int area);
    static void removeFooterText(Page* page, int area);

    static TextBlock replaceTextMacros(const LayoutContext& ctx, const Page* page, const TextBlock& tb);
    static CharFormat formatForMacro(const LayoutContext& ctx, const String& macro);
    static void appendFormattedString(std::list<TextFragment>& fragments, const String& s, const CharFormat& defaultFormat,
                                      const CharFormat& macroFormat);
};
}
