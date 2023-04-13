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

#include "timesig.h"

#include "rw/xml.h"

#include "style/style.h"
#include "translation.h"
#include "iengravingfont.h"

#include "score.h"
#include "segment.h"
#include "staff.h"
#include "stafftype.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
static const ElementStyle timesigStyle {
    { Sid::timesigScale,                       Pid::SCALE },
};

//---------------------------------------------------------
//   TimeSig
//    Constructs an invalid time signature element.
//    After construction first call setTrack() then
//    call setSig().
//    Layout() is static and called in setSig().
//---------------------------------------------------------

TimeSig::TimeSig(Segment* parent)
    : EngravingItem(ElementType::TIMESIG, parent, ElementFlag::ON_STAFF | ElementFlag::MOVABLE)
{
    initElementStyle(&timesigStyle);

    _showCourtesySig = true;
    _stretch.set(1, 1);
    _sig.set(0, 1);                 // initialize to invalid
    _timeSigType      = TimeSigType::NORMAL;
    _largeParentheses = false;
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
    return staff() ? staff()->staffMag(tick()) : 1.0;
}

//---------------------------------------------------------
//   setSig
//    custom text has to be set after setSig()
//---------------------------------------------------------

void TimeSig::setSig(const Fraction& f, TimeSigType st)
{
    _sig              = f;
    _timeSigType      = st;
    _largeParentheses = false;
    _numeratorString.clear();
    _denominatorString.clear();
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
        score()->cmdAddTimeSig(measure(), staffIdx(), toTimeSig(e), false);
        return 0;
    }
    delete e;
    return 0;
}

//---------------------------------------------------------
//   setNumeratorString
//    setSig() has to be called first
//---------------------------------------------------------

void TimeSig::setNumeratorString(const String& a)
{
    if (_timeSigType == TimeSigType::NORMAL) {
        _numeratorString = a;
    }
}

//---------------------------------------------------------
//   setDenominatorString
//    setSig() has to be called first
//---------------------------------------------------------

void TimeSig::setDenominatorString(const String& a)
{
    if (_timeSigType == TimeSigType::NORMAL) {
        _denominatorString = a;
    }
}

//---------------------------------------------------------
//   write TimeSig
//---------------------------------------------------------

void TimeSig::write(XmlWriter& xml) const
{
    UNREACHABLE;
    xml.startElement(this);
    writeProperty(xml, Pid::TIMESIG_TYPE);
    EngravingItem::writeProperties(xml);

    xml.tag("sigN",  _sig.numerator());
    xml.tag("sigD",  _sig.denominator());
    if (stretch() != Fraction(1, 1)) {
        xml.tag("stretchN", stretch().numerator());
        xml.tag("stretchD", stretch().denominator());
    }
    writeProperty(xml, Pid::NUMERATOR_STRING);
    writeProperty(xml, Pid::DENOMINATOR_STRING);
    if (!_groups.empty()) {
        _groups.write(xml);
    }
    writeProperty(xml, Pid::SHOW_COURTESY);
    writeProperty(xml, Pid::SCALE);

    xml.endElement();
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TimeSig::layout()
{
    setPos(0.0, 0.0);
    double _spatium = spatium();

    setbbox(RectF());                    // prepare for an empty time signature
    pointLargeLeftParen = PointF();
    pz = PointF();
    pn = PointF();
    pointLargeRightParen = PointF();

    double lineDist;
    int numOfLines;
    TimeSigType sigType = timeSigType();
    const Staff* _staff       = staff();

    if (_staff) {
        // if staff is without time sig, format as if no text at all
        if (!_staff->staffTypeForElement(this)->genTimesig()) {
            // reset position and box sizes to 0
            // LOGD("staff: no time sig");
            pointLargeLeftParen.rx() = 0.0;
            pn.rx() = 0.0;
            pz.rx() = 0.0;
            pointLargeRightParen.rx() = 0.0;
            setbbox(RectF());
            // leave everything else as it is:
            // draw() will anyway skip any drawing if staff type has no time sigs
            return;
        }
        numOfLines  = _staff->lines(tick());
        lineDist    = _staff->lineDistance(tick());
    } else {
        // assume dimensions of a standard staff
        lineDist = 1.0;
        numOfLines = 5;
    }

    // if some symbol
    // compute vert. displacement to center in the staff height
    // determine middle staff position:

    double yoff = _spatium * (numOfLines - 1) * .5 * lineDist;

    // C and Ccut are placed at the middle of the staff: use yoff directly
    IEngravingFontPtr font = score()->engravingFont();
    SizeF mag(magS() * _scale);

    if (sigType == TimeSigType::FOUR_FOUR) {
        pz = PointF(0.0, yoff);
        RectF bbox = font->bbox(SymId::timeSigCommon, mag);
        setbbox(bbox.translated(pz));
        ns.clear();
        ns.push_back(SymId::timeSigCommon);
        ds.clear();
    } else if (sigType == TimeSigType::ALLA_BREVE) {
        pz = PointF(0.0, yoff);
        RectF bbox = font->bbox(SymId::timeSigCutCommon, mag);
        setbbox(bbox.translated(pz));
        ns.clear();
        ns.push_back(SymId::timeSigCutCommon);
        ds.clear();
    } else if (sigType == TimeSigType::CUT_BACH) {
        pz = PointF(0.0, yoff);
        RectF bbox = font->bbox(SymId::timeSigCut2, mag);
        setbbox(bbox.translated(pz));
        ns.clear();
        ns.push_back(SymId::timeSigCut2);
        ds.clear();
    } else if (sigType == TimeSigType::CUT_TRIPLE) {
        pz = PointF(0.0, yoff);
        RectF bbox = font->bbox(SymId::timeSigCut3, mag);
        setbbox(bbox.translated(pz));
        ns.clear();
        ns.push_back(SymId::timeSigCut3);
        ds.clear();
    } else {
        if (_numeratorString.isEmpty()) {
            ns = timeSigSymIdsFromString(_numeratorString.isEmpty() ? String::number(_sig.numerator()) : _numeratorString);
            ds = timeSigSymIdsFromString(_denominatorString.isEmpty() ? String::number(_sig.denominator()) : _denominatorString);
        } else {
            ns = timeSigSymIdsFromString(_numeratorString);
            ds = timeSigSymIdsFromString(_denominatorString);
        }

        RectF numRect = font->bbox(ns, mag);
        RectF denRect = font->bbox(ds, mag);

        // position numerator and denominator; vertical displacement:
        // number of lines is odd: 0.0 (strings are directly above and below the middle line)
        // number of lines even:   0.05 (strings are moved up/down to leave 1/10sp between them)

        double displ = (numOfLines & 1) ? 0.0 : (0.05 * _spatium);

        //align on the wider
        double pzY = yoff - (denRect.width() < 0.01 ? 0.0 : (displ + numRect.height() * .5));
        double pnY = yoff + displ + denRect.height() * .5;

        if (numRect.width() >= denRect.width()) {
            // numerator: one space above centre line, unless denomin. is empty (if so, directly centre in the middle)
            pz = PointF(0.0, pzY);
            // denominator: horiz: centred around centre of numerator | vert: one space below centre line
            pn = PointF((numRect.width() - denRect.width()) * .5, pnY);
        } else {
            // numerator: one space above centre line, unless denomin. is empty (if so, directly centre in the middle)
            pz = PointF((denRect.width() - numRect.width()) * .5, pzY);
            // denominator: horiz: centred around centre of numerator | vert: one space below centre line
            pn = PointF(0.0, pnY);
        }

        // centering of parenthesis so the middle of the parenthesis is at the divisor marking level
        int centerY = yoff / 2 + _spatium;
        int widestPortion = numRect.width() > denRect.width() ? numRect.width() : denRect.width();
        pointLargeLeftParen = PointF(-_spatium, centerY);
        pointLargeRightParen = PointF(widestPortion + _spatium, centerY);

        setbbox(numRect.translated(pz));       // translate bounding boxes to actual string positions
        addbbox(denRect.translated(pn));
        if (_largeParentheses) {
            addbbox(RectF(pointLargeLeftParen.x(), pointLargeLeftParen.y() - denRect.height(), _spatium / 2,
                          numRect.height() + denRect.height()));
            addbbox(RectF(pointLargeRightParen.x(), pointLargeRightParen.y() - denRect.height(),  _spatium / 2,
                          numRect.height() + denRect.height()));
        }
    }
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TimeSig::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    if (staff() && !const_cast<const Staff*>(staff())->staffType(tick())->genTimesig()) {
        return;
    }
    painter->setPen(curColor());

    drawSymbols(ns, painter, pz, _scale);
    drawSymbols(ds, painter, pn, _scale);

    if (_largeParentheses) {
        drawSymbol(SymId::timeSigParensLeft,  painter, pointLargeLeftParen,  _scale.width());
        drawSymbol(SymId::timeSigParensRight, painter, pointLargeRightParen, _scale.width());
    }
}

//---------------------------------------------------------
//   setFrom
//---------------------------------------------------------

void TimeSig::setFrom(const TimeSig* ts)
{
    _timeSigType       = ts->timeSigType();
    _numeratorString   = ts->_numeratorString;
    _denominatorString = ts->_denominatorString;
    _sig               = ts->_sig;
    _stretch           = ts->_stretch;
}

//---------------------------------------------------------
//   ssig
//---------------------------------------------------------

String TimeSig::ssig() const
{
    return String(u"%1/%2").arg(_sig.numerator()).arg(_sig.denominator());
}

//---------------------------------------------------------
//   setSSig
//---------------------------------------------------------

void TimeSig::setSSig(const String& s)
{
    StringList sl = s.split(u'/');
    if (sl.size() == 2) {
        _sig.setNumerator(sl[0].toInt());
        _sig.setDenominator(sl[1].toInt());
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
        return PropertyValue::fromValue(_sig);
    case Pid::TIMESIG_GLOBAL:
        return PropertyValue::fromValue(globalSig());
    case Pid::TIMESIG_STRETCH:
        return PropertyValue::fromValue(stretch());
    case Pid::TIMESIG_TYPE:
        return int(_timeSigType);
    case Pid::SCALE:
        return _scale;
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
    case Pid::TIMESIG_GLOBAL:
        setGlobalSig(v.value<Fraction>());
        break;
    case Pid::TIMESIG_STRETCH:
        setStretch(v.value<Fraction>());
        break;
    case Pid::TIMESIG_TYPE:
        _timeSigType = (TimeSigType)(v.toInt());
        break;
    case Pid::SCALE:
        _scale = v.value<ScaleF>();
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
    case Pid::TIMESIG_GLOBAL:
        return PropertyValue::fromValue(Fraction(1, 1));
    case Pid::TIMESIG_TYPE:
        return int(TimeSigType::NORMAL);
    case Pid::SCALE:
        return score()->styleV(Sid::timesigScale);
    default:
        return EngravingItem::propertyDefault(id);
    }
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
//   accessibleInfo
//---------------------------------------------------------

String TimeSig::accessibleInfo() const
{
    String timeSigString;
    switch (timeSigType()) {
    case TimeSigType::FOUR_FOUR:
        timeSigString = mtrc("engraving/timesig", "Common time");
        break;
    case TimeSigType::ALLA_BREVE:
        timeSigString = mtrc("engraving/timesig", "Cut time");
        break;
    case TimeSigType::CUT_BACH:
        timeSigString = mtrc("engraving/timesig", "Cut time (Bach)");
        break;
    case TimeSigType::CUT_TRIPLE:
        timeSigString = mtrc("engraving/timesig", "Cut triple time (9/8)");
        break;
    default:
        timeSigString = mtrc("engraving/timesig", "%1/%2 time").arg(numerator(), denominator());
    }
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), timeSigString);
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
           && (_numeratorString == ts._numeratorString)
           && (_denominatorString == ts._denominatorString)
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
