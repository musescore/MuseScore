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

#include <QFont>
#include <QPaintDevice>
#include <QFontDatabase>
#include <QFontMetricsF>

#include "engraving/libmscore/mscore.h"
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
            return static_cast<int>(mu::engraving::DPI);
        default:
            return 1;
        }
    }
};

static FontPaintDevice device;

int QFontProvider::addSymbolFont(const String& family, const io::path_t& path)
{
    m_symbolsFonts[family] = path;
    return QFontDatabase::addApplicationFont(path.toQString());
}

int QFontProvider::addTextFont(const io::path_t& path)
{
    return QFontDatabase::addApplicationFont(path.toQString());
}

void QFontProvider::insertSubstitution(const String& familyName, const String& substituteName)
{
    QFont::insertSubstitution(familyName, substituteName);
}

double QFontProvider::lineSpacing(const Font& f) const
{
    return QFontMetricsF(f.toQFont(), &device).lineSpacing();
}

double QFontProvider::xHeight(const Font& f) const
{
    return QFontMetricsF(f.toQFont(), &device).xHeight();
}

double QFontProvider::height(const Font& f) const
{
    return QFontMetricsF(f.toQFont(), &device).height();
}

double QFontProvider::ascent(const Font& f) const
{
    return QFontMetricsF(f.toQFont(), &device).ascent();
}

double QFontProvider::descent(const Font& f) const
{
    return QFontMetricsF(f.toQFont(), &device).descent();
}

bool QFontProvider::inFont(const Font& f, Char ch) const
{
    return QFontMetricsF(f.toQFont(), &device).inFont(ch);
}

bool QFontProvider::inFontUcs4(const Font& f, char32_t ucs4) const
{
    if (!QFontMetricsF(f.toQFont(), &device).inFontUcs4(ucs4)) {
        return false;
    }

    //! @NOTE some symbols in fonts dont have glyph. For example U+ee80
    //! exists in Bravura.otf but doesn't have glyph
    //! so QFontMetricsF returns true in that case
    return symBBox(f, ucs4, 1.).isValid();
}

double QFontProvider::horizontalAdvance(const Font& f, const String& string) const
{
    return QFontMetricsF(f.toQFont(), &device).horizontalAdvance(string);
}

double QFontProvider::horizontalAdvance(const Font& f, const Char& ch) const
{
    return QFontMetricsF(f.toQFont(), &device).horizontalAdvance(ch);
}

RectF QFontProvider::boundingRect(const Font& f, const String& string) const
{
    return RectF::fromQRectF(QFontMetricsF(f.toQFont(), &device).boundingRect(string));
}

RectF QFontProvider::boundingRect(const Font& f, const Char& ch) const
{
    return RectF::fromQRectF(QFontMetricsF(f.toQFont(), &device).boundingRect(ch));
}

RectF QFontProvider::boundingRect(const Font& f, const RectF& r, int flags, const String& string) const
{
    return RectF::fromQRectF(QFontMetricsF(f.toQFont(), &device).boundingRect(r.toQRectF(), flags, string));
}

RectF QFontProvider::tightBoundingRect(const Font& f, const String& string) const
{
    return RectF::fromQRectF(QFontMetricsF(f.toQFont(), &device).tightBoundingRect(string));
}

// Score symbols
RectF QFontProvider::symBBox(const Font& f, char32_t ucs4, double dpi_f) const
{
    FontEngineFT* engine = symEngine(f);
    if (!engine) {
        return RectF();
    }

    RectF rect = RectF::fromQRectF(engine->bbox(ucs4, dpi_f));
    if (!rect.isValid()) {
        for (const auto& fontName : QFont::substitutes(f.family())) {
            Font subFont(f);
            subFont.setFamily(fontName, f.type());
            engine = symEngine(subFont);
            if (!engine) {
                continue;
            }

            rect = RectF::fromQRectF(engine->bbox(ucs4, dpi_f));
            if (rect.isValid()) {
                break;
            }
        }
    }

    return rect;
}

double QFontProvider::symAdvance(const Font& f, char32_t ucs4, double dpi_f) const
{
    FontEngineFT* engine = symEngine(f);
    if (!engine) {
        return 0.0;
    }

    double symAdvance = engine->advance(ucs4, dpi_f);
    if (RealIsNull(symAdvance)) {
        for (const auto& fontName : QFont::substitutes(f.family())) {
            Font subFont(f);
            subFont.setFamily(fontName, f.type());
            engine = symEngine(subFont);
            if (!engine) {
                continue;
            }

            symAdvance = engine->advance(ucs4, dpi_f);
            if (!RealIsNull(symAdvance)) {
                break;
            }
        }
    }

    return symAdvance;
}

FontEngineFT* QFontProvider::symEngine(const Font& f) const
{
    QString path = m_symbolsFonts.value(f.family()).toQString();
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
