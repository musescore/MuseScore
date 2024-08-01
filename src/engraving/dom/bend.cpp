/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "bend.h"

#include "note.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;

//---------------------------------------------------------
//   label
//---------------------------------------------------------

const char* Bend::label[13] = {
    "", "1/4", "1/2", "3/4", "full",
    "1 1/4", "1 1/2", "1 3/4", "2",
    "2 1/4", "2 1/2", "2 3/4", "3"
};

static const ElementStyle bendStyle {
    { Sid::bendFontFace,                       Pid::FONT_FACE },
    { Sid::bendFontSize,                       Pid::FONT_SIZE },
    { Sid::bendFontStyle,                      Pid::FONT_STYLE },
    { Sid::bendLineWidth,                      Pid::LINE_WIDTH },
};

static const PitchValues BEND_CURVE = { PitchValue(0, 0),
                                        PitchValue(15, 100),
                                        PitchValue(60, 100) };

static const PitchValues BEND_RELEASE_CURVE = { PitchValue(0, 0),
                                                PitchValue(10, 100),
                                                PitchValue(20, 100),
                                                PitchValue(30, 0),
                                                PitchValue(60, 0) };

static const PitchValues BEND_RELEASE_BEND_CURVE = { PitchValue(0, 0),
                                                     PitchValue(10, 100),
                                                     PitchValue(20, 100),
                                                     PitchValue(30, 0),
                                                     PitchValue(40, 0),
                                                     PitchValue(50, 100),
                                                     PitchValue(60, 100) };

static const PitchValues PREBEND_CURVE = { PitchValue(0, 100),
                                           PitchValue(60, 100) };

static const PitchValues PREBEND_RELEASE_CURVE = { PitchValue(0, 100),
                                                   PitchValue(15, 100),
                                                   PitchValue(30, 0),
                                                   PitchValue(60, 0) };

//---------------------------------------------------------
//   Bend
//---------------------------------------------------------

Bend::Bend(Note* parent)
    : EngravingItem(ElementType::BEND, parent, ElementFlag::MOVABLE)
{
    initElementStyle(&bendStyle);
}

//---------------------------------------------------------
//   font
//---------------------------------------------------------

Font Bend::font(double sp) const
{
    Font f(m_fontFace, Font::Type::Unknown);
    f.setBold(m_fontStyle & FontStyle::Bold);
    f.setItalic(m_fontStyle & FontStyle::Italic);
    f.setUnderline(m_fontStyle & FontStyle::Underline);
    f.setStrike(m_fontStyle & FontStyle::Strike);
    double m = m_fontSize;
    m *= sp / SPATIUM20;

    f.setPointSizeF(m);
    return f;
}

BendType Bend::parseBendTypeFromCurve() const
{
    if (m_points == BEND_CURVE) {
        return BendType::BEND;
    } else if (m_points == BEND_RELEASE_CURVE) {
        return BendType::BEND_RELEASE;
    } else if (m_points == BEND_RELEASE_BEND_CURVE) {
        return BendType::BEND_RELEASE_BEND;
    } else if (m_points == PREBEND_CURVE) {
        return BendType::PREBEND;
    } else if (m_points == PREBEND_RELEASE_CURVE) {
        return BendType::PREBEND_RELEASE;
    } else {
        return BendType::CUSTOM;
    }
}

void Bend::updatePointsByBendType(const BendType bendType)
{
    switch (bendType) {
    case BendType::BEND:
        m_points = BEND_CURVE;
        break;
    case BendType::BEND_RELEASE:
        m_points = BEND_RELEASE_CURVE;
        break;
    case BendType::BEND_RELEASE_BEND:
        m_points = BEND_RELEASE_BEND_CURVE;
        break;
    case BendType::PREBEND:
        m_points = PREBEND_CURVE;
        break;
    case BendType::PREBEND_RELEASE:
        m_points = PREBEND_RELEASE_CURVE;
        break;
    default:
        break;
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Bend::getProperty(Pid id) const
{
    switch (id) {
    case Pid::FONT_FACE:
        return m_fontFace;
    case Pid::FONT_SIZE:
        return m_fontSize;
    case Pid::FONT_STYLE:
        return int(m_fontStyle);
    case Pid::PLAY:
        return bool(playBend());
    case Pid::LINE_WIDTH:
        return m_lineWidth;
    case Pid::BEND_TYPE:
        return static_cast<int>(parseBendTypeFromCurve());
    case Pid::BEND_CURVE:
        return m_points;
    default:
        return EngravingItem::getProperty(id);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Bend::setProperty(Pid id, const PropertyValue& v)
{
    switch (id) {
    case Pid::FONT_FACE:
        m_fontFace = v.value<String>();
        break;
    case Pid::FONT_SIZE:
        m_fontSize = v.toReal();
        break;
    case Pid::FONT_STYLE:
        m_fontStyle = FontStyle(v.toInt());
        break;
    case Pid::PLAY:
        setPlayBend(v.toBool());
        break;
    case Pid::LINE_WIDTH:
        m_lineWidth = v.value<Spatium>();
        break;
    case Pid::BEND_TYPE:
        updatePointsByBendType(static_cast<BendType>(v.toInt()));
        break;
    case Pid::BEND_CURVE:
        setPoints(v.value<PitchValues>());
        break;
    default:
        return EngravingItem::setProperty(id, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Bend::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::PLAY:
        return true;
    case Pid::BEND_TYPE:
        return static_cast<int>(BendType::BEND);
    case Pid::BEND_CURVE:
        return BEND_CURVE;
    default:
        return EngravingItem::propertyDefault(id);
    }
}
