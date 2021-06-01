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
#include "qfontprovider.h"

#include <QFontMetricsF>

#include "libmscore/mscore.h"
#include "fontcompat.h"

using namespace mu::draw;

qreal QFontProvider::lineSpacing(const Font& f) const
{
    return QFontMetricsF(toQFont(f), Ms::MScore::paintDevice()).lineSpacing();
}

qreal QFontProvider::xHeight(const Font& f) const
{
    return QFontMetricsF(toQFont(f), Ms::MScore::paintDevice()).xHeight();
}

qreal QFontProvider::height(const Font& f) const
{
    return QFontMetricsF(toQFont(f), Ms::MScore::paintDevice()).height();
}

qreal QFontProvider::ascent(const Font& f) const
{
    return QFontMetricsF(toQFont(f), Ms::MScore::paintDevice()).ascent();
}

qreal QFontProvider::descent(const Font& f) const
{
    return QFontMetricsF(toQFont(f), Ms::MScore::paintDevice()).descent();
}

QRectF QFontProvider::boundingRect(const Font& f, const QString& string) const
{
    return QFontMetricsF(toQFont(f), Ms::MScore::paintDevice()).boundingRect(string);
}

QRectF QFontProvider::boundingRect(const Font& f, const QChar& ch) const
{
    return QFontMetricsF(toQFont(f), Ms::MScore::paintDevice()).boundingRect(ch);
}

QRectF QFontProvider::boundingRect(const Font& f, const QRectF& r, int flags, const QString& string) const
{
    return QFontMetricsF(toQFont(f), Ms::MScore::paintDevice()).boundingRect(r, flags, string);
}

QRectF QFontProvider::tightBoundingRect(const Font& f, const QString& string) const
{
    return QFontMetricsF(toQFont(f), Ms::MScore::paintDevice()).tightBoundingRect(string);
}

bool QFontProvider::inFont(const Font& f, QChar ch) const
{
    return QFontMetricsF(toQFont(f), Ms::MScore::paintDevice()).inFont(ch);
}

bool QFontProvider::inFontUcs4(const Font& f, uint ucs4) const
{
    return QFontMetricsF(toQFont(f), Ms::MScore::paintDevice()).inFontUcs4(ucs4);
}
