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
#ifndef MU_DRAW_IFONTSENGINE_H
#define MU_DRAW_IFONTSENGINE_H

#include <string>

#include "global/modularity/imoduleinterface.h"
#include "types/font.h"
#include "types/geometry.h"
#include "types/fontstypes.h"

namespace mu::draw {
class IFontsEngine : public mu::modularity::IModuleExportInterface
{
    INTERFACE_ID(mu::draw::IFontsEngine)
public:
    virtual ~IFontsEngine() = default;

    virtual double lineSpacing(const mu::draw::Font& f) const = 0;
    virtual double xHeight(const mu::draw::Font& f) const = 0;
    virtual double height(const mu::draw::Font& f) const = 0;
    virtual double ascent(const mu::draw::Font& f) const = 0;
    virtual double descent(const mu::draw::Font& f) const = 0;

    virtual bool inFontUcs4(const mu::draw::Font& f, char32_t ucs4) const = 0;

    virtual double horizontalAdvance(const mu::draw::Font& f, const char32_t& ch) const = 0;
    virtual double horizontalAdvance(const mu::draw::Font& f, const std::u32string& text) const = 0;

    virtual mu::RectF boundingRect(const mu::draw::Font& f, const char32_t& ch) const = 0;
    virtual mu::RectF boundingRect(const mu::draw::Font& f, const std::u32string& text) const = 0;
    virtual mu::RectF tightBoundingRect(const mu::draw::Font& f, const std::u32string& text) const = 0;

    // Score symbols
    virtual mu::RectF symBBox(const mu::draw::Font& f, char32_t ucs4) const = 0;
    virtual double symAdvance(const mu::draw::Font& f, char32_t ucs4) const = 0;

    // Draw
    virtual std::vector<GlyphImage> render(const mu::draw::Font& f, const std::u32string& text) const = 0;
};
}

#endif // MU_DRAW_IFONTSENGINE_H
