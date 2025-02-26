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

/**
 \file
 Implementation of classes Clef (partial) and ClefList (complete).
*/

#include "clef.h"

#include "translation.h"

#include "types/typesconv.h"

#include "ambitus.h"
#include "factory.h"
#include "measure.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
// table must be in sync with enum ClefType in types.h
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

    { ClefType::C4_8VB,  4, 30, { 6, 2, 5, 1, 4, 0, 3, 3, 0, 4, 1, 5, 2, 6 },  SymId::cClef8vb,         StaffGroup::STANDARD },
};

//---------------------------------------------------------
//   Clef
//---------------------------------------------------------

Clef::Clef(Segment* parent)
    : EngravingItem(ElementType::CLEF, parent, ElementFlag::ON_STAFF)
{
    m_clefToBarlinePosition = ClefToBarlinePosition::AUTO;
    m_isHeader = parent->isHeaderClefType();
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double Clef::mag() const
{
    double mag = staff() ? staff()->staffMag(tick()) : 1.0;
    if (m_isSmall) {
        mag *= style().styleD(Sid::smallClefMag);
    }
    return mag;
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
    m_isSmall = val;
}

//---------------------------------------------------------
//   setClefType
//---------------------------------------------------------

void Clef::setClefType(ClefType i)
{
    if (concertPitch()) {
        m_clefTypes.concertClef = i;
        if (m_clefTypes.transposingClef == ClefType::INVALID) {
            m_clefTypes.transposingClef = i;
        }
    } else {
        m_clefTypes.transposingClef = i;
        if (m_clefTypes.concertClef == ClefType::INVALID) {
            m_clefTypes.concertClef = i;
        }
    }
}

//---------------------------------------------------------
//   setConcertClef
//---------------------------------------------------------

void Clef::setConcertClef(ClefType val)
{
    m_clefTypes.concertClef = val;
}

//---------------------------------------------------------
//   setTransposingClef
//---------------------------------------------------------

void Clef::setTransposingClef(ClefType val)
{
    m_clefTypes.transposingClef = val;
}

//---------------------------------------------------------
//   clefType
//---------------------------------------------------------

ClefType Clef::clefType() const
{
    if (concertPitch()) {
        return m_clefTypes.concertClef;
    } else {
        return m_clefTypes.transposingClef;
    }
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Clef::spatiumChanged(double oldValue, double newValue)
{
    EngravingItem::spatiumChanged(oldValue, newValue);
    renderer()->layoutItem(this);
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
    case Pid::CLEF_TYPE_CONCERT:     return m_clefTypes.concertClef;
    case Pid::CLEF_TYPE_TRANSPOSING: return m_clefTypes.transposingClef;
    case Pid::SHOW_COURTESY: return showCourtesy();
    case Pid::SMALL:         return isSmall();
    case Pid::CLEF_TO_BARLINE_POS: return m_clefToBarlinePosition;
    case Pid::IS_HEADER: return m_isHeader;
    case Pid::IS_COURTESY: return m_isCourtesy;
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
    case Pid::SHOW_COURTESY:
        m_showCourtesy = v.toBool();
        if (m_showCourtesy && m_isHeader && selected()) {
            Clef* courtesyClef = otherClef();
            if (courtesyClef) {
                score()->deselect(this);
                score()->select(courtesyClef, SelectType::ADD, staffIdx());
            }
        }
        break;
    case Pid::SMALL:
        setSmall(v.toBool());
        break;
    case Pid::CLEF_TO_BARLINE_POS:
        if (v.value<ClefToBarlinePosition>() != m_clefToBarlinePosition && !m_isHeader) {
            changeClefToBarlinePos(v.value<ClefToBarlinePosition>());
        }
        break;
    case Pid::IS_HEADER:
        m_isHeader = v.toBool();
        break;
    case Pid::IS_COURTESY:
        m_isCourtesy = v.toBool();
        break;
    default:
        return EngravingItem::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

void Clef::changeClefToBarlinePos(ClefToBarlinePosition newPos)
{
    m_clefToBarlinePosition = newPos;

    if (!explicitParent()) {
        return;
    }

    Segment* seg = segment();

    staff_idx_t nStaves = score()->nstaves();
    for (staff_idx_t staffIndex = 0; staffIndex < nStaves; ++staffIndex) {
        Clef* clef = static_cast<Clef*>(seg->elementAt(staffIndex * VOICES));
        if (clef) {
            clef->m_clefToBarlinePosition = newPos;
        }
    }
}

void Clef::undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps)
{
    if (id == Pid::SHOW_COURTESY) {
        if (v.toBool() != m_showCourtesy) {
            score()->undo(new ChangeProperty(this, id, v, ps));
            Clef* pairedClef = otherClef();
            if (pairedClef) {
                score()->undo(new ChangeProperty(pairedClef, id, v, ps));
            }
            if (!segment()->isHeaderClefType()) {
                setGenerated(false);
            }
        }
    } else {
        EngravingObject::undoChangeProperty(id, v, ps);
    }
}

bool Clef::isMidMeasureClef() const
{
    return segment() && segment()->rtick().isNotZero();
}

void Clef::manageExclusionFromParts(bool exclude)
{
    if (exclude) {
        EngravingItem::manageExclusionFromParts(exclude);
    } else {
        Measure* measure = findMeasure();
        EngravingItem* nextEl = measure && rtick() == measure->ticks() ? measure->next() : nullptr;
        if (!nextEl) {
            nextEl = nextElement();
        }
        score()->undoChangeClef(staff(), nextEl, clefType(), false, this);
    }
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
    case Pid::CLEF_TO_BARLINE_POS: return ClefToBarlinePosition::AUTO;
    case Pid::IS_HEADER: return false;
    case Pid::IS_COURTESY: return false;
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
    return muse::mtrc("engraving", TConv::translatedUserName(clefType()));
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

TranslatableString Clef::subtypeUserName() const
{
    return TConv::userName(clefType());
}

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Clef::clear()
{
    LayoutData* ldata = mutldata();
    ldata->clearBbox();
    ldata->symId = SymId::noSym;
    Clef* pairedClef = otherClef();
    if (selected() && !m_isHeader && pairedClef) {
        score()->deselect(this);
        score()->select(pairedClef, SelectType::ADD, staffIdx());
    }
}
}
