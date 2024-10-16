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
#pragma once

#include <vector>
#include <functional>

#include "ifontsengine.h"

#include "global/modularity/ioc.h"
#include "ifontsdatabase.h"

//#include "fontrendercache.h"

namespace muse::draw {
class IFontFace;
class FontsEngine : public IFontsEngine, public Injectable
{
    Inject<IFontsDatabase> fontsDatabase = { this };

public:
    FontsEngine(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}
    ~FontsEngine();

    void init();

    double lineSpacing(const Font& f) const override;
    double xHeight(const Font& f) const override;
    double height(const Font& f) const override;
    double capHeight(const Font& f) const override;
    double ascent(const Font& f) const override;
    double descent(const Font& f) const override;

    bool inFontUcs4(const Font& f, char32_t ucs4) const override;

    double horizontalAdvance(const Font& f, const char32_t& ch) const override;
    double horizontalAdvance(const Font& f, const std::u32string& text) const override;

    RectF boundingRect(const Font& f, const char32_t& ch) const override;
    RectF boundingRect(const Font& f, const std::u32string& text) const override;
    RectF tightBoundingRect(const Font& f, const std::u32string& text) const override;

    // Score symbols
    RectF symBBox(const Font& f, char32_t ucs4) const override;
    double symAdvance(const Font& f, char32_t ucs4) const override;

    // For draw
    std::vector<GlyphImage> render(const Font& f, const std::u32string& text) const override;

    // For dev
    using FontFaceFactory = std::function<IFontFace* (const io::path_t&)>;
    void setFontFaceFactory(const FontFaceFactory& f);

private:

    struct TextBlock {
        const char32_t* text = nullptr;
        int lenght = 0;
    };

    struct RequireFace {
        IFontFace* face = nullptr;   // real loaded face
        std::vector<IFontFace*> subtitutionFaces;
        FaceKey requireKey;          // require face

        bool isSymbolMode() const;
        double pixelScale() const;
    };

    IFontFace* createFontFace(const io::path_t& path) const;
    RequireFace* fontFace(const Font& f, bool isSymbolMode = false) const;

    std::vector<TextBlock> splitTextByLines(const std::u32string& text) const;
    std::vector<TextBlock> splitTextByFontFaces(const RequireFace* rf, const TextBlock& text) const;

    FontFaceFactory m_fontFaceFactory;

    mutable std::vector<IFontFace*> m_loadedFaces;
    mutable std::vector<RequireFace*> m_requiredFaces;

    //mutable FontRenderCache m_renderCache;
};
}
