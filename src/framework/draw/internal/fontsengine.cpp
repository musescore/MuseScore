/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#ifndef MUSE_MODULE_DRAW_USE_QTTEXTDRAW
#include <msdfgen.h>
#include <ext/import-font.h>
#endif

#include "global/io/fileinfo.h"

#include "ifontface.h"
#include "fontfaceft.h"
//#include "fontfacext.h"
#include "fontfacedu.h"

#include "log.h"

using namespace muse;
using namespace muse::draw;

static const double DEFAULT_PIXEL_SIZE = 100.0;
static const double SYMBOLS_PIXEL_SIZE = 200.0;
static const double LOADED_PIXEL_SIZE = 200.0;

static inline RectF fromFBBox(const FBBox& bb, double scale)
{
    return RectF(from_f26d6(bb.left()) * scale, from_f26d6(bb.top()) * scale,
                 from_f26d6(bb.width()) * scale, from_f26d6(bb.height()) * scale);
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

bool FontsEngine::RequireFace::isSymbolMode() const
{
    return face ? face->isSymbolMode() : false;
}

double FontsEngine::RequireFace::pixelScale() const
{
    if (!face) {
        return 0.0;
    }
    double scale = static_cast<double>(requireKey.pixelSize) / static_cast<double>(face->key().pixelSize);
    return scale;
}

FontsEngine::~FontsEngine()
{
    for (RequireFace* f : m_requiredFaces) {
        delete f;
    }

    for (IFontFace* f : m_loadedFaces) {
        delete f;
    }
}

void FontsEngine::init()
{
    //m_renderCache.init();
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

bool FontsEngine::inFontUcs4(const Font& f, char32_t ucs4) const
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

    std::vector<GlyphPos> glyphs = rf->face->glyphs(&text[0], (int)text.size());
    f26dot6_t advance = 0;
    for (const GlyphPos& g : glyphs) {
        advance += g.x_advance;
    }

    return from_f26d6(advance) * rf->pixelScale();
}

RectF FontsEngine::boundingRect(const Font& f, const char32_t& ch) const
{
    RequireFace* rf = fontFace(f);
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

    FBBox rect;      // f26dot6_t units
    FBBox lineRect;  // f26dot6_t units
    bool isFirstLine = true;
    bool isFirstInLine = true;

    std::vector<TextBlock> lines = splitTextByLines(text);
    for (const TextBlock& l : lines) {
        lineRect = FBBox();
        isFirstInLine = true;

        std::vector<TextBlock> fontFaceBlocks = splitTextByFontFaces(rf, l);
        for (const TextBlock& ffBlock : fontFaceBlocks) {
            const IFontFace* fontFace = nullptr;
            if (rf->face->glyphIndex(*ffBlock.text) != 0) {
                fontFace = rf->face;
            } else {
                fontFace = findSubtitutionFont(*ffBlock.text, rf->subtitutionFaces);
            }
            if (!fontFace) {
                continue;
            }

            std::vector<GlyphPos> glyphs = fontFace->glyphs(ffBlock.text, ffBlock.lenght);

            for (const GlyphPos& g : glyphs) {
                FBBox bbox = rf->face->glyphBbox(g.idx);
                if (isFirstInLine) {
                    lineRect = bbox;
                    isFirstInLine = false;
                } else {
                    lineRect.setWidth(lineRect.width() + bbox.width());
                    lineRect.setHeight(std::max(lineRect.height(), bbox.height()));
                    lineRect.setTop(std::min(lineRect.top(), bbox.top()));
                    lineRect.setLeft(std::min(lineRect.left(), bbox.left()));
                }
            }
        }

        if (isFirstLine) {
            rect = lineRect;
            isFirstLine = false;
        } else {
            rect.setWidth(std::max(rect.width(), lineRect.width()));
            rect.setHeight(rect.height() + lineRect.height());
        }
    }

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

    FBBox rect;      // f26dot6_t units
    FBBox lineRect;  // f26dot6_t units
    bool isFirstLine = true;
    bool isFirstInLine = true;

    std::vector<TextBlock> lines = splitTextByLines(text);
    for (const TextBlock& l : lines) {
        lineRect = FBBox();
        isFirstInLine = true;
        f26dot6_t advance = 0;

        std::vector<TextBlock> fontFaceBlocks = splitTextByFontFaces(rf, l);

        GlyphPos lastGlyph;
        for (const TextBlock& ffBlock : fontFaceBlocks) {
            const IFontFace* fontFace = nullptr;
            if (rf->face->glyphIndex(*ffBlock.text) != 0) {
                fontFace = rf->face;
            } else {
                fontFace = findSubtitutionFont(*ffBlock.text, rf->subtitutionFaces);
            }
            if (!fontFace) {
                continue;
            }

            std::vector<GlyphPos> glyphs = fontFace->glyphs(ffBlock.text, ffBlock.lenght);

            for (const GlyphPos& g : glyphs) {
                FBBox bbox = rf->face->glyphBbox(g.idx);
                if (isFirstInLine) {
                    lineRect = bbox;
                    isFirstInLine = false;
                } else {
                    /// width is calculated as x_advance instead
                    lineRect.setTop(std::min(lineRect.top(), bbox.top()));
                    lineRect.setLeft(std::min(lineRect.left(), bbox.left()));
                    lineRect.setBottom(std::max(lineRect.bottom(), bbox.bottom()));
                }
                advance += g.x_advance;
            }
            lastGlyph = glyphs.back();
        }

        advance -= (lastGlyph.x_advance - rf->face->glyphBbox(lastGlyph.idx).width());
        lineRect.setWidth(advance);

        if (isFirstLine) {
            rect = lineRect;
            isFirstLine = false;
        } else {
            rect.setWidth(std::max(rect.width(), lineRect.width()));
            rect.setHeight(rect.height() + lineRect.height());
        }
    }

    return fromFBBox(rect, rf->pixelScale());
}

RectF FontsEngine::symBBox(const Font& f, char32_t ucs4) const
{
    RequireFace* rf = fontFace(f, true);
    IF_ASSERT_FAILED(rf && rf->face) {
        return RectF();
    }

    glyph_idx_t glyphIdx = rf->face->glyphIndex(ucs4);
    FBBox bb = rf->face->glyphBbox(glyphIdx);
    return fromFBBox(bb, rf->pixelScale());
}

double FontsEngine::symAdvance(const Font& f, char32_t ucs4) const
{
    RequireFace* rf = fontFace(f, true);
    IF_ASSERT_FAILED(rf && rf->face) {
        return 0.0;
    }

    glyph_idx_t glyphIdx = rf->face->glyphIndex(ucs4);
    f26dot6_t advance = rf->face->glyphAdvance(glyphIdx);
    return from_f26d6(advance) * rf->pixelScale();
}

#ifndef MUSE_MODULE_DRAW_USE_QTTEXTDRAW
static void generateSdf(GlyphImage& out, glyph_idx_t glyphIdx, const IFontFace* face)
{
    struct Bounds
    {
        double l, b, r, t;
    };
    Bounds bounds = { 1e240, 1e240, -1e240, -1e240 };

    msdfgen::Shape shape = face->glyphShape(glyphIdx);
    if (shape.contours.empty()) {
        //! NOTE Maybe not printable, like ' '
        return;
    }

    shape.bounds(bounds.l, bounds.b, bounds.r, bounds.t);

    uint32_t pxRange = std::min(SDF_WIDTH, SDF_HEIGHT) >> 3;

    std::pair<double, double> sdfScale;
    msdfgen::Vector2 translate;
    double scale = 0.0;
    msdfgen::Vector2 frame(SDF_WIDTH, SDF_HEIGHT);
    frame -= 2 * pxRange;
    assert(frame.x >= 0 && frame.y >= 0 && bounds.l < bounds.r && bounds.b < bounds.t);
    msdfgen::Vector2 dims(bounds.r - bounds.l, bounds.t - bounds.b);
    if (dims.x * frame.y < dims.y * frame.x) { // fit restricted by height
        translate = { -bounds.l, -bounds.b };
        scale = frame.y / dims.y;
        sdfScale = { (frame.x - dims.x * scale) / (dims.x * scale), 0.0f };
    } else { // fit restricted by width
        translate = { -bounds.l, -bounds.b };
        scale = frame.x / dims.x;
        sdfScale = { 0.0, (frame.y - dims.y * scale) / (dims.y * scale) };
    }

    double boundsWidth = bounds.r - bounds.l;
    double boundsHeight = bounds.t - bounds.b;
    double widthWhitespace = boundsWidth * sdfScale.first;
    double heightWhitespace = boundsHeight * sdfScale.second;
    double pxRangeScaled = pxRange / scale;

    double left = bounds.l - pxRangeScaled;
    double top = -bounds.t - heightWhitespace - pxRangeScaled;
    double width = boundsWidth + widthWhitespace + pxRangeScaled * 2;
    double height = boundsHeight + heightWhitespace + pxRangeScaled * 2;

    double range = pxRange / scale;
    translate += range;

    shape.mergeContours();

    auto sdf = msdfgen::Bitmap<uint8_t>(SDF_WIDTH, SDF_HEIGHT);
    msdfgen::generateSDF(sdf, shape, bounds.l, range, scale, translate);

    out.sdf.bitmap = mu::ByteArray(sdf.takeMemoryAway(), SDF_WIDTH * SDF_HEIGHT);
    out.sdf.width = SDF_WIDTH;
    out.sdf.height = SDF_HEIGHT;
    out.sdf.hash = std::hash<std::string_view> {}({ reinterpret_cast<const char*>(out.sdf.bitmap.data()), out.sdf.bitmap.size() });

    out.rect.setTop(top);
    out.rect.setLeft(left);
    out.rect.setWidth(width);
    out.rect.setHeight(height);
}

#endif

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

    UNUSED(text);

#ifndef MUSE_MODULE_DRAW_USE_QTTEXTDRAW
    int pixelSize = rf->requireKey.pixelSize;
    double pixelScale = rf->pixelScale();
    double glyphTop = 0;

    std::vector<TextBlock> lines = splitTextByLines(text);
    for (const TextBlock& l : lines) {
        double glyphLeft = 0;

        std::vector<TextBlock> fontFaceBlocks = splitTextByFontFaces(rf, l);
        for (const TextBlock& ffBlock : fontFaceBlocks) {
            const IFontFace* fontFace = nullptr;
            if (rf->face->glyphIndex(*ffBlock.text) != 0) {
                fontFace = rf->face;
            } else {
                fontFace = findSubtitutionFont(*ffBlock.text, rf->subtitutionFaces);
            }
            if (!fontFace) {
                continue;
            }

            std::vector<GlyphPos> glyphs = fontFace->glyphs(ffBlock.text, ffBlock.lenght);

            for (const GlyphPos& g : glyphs) {
                if (NOT_RENDER_GLYPHS.find(g.idx) == NOT_RENDER_GLYPHS.end()) {
                    GlyphImage image;// = m_renderCache.load(fontFace->key(), g.idx);
                    if (image.isNull()) {
                        generateSdf(image, g.idx, fontFace);
                        //m_renderCache.store(fontFace->key(), g.idx, image);
                    }

                    image.rect = scaleRect(image.rect, pixelScale);
                    image.rect.translate(glyphLeft, glyphTop);

                    images.push_back(std::move(image));
                }

                glyphLeft += from_f26d6(g.x_advance) * pixelScale;
            }
        }

        glyphTop += (pixelSize * TEXT_LINE_SCALE);
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

    IFontFace* origin = new FontFaceFT();
    if (io::FileInfo::suffix(path) == u"ftx") {
        //origin = new FontFaceXT();
    } else {
        origin = new FontFaceFT();
    }

    return new FontFaceDU(origin);
}

FontsEngine::RequireFace* FontsEngine::fontFace(const Font& f, bool isSymbolMode) const
{
    //! NOTE This font is required
    FaceKey requireKey = faceKeyForFont(f);

    //! NOTE If pixelSize is not set, then specify the default
    //! (this is the default pixelSize in Qt)
    if (!(requireKey.pixelSize > 0)) {
        requireKey.pixelSize = DEFAULT_PIXEL_SIZE;
    }

    //! NOTE For symbol mode, a fixed pixelSize is used
    if (isSymbolMode) {
        requireKey.pixelSize = SYMBOLS_PIXEL_SIZE;
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

    //! If we didn't find it, we create a new require font
    RequireFace* newFont = new RequireFace();
    newFont->requireKey = requireKey;

    //! Let's find out which real font will be used
    //! (for example, if there is no required one)
    FontDataKey actualDataKey = fontsDatabase()->actualFont(requireKey.dataKey, requireKey.type);

    //! NOTE We are looking for the font face we real need among the previously loaded ones
    //! IMPORTANT We use font faces with a fixed pixelSize, so we need to find the right face only from the data
    IFontFace* face = nullptr;
    for (IFontFace* ff : m_loadedFaces) {
        if (ff->key().dataKey == actualDataKey && ff->isSymbolMode() == isSymbolMode) {
            face = ff;
            break;
        }
    }

    //! NOTE If we haven't found a face, we'll create a new one
    if (!face) {
        io::path_t fontPath = fontsDatabase()->fontPath(requireKey.dataKey, requireKey.type);
        IF_ASSERT_FAILED(!fontPath.empty()) {
            return nullptr;
        }

        FaceKey loadedKey;
        loadedKey.dataKey = actualDataKey;
        loadedKey.type = requireKey.type;
        loadedKey.pixelSize = LOADED_PIXEL_SIZE;

        face = createFontFace(fontPath);

        face->load(loadedKey, fontPath, isSymbolMode);
        m_loadedFaces.push_back(face);
    }

    newFont->face = face;

    IFontFace* subtitutionFace = nullptr;
    auto subtitutionFontDataKeys = fontsDatabase()->substitutionFonts(requireKey.type);
    for (const FontDataKey& dataKey : subtitutionFontDataKeys) {
        for (IFontFace* ff : m_loadedFaces) {
            if (ff->key().dataKey == dataKey && ff->isSymbolMode() == isSymbolMode) {
                subtitutionFace = ff;
                break;
            }
        }

        if (!subtitutionFace) {
            io::path_t fontPath = fontsDatabase()->fontPath(dataKey, requireKey.type);
            IF_ASSERT_FAILED(!fontPath.empty()) {
                return nullptr;
            }

            FaceKey loadedKey;
            loadedKey.dataKey = dataKey;
            loadedKey.type = requireKey.type;
            loadedKey.pixelSize = LOADED_PIXEL_SIZE;

            subtitutionFace = createFontFace(fontPath);

            subtitutionFace->load(loadedKey, fontPath, isSymbolMode);
            m_loadedFaces.push_back(subtitutionFace);
        }
        newFont->subtitutionFaces.push_back(subtitutionFace);
    }

    m_requiredFaces.push_back(newFont);

    return newFont;
}

std::vector<FontsEngine::TextBlock> FontsEngine::splitTextByLines(const std::u32string& text) const
{
    std::vector<TextBlock> lines;

    TextBlock l;
    for (size_t i = 0; i < text.size(); ++i) {
        if (!l.text) {
            l.text = &text[i];
        }

        if (text.at(i) == U'\n') {
            lines.push_back(l);
            l.text = nullptr;
            l.lenght = 0;
        }
        ++l.lenght;

        if (i == (text.size() - 1)) {
            lines.push_back(l);
            l.text = nullptr;
            l.lenght = 0;
        }
    }

    return lines;
}

std::vector<FontsEngine::TextBlock> FontsEngine::splitTextByFontFaces(const RequireFace* rf, const TextBlock& text) const
{
    std::vector<TextBlock> textBlocks;

    TextBlock txtBlock;
    const IFontFace* current = rf->face;
    for (int i = 0; i < text.lenght; ++i) {
        if (!txtBlock.text) {
            txtBlock.text = &text.text[i];
        }

        if (rf->face->glyphIndex(text.text[i]) != 0) {
            if (current->key() != rf->face->key()) {
                current = rf->face;
                textBlocks.push_back(txtBlock);
                txtBlock.text = &text.text[i];
                txtBlock.lenght = 0;
            }
        } else if (rf->face->glyphIndex(text.text[i]) == 0) {
            auto newSubFace = findSubtitutionFont(text.text[i], rf->subtitutionFaces);
            if (newSubFace && newSubFace->key() != current->key() && txtBlock.lenght != 0) {
                current = newSubFace;
                textBlocks.push_back(txtBlock);
                txtBlock.text = &text.text[i];
                txtBlock.lenght = 0;
            }
        }

        ++txtBlock.lenght;

        if (i == (text.lenght - 1)) {
            textBlocks.push_back(txtBlock);
            txtBlock.text = nullptr;
            txtBlock.lenght = 0;
        }
    }

    return textBlocks;
}
