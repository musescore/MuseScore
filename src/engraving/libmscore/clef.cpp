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

/**
 \file
 Implementation of classes Clef (partial) and ClefList (complete).
*/

#include "clef.h"

#include "translation.h"

#include "rw/xml.h"

#include "types/typesconv.h"

#include "ambitus.h"
#include "factory.h"
#include "measure.h"
#include "part.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "stafftype.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
// table must be in sync with enum ClefType
const ClefInfo ClefInfo::clefTable[] = {
//                     line pOff|-lines for sharps---||---lines for flats--   |  symbol                | valid in staff group
    { ClefType::G,       2, 45, { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef,            StaffGroup::STANDARD },
    { ClefType::G15_MB,  2, 31, { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef15mb,        StaffGroup::STANDARD },
    { ClefType::G8_VB,   2, 38, { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef8vb,         StaffGroup::STANDARD },
    { ClefType::G8_VA,   2, 52, { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef8va,         StaffGroup::STANDARD },
    { ClefType::G15_MA,  2, 59, { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef15ma,        StaffGroup::STANDARD },
    { ClefType::G8_VB_O, 2, 38, { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef8vbOld,      StaffGroup::STANDARD },
    { ClefType::G8_VB_P, 2, 45, { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::gClef8vbParens,   StaffGroup::STANDARD },
    { ClefType::G_1,     1, 47, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 },  SymId::gClef,            StaffGroup::STANDARD },

    { ClefType::C1,      1, 43, { 5, 1, 4, 0, 3, -1, 2, 2, 6, 3, 7, 4, 8, 5 }, SymId::cClef,            StaffGroup::STANDARD },
    { ClefType::C2,      2, 41, { 3, 6, 2, 5, 1, 4, 0, 0, 4, 1, 5, 2, 6, 3 },  SymId::cClef,            StaffGroup::STANDARD },
    { ClefType::C3,      3, 39, { 1, 4, 0, 3, 6, 2, 5, 5, 2, 6, 3, 7, 4, 8 },  SymId::cClef,            StaffGroup::STANDARD },
    { ClefType::C4,      4, 37, { 6, 2, 5, 1, 4, 0, 3, 3, 0, 4, 1, 5, 2, 6 },  SymId::cClef,            StaffGroup::STANDARD },
    { ClefType::C5,      5, 35, { 4, 0, 3, -1, 2, 5, 1, 1, 5, 2, 6, 3, 7, 4 }, SymId::cClef,            StaffGroup::STANDARD },
    { ClefType::C_19C,   2, 45, { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::cClefSquare,      StaffGroup::STANDARD },
    { ClefType::C1_F18C, 1, 43, { 5, 1, 4, 0, 3, -1, 2, 2, 6, 3, 7, 4, 8, 5 }, SymId::cClefFrench,      StaffGroup::STANDARD },
    { ClefType::C3_F18C, 3, 39, { 1, 4, 0, 3, 6, 2, 5, 5, 2, 6, 3, 7, 4, 8 },  SymId::cClefFrench,      StaffGroup::STANDARD },
    { ClefType::C4_F18C, 4, 37, { 6, 2, 5, 1, 4, 0, 3, 3, 0, 4, 1, 5, 2, 6 },  SymId::cClefFrench,      StaffGroup::STANDARD },
    { ClefType::C1_F20C, 1, 43, { 5, 1, 4, 0, 3, -1, 2, 2, 6, 3, 7, 4, 8, 5 }, SymId::cClefFrench20C,   StaffGroup::STANDARD },
    { ClefType::C3_F20C, 3, 39, { 1, 4, 0, 3, 6, 2, 5, 5, 2, 6, 3, 7, 4, 8 },  SymId::cClefFrench20C,   StaffGroup::STANDARD },
    { ClefType::C4_F20C, 4, 37, { 6, 2, 5, 1, 4, 0, 3, 3, 0, 4, 1, 5, 2, 6 },  SymId::cClefFrench20C,   StaffGroup::STANDARD },

    { ClefType::F,       4, 33, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 },  SymId::fClef,            StaffGroup::STANDARD },
    { ClefType::F15_MB,  4, 19, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 },  SymId::fClef15mb,        StaffGroup::STANDARD },
    { ClefType::F8_VB,   4, 26, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 },  SymId::fClef8vb,         StaffGroup::STANDARD },
    { ClefType::F_8VA,   4, 40, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 },  SymId::fClef8va,         StaffGroup::STANDARD },
    { ClefType::F_15MA,  4, 47, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 },  SymId::fClef15ma,        StaffGroup::STANDARD },
    { ClefType::F_B,     3, 35, { 4, 0, 3, -1, 2, 5, 1, 1, 5, 2, 6, 3, 7, 4 }, SymId::fClef,            StaffGroup::STANDARD },
    { ClefType::F_C,     5, 31, { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::fClef,            StaffGroup::STANDARD },
    { ClefType::F_F18C,  4, 33, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 },  SymId::fClefFrench,      StaffGroup::STANDARD },
    { ClefType::F_19C,   4, 33, { 2, 5, 1, 4, 7, 3, 6, 6, 3, 7, 4, 8, 5, 9 },  SymId::fClef19thCentury, StaffGroup::STANDARD },

    { ClefType::PERC,    2, 45, { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::unpitchedPercussionClef1, StaffGroup::PERCUSSION },
    { ClefType::PERC2,   2, 45, { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::unpitchedPercussionClef2, StaffGroup::PERCUSSION },

    { ClefType::TAB,     5, 45,  { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::sixStringTabClef,         StaffGroup::TAB },
    { ClefType::TAB4,    5, 45,  { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::fourStringTabClef,        StaffGroup::TAB },
    { ClefType::TAB_SERIF, 5, 45,  { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::sixStringTabClefSerif,  StaffGroup::TAB },
    { ClefType::TAB4_SERIF, 5, 45,  { 0, 3, -1, 2, 5, 1, 4, 4, 1, 5, 2, 6, 3, 7 }, SymId::fourStringTabClefSerif, StaffGroup::TAB },
};

//---------------------------------------------------------
//   Clef
//---------------------------------------------------------

Clef::Clef(Segment* parent)
    : EngravingItem(ElementType::CLEF, parent, ElementFlag::ON_STAFF), symId(SymId::noSym)
{}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double Clef::mag() const
{
    double mag = staff() ? staff()->staffMag(tick()) : 1.0;
    if (m_isSmall) {
        mag *= score()->styleD(Sid::smallClefMag);
    }
    return mag;
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Clef::layout()
{
    // determine current number of lines and line distance
    int lines;
    double lineDist;
    Segment* clefSeg  = segment();
    int stepOffset;

    // check clef visibility and type compatibility
    if (clefSeg && staff()) {
        Fraction tick = clefSeg->tick();
        const StaffType* st = staff()->staffType(tick);
        bool show     = st->genClef();            // check staff type allows clef display
        StaffGroup staffGroup = st->group();

        // if not tab, use instrument->useDrumset to set staffGroup (to allow pitched to unpitched in same staff)
        if (staffGroup != StaffGroup::TAB) {
            staffGroup = staff()->part()->instrument(this->tick())->useDrumset() ? StaffGroup::PERCUSSION : StaffGroup::STANDARD;
        }

        // check clef is compatible with staff type group:
        if (ClefInfo::staffGroup(clefType()) != staffGroup) {
            if (tick > Fraction(0, 1) && !generated()) {     // if clef is not generated, hide it
                show = false;
            } else {                            // if generated, replace with initial clef type
                // TODO : instead of initial staff clef (which is assumed to be compatible)
                // use the last compatible clef previously found in staff
                _clefTypes = staff()->clefType(Fraction(0, 1));
            }
        }

        // if clef not to show or not compatible with staff group
        if (!show) {
            setbbox(RectF());
            symId = SymId::noSym;
            LOGD("Clef::layout(): invisible clef at tick %d(%d) staff %zu",
                 segment()->tick().ticks(), segment()->tick().ticks() / 1920, staffIdx());
            return;
        }
        lines      = st->lines();             // init values from staff type
        lineDist   = st->lineDistance().val();
        stepOffset = st->stepOffset();
    } else {
        lines      = 5;
        lineDist   = 1.0;
        stepOffset = 0;
    }

    double _spatium = spatium();
    double yoff     = 0.0;
    if (clefType() != ClefType::INVALID && clefType() != ClefType::MAX) {
        symId = ClefInfo::symId(clefType());
        yoff = lineDist * (5 - ClefInfo::line(clefType()));
    } else {
        symId = SymId::noSym;
    }

    switch (clefType()) {
    case ClefType::C_19C:                                    // 19th C clef is like a G clef
        yoff = lineDist * 1.5;
        break;
    case ClefType::TAB:                                    // TAB clef
        // on tablature, position clef at half the number of spaces * line distance
        yoff = lineDist * (lines - 1) * .5;
        stepOffset = 0;           //  ignore stepOffset for TAB and percussion clefs
        break;
    case ClefType::TAB4:                                    // TAB clef 4 strings
        // on tablature, position clef at half the number of spaces * line distance
        yoff = lineDist * (lines - 1) * .5;
        stepOffset = 0;
        break;
    case ClefType::TAB_SERIF:                                   // TAB clef alternate style
        // on tablature, position clef at half the number of spaces * line distance
        yoff = lineDist * (lines - 1) * .5;
        stepOffset = 0;
        break;
    case ClefType::TAB4_SERIF:                                   // TAB clef alternate style
        // on tablature, position clef at half the number of spaces * line distance
        yoff = lineDist * (lines - 1) * .5;
        stepOffset = 0;
        break;
    case ClefType::PERC:                                   // percussion clefs
        yoff = lineDist * (lines - 1) * 0.5;
        stepOffset = 0;
        break;
    case ClefType::PERC2:
        yoff = lineDist * (lines - 1) * 0.5;
        stepOffset = 0;
        break;
    case ClefType::INVALID:
    case ClefType::MAX:
        LOGD("Clef::layout: invalid type");
        return;
    default:
        break;
    }
    // clefs on palette or at start of system/measure are left aligned
    // other clefs are right aligned
    RectF r(symBbox(symId));
    double x = segment() && segment()->rtick().isNotZero() ? -r.right() : 0.0;
    setPos(x, yoff * _spatium + (stepOffset * 0.5 * _spatium));

    setbbox(r);
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Clef::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    if (symId == SymId::noSym || (staff() && !const_cast<const Staff*>(staff())->staffType(tick())->genClef())) {
        return;
    }
    painter->setPen(curColor());
    drawSymbol(symId, painter);
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Clef::acceptDrop(EditData& data) const
{
    return data.dropElement->type() == ElementType::CLEF
           || (/*!generated() &&*/ data.dropElement->type() == ElementType::AMBITUS);
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* Clef::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    Clef* c = 0;
    if (e->isClef()) {
        Clef* clef = toClef(e);
        ClefType stype  = clef->clefType();
        if (clefType() != stype) {
            score()->undoChangeClef(staff(), this, stype);
            c = this;
        }
    } else if (e->isAmbitus()) {
        /*if (!generated())*/
        {
            Measure* meas  = measure();
            Segment* segm  = meas->getSegment(SegmentType::Ambitus, meas->tick());
            if (segm->element(track())) {
                score()->undoRemoveElement(segm->element(track()));
            }
            Ambitus* r = Factory::createAmbitus(segm);
            r->setParent(segm);
            r->setTrack(track());
            score()->undoAddElement(r);
        }
    }
    delete e;
    return c;
}

//---------------------------------------------------------
//   setSmall
//---------------------------------------------------------

void Clef::setSmall(bool val)
{
    if (val != m_isSmall) {
        m_isSmall = val;
    }
}

//---------------------------------------------------------
//   setClefType
//---------------------------------------------------------

void Clef::setClefType(ClefType i)
{
    if (concertPitch()) {
        _clefTypes._concertClef = i;
        if (_clefTypes._transposingClef == ClefType::INVALID) {
            _clefTypes._transposingClef = i;
        }
    } else {
        _clefTypes._transposingClef = i;
        if (_clefTypes._concertClef == ClefType::INVALID) {
            _clefTypes._concertClef = i;
        }
    }
}

//---------------------------------------------------------
//   setConcertClef
//---------------------------------------------------------

void Clef::setConcertClef(ClefType val)
{
    _clefTypes._concertClef = val;
}

//---------------------------------------------------------
//   setTransposingClef
//---------------------------------------------------------

void Clef::setTransposingClef(ClefType val)
{
    _clefTypes._transposingClef = val;
}

//---------------------------------------------------------
//   clefType
//---------------------------------------------------------

ClefType Clef::clefType() const
{
    if (concertPitch()) {
        return _clefTypes._concertClef;
    } else {
        return _clefTypes._transposingClef;
    }
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Clef::spatiumChanged(double oldValue, double newValue)
{
    EngravingItem::spatiumChanged(oldValue, newValue);
    layout();
}

//---------------------------------------------------------
//   undoSetShowCourtesy
//---------------------------------------------------------

void Clef::undoSetShowCourtesy(bool v)
{
    undoChangeProperty(Pid::SHOW_COURTESY, v);
}

//---------------------------------------------------------
//   otherClef
//    try to locate the 'other clef' of a courtesy / main pair
//---------------------------------------------------------

Clef* Clef::otherClef()
{
    // if not in a clef-segment-measure hierarchy, do nothing
    if (!explicitParent() || !explicitParent()->isSegment()) {
        return nullptr;
    }
    Segment* segm = toSegment(explicitParent());
    if (!segm->explicitParent() || !segm->explicitParent()->isMeasure()) {
        return 0;
    }
    Measure* meas = toMeasure(segm->explicitParent());
    Measure* otherMeas = nullptr;
    Segment* otherSegm = nullptr;
    Fraction segmTick  = segm->tick();
    SegmentType type = SegmentType::Clef;
    if (segmTick == meas->tick() && segm->segmentType() == SegmentType::HeaderClef) { // if clef segm is measure-initial
        otherMeas = meas->prevMeasure();                                              // look for a previous measure
    } else if (segmTick == meas->tick() + meas->ticks()) {                            // if clef segm is measure-final
        otherMeas = meas->nextMeasure();                                              // look for a next measure
        type = SegmentType::HeaderClef;
    }
    if (!otherMeas) {
        return nullptr;
    }
    // look for a clef segment in the 'other' measure at the same tick of this clef segment
    otherSegm = otherMeas->findSegment(type, segmTick);
    if (!otherSegm) {
        return nullptr;
    }
    // if any 'other' segment found, look for a clef in the same track as this
    return toClef(otherSegm->element(track()));
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Clef::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::CLEF_TYPE_CONCERT:     return _clefTypes._concertClef;
    case Pid::CLEF_TYPE_TRANSPOSING: return _clefTypes._transposingClef;
    case Pid::SHOW_COURTESY: return showCourtesy();
    case Pid::SMALL:         return isSmall();
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Clef::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::CLEF_TYPE_CONCERT:
        setConcertClef(v.value<ClefType>());
        break;
    case Pid::CLEF_TYPE_TRANSPOSING:
        setTransposingClef(v.value<ClefType>());
        break;
    case Pid::SHOW_COURTESY: _showCourtesy = v.toBool();
        break;
    case Pid::SMALL:         setSmall(v.toBool());
        break;
    default:
        return EngravingItem::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Clef::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::CLEF_TYPE_CONCERT:     return ClefType::INVALID;
    case Pid::CLEF_TYPE_TRANSPOSING: return ClefType::INVALID;
    case Pid::SHOW_COURTESY: return true;
    case Pid::SMALL:         return false;
    default:              return EngravingItem::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

EngravingItem* Clef::nextSegmentElement()
{
    return segment()->firstInNextSegments(staffIdx());
}

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

EngravingItem* Clef::prevSegmentElement()
{
    return segment()->lastInPrevSegments(staffIdx());
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Clef::accessibleInfo() const
{
    ClefType type = clefType();
    if (type == ClefType::INVALID) {
        return String();
    }
    return mtrc("engraving", TConv::translatedUserName(clefType()));
}

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Clef::clear()
{
    setbbox(RectF());
    symId = SymId::noSym;
}
}
