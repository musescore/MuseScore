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
#ifndef MUSE_DRAW_FONTMETRICS_H
#define MUSE_DRAW_FONTMETRICS_H

#include "global/types/string.h"
#include "global/modularity/ioc.h"

#include "types/font.h"
#include "types/geometry.h"

#include "ifontprovider.h"

namespace muse::draw {
class FontMetrics
{
    GlobalInject<IFontProvider> fontProvider;

public:
    FontMetrics(const Font& font);

    double lineSpacing() const;
    double xHeight() const;
    double height() const;
    double capHeight() const;
    double ascent() const;
    double descent() const;

    double width(const String& string) const;
    double width(const Char& ch) const;

    double horizontalAdvance(const String& string) const;
    double horizontalAdvance(const Char& ch) const;

    RectF boundingRect(const String& string) const;
    RectF boundingRect(const Char& ch) const;
    RectF boundingRect(const RectF& r, int flags, const String& string) const;
    RectF tightBoundingRect(const String& string) const;
    RectF tightBoundingRect(const Char& ch) const;

    bool inFont(Char ch) const;
    bool inFontUcs4(char32_t ucs4) const;

    static double width(const Font& f, const String& string);
    static RectF boundingRect(const Font& f, const String& string);
    static RectF tightBoundingRect(const Font& f, const String& string);
    static double ascent(const Font& f);
    static double capHeight(const Font& f);

private:
    Font m_font;
};
}

#endif // MUSE_DRAW_FONTMETRICS_H
