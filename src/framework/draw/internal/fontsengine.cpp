/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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
#include "fontsengine.h"

#include "global/io/fileinfo.h"

#include "ifontface.h"
#ifdef MUSE_MODULE_DRAW_USE_FONTFACE_FT
#include "fontfaceft.h"
#endif
#ifdef MUSE_MODULE_DRAW_USE_FONTFACE_XT
#include "fontfacext.h"
#endif
#include "fontfacedu.h"

#include "log.h"

using namespace muse;
using namespace muse::draw;

static const double DEFAULT_PIXEL_SIZE = 100.0;
static const double LOADED_PIXEL_SIZE = 200.0;
static const double FONT_METRICS_DPI = 1200.0;
static constexpr double PPI = 72.0;

static int fontMetricsPixelSize(const Font& f)
{
    if (f.pixelSize() > 0) {
        return f.pixelSize();
    }

    double pixelSize = f.pointSizeF() * FONT_METRICS_DPI / PPI;
    return static_cast<int>(std::round(pixelSize));
}

static FaceKey faceKeyForMetricsFont(const Font& f)
{
    return FaceKey(dataKeyForFont(f), f.type(), fontMetricsPixelSize(f));
}

static int loadedPixelSizeForFontPath(const io::path_t& path, int requirePixelSize)
{
    // FTX fonts store glyph metrics and images baked at LOADED_PIXEL_SIZE.
    if (io::FileInfo::suffix(path).toLower() == u"ftx") {
        return static_cast<int>(LOADED_PIXEL_SIZE);
    }

    return requirePixelSize;
}

static inline RectF fromFBBox(const FBBox& bb, double scale)
{
    return RectF(from_f26d6(bb.left()) * scale, from_f26d6(bb.top()) * scale,
                 from_f26d6(bb.width()) * scale, from_f26d6(bb.height()) * scale);
}

static inline RectF scaleRect(const RectF& r, double scale)
{
    return RectF(r.x() * scale, r.y() * scale, r.width() * scale, r.height() * scale);
}

static const IFontFace* findSubtitutionFont(char32_t ch, const std::vector<IFontFace*>& subtitutionFaces)
{
    const IFontFace* founded = nullptr;
    for (const IFontFace* subFace : subtitutionFaces) {
        if (subFace->glyphIndex(ch) != 0) {
            founded = subFace;
        }
    }
    return founded;
}

static bool fitsQtGlyphCacheMetrics(const FBBox& bbox, f26dot6_t advance)
{
    return (bbox.width() >> 6) <= 0xFF && (bbox.height() >> 6) <= 0xFF && advance <= 0x7FFF;
}

static bool isZeroAdvanceChar(char32_t ch)
{
    return ch == U'\n' || ch == U'\r';
}

static bool isZeroAdvanceText(const char32_t* text, int length)
{
    for (int i = 0; i < length; ++i) {
        if (!isZeroAdvanceChar(text[i])) {
            return false;
        }
    }
    return true;
}

bool FontsEngine::RequireFace::isSymbolMode() const
{
    return face ? face->isSymbolMode() : false;
}

double FontsEngine::RequireFace::pixelScale() const
{
    return pixelScaleFor(face);
}

double FontsEngine::RequireFace::pixelScaleFor(const IFontFace* loadedFace) const
{
    if (!loadedFace) {
        return 0.0;
    }

    return static_cast<double>(requireKey.pixelSize) / static_cast<double>(loadedFace->key().pixelSize);
}

FontsEngine::~FontsEngine()
{
    clearLoadedFaces();
}

void FontsEngine::init()
{
    m_renderCache.init();
}

void FontsEngine::setRenderCacheDirPath(const io::path_t& path, const std::string& revision)
{
    m_renderCache.setCacheDirPath(path, revision);
}

void FontsEngine::clearLoadedFaces()
{
    for (RequireFace* f : m_requiredFaces) {
        delete f;
    }
    m_requiredFaces.clear();

    for (IFontFace* f : m_loadedFaces) {
        delete f;
    }
    m_loadedFaces.clear();
}

double FontsEngine::lineSpacing(const Font& f) const
{
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return 0.0;
    }

    return from_f26d6(rf->face->leading() + rf->face->ascent() + rf->face->descent()) * rf->pixelScale();
}

double FontsEngine::xHeight(const Font& f) const
{
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return 0.0;
    }

    return from_f26d6(rf->face->xHeight()) * rf->pixelScale();
}

double FontsEngine::height(const Font& f) const
{
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return 0.0;
    }

    return from_f26d6(rf->face->ascent() + rf->face->descent()) * rf->pixelScale();
}

double FontsEngine::capHeight(const Font& f) const
{
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return 0.0;
    }

    return from_f26d6(rf->face->capHeight()) * rf->pixelScale();
}

double FontsEngine::ascent(const Font& f) const
{
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return 0.0;
    }

    return from_f26d6(rf->face->ascent()) * rf->pixelScale();
}

double FontsEngine::descent(const Font& f) const
{
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return 0.0;
    }

    return from_f26d6(rf->face->descent()) * rf->pixelScale();
}

bool FontsEngine::inFont(const Font& f, char32_t ucs4) const
{
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return false;
    }

    return rf->face->glyphIndex(ucs4) != 0;
}

double FontsEngine::horizontalAdvance(const Font& f, const char32_t& ch) const
{
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return 0.0;
    }

    glyph_idx_t glyphIdx = rf->face->glyphIndex(ch);
    return from_f26d6(rf->face->glyphAdvance(glyphIdx)) * rf->pixelScale();
}

double FontsEngine::horizontalAdvance(const Font& f, const std::u32string& text) const
{
    if (text.empty()) {
        return 0.0;
    }

    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return 0.0;
    }

    double advance = 0.0;

    TextBlock textBlock;
    textBlock.text = &text[0];
    textBlock.lenght = static_cast<int>(text.size());

    std::vector<FontFaceTextBlock> fontFaceBlocks = splitTextByFontFaces(rf, textBlock);
    for (const FontFaceTextBlock& ffBlock : fontFaceBlocks) {
        const IFontFace* fontFace = ffBlock.face;
        if (!fontFace) {
            if (isZeroAdvanceText(ffBlock.text.text, ffBlock.text.lenght)) {
                continue;
            }

            fontFace = rf->face;
        }

        const double pixelScale = rf->pixelScaleFor(fontFace);
        std::vector<GlyphPos> glyphs = fontFace->glyphs(ffBlock.text.text, ffBlock.text.lenght);
        for (const GlyphPos& g : glyphs) {
            advance += from_f26d6(g.x_advance) * pixelScale;
        }
    }

    return advance;
}

RectF FontsEngine::boundingRect(const Font& f, const char32_t& ch) const
{
    RequireFace* rf = fontFace(f, f.type() == Font::Type::MusicSymbol);
    IF_ASSERT_FAILED(rf && rf->face) {
        return RectF();
    }

    glyph_idx_t glyphIdx = rf->face->glyphIndex(ch);
    return fromFBBox(rf->face->glyphBbox(glyphIdx), rf->pixelScale());
}

RectF FontsEngine::boundingRect(const Font& f, const std::u32string& text) const
{
    if (text.empty()) {
        return RectF();
    }

    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return RectF();
    }

    FBBox rect; // f26dot6_t units
    rect.setY(-rf->face->ascent());
    rect.setHeight(rf->face->ascent() + rf->face->descent());

    f26dot6_t xOffset = 0;
    f26dot6_t xmax = 0;
    f26dot6_t ymax = 0;
    bool hasGlyph = false;

    TextBlock textBlock;
    textBlock.text = &text[0];
    textBlock.lenght = static_cast<int>(text.size());

    std::vector<FontFaceTextBlock> fontFaceBlocks = splitTextByFontFaces(rf, textBlock);
    for (const FontFaceTextBlock& ffBlock : fontFaceBlocks) {
        const IFontFace* fontFace = ffBlock.face;
        if (!fontFace) {
            if (isZeroAdvanceText(ffBlock.text.text, ffBlock.text.lenght)) {
                continue;
            }

            fontFace = rf->face;
        }

        std::vector<GlyphPos> glyphs = fontFace->glyphs(ffBlock.text.text, ffBlock.text.lenght);

        for (const GlyphPos& g : glyphs) {
            FBBox bbox = fontFace->glyphBbox(g.idx);
            f26dot6_t x = xOffset + bbox.x();
            f26dot6_t y = bbox.y();

            bool useCachedGlyphMetrics = fitsQtGlyphCacheMetrics(bbox, fontFace->glyphAdvance(g.idx));
            f26dot6_t glyphRight = useCachedGlyphMetrics ? ((x + 63) & -64) + bbox.width() : x + bbox.width();
            f26dot6_t glyphBottom = useCachedGlyphMetrics ? ((y + 63) & -64) + bbox.height() : y + bbox.height();

            if (!hasGlyph) {
                rect.setX(x);
                xmax = glyphRight;
                ymax = glyphBottom;
                hasGlyph = true;
            } else {
                rect.setX(std::min(rect.x(), x));
                xmax = std::max(xmax, glyphRight);
                ymax = std::max(ymax, glyphBottom);
            }

            rect.setY(std::min(rect.y(), y));
            xOffset += g.x_advance;
        }
    }

    if (!hasGlyph) {
        rect.setX(0);
        rect.setWidth(0);
        return fromFBBox(rect, rf->pixelScale());
    }

    rect.setWidth(xmax - rect.x());
    rect.setHeight(std::max(rect.height(), ymax - rect.y()));

    return fromFBBox(rect, rf->pixelScale());
}

RectF FontsEngine::tightBoundingRect(const Font& f, const std::u32string& text) const
{
    if (text.empty()) {
        return RectF();
    }

    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return RectF();
    }

    FBBox rect; // f26dot6_t units
    f26dot6_t xOffset = 0;
    f26dot6_t xmax = 0;
    f26dot6_t ymax = 0;
    bool hasGlyph = false;

    TextBlock textBlock;
    textBlock.text = &text[0];
    textBlock.lenght = static_cast<int>(text.size());

    std::vector<FontFaceTextBlock> fontFaceBlocks = splitTextByFontFaces(rf, textBlock);
    for (const FontFaceTextBlock& ffBlock : fontFaceBlocks) {
        const IFontFace* fontFace = ffBlock.face;
        if (!fontFace) {
            if (isZeroAdvanceText(ffBlock.text.text, ffBlock.text.lenght)) {
                continue;
            }

            fontFace = rf->face;
        }

        std::vector<GlyphPos> glyphs = fontFace->glyphs(ffBlock.text.text, ffBlock.text.lenght);

        for (const GlyphPos& g : glyphs) {
            FBBox bbox = fontFace->glyphBbox(g.idx);
            f26dot6_t x = xOffset + bbox.x();
            f26dot6_t y = bbox.y();

            f26dot6_t glyphRight = ((x + 63) & -64) + bbox.width();
            f26dot6_t glyphBottom = ((y + 63) & -64) + bbox.height();

            if (!hasGlyph) {
                rect.setX(x);
                rect.setY(y);
                xmax = glyphRight;
                ymax = glyphBottom;
                hasGlyph = true;
            } else {
                rect.setX(std::min(rect.x(), x));
                rect.setY(std::min(rect.y(), y));
                xmax = std::max(xmax, glyphRight);
                ymax = std::max(ymax, glyphBottom);
            }

            xOffset += g.x_advance;
        }
    }

    if (!hasGlyph) {
        return RectF();
    }

    rect.setWidth(xmax - rect.x());
    rect.setHeight(ymax - rect.y());

    return fromFBBox(rect, rf->pixelScale());
}

std::vector<GlyphImage> FontsEngine::render(const Font& f, const std::u32string& text) const
{
    //! NOTE for rendering, all fonts, including symbols fonts, are processed as text
    RequireFace* rf = fontFace(f);
    IF_ASSERT_FAILED(rf && rf->face) {
        return std::vector<GlyphImage>();
    }

    static const std::set<glyph_idx_t> NOT_RENDER_GLYPHS = {
        3 // space
    };

    std::vector<GlyphImage> images;

    if (text.empty()) {
        return images;
    }

#ifndef MUSE_MODULE_DRAW_USE_QTTEXTDRAW
    double glyphLeft = 0;

    TextBlock textBlock;
    textBlock.text = &text[0];
    textBlock.lenght = static_cast<int>(text.size());

    std::vector<FontFaceTextBlock> fontFaceBlocks = splitTextByFontFaces(rf, textBlock);
    for (const FontFaceTextBlock& ffBlock : fontFaceBlocks) {
        const IFontFace* layoutFace = ffBlock.face;
        if (!layoutFace) {
            if (isZeroAdvanceText(ffBlock.text.text, ffBlock.text.lenght)) {
                continue;
            }

            layoutFace = rf->face;
        }

        const IFontFace* sdfFace = sdfFontFaceFor(layoutFace);
        if (!sdfFace) {
            //! NOTE Fall back to the layout face itself rather than dropping this run's glyphs
            sdfFace = layoutFace;
        }

        double layoutScale = rf->pixelScaleFor(layoutFace);
        double sdfToLayoutScale = rf->pixelScaleFor(sdfFace);
        std::vector<GlyphPos> glyphs = layoutFace->glyphs(ffBlock.text.text, ffBlock.text.lenght);

        for (const GlyphPos& g : glyphs) {
            if (NOT_RENDER_GLYPHS.find(g.idx) == NOT_RENDER_GLYPHS.end()) {
                GlyphImage image = m_renderCache.load(sdfFace->key(), g.idx);
                if (image.isNull()) {
                    image = sdfFace->glyphImage(g.idx);
                    m_renderCache.store(sdfFace->key(), g.idx, image);
                }

                image.rect = scaleRect(image.rect, sdfToLayoutScale);
                image.rect.translate(glyphLeft, 0);

                images.push_back(std::move(image));
            }

            glyphLeft += from_f26d6(g.x_advance) * layoutScale;
        }
    }
#endif

    return images;
}

void FontsEngine::setFontFaceFactory(const FontFaceFactory& f)
{
    m_fontFaceFactory = f;
}

IFontFace* FontsEngine::createFontFace(const io::path_t& path) const
{
    if (m_fontFaceFactory) {
        return m_fontFaceFactory(path);
    }

    IFontFace* origin = nullptr;
    if (io::FileInfo::suffix(path).toLower() == u"ftx") {
#ifdef MUSE_MODULE_DRAW_USE_FONTFACE_XT
        origin = new FontFaceXT();
#else
        LOGE() << "XT font face backend is disabled: " << path;
        return nullptr;
#endif
    } else {
#ifdef MUSE_MODULE_DRAW_USE_FONTFACE_FT
        origin = new FontFaceFT();
#else
        LOGE() << "FreeType font face backend is disabled: " << path;
        return nullptr;
#endif
    }

    return new FontFaceDU(origin);
}

IFontFace* FontsEngine::fontFace(const FontDataKey& dataKey, Font::Type type, int pixelSize, bool isSymbolMode) const
{
    io::path_t fontPath = fontsDatabase()->fontPath(dataKey, type);
    if (fontPath.empty()) {
        LOGE() << "font path is empty: " << dataKey.family().id();
        return nullptr;
    }

    const int loadedPixelSize = loadedPixelSizeForFontPath(fontPath, pixelSize);

    for (IFontFace* face : m_loadedFaces) {
        if (face->key().dataKey == dataKey && face->key().type == type && face->key().pixelSize == loadedPixelSize
            && face->isSymbolMode() == isSymbolMode) {
            return face;
        }
    }

    FaceKey loadedKey;
    loadedKey.dataKey = dataKey;
    loadedKey.type = type;
    loadedKey.pixelSize = loadedPixelSize;

    IFontFace* face = createFontFace(fontPath);
    IF_ASSERT_FAILED(face) {
        return nullptr;
    }

    if (!face->load(loadedKey, fontPath, isSymbolMode)) {
        LOGE() << "failed load font face: " << fontPath;
        delete face;
        return nullptr;
    }

    m_loadedFaces.push_back(face);
    return face;
}

IFontFace* FontsEngine::sdfFontFaceFor(const IFontFace* layoutFace) const
{
    if (!layoutFace) {
        return nullptr;
    }

    return fontFace(layoutFace->key().dataKey, layoutFace->key().type, static_cast<int>(LOADED_PIXEL_SIZE), layoutFace->isSymbolMode());
}

FontsEngine::RequireFace* FontsEngine::fontFace(const Font& f, bool isSymbolMode) const
{
    //! NOTE This font is required
    FaceKey requireKey = faceKeyForMetricsFont(f);

    //! NOTE If pixelSize is not set, then specify the default
    //! (this is the default pixelSize in Qt)
    if (!(requireKey.pixelSize > 0)) {
        requireKey.pixelSize = DEFAULT_PIXEL_SIZE;
    }

    //! NOTE At the moment, in some cases, the type may not be specified,
    //! so set as Text
    if (requireKey.type == Font::Type::Undefined || requireKey.type == Font::Type::Unknown) {
        requireKey.type = Font::Type::Text;
    }

    //! NOTE We are looking for the require font we need among the previously loaded ones
    for (RequireFace* face : m_requiredFaces) {
        if (face->requireKey == requireKey && face->isSymbolMode() == isSymbolMode) {
            return face;
        }
    }

    //! Let's find out which real font will be used
    //! (for example, if there is no required one)
    FontDataKey actualDataKey = fontsDatabase()->actualFont(requireKey.dataKey, requireKey.type);

    IFontFace* face = fontFace(actualDataKey, requireKey.type, requireKey.pixelSize, isSymbolMode);
    if (!face) {
        return nullptr;
    }

    //! If we didn't find it, we create a new require font
    RequireFace* newFont = new RequireFace();
    newFont->requireKey = requireKey;
    newFont->face = face;

    auto subtitutionFontDataKeys = fontsDatabase()->substitutionFonts(requireKey.dataKey);
    for (const FontDataKey& dataKey : subtitutionFontDataKeys) {
        IFontFace* subtitutionFace = fontFace(dataKey, requireKey.type, requireKey.pixelSize, isSymbolMode);
        if (subtitutionFace) {
            newFont->subtitutionFaces.push_back(subtitutionFace);
        }
    }

    m_requiredFaces.push_back(newFont);

    return newFont;
}

std::vector<FontsEngine::FontFaceTextBlock> FontsEngine::splitTextByFontFaces(const RequireFace* rf, const TextBlock& text) const
{
    std::vector<FontFaceTextBlock> textBlocks;

    FontFaceTextBlock txtBlock;
    txtBlock.text.text = &text.text[0];
    const IFontFace* current = nullptr;
    for (int i = 0; i < text.lenght; ++i) {
        const IFontFace* newFace = nullptr;
        if (rf->face->glyphIndex(text.text[i]) != 0) {
            newFace = rf->face;
        } else {
            newFace = findSubtitutionFont(text.text[i], rf->subtitutionFaces);
        }

        if (txtBlock.text.lenght > 0 && newFace != current) {
            txtBlock.face = current;
            textBlocks.push_back(txtBlock);
            txtBlock.text.text = &text.text[i];
            txtBlock.text.lenght = 0;
        }

        current = newFace;
        ++txtBlock.text.lenght;

        if (i == (text.lenght - 1)) {
            txtBlock.face = current;
            textBlocks.push_back(txtBlock);
            txtBlock.text.text = nullptr;
            txtBlock.text.lenght = 0;
        }
    }

    return textBlocks;
}
