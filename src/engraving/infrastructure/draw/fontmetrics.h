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
#ifndef MU_DRAW_FONTMETRICS_H
#define MU_DRAW_FONTMETRICS_H

#include "font.h"

#include "types/string.h"
#include "modularity/ioc.h"
#include "ifontprovider.h"
#include "geometry.h"

namespace mu::draw {
class FontMetrics
{
    INJECT(draw, IFontProvider, fontProvider)
public:
    FontMetrics(const Font& font);

    qreal lineSpacing() const;
    qreal xHeight() const;
    qreal height() const;
    qreal ascent() const;
    qreal descent() const;

    qreal width(const String& string) const;
    qreal width(const Char& ch) const;

    qreal horizontalAdvance(const String& string) const;
    qreal horizontalAdvance(const Char& ch) const;

    RectF boundingRect(const String& string) const;
    RectF boundingRect(const Char& ch) const;
    RectF boundingRect(const RectF& r, int flags, const String& string) const;
    RectF tightBoundingRect(const String& string) const;
    RectF tightBoundingRect(const Char& ch) const;

    bool inFont(Char ch) const;
    bool inFontUcs4(uint ucs4) const;

    static qreal width(const Font& f, const String& string);
    static RectF boundingRect(const Font& f, const String& string);
    static RectF tightBoundingRect(const Font& f, const String& string);
    static qreal ascent(const Font& f);

private:
    Font m_font;
};
}

#endif // MU_DRAW_FONTMETRICS_H
