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
#include "vibrato.h"

#include <cmath>

#include "types/typesconv.h"

#include "iengravingfont.h"

#include "score.h"
#include "system.h"
#include "trill.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
VibratoSegment::VibratoSegment(Vibrato* sp, System* parent)
    : LineSegment(ElementType::VIBRATO_SEGMENT, sp, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
}

//---------------------------------------------------------
//   symbolLine
//---------------------------------------------------------

void VibratoSegment::symbolLine(SymId start, SymId fill)
{
    double x1 = 0;
    double x2 = pos2().x();
    double w   = x2 - x1;
    double mag = magS();
    IEngravingFontPtr f = score()->engravingFont();

    m_symbols.clear();
    m_symbols.push_back(start);
    double w1 = f->advance(start, mag);
    double w2 = f->advance(fill, mag);
    int n    = lrint((w - w1) / w2);
    for (int i = 0; i < n; ++i) {
        m_symbols.push_back(fill);
    }
    RectF r(f->bbox(m_symbols, mag));
    setbbox(r);
}

void VibratoSegment::symbolLine(SymId start, SymId fill, SymId end)
{
    double x1 = 0;
    double x2 = pos2().x();
    double w   = x2 - x1;
    double mag = magS();
    IEngravingFontPtr f = score()->engravingFont();

    m_symbols.clear();
    m_symbols.push_back(start);
    double w1 = f->bbox(start, mag).width();
    double w2 = f->width(fill, mag);
    double w3 = f->width(end, mag);
    int n    = lrint((w - w1 - w3) / w2);
    for (int i = 0; i < n; ++i) {
        m_symbols.push_back(fill);
    }
    m_symbols.push_back(end);
    RectF r(f->bbox(m_symbols, mag));
    setbbox(r);
}

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

EngravingItem* VibratoSegment::propertyDelegate(Pid pid)
{
    if (pid == Pid::VIBRATO_TYPE || pid == Pid::PLACEMENT) {
        return spanner();
    }
    return LineSegment::propertyDelegate(pid);
}

//---------------------------------------------------------
//   vibratoStyle
//---------------------------------------------------------

static const ElementStyle vibratoStyle {
    { Sid::vibratoPlacement,      Pid::PLACEMENT },
    { Sid::vibratoPosAbove,       Pid::OFFSET },
};

//---------------------------------------------------------
//   Vibrato
//---------------------------------------------------------

Vibrato::Vibrato(EngravingItem* parent)
    : SLine(ElementType::VIBRATO, parent)
{
    initElementStyle(&vibratoStyle);
    m_vibratoType = VibratoType::GUITAR_VIBRATO;
}

Vibrato::~Vibrato()
{
}

static const ElementStyle vibratoSegmentStyle {
    { Sid::vibratoPosAbove,       Pid::OFFSET },
    { Sid::vibratoMinDistance,    Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Vibrato::createLineSegment(System* parent)
{
    VibratoSegment* seg = new VibratoSegment(this, parent);
    seg->setTrack(track());
    seg->setColor(color());
    seg->initElementStyle(&vibratoSegmentStyle);
    return seg;
}

PointF Vibrato::linePos(Grip grip, System** system) const
{
    return Trill::trillLinePos(this, grip, system);
}

//---------------------------------------------------------
//   vibratoTypeName
//---------------------------------------------------------

String Vibrato::vibratoTypeUserName() const
{
    return TConv::translatedUserName(vibratoType());
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString Vibrato::subtypeUserName() const
{
    return TConv::userName(vibratoType());
}

muse::TranslatableString VibratoSegment::subtypeUserName() const
{
    return vibrato()->subtypeUserName();
}

int VibratoSegment::subtype() const
{
    return vibrato()->subtype();
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid VibratoSegment::getPropertyStyle(Pid pid) const
{
    if (pid == Pid::OFFSET) {
        return spanner()->placeAbove() ? Sid::vibratoPosAbove : Sid::vibratoPosBelow;
    }
    return LineSegment::getPropertyStyle(pid);
}

Sid Vibrato::getPropertyStyle(Pid pid) const
{
    if (pid == Pid::OFFSET) {
        return placeAbove() ? Sid::vibratoPosAbove : Sid::vibratoPosBelow;
    }
    return SLine::getPropertyStyle(pid);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Vibrato::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::VIBRATO_TYPE:
        return int(vibratoType());
    default:
        break;
    }
    return SLine::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Vibrato::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::VIBRATO_TYPE:
        setVibratoType(VibratoType(val.toInt()));
        break;
    case Pid::COLOR:
        setColor(val.value<Color>());
        [[fallthrough]];
    default:
        if (!SLine::setProperty(propertyId, val)) {
            return false;
        }
        break;
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Vibrato::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::VIBRATO_TYPE:
        return 0;
    case Pid::PLACEMENT:
        return style().styleV(Sid::vibratoPlacement);
    default:
        return SLine::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   undoSetVibratoType
//---------------------------------------------------------

void Vibrato::undoSetVibratoType(VibratoType val)
{
    undoChangeProperty(Pid::VIBRATO_TYPE, int(val));
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Vibrato::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), vibratoTypeUserName());
}
}
