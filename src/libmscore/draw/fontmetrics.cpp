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

using namespace mu::draw;

FontMetrics::FontMetrics(const Font& font)
    : m_font(font)
{
}

qreal FontMetrics::lineSpacing() const
{
    return fontProvider()->lineSpacing(m_font);
}

qreal FontMetrics::xHeight() const
{
    return fontProvider()->xHeight(m_font);
}

qreal FontMetrics::height() const
{
    return fontProvider()->height(m_font);
}

qreal FontMetrics::ascent() const
{
    return fontProvider()->ascent(m_font);
}

qreal FontMetrics::descent() const
{
    return fontProvider()->descent(m_font);
}

qreal FontMetrics::width(const QString& string) const
{
    return boundingRect(string).width();
}

qreal FontMetrics::width(const QChar& ch) const
{
    return boundingRect(ch).width();
}

QRectF FontMetrics::boundingRect(const QString& string) const
{
    return fontProvider()->boundingRect(m_font, string);
}

QRectF FontMetrics::boundingRect(const QChar& ch) const
{
    return fontProvider()->boundingRect(m_font, ch);
}

QRectF FontMetrics::boundingRect(const QRectF& r, int flags, const QString& string) const
{
    return fontProvider()->boundingRect(m_font, r, flags, string);
}

QRectF FontMetrics::tightBoundingRect(const QString& string) const
{
    return fontProvider()->tightBoundingRect(m_font, string);
}

bool FontMetrics::inFont(QChar ch) const
{
    return fontProvider()->inFont(m_font, ch);
}

bool FontMetrics::inFontUcs4(uint ucs4) const
{
    return fontProvider()->inFontUcs4(m_font, ucs4);
}

// Static

qreal FontMetrics::width(const Font& f, const QString& string)
{
    return FontMetrics(f).width(string);
}

QRectF FontMetrics::boundingRect(const Font& f, const QString& string)
{
    return FontMetrics(f).boundingRect(string);
}

QRectF FontMetrics::tightBoundingRect(const Font& f, const QString& string)
{
    return FontMetrics(f).tightBoundingRect(string);
}

qreal FontMetrics::ascent(const Font& f)
{
    return FontMetrics(f).ascent();
}
