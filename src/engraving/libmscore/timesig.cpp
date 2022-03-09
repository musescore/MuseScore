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

#include "translation.h"
#include "style/style.h"
#include "rw/xml.h"
#include "log.h"

#include "score.h"
#include "scorefont.h"
#include "symbol.h"
#include "staff.h"
#include "stafftype.h"
#include "segment.h"
#include "utils.h"
#include "masterscore.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
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

qreal TimeSig::mag() const
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

void TimeSig::setNumeratorString(const QString& a)
{
    if (_timeSigType == TimeSigType::NORMAL) {
        _numeratorString = a;
    }
}

//---------------------------------------------------------
//   setDenominatorString
//    setSig() has to be called first
//---------------------------------------------------------

void TimeSig::setDenominatorString(const QString& a)
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
    xml.startObject(this);
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

    xml.endObject();
}

//---------------------------------------------------------
//   TimeSig::read
//---------------------------------------------------------

void TimeSig::read(XmlReader& e)
{
    int n=0, z1=0, z2=0, z3=0, z4=0;
    bool old = false;

    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());

        if (tag == "den") {
            old = true;
            n = e.readInt();
        } else if (tag == "nom1") {
            old = true;
            z1 = e.readInt();
        } else if (tag == "nom2") {
            old = true;
            z2 = e.readInt();
        } else if (tag == "nom3") {
            old = true;
            z3 = e.readInt();
        } else if (tag == "nom4") {
            old = true;
            z4 = e.readInt();
        } else if (tag == "subtype") {
            int i = e.readInt();
            if (score()->mscVersion() <= 114) {
                if (i == 0x40000104) {
                    _timeSigType = TimeSigType::FOUR_FOUR;
                } else if (i == 0x40002084) {
                    _timeSigType = TimeSigType::ALLA_BREVE;
                } else {
                    _timeSigType = TimeSigType::NORMAL;
                }
            } else {
                _timeSigType = TimeSigType(i);
            }
        } else if (tag == "showCourtesySig") {
            _showCourtesySig = e.readInt();
        } else if (tag == "sigN") {
            _sig.setNumerator(e.readInt());
        } else if (tag == "sigD") {
            _sig.setDenominator(e.readInt());
        } else if (tag == "stretchN") {
            _stretch.setNumerator(e.readInt());
        } else if (tag == "stretchD") {
            _stretch.setDenominator(e.readInt());
        } else if (tag == "textN") {
            setNumeratorString(e.readElementText());
        } else if (tag == "textD") {
            setDenominatorString(e.readElementText());
        } else if (tag == "Groups") {
            _groups.read(e);
        } else if (readStyledProperty(e, tag)) {
        } else if (!EngravingItem::readProperties(e)) {
            e.unknown();
        }
    }
    if (old) {
        _sig.set(z1 + z2 + z3 + z4, n);
    }
    _stretch.reduce();

    // HACK: handle time signatures from scores before 3.5 differently on some special occasions.
    // See https://musescore.org/node/308139.
    QString version = mscoreVersion();
    if (!version.isEmpty() && (version >= "3.0") && (version < "3.5")) {
        if ((_timeSigType == TimeSigType::NORMAL) && !_numeratorString.isEmpty() && _denominatorString.isEmpty()) {
            if (_numeratorString == QString::number(_sig.numerator())) {
                _numeratorString.clear();
            } else {
                setDenominatorString(QString::number(_sig.denominator()));
            }
        }
    }
}

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid TimeSig::propertyId(const QStringRef& name) const
{
    if (name == "subtype") {
        return Pid::TIMESIG_TYPE;
    }
    if (name == "sigN" || name == "sigD") {
        return Pid::TIMESIG;
    }
    if (name == "stretchN" || name == "stretchD") {
        return Pid::TIMESIG_STRETCH;
    }
    if (name == "Groups") {
        return Pid::GROUP_NODES;
    }
    return EngravingItem::propertyId(name);
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TimeSig::layout()
{
    setPos(0.0, 0.0);
    qreal _spatium = spatium();

    setbbox(RectF());                    // prepare for an empty time signature
    pointLargeLeftParen = PointF();
    pz = PointF();
    pn = PointF();
    pointLargeRightParen = PointF();

    qreal lineDist;
    int numOfLines;
    TimeSigType sigType = timeSigType();
    const Staff* _staff       = staff();

    if (_staff) {
        // if staff is without time sig, format as if no text at all
        if (!_staff->staffTypeForElement(this)->genTimesig()) {
            // reset position and box sizes to 0
            // qDebug("staff: no time sig");
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

    qreal yoff = _spatium * (numOfLines - 1) * .5 * lineDist;

    // C and Ccut are placed at the middle of the staff: use yoff directly
    if (sigType == TimeSigType::FOUR_FOUR) {
        pz = PointF(0.0, yoff);
        setbbox(symBbox(SymId::timeSigCommon).translated(pz));
        ns.clear();
        ns.push_back(SymId::timeSigCommon);
        ds.clear();
    } else if (sigType == TimeSigType::ALLA_BREVE) {
        pz = PointF(0.0, yoff);
        setbbox(symBbox(SymId::timeSigCutCommon).translated(pz));
        ns.clear();
        ns.push_back(SymId::timeSigCutCommon);
        ds.clear();
    } else if (sigType == TimeSigType::CUT_BACH) {
        pz = PointF(0.0, yoff);
        setbbox(symBbox(SymId::timeSigCut2).translated(pz));
        ns.clear();
        ns.push_back(SymId::timeSigCut2);
        ds.clear();
    } else if (sigType == TimeSigType::CUT_TRIPLE) {
        pz = PointF(0.0, yoff);
        setbbox(symBbox(SymId::timeSigCut3).translated(pz));
        ns.clear();
        ns.push_back(SymId::timeSigCut3);
        ds.clear();
    } else {
        if (_numeratorString.isEmpty()) {
            ns = timeSigSymIdsFromString(_numeratorString.isEmpty() ? QString::number(_sig.numerator()) : _numeratorString);
            ds = timeSigSymIdsFromString(_denominatorString.isEmpty() ? QString::number(_sig.denominator()) : _denominatorString);
        } else {
            ns = timeSigSymIdsFromString(_numeratorString);
            ds = timeSigSymIdsFromString(_denominatorString);
        }

        ScoreFont* font = score()->scoreFont();
        SizeF mag(magS() * _scale);

        RectF numRect = font->bbox(ns, mag);
        RectF denRect = font->bbox(ds, mag);

        // position numerator and denominator; vertical displacement:
        // number of lines is odd: 0.0 (strings are directly above and below the middle line)
        // number of lines even:   0.05 (strings are moved up/down to leave 1/10sp between them)

        qreal displ = (numOfLines & 1) ? 0.0 : (0.05 * _spatium);

        //align on the wider
        qreal pzY = yoff - (denRect.width() < 0.01 ? 0.0 : (displ + numRect.height() * .5));
        qreal pnY = yoff + displ + denRect.height() * .5;

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
    TRACE_OBJ_DRAW;
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

QString TimeSig::ssig() const
{
    return QString("%1/%2").arg(_sig.numerator()).arg(_sig.denominator());
}

//---------------------------------------------------------
//   setSSig
//---------------------------------------------------------

void TimeSig::setSSig(const QString& s)
{
    QStringList sl = s.split("/");
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
        setNumeratorString(v.toString());
        break;
    case Pid::DENOMINATOR_STRING:
        setDenominatorString(v.toString());
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
        return QString();
    case Pid::DENOMINATOR_STRING:
        return QString();
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

QString TimeSig::accessibleInfo() const
{
    QString timeSigString;
    switch (timeSigType()) {
    case TimeSigType::FOUR_FOUR:
        timeSigString = qtrc("symUserNames", "Common time");
        break;
    case TimeSigType::ALLA_BREVE:
        timeSigString = qtrc("symUserNames", "Cut time");
        break;
    case TimeSigType::CUT_BACH:
        timeSigString = qtrc("symUserNames", "Cut time (Bach)");
        break;
    case TimeSigType::CUT_TRIPLE:
        timeSigString = qtrc("symUserNames", "Cut triple time (9/8)");
        break;
    default:
        timeSigString = qtrc("engraving", "%1/%2 time").arg(QString::number(numerator()), QString::number(denominator()));
    }
    return QString("%1: %2").arg(EngravingItem::accessibleInfo(), timeSigString);
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

    score()->setUpTempoMap();
}

void TimeSig::removed()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->setUpTempoMap();
}
}
