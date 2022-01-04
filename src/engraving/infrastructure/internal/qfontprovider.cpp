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
#include "fontengineft.h"

using namespace mu;
using namespace mu::draw;

class FontPaintDevice : public QPaintDevice
{
public:
    QPaintEngine* paintEngine() const override
    {
        return nullptr;
    }

protected:
    int metric(PaintDeviceMetric m) const override
    {
        switch (m) {
        case QPaintDevice::PdmDpiY:
            return static_cast<int>(Ms::DPI);
        default:
            return 1;
        }
    }
};

static FontPaintDevice device;

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
    return QFontMetricsF(f.toQFont(), &device).lineSpacing();
}

qreal QFontProvider::xHeight(const Font& f) const
{
    return QFontMetricsF(f.toQFont(), &device).xHeight();
}

qreal QFontProvider::height(const Font& f) const
{
    return QFontMetricsF(f.toQFont(), &device).height();
}

qreal QFontProvider::ascent(const Font& f) const
{
    return QFontMetricsF(f.toQFont(), &device).ascent();
}

qreal QFontProvider::descent(const Font& f) const
{
    return QFontMetricsF(f.toQFont(), &device).descent();
}

bool QFontProvider::inFont(const Font& f, QChar ch) const
{
    return QFontMetricsF(f.toQFont(), &device).inFont(ch);
}

bool QFontProvider::inFontUcs4(const Font& f, uint ucs4) const
{
    return QFontMetricsF(f.toQFont(), &device).inFontUcs4(ucs4);
}

qreal QFontProvider::horizontalAdvance(const Font& f, const QString& string) const
{
    return QFontMetricsF(f.toQFont(), &device).horizontalAdvance(string);
}

qreal QFontProvider::horizontalAdvance(const Font& f, const QChar& ch) const
{
    return QFontMetricsF(f.toQFont(), &device).horizontalAdvance(ch);
}

RectF QFontProvider::boundingRect(const Font& f, const QString& string) const
{
    return RectF::fromQRectF(QFontMetricsF(f.toQFont(), &device).boundingRect(string));
}

RectF QFontProvider::boundingRect(const Font& f, const QChar& ch) const
{
    return RectF::fromQRectF(QFontMetricsF(f.toQFont(), &device).boundingRect(ch));
}

RectF QFontProvider::boundingRect(const Font& f, const RectF& r, int flags, const QString& string) const
{
    return RectF::fromQRectF(QFontMetricsF(f.toQFont(), &device).boundingRect(r.toQRectF(), flags, string));
}

RectF QFontProvider::tightBoundingRect(const Font& f, const QString& string) const
{
    return RectF::fromQRectF(QFontMetricsF(f.toQFont(), &device).tightBoundingRect(string));
}

// Score symbols
RectF QFontProvider::symBBox(const Font& f, uint ucs4, qreal dpi_f) const
{
    FontEngineFT* engine = symEngine(f);
    if (!engine) {
        return RectF();
    }
    return RectF::fromQRectF(engine->bbox(ucs4, dpi_f));
}

qreal QFontProvider::symAdvance(const Font& f, uint ucs4, qreal dpi_f) const
{
    FontEngineFT* engine = symEngine(f);
    if (!engine) {
        return 0.0;
    }
    return engine->advance(ucs4, dpi_f);
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
