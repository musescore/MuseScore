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

#include "timesig.h"

#include <functional>

#include "style/style.h"

#include "score.h"
#include "segment.h"
#include "staff.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
static const ElementStyle tsStyle {
    { Sid::timeSigNormalScale, Pid::SCALE },
};

//---------------------------------------------------------
//   TimeSig
//    Constructs an invalid time signature element.
//    After construction first call setTrack() then
//    call setSig().
//    Layout() is static and called in setSig().
//---------------------------------------------------------

TimeSig::TimeSig(Segment* parent)
    : EngravingItem(ElementType::TIMESIG, parent, ElementFlag::ON_STAFF | ElementFlag::MOVABLE | ElementFlag::PLACE_ABOVE)
{
    initElementStyle(&tsStyle);

    m_showCourtesySig = true;
    m_stretch.set(1, 1);
    m_sig.set(0, 1);                 // initialize to invalid
    m_timeSigType      = TimeSigType::NORMAL;
    m_largeParentheses = false;
    setMinDistance(Spatium(0.5)); // TODO: style
}

void TimeSig::setParent(Segment* parent)
{
    EngravingItem::setParent(parent);
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double TimeSig::mag() const
{
    return timeSigPlacement() == TimeSigPlacement::NORMAL && staff() ? staff()->staffMag(tick()) : 1.0;
}

//---------------------------------------------------------
//   setSig
//    custom text has to be set after setSig()
//---------------------------------------------------------

void TimeSig::setSig(const Fraction& f, TimeSigType st)
{
    m_sig              = f;
    m_timeSigType      = st;
    m_largeParentheses = false;
    m_numeratorString.clear();
    m_denominatorString.clear();
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool TimeSig::acceptDrop(EditData& data) const
{
    return data.dropElement->isTimeSig();
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* TimeSig::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    if (e->isTimeSig()) {
        // change timesig applies to all staves, can't simply set subtype
        // for this one only
        // ownership of e is transferred to cmdAddTimeSig

        if (tick() != measure()->endTick()) {
            score()->cmdAddTimeSig(measure(), staffIdx(), toTimeSig(e), false);
            return nullptr;
        }

        // This is a timesig at the end of a measure.
        if (*toTimeSig(e) == *this) {
            delete e;
            return nullptr;
        }

        if (!measure()->nextMeasure()) {
            return nullptr;
        }

        // Apply change to next measure
        score()->cmdAddTimeSig(measure()->nextMeasure(), staffIdx(), toTimeSig(e), false);
        return nullptr;
    }
    delete e;
    return nullptr;
}

//---------------------------------------------------------
//   setNumeratorString
//    setSig() has to be called first
//---------------------------------------------------------

void TimeSig::setNumeratorString(const String& a)
{
    if (m_timeSigType == TimeSigType::NORMAL) {
        m_numeratorString = a;
    }
}

//---------------------------------------------------------
//   setDenominatorString
//    setSig() has to be called first
//---------------------------------------------------------

void TimeSig::setDenominatorString(const String& a)
{
    if (m_timeSigType == TimeSigType::NORMAL) {
        m_denominatorString = a;
    }
}

//---------------------------------------------------------
//   setFrom
//---------------------------------------------------------

void TimeSig::setFrom(const TimeSig* ts)
{
    m_timeSigType       = ts->timeSigType();
    m_numeratorString   = ts->m_numeratorString;
    m_denominatorString = ts->m_denominatorString;
    m_sig               = ts->m_sig;
    m_stretch           = ts->m_stretch;
}

//---------------------------------------------------------
//   ssig
//---------------------------------------------------------

String TimeSig::ssig() const
{
    return String(u"%1/%2").arg(m_sig.numerator()).arg(m_sig.denominator());
}

//---------------------------------------------------------
//   setSSig
//---------------------------------------------------------

void TimeSig::setSSig(const String& s)
{
    StringList sl = s.split(u'/');
    if (sl.size() == 2) {
        m_sig.setNumerator(sl[0].toInt());
        m_sig.setDenominator(sl[1].toInt());
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue TimeSig::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SHOW_COURTESY:
        return int(showCourtesySig());
    case Pid::NUMERATOR_STRING:
        return numeratorString();
    case Pid::DENOMINATOR_STRING:
        return denominatorString();
    case Pid::GROUP_NODES:
        return groups().nodes();
    case Pid::TIMESIG:
        return PropertyValue::fromValue(m_sig);
    case Pid::TIMESIG_STRETCH:
        return PropertyValue::fromValue(stretch());
    case Pid::TIMESIG_TYPE:
        return int(m_timeSigType);
    case Pid::SCALE:
        return m_scale;
    case Pid::IS_COURTESY:
        return _isCourtesy;

    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TimeSig::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::SHOW_COURTESY:
        if (generated()) {
            return false;
        }
        setShowCourtesySig(v.toBool());
        break;
    case Pid::NUMERATOR_STRING:
        setNumeratorString(v.value<String>());
        break;
    case Pid::DENOMINATOR_STRING:
        setDenominatorString(v.value<String>());
        break;
    case Pid::GROUP_NODES:
        setGroups(v.value<GroupNodes>());
        break;
    case Pid::TIMESIG:
        setSig(v.value<Fraction>());
        break;
    case Pid::TIMESIG_STRETCH:
        setStretch(v.value<Fraction>());
        break;
    case Pid::TIMESIG_TYPE:
        m_timeSigType = (TimeSigType)(v.toInt());
        break;
    case Pid::SCALE:
        m_scale = v.value<ScaleF>();
        break;
    case Pid::IS_COURTESY:
        _isCourtesy = v.toBool();
        break;
    default:
        if (!EngravingItem::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    triggerLayoutAll();        // TODO
    setGenerated(false);
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue TimeSig::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::SHOW_COURTESY:
        return 1;
    case Pid::NUMERATOR_STRING:
        return String();
    case Pid::DENOMINATOR_STRING:
        return String();
    case Pid::TIMESIG:
        return PropertyValue::fromValue(Fraction(4, 4));
    case Pid::TIMESIG_TYPE:
        return int(TimeSigType::NORMAL);
    case Pid::PLACEMENT:
        return PlacementV::ABOVE;
    case Pid::IS_COURTESY:
        return false;
    default:
        return EngravingItem::propertyDefault(id);
    }
}

PointF TimeSig::staffOffset() const
{
    const Segment* seg = segment();
    const Measure* meas = seg ? seg->measure() : nullptr;
    const Fraction tsTick = meas ? meas->tick() : tick();
    const StaffType* st = staff()->constStaffType(tsTick);
    const double yOffset = st ? st->yoffset().val() * spatium() : 0.0;
    return PointF(0.0, yOffset);
}

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

EngravingItem* TimeSig::nextSegmentElement()
{
    return segment()->firstInNextSegments(staffIdx());
}

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

EngravingItem* TimeSig::prevSegmentElement()
{
    return segment()->lastInPrevSegments(staffIdx());
}

//---------------------------------------------------------
//   subtype
//---------------------------------------------------------

int TimeSig::subtype() const
{
    size_t h1 = std::hash<int> {}(numerator());
    size_t h2 = std::hash<int> {}(denominator());
    size_t h3 = std::hash<TimeSigType> {}(timeSigType());

    return static_cast<int>(h1 ^ (h2 << 1) ^ (h3 << 2));
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString TimeSig::subtypeUserName() const
{
    switch (timeSigType()) {
    case TimeSigType::FOUR_FOUR:
        return TranslatableString("engraving/timesig", "Common time");
    case TimeSigType::ALLA_BREVE:
        return TranslatableString("engraving/timesig", "Cut time");
    case TimeSigType::CUT_BACH:
        return TranslatableString("engraving/timesig", "Cut time (Bach)");
    case TimeSigType::CUT_TRIPLE:
        return TranslatableString("engraving/timesig", "Cut triple time (9/8)");
    default:
        return TranslatableString("engraving/timesig", "%1/%2 time").arg(numerator(), denominator());
    }
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String TimeSig::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), translatedSubtypeUserName());
}

void TimeSig::initElementStyle(const ElementStyle* elementStype)
{
    EngravingItem::initElementStyle(elementStype);

    m_scale = propertyDefault(Pid::SCALE).value<ScaleF>();
}

void TimeSig::styleChanged()
{
    if (isStyled(Pid::SCALE)) {
        m_scale = propertyDefault(Pid::SCALE).value<ScaleF>();
    }
    EngravingItem::styleChanged();
}

Sid TimeSig::getPropertyStyle(Pid id) const
{
    if (id == Pid::SCALE) {
        switch (timeSigPlacement()) {
        case TimeSigPlacement::NORMAL: return Sid::timeSigNormalScale;
        case TimeSigPlacement::ABOVE_STAVES: return Sid::timeSigAboveScale;
        case TimeSigPlacement::ACROSS_STAVES: return Sid::timeSigAcrossScale;
        default:
            return Sid::NOSTYLE;
        }
    }

    return EngravingItem::getPropertyStyle(id);
}

TimeSigPlacement TimeSig::timeSigPlacement() const
{
    return style().styleV(Sid::timeSigPlacement).value<TimeSigPlacement>();
}

TimeSigStyle TimeSig::timeSigStyle() const
{
    switch (timeSigPlacement()) {
    case TimeSigPlacement::NORMAL: return style().styleV(Sid::timeSigNormalStyle).value<TimeSigStyle>();
    case TimeSigPlacement::ABOVE_STAVES: return style().styleV(Sid::timeSigAboveStyle).value<TimeSigStyle>();
    case TimeSigPlacement::ACROSS_STAVES: return style().styleV(Sid::timeSigAcrossStyle).value<TimeSigStyle>();
    default:
        return TimeSigStyle::NORMAL;
    }
}

double TimeSig::numDist() const
{
    switch (timeSigPlacement()) {
    case TimeSigPlacement::NORMAL: return style().styleMM(Sid::timeSigNormalNumDist);
    case TimeSigPlacement::ABOVE_STAVES: return style().styleMM(Sid::timeSigAboveNumDist);
    case TimeSigPlacement::ACROSS_STAVES: return style().styleMM(Sid::timeSigAcrossNumDist);
    default:
        return 0.0;
    }
}

double TimeSig::yPos() const
{
    switch (timeSigPlacement()) {
    case TimeSigPlacement::NORMAL: return style().styleMM(Sid::timeSigNormalY);
    case TimeSigPlacement::ABOVE_STAVES: return (staff()->hasSystemObjectsBelowBottomStaff() ? -1.0 : 1.0)
               * style().styleMM(Sid::timeSigAboveY);
    case TimeSigPlacement::ACROSS_STAVES: return style().styleMM(Sid::timeSigAcrossY);
    default:
        return 0.0;
    }
}

bool TimeSig::showOnThisStaff() const
{
    return timeSigPlacement() == TimeSigPlacement::NORMAL || staffIdx() == 0 || staff()->isSystemObjectStaff();
}

bool TimeSig::isAboveStaves() const
{
    return timeSigPlacement() == TimeSigPlacement::ABOVE_STAVES;
}

bool TimeSig::isAcrossStaves() const
{
    return timeSigPlacement() == TimeSigPlacement::ACROSS_STAVES;
}

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool TimeSig::operator==(const TimeSig& ts) const
{
    return (timeSigType() == ts.timeSigType())
           && (sig().identical(ts.sig()))
           && (stretch() == ts.stretch())
           && (groups() == ts.groups())
           && (m_numeratorString == ts.m_numeratorString)
           && (m_denominatorString == ts.m_denominatorString)
    ;
}

void TimeSig::added()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->setUpTempoMapLater();
}

void TimeSig::removed()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->setUpTempoMapLater();
}
}
