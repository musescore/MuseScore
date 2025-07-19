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
#include "fontmetrics.h"

using namespace muse;
using namespace muse::draw;

FontMetrics::FontMetrics(const Font& font)
    : m_font(font)
{
}

double FontMetrics::lineSpacing() const
{
    return fontProvider()->lineSpacing(m_font);
}

double FontMetrics::xHeight() const
{
    return fontProvider()->xHeight(m_font);
}

double FontMetrics::height() const
{
    return fontProvider()->height(m_font);
}

double FontMetrics::capHeight() const
{
    return fontProvider()->capHeight(m_font);
}

double FontMetrics::ascent() const
{
    return fontProvider()->ascent(m_font);
}

double FontMetrics::descent() const
{
    return fontProvider()->descent(m_font);
}

double FontMetrics::width(const String& string) const
{
    return horizontalAdvance(string);
}

double FontMetrics::width(const Char& ch) const
{
    return horizontalAdvance(ch);
}

double FontMetrics::horizontalAdvance(const String& string) const
{
    return fontProvider()->horizontalAdvance(m_font, string);
}

double FontMetrics::horizontalAdvance(const Char& ch) const
{
    return fontProvider()->horizontalAdvance(m_font, ch);
}

RectF FontMetrics::boundingRect(const String& string) const
{
    return fontProvider()->boundingRect(m_font, string);
}

RectF FontMetrics::boundingRect(const Char& ch) const
{
    return fontProvider()->boundingRect(m_font, ch);
}

RectF FontMetrics::boundingRect(const RectF& r, int flags, const String& string) const
{
    return fontProvider()->boundingRect(m_font, r, flags, string);
}

RectF FontMetrics::tightBoundingRect(const String& string) const
{
    return fontProvider()->tightBoundingRect(m_font, string);
}

RectF FontMetrics::tightBoundingRect(const Char& ch) const
{
    return fontProvider()->tightBoundingRect(m_font, ch);
}

bool FontMetrics::inFont(Char ch) const
{
    return fontProvider()->inFont(m_font, ch);
}

bool FontMetrics::inFontUcs4(char32_t ucs4) const
{
    return fontProvider()->inFontUcs4(m_font, ucs4);
}

// Static

double FontMetrics::width(const Font& f, const String& string)
{
    return FontMetrics(f).width(string);
}

RectF FontMetrics::boundingRect(const Font& f, const String& string)
{
    return FontMetrics(f).boundingRect(string);
}

RectF FontMetrics::tightBoundingRect(const Font& f, const String& string)
{
    return FontMetrics(f).tightBoundingRect(string);
}

double FontMetrics::ascent(const Font& f)
{
    return FontMetrics(f).ascent();
}

double FontMetrics::capHeight(const Font& f)
{
    return FontMetrics(f).capHeight();
}
