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

#include "hairpin.h"

#include <cmath>

#include "translation.h"

#include "draw/types/pen.h"
#include "draw/types/transform.h"

#include "types/typesconv.h"

#include "dynamic.h"
#include "dynamichairpingroup.h"
#include "measure.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "system.h"
#include "text.h"

#include "log.h"

using namespace mu;
using namespace mu::draw;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   hairpinStyle
//---------------------------------------------------------

static const ElementStyle hairpinStyle {
    { Sid::hairpinFontFace,                    Pid::BEGIN_FONT_FACE },
    { Sid::hairpinFontSize,                    Pid::BEGIN_FONT_SIZE },
    { Sid::hairpinFontStyle,                   Pid::BEGIN_FONT_STYLE },
    { Sid::hairpinText,                        Pid::BEGIN_TEXT },
    { Sid::hairpinTextAlign,                   Pid::BEGIN_TEXT_ALIGN },
    { Sid::hairpinFontFace,                    Pid::CONTINUE_FONT_FACE },
    { Sid::hairpinFontSize,                    Pid::CONTINUE_FONT_SIZE },
    { Sid::hairpinFontStyle,                   Pid::CONTINUE_FONT_STYLE },
    { Sid::hairpinText,                        Pid::CONTINUE_TEXT },
    { Sid::hairpinTextAlign,                   Pid::CONTINUE_TEXT_ALIGN },
    { Sid::hairpinFontFace,                    Pid::END_FONT_FACE },
    { Sid::hairpinFontSize,                    Pid::END_FONT_SIZE },
    { Sid::hairpinFontStyle,                   Pid::END_FONT_STYLE },
    { Sid::hairpinTextAlign,                   Pid::END_TEXT_ALIGN },
    { Sid::hairpinLineWidth,                   Pid::LINE_WIDTH },
    { Sid::hairpinHeight,                      Pid::HAIRPIN_HEIGHT },
    { Sid::hairpinContHeight,                  Pid::HAIRPIN_CONT_HEIGHT },
    { Sid::hairpinPlacement,                   Pid::PLACEMENT },
    { Sid::hairpinPosBelow,                    Pid::OFFSET },
    { Sid::hairpinLineStyle,                   Pid::LINE_STYLE },
    { Sid::hairpinLineDashLineLen,             Pid::DASH_LINE_LEN },
    { Sid::hairpinLineDashGapLen,              Pid::DASH_GAP_LEN },
    { Sid::hairpinFontSpatiumDependent,        Pid::TEXT_SIZE_SPATIUM_DEPENDENT, },
};

//---------------------------------------------------------
//   HairpinSegment
//---------------------------------------------------------

HairpinSegment::HairpinSegment(Hairpin* sp, System* parent)
    : TextLineBaseSegment(ElementType::HAIRPIN_SEGMENT, sp, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
}

int HairpinSegment::subtype() const
{
    return hairpin()->subtype();
}

bool HairpinSegment::acceptDrop(EditData& data) const
{
    EngravingItem* e = data.dropElement;
    if (e->isDynamic()) {
        return true;
    }
    return false;
}

EngravingItem* HairpinSegment::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    if (e->isDynamic()) {
        Dynamic* d = toDynamic(e);
        hairpin()->undoChangeProperty(Pid::END_TEXT, d->xmlText());
    }
    return 0;
}

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape HairpinSegment::shape() const
{
    switch (hairpin()->hairpinType()) {
    case HairpinType::CRESC_HAIRPIN:
    case HairpinType::DECRESC_HAIRPIN:
        return Shape(bbox());
    case HairpinType::DECRESC_LINE:
    case HairpinType::CRESC_LINE:
    default:
        return TextLineBaseSegment::shape();
    }
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

int HairpinSegment::gripsCount() const
{
    return hairpin()->isLineType() ? 3 : 4;
}

std::vector<PointF> HairpinSegment::gripsPositions(const EditData&) const
{
    double _spatium = spatium();
    double x = pos2().x();
    if (x < _spatium) {             // minimum size of hairpin
        x = _spatium;
    }
    double y = pos2().y();
    PointF p(x, y);

    std::vector<PointF> grips(gripsCount());
    PointF pp(pagePos());
    grips[int(Grip::START)] = pp;
    grips[int(Grip::END)] = p + pp;
    grips[int(Grip::MIDDLE)] = p * .5 + pp;

    if (!hairpin()->isLineType()) {
        // Calc PointF for Grip Aperture
        Transform doRotation;
        PointF gripLineAperturePoint;
        double h1 = hairpin()->hairpinHeight().val() * spatium() * .5;
        double len = sqrt(x * x + y * y);
        doRotation.rotateRadians(asin(y / len));
        double lineApertureX;
        double offsetX = 10;                                 // Horizontal offset for x Grip
        if (len < offsetX * 3) {                            // For small hairpin, offset = 30% of len
            offsetX = len / 3;                              // else offset is fixed to 10
        }
        if (hairpin()->hairpinType() == HairpinType::CRESC_HAIRPIN) {
            lineApertureX = len - offsetX;                  // End of CRESCENDO - Offset
        } else {
            lineApertureX = offsetX;                        // Begin of DECRESCENDO + Offset
        }
        double lineApertureH = (len - offsetX) * h1 / len;   // Vertical position for y grip
        gripLineAperturePoint.setX(lineApertureX);
        gripLineAperturePoint.setY(lineApertureH);
        gripLineAperturePoint = doRotation.map(gripLineAperturePoint);

        // End calc position grip aperture
        grips[int(Grip::APERTURE)] = gripLineAperturePoint + pp;
    }

    return grips;
}

//---------------------------------------------------------
//   getDragGroup
//---------------------------------------------------------

std::unique_ptr<ElementGroup> HairpinSegment::getDragGroup(std::function<bool(const EngravingItem*)> isDragged)
{
    if (auto g = HairpinWithDynamicsDragGroup::detectFor(this, isDragged)) {
        return g;
    }
    return TextLineBaseSegment::getDragGroup(isDragged);
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void HairpinSegment::startEditDrag(EditData& ed)
{
    TextLineBaseSegment::startEditDrag(ed);
    ElementEditDataPtr eed = ed.getData(this);

    eed->pushProperty(Pid::HAIRPIN_HEIGHT);
    eed->pushProperty(Pid::HAIRPIN_CONT_HEIGHT);
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void HairpinSegment::editDrag(EditData& ed)
{
    if (ed.curGrip == Grip::APERTURE) {
        double newHeight = hairpin()->hairpinHeight().val() + ed.delta.y() / spatium() / .5;
        if (newHeight < 0.5) {
            newHeight = 0.5;
        }
        hairpin()->setHairpinHeight(Spatium(newHeight));
        triggerLayout();
    }
    TextLineBaseSegment::editDrag(ed);
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void HairpinSegment::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;
    TextLineBaseSegment::draw(painter);

    if (m_drawCircledTip) {
        Color color = curColor(hairpin()->visible(), hairpin()->lineColor());
        double w = hairpin()->lineWidth();
        if (staff()) {
            w *= staff()->staffMag(hairpin()->tick());
        }

        Pen pen(color, w);
        painter->setPen(pen);
        painter->setBrush(BrushStyle::NoBrush);
        painter->drawEllipse(m_circledTip, m_circledTipRadius, m_circledTipRadius);
    }
}

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

EngravingItem* HairpinSegment::propertyDelegate(Pid pid)
{
    if (pid == Pid::HAIRPIN_TYPE
        || pid == Pid::VELO_CHANGE
        || pid == Pid::VELO_CHANGE_METHOD
        || pid == Pid::SINGLE_NOTE_DYNAMICS
        || pid == Pid::HAIRPIN_CIRCLEDTIP
        || pid == Pid::HAIRPIN_HEIGHT
        || pid == Pid::HAIRPIN_CONT_HEIGHT
        || pid == Pid::DYNAMIC_RANGE
        || pid == Pid::LINE_STYLE
        ) {
        return spanner();
    }
    return TextLineBaseSegment::propertyDelegate(pid);
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid HairpinSegment::getPropertyStyle(Pid pid) const
{
    switch (pid) {
    case Pid::OFFSET:
        if (hairpin()->isLineType()) {
            return spanner()->placeAbove() ? Sid::hairpinLinePosAbove : Sid::hairpinLinePosBelow;
        }
        return spanner()->placeAbove() ? Sid::hairpinPosAbove : Sid::hairpinPosBelow;
    case Pid::BEGIN_TEXT:
        switch (hairpin()->hairpinType()) {
        default:
            return Sid::hairpinText;
        case HairpinType::CRESC_LINE:
            return Sid::hairpinCrescText;
        case HairpinType::DECRESC_LINE:
            return Sid::hairpinDecrescText;
        }
        break;
    case Pid::CONTINUE_TEXT:
        switch (hairpin()->hairpinType()) {
        default:
            return Sid::hairpinText;
        case HairpinType::CRESC_LINE:
            return Sid::hairpinCrescContText;
        case HairpinType::DECRESC_LINE:
            return Sid::hairpinDecrescContText;
        }
        break;
    case Pid::LINE_STYLE:
        return hairpin()->isLineType() ? Sid::hairpinLineLineStyle : Sid::hairpinLineStyle;
    case Pid::DASH_LINE_LEN:
        return hairpin()->isLineType() ? Sid::hairpinLineDashLineLen : Sid::hairpinDashLineLen;
    case Pid::DASH_GAP_LEN:
        return hairpin()->isLineType() ? Sid::hairpinLineDashGapLen : Sid::hairpinDashGapLen;
    default:
        break;
    }
    return TextLineBaseSegment::getPropertyStyle(pid);
}

Sid Hairpin::getPropertyStyle(Pid pid) const
{
    switch (pid) {
    case Pid::OFFSET:
        if (isLineType()) {
            return placeAbove() ? Sid::hairpinLinePosAbove : Sid::hairpinLinePosBelow;
        }
        return placeAbove() ? Sid::hairpinPosAbove : Sid::hairpinPosBelow;
    case Pid::BEGIN_TEXT:
        switch (hairpinType()) {
        default:
            return Sid::hairpinText;
        case HairpinType::CRESC_LINE:
            return Sid::hairpinCrescText;
        case HairpinType::DECRESC_LINE:
            return Sid::hairpinDecrescText;
        }
        break;
    case Pid::CONTINUE_TEXT:
        switch (hairpinType()) {
        default:
            return Sid::hairpinText;
        case HairpinType::CRESC_LINE:
            return Sid::hairpinCrescContText;
        case HairpinType::DECRESC_LINE:
            return Sid::hairpinDecrescContText;
        }
        break;
    case Pid::LINE_STYLE:
        return isLineType() ? Sid::hairpinLineLineStyle : Sid::hairpinLineStyle;
    case Pid::DASH_LINE_LEN:
        return isLineType() ? Sid::hairpinLineDashLineLen : Sid::hairpinDashLineLen;
    case Pid::DASH_GAP_LEN:
        return isLineType() ? Sid::hairpinLineDashGapLen : Sid::hairpinDashGapLen;
    default:
        break;
    }
    return TextLineBase::getPropertyStyle(pid);
}

//---------------------------------------------------------
//   Hairpin
//---------------------------------------------------------

Hairpin::Hairpin(Segment* parent)
    : TextLineBase(ElementType::HAIRPIN, parent)
{
    initElementStyle(&hairpinStyle);

    resetProperty(Pid::BEGIN_TEXT_PLACE);
    resetProperty(Pid::CONTINUE_TEXT_PLACE);
    resetProperty(Pid::HAIRPIN_TYPE);
    resetProperty(Pid::LINE_VISIBLE);

    _hairpinCircledTip     = false;
    _veloChange            = 0;
    _dynRange              = DynamicRange::PART;
    _singleNoteDynamics    = true;
    _veloChangeMethod      = ChangeMethod::NORMAL;
}

int Hairpin::subtype() const
{
    return static_cast<int>(_hairpinType);
}

DynamicType Hairpin::dynamicTypeFrom() const
{
    ByteArray ba = beginText().toAscii();
    return TConv::dynamicType(ba.constChar());
}

DynamicType Hairpin::dynamicTypeTo() const
{
    ByteArray ba = endText().toAscii();
    return TConv::dynamicType(ba.constChar());
}

//---------------------------------------------------------
//   setHairpinType
//---------------------------------------------------------

void Hairpin::setHairpinType(HairpinType val)
{
    if (_hairpinType == val) {
        return;
    }
    _hairpinType = val;
    styleChanged();
}

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

static const ElementStyle hairpinSegmentStyle {
    { Sid::hairpinPosBelow, Pid::OFFSET },
    { Sid::hairpinMinDistance, Pid::MIN_DISTANCE },
};

LineSegment* Hairpin::createLineSegment(System* parent)
{
    HairpinSegment* h = new HairpinSegment(this, parent);
    h->setTrack(track());
    h->initElementStyle(&hairpinSegmentStyle);
    return h;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Hairpin::getProperty(Pid id) const
{
    switch (id) {
    case Pid::HAIRPIN_CIRCLEDTIP:
        return _hairpinCircledTip;
    case Pid::HAIRPIN_TYPE:
        return int(_hairpinType);
    case Pid::VELO_CHANGE:
        return _veloChange;
    case Pid::DYNAMIC_RANGE:
        return int(_dynRange);
    case Pid::HAIRPIN_HEIGHT:
        return _hairpinHeight;
    case Pid::HAIRPIN_CONT_HEIGHT:
        return _hairpinContHeight;
    case Pid::SINGLE_NOTE_DYNAMICS:
        return _singleNoteDynamics;
    case Pid::VELO_CHANGE_METHOD:
        return _veloChangeMethod;
    default:
        return TextLineBase::getProperty(id);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Hairpin::setProperty(Pid id, const PropertyValue& v)
{
    switch (id) {
    case Pid::HAIRPIN_CIRCLEDTIP:
        _hairpinCircledTip = v.toBool();
        break;
    case Pid::HAIRPIN_TYPE:
        setHairpinType(HairpinType(v.toInt()));
        break;
    case Pid::VELO_CHANGE:
        _veloChange = v.toInt();
        break;
    case Pid::DYNAMIC_RANGE:
        _dynRange = v.value<DynamicRange>();
        break;
    case Pid::HAIRPIN_HEIGHT:
        _hairpinHeight = v.value<Spatium>();
        break;
    case Pid::HAIRPIN_CONT_HEIGHT:
        _hairpinContHeight = v.value<Spatium>();
        break;
    case Pid::SINGLE_NOTE_DYNAMICS:
        _singleNoteDynamics = v.toBool();
        break;
    case Pid::VELO_CHANGE_METHOD:
        _veloChangeMethod = v.value<ChangeMethod>();
        break;
    default:
        return TextLineBase::setProperty(id, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Hairpin::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::HAIRPIN_CIRCLEDTIP:
        return false;

    case Pid::VELO_CHANGE:
        return 0;

    case Pid::DYNAMIC_RANGE:
        return DynamicRange::PART;

    case Pid::BEGIN_TEXT:
        if (_hairpinType == HairpinType::CRESC_LINE) {
            return style().styleV(Sid::hairpinCrescText);
        }
        if (_hairpinType == HairpinType::DECRESC_LINE) {
            return style().styleV(Sid::hairpinDecrescText);
        }
        return String();

    case Pid::CONTINUE_TEXT:
        if (_hairpinType == HairpinType::CRESC_LINE) {
            return style().styleV(Sid::hairpinCrescContText);
        }
        if (_hairpinType == HairpinType::DECRESC_LINE) {
            return style().styleV(Sid::hairpinDecrescContText);
        }
        return String();

    case Pid::END_TEXT:
        return String();

    case Pid::BEGIN_TEXT_PLACE:
    case Pid::CONTINUE_TEXT_PLACE:
        return TextPlace::LEFT;

    case Pid::BEGIN_TEXT_OFFSET:
    case Pid::CONTINUE_TEXT_OFFSET:
    case Pid::END_TEXT_OFFSET:
        return PropertyValue::fromValue(PointF());

    case Pid::BEGIN_HOOK_TYPE:
    case Pid::END_HOOK_TYPE:
        return HookType::NONE;

    case Pid::BEGIN_HOOK_HEIGHT:
    case Pid::END_HOOK_HEIGHT:
        return Spatium(0.0);

    case Pid::LINE_VISIBLE:
        return true;

    case Pid::HAIRPIN_TYPE:
        return int(HairpinType::CRESC_HAIRPIN);

    case Pid::SINGLE_NOTE_DYNAMICS:
        return true;

    case Pid::VELO_CHANGE_METHOD:
        return ChangeMethod::NORMAL;

    case Pid::PLACEMENT:
        return style().styleV(Sid::hairpinPlacement);

    default:
        return TextLineBase::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Hairpin::accessibleInfo() const
{
    String rez = TextLineBase::accessibleInfo();
    switch (hairpinType()) {
    case HairpinType::CRESC_HAIRPIN:
        rez += u": " + mtrc("engraving", "Crescendo");
        break;
    case HairpinType::DECRESC_HAIRPIN:
        rez += u": " + mtrc("engraving", "Decrescendo");
        break;
    default:
        rez += u": " + mtrc("engraving", "Custom");
    }
    return rez;
}
}
