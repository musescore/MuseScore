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

#include <string>

#include "global/modularity/imoduleinterface.h"
#include "types/font.h"
#include "types/geometry.h"
#include "types/fontstypes.h"

namespace muse::draw {
class IFontsEngine : public modularity::IModuleExportInterface
{
    INTERFACE_ID(muse::draw::IFontsEngine)
public:
    virtual ~IFontsEngine() = default;

    virtual double lineSpacing(const Font& f) const = 0;
    virtual double xHeight(const Font& f) const = 0;
    virtual double height(const Font& f) const = 0;
    virtual double capHeight(const Font& ff) const = 0;
    virtual double ascent(const Font& f) const = 0;
    virtual double descent(const Font& f) const = 0;

    virtual bool inFontUcs4(const Font& f, char32_t ucs4) const = 0;

    virtual double horizontalAdvance(const Font& f, const char32_t& ch) const = 0;
    virtual double horizontalAdvance(const Font& f, const std::u32string& text) const = 0;

    virtual RectF boundingRect(const Font& f, const char32_t& ch) const = 0;
    virtual RectF boundingRect(const Font& f, const std::u32string& text) const = 0;
    virtual RectF tightBoundingRect(const Font& f, const std::u32string& text) const = 0;

    // Score symbols
    virtual RectF symBBox(const Font& f, char32_t ucs4) const = 0;
    virtual double symAdvance(const Font& f, char32_t ucs4) const = 0;

    // Draw
    virtual std::vector<GlyphImage> render(const Font& f, const std::u32string& text) const = 0;
};
}
