/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#ifndef MUSE_DRAW_FONTPROVIDER_H
#define MUSE_DRAW_FONTPROVIDER_H

#include "ifontprovider.h"

#include "global/modularity/ioc.h"
#include "ifontsdatabase.h"
#include "ifontsengine.h"

namespace muse::draw {
class FontProvider : public IFontProvider
{
    Inject<IFontsDatabase> fontsDatabase;
    Inject<IFontsEngine> fontsEngine;

public:
    FontProvider() = default;

    int addSymbolFont(const mu::String& family, const mu::io::path_t& path) override;
    int addTextFont(const mu::io::path_t& path) override;
    void insertSubstitution(const mu::String& familyName, const mu::String& to) override;

    double lineSpacing(const Font& f) const override;
    double xHeight(const Font& f) const override;
    double height(const Font& f) const override;
    double ascent(const Font& f) const override;
    double descent(const Font& f) const override;

    bool inFont(const Font& f, mu::Char ch) const override;
    bool inFontUcs4(const Font& f, char32_t ucs4) const override;

    // Text
    double horizontalAdvance(const Font& f, const mu::String& string) const override;
    double horizontalAdvance(const Font& f, const mu::Char& ch) const override;

    mu::RectF boundingRect(const Font& f, const mu::String& string) const override;
    mu::RectF boundingRect(const Font& f, const mu::Char& ch) const override;
    mu::RectF boundingRect(const Font& f, const mu::RectF& r, int flags, const mu::String& string) const override;
    mu::RectF tightBoundingRect(const Font& f, const mu::String& string) const override;

    // Score symbols
    mu::RectF symBBox(const Font& f, char32_t ucs4, double DPI_F) const override;
    double symAdvance(const Font& f, char32_t ucs4, double DPI_F) const override;
};
}

#endif // MUSE_DRAW_FONTPROVIDER_H
