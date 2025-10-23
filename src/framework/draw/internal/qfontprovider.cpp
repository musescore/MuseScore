/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#include <QRawFont>

#include "log.h"

using namespace muse;
using namespace muse::draw;

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
            return 1200; // Must be equal to mu::engraving::DPI
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

double QFontProvider::capHeight(const Font& f) const
{
    return QFontMetrics(f.toQFont(), &device).capHeight();
}

double QFontProvider::ascent(const Font& f) const
{
    return QFontMetricsF(f.toQFont(), &device).ascent();
}

double QFontProvider::descent(const Font& f) const
{
    return QFontMetricsF(f.toQFont(), &device).descent();
}

bool QFontProvider::inFont(const Font& f, char32_t ucs4) const
{
    // NOTE: QFontMetricsF::inFontUcs4 is unreliable for our use case because it uses Qt's fallback
    // system even if the flag noFontMerging is set, and returns true if the character
    // is found in any of the fallbacks. We need to use instead QRawFont, which represents the
    // *actual* font, not Qt's interpretation of the query. From QRawFont we can query the glyph index
    // of a character (zero if not in font) and the bounding rectangle of the actual glyph at that index.
    QRawFont qRawFont = QRawFont::fromFont(f.toQFont());
    int glyphIndex = qRawFont.glyphIndexesForString(QString(QChar::fromUcs4(ucs4)))[0];
    if (glyphIndex == 0) {
        return false;
    }

    //! @NOTE some symbols in fonts dont have glyph. For example U+ee80
    //! exists in Bravura.otf but doesn't have glyph
    //! so QFontMetricsF returns true in that case
    return qRawFont.boundingRect(glyphIndex).isValid();
}

double QFontProvider::horizontalAdvance(const Font& f, const String& string) const
{
    return QFontMetricsF(f.toQFont(), &device).horizontalAdvance(string);
}

double QFontProvider::horizontalAdvance(const Font& f, char32_t ucs4) const
{
    if (Char::requiresSurrogates(ucs4)) {
        return QFontMetricsF(f.toQFont(), &device).horizontalAdvance(String::fromUcs4(ucs4));
    }

    return QFontMetricsF(f.toQFont(), &device).horizontalAdvance(static_cast<char16_t>(ucs4));
}

RectF QFontProvider::boundingRect(const Font& f, const String& string) const
{
    return RectF::fromQRectF(QFontMetricsF(f.toQFont(), &device).boundingRect(string));
}

RectF QFontProvider::boundingRect(const Font& f, char32_t ucs4) const
{
    if (Char::requiresSurrogates(ucs4)) {
        return RectF::fromQRectF(QFontMetrics(f.toQFont(), &device).boundingRect(String::fromUcs4(ucs4)));
    }

    return RectF::fromQRectF(QFontMetricsF(f.toQFont(), &device).boundingRect(static_cast<char16_t>(ucs4)));
}

RectF QFontProvider::tightBoundingRect(const Font& f, const String& string) const
{
    auto boundingRect = QFontMetricsF(f.toQFont(), &device).tightBoundingRect(string);
    if (!boundingRect.isValid()) {
        // fix for https://github.com/musescore/MuseScore/issues/19503 - Qt can return garbage bounding rectangles that corrupt layout
        return RectF();
    }
    return RectF::fromQRectF(boundingRect);
}
