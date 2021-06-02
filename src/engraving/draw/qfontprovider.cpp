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

#include <QFontDatabase>
#include <QFontMetricsF>

#include "libmscore/mscore.h"
#include "fontcompat.h"
#include "fontengineft.h"

using namespace mu::draw;

int QFontProvider::addApplicationFont(const QString& family, const QString& path)
{
    m_paths[family] = path;
    return QFontDatabase::addApplicationFont(path);
}

void QFontProvider::insertSubstitution(const QString& familyName, const QString& substituteName)
{
    QFont::insertSubstitution(familyName, substituteName);
}

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

bool QFontProvider::inFont(const Font& f, QChar ch) const
{
    return QFontMetricsF(toQFont(f), Ms::MScore::paintDevice()).inFont(ch);
}

bool QFontProvider::inFontUcs4(const Font& f, uint ucs4) const
{
    return QFontMetricsF(toQFont(f), Ms::MScore::paintDevice()).inFontUcs4(ucs4);
}

qreal QFontProvider::horizontalAdvance(const Font& f, const QString& string) const
{
    return QFontMetricsF(toQFont(f), Ms::MScore::paintDevice()).horizontalAdvance(string);
}

qreal QFontProvider::horizontalAdvance(const Font& f, const QChar& ch) const
{
    return QFontMetricsF(toQFont(f), Ms::MScore::paintDevice()).horizontalAdvance(ch);
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

// Score symbols
QRectF QFontProvider::symBBox(const Font& f, uint ucs4, qreal DPI_F) const
{
    FontEngineFT* engine = symEngine(f);
    if (!engine) {
        return QRectF();
    }
    return engine->bbox(ucs4, DPI_F);
}

qreal QFontProvider::symAdvance(const Font& f, uint ucs4, qreal DPI_F) const
{
    FontEngineFT* engine = symEngine(f);
    if (!engine) {
        return 0.0;
    }
    return engine->advance(ucs4, DPI_F);
}

FontEngineFT* QFontProvider::symEngine(const Font& f) const
{
    QString path = m_paths.value(f.family());
    if (path.isEmpty()) {
        return nullptr;
    }

    FontEngineFT* engine = m_symEngines.value(path, nullptr);
    if (!engine) {
        engine = new FontEngineFT();
        if (!engine->load(path)) {
            delete engine;
            return nullptr;
        }
        m_symEngines[path] = engine;
    }
    return engine;
}
