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
#include "textbaserw.h"

#include "../../libmscore/textbase.h"

#include "../xmlreader.h"

#include "propertyrw.h"
#include "engravingitemrw.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

static constexpr std::array<Pid, 18> TextBasePropertyId { {
    Pid::TEXT_STYLE,
    Pid::FONT_FACE,
    Pid::FONT_SIZE,
    Pid::TEXT_LINE_SPACING,
    Pid::FONT_STYLE,
    Pid::COLOR,
    Pid::FRAME_TYPE,
    Pid::FRAME_WIDTH,
    Pid::FRAME_PADDING,
    Pid::FRAME_ROUND,
    Pid::FRAME_FG_COLOR,
    Pid::FRAME_BG_COLOR,
    Pid::ALIGN,
} };

bool TextBaseRW::readProperties(TextBase* t, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());
    for (Pid i : TextBasePropertyId) {
        if (PropertyRW::readProperty(t, tag, e, ctx, i)) {
            return true;
        }
    }

    if (tag == "text") {
        String str = e.readXml();
        t->setXmlText(str);
        t->checkCustomFormatting(str);
    } else if (tag == "bold") {
        bool val = e.readInt();
        if (val) {
            t->setFontStyle(t->fontStyle() + FontStyle::Bold);
        } else {
            t->setFontStyle(t->fontStyle() - FontStyle::Bold);
        }
        if (t->isStyled(Pid::FONT_STYLE)) {
            t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
        }
    } else if (tag == "italic") {
        bool val = e.readInt();
        if (val) {
            t->setFontStyle(t->fontStyle() + FontStyle::Italic);
        } else {
            t->setFontStyle(t->fontStyle() - FontStyle::Italic);
        }
        if (t->isStyled(Pid::FONT_STYLE)) {
            t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
        }
    } else if (tag == "underline") {
        bool val = e.readInt();
        if (val) {
            t->setFontStyle(t->fontStyle() + FontStyle::Underline);
        } else {
            t->setFontStyle(t->fontStyle() - FontStyle::Underline);
        }
        if (t->isStyled(Pid::FONT_STYLE)) {
            t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
        }
    } else if (tag == "strike") {
        bool val = e.readInt();
        if (val) {
            t->setFontStyle(t->fontStyle() + FontStyle::Strike);
        } else {
            t->setFontStyle(t->fontStyle() - FontStyle::Strike);
        }
        if (t->isStyled(Pid::FONT_STYLE)) {
            t->setPropertyFlags(Pid::FONT_STYLE, PropertyFlags::UNSTYLED);
        }
    } else if (!EngravingItemRW::readProperties(t, e, ctx)) {
        return false;
    }
    return true;
}
