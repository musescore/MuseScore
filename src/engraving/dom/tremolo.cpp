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

#include "tremolo.h"

#include "draw/types/brush.h"
#include "draw/types/pen.h"
#include "draw/types/transform.h"

#include "types/translatablestring.h"
#include "types/typesconv.h"

#include "style/style.h"

#include "rendering/dev/beamtremololayout.h"

#include "beam.h"
#include "chord.h"
#include "stem.h"
#include "system.h"
#include "stafftype.h"

#include "tremolotwochord.h"
#include "tremolosinglechord.h"

#include "log.h"

using namespace mu;
using namespace mu::draw;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   tremoloStyle
//---------------------------------------------------------

static const ElementStyle TREMOLO_STYLE {
    { Sid::tremoloStyle, Pid::TREMOLO_STYLE }
};

//---------------------------------------------------------
//   Tremolo
//---------------------------------------------------------

Tremolo::Tremolo(Chord* parent)
    : EngravingItem(ElementType::TREMOLO, parent, ElementFlag::MOVABLE)
{
    //initElementStyle(&TREMOLO_STYLE);
}

Tremolo::Tremolo(const Tremolo& t)
    : EngravingItem(t)
{
    m_tremoloType = t.tremoloType();
    if (twoNotes()) {
        m_tremoloTwoChord = new TremoloTwoChord(*t.m_tremoloTwoChord);
        m_tremoloTwoChord->dispatcher = this;
    } else {
        m_tremoloSingleChord = new TremoloSingleChord(*t.m_tremoloSingleChord);
        m_tremoloSingleChord->dispatcher = this;
    }

    styleChanged();
}

Tremolo::~Tremolo()
{
    if (m_tremoloTwoChord) {
        m_tremoloTwoChord->setParent(nullptr);
    }

    if (m_tremoloSingleChord) {
        m_tremoloSingleChord->setParent(nullptr);
    }

    delete m_tremoloTwoChord;
    delete m_tremoloSingleChord;
}

bool Tremolo::twoNotes() const
{
    DO_ASSERT(m_tremoloType != TremoloType::INVALID_TREMOLO);
    return m_tremoloType >= TremoloType::C8;
}

void Tremolo::setTrack(track_idx_t val)
{
    EngravingItem::setTrack(val);

    if (m_tremoloType == TremoloType::INVALID_TREMOLO) {
        return;
    }

    if (twoNotes()) {
        m_tremoloTwoChord->setTrack(val);
    } else {
        m_tremoloSingleChord->setTrack(val);
    }
}

void Tremolo::setTremoloType(TremoloType t)
{
    m_tremoloType = t;

    if (twoNotes()) {
        m_tremoloTwoChord = new TremoloTwoChord(toChord(this->parent()));
        m_tremoloTwoChord->dispatcher = this;
        m_tremoloTwoChord->setTrack(track());
        m_tremoloTwoChord->setTremoloType(t);
    } else {
        m_tremoloSingleChord = new TremoloSingleChord(toChord(this->parent()));
        m_tremoloSingleChord->dispatcher = this;
        m_tremoloSingleChord->setTrack(track());
        m_tremoloSingleChord->setTremoloType(t);
    }

    styleChanged();
}

void Tremolo::setParent(Chord* ch)
{
    EngravingItem::setParent(ch);

    if (twoNotes()) {
        m_tremoloTwoChord->setParent(ch);
    } else {
        m_tremoloSingleChord->setParent(ch);
    }
}

//---------------------------------------------------------
//   chordMag
//---------------------------------------------------------

double Tremolo::chordMag() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->chordMag();
    } else {
        return m_tremoloSingleChord->chordMag();
    }
}

//---------------------------------------------------------
//   minHeight
//---------------------------------------------------------

double Tremolo::minHeight() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->minHeight();
    } else {
        return m_tremoloSingleChord->minHeight();
    }
}

//---------------------------------------------------------
//   chordBeamAnchor
//---------------------------------------------------------

PointF Tremolo::chordBeamAnchor(const ChordRest* chord, ChordBeamAnchorType anchorType) const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->chordBeamAnchor(chord, anchorType);
    } else {
        return m_tremoloSingleChord->chordBeamAnchor(chord, anchorType);
    }
}

double Tremolo::beamWidth() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->beamWidth();
    } else {
        return m_tremoloSingleChord->beamWidth();
    }
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

RectF Tremolo::drag(EditData& ed)
{
    if (twoNotes()) {
        return m_tremoloTwoChord->drag(ed);
    } else {
        return m_tremoloSingleChord->drag(ed);
    }
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Tremolo::spatiumChanged(double oldValue, double newValue)
{
    if (twoNotes()) {
        m_tremoloTwoChord->spatiumChanged(oldValue, newValue);
    } else {
        m_tremoloSingleChord->spatiumChanged(oldValue, newValue);
    }

    EngravingItem::spatiumChanged(oldValue, newValue);
    computeShape();
}

//---------------------------------------------------------
//   localSpatiumChanged
//    the scale of a staff changed
//---------------------------------------------------------

void Tremolo::localSpatiumChanged(double oldValue, double newValue)
{
    if (twoNotes()) {
        m_tremoloTwoChord->localSpatiumChanged(oldValue, newValue);
    } else {
        m_tremoloSingleChord->localSpatiumChanged(oldValue, newValue);
    }

    EngravingItem::localSpatiumChanged(oldValue, newValue);
    computeShape();
}

//---------------------------------------------------------
//   styleChanged
//    the scale of a staff changed
//---------------------------------------------------------

void Tremolo::styleChanged()
{
    if (twoNotes()) {
        m_tremoloTwoChord->styleChanged();
    } else {
        m_tremoloSingleChord->styleChanged();
    }

    EngravingItem::styleChanged();
    computeShape();
}

//---------------------------------------------------------
//   basePath
//---------------------------------------------------------

PainterPath Tremolo::basePath(double stretch) const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->basePath(stretch);
    } else {
        return m_tremoloSingleChord->basePath(stretch);
    }
}

const mu::draw::PainterPath& Tremolo::path() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->path();
    } else {
        return m_tremoloSingleChord->path();
    }
}

void Tremolo::setPath(const mu::draw::PainterPath& p)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setPath(p);
    } else {
        m_tremoloSingleChord->setPath(p);
    }
}

const mu::PointF& Tremolo::startAnchor() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->startAnchor();
    } else {
        return m_tremoloSingleChord->startAnchor();
    }
}

mu::PointF& Tremolo::startAnchor()
{
    if (twoNotes()) {
        return m_tremoloTwoChord->startAnchor();
    } else {
        return m_tremoloSingleChord->startAnchor();
    }
}

void Tremolo::setStartAnchor(const mu::PointF& p)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setStartAnchor(p);
    } else {
        m_tremoloSingleChord->setStartAnchor(p);
    }
}

const mu::PointF& Tremolo::endAnchor() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->endAnchor();
    } else {
        return m_tremoloSingleChord->endAnchor();
    }
}

mu::PointF& Tremolo::endAnchor()
{
    if (twoNotes()) {
        return m_tremoloTwoChord->endAnchor();
    } else {
        return m_tremoloSingleChord->endAnchor();
    }
}

void Tremolo::setEndAnchor(const mu::PointF& p)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setEndAnchor(p);
    } else {
        m_tremoloSingleChord->setEndAnchor(p);
    }
}

//---------------------------------------------------------
//   computeShape
//---------------------------------------------------------

void Tremolo::computeShape()
{
    RectF bbox;
    if (twoNotes()) {
        m_tremoloTwoChord->computeShape();
        bbox = m_tremoloTwoChord->ldata()->bbox();
    } else {
        m_tremoloSingleChord->computeShape();
        bbox = m_tremoloSingleChord->ldata()->bbox();
    }

    setbbox(bbox);
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Tremolo::reset()
{
    if (twoNotes()) {
        m_tremoloTwoChord->reset();
    } else {
        m_tremoloSingleChord->reset();
    }

    setGenerated(true);
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

PointF Tremolo::pagePos() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->pagePos();
    } else {
        return m_tremoloSingleChord->pagePos();
    }
}

TremoloStyle Tremolo::tremoloStyle() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->tremoloStyle();
    } else {
        return m_tremoloSingleChord->tremoloStyle();
    }
}

void Tremolo::setTremoloStyle(TremoloStyle v)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setTremoloStyle(v);
    } else {
        m_tremoloSingleChord->setTremoloStyle(v);
    }
}

//---------------------------------------------------------
//   setBeamDirection
//---------------------------------------------------------

void Tremolo::setBeamDirection(DirectionV d)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setBeamDirection(d);
    } else {
        m_tremoloSingleChord->setBeamDirection(d);
    }
}

void Tremolo::setBeamFragment(const BeamFragment& bf)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setBeamFragment(bf);
    } else {
        m_tremoloSingleChord->setBeamFragment(bf);
    }
}

const BeamFragment& Tremolo::beamFragment() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->beamFragment();
    } else {
        return m_tremoloSingleChord->beamFragment();
    }
}

BeamFragment& Tremolo::beamFragment()
{
    if (twoNotes()) {
        return m_tremoloTwoChord->beamFragment();
    } else {
        return m_tremoloSingleChord->beamFragment();
    }
}

bool Tremolo::playTremolo() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->playTremolo();
    } else {
        return m_tremoloSingleChord->playTremolo();
    }
}

void Tremolo::setPlayTremolo(bool v)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setPlayTremolo(v);
    } else {
        m_tremoloSingleChord->setPlayTremolo(v);
    }
}

//---------------------------------------------------------
//   crossStaffBeamBetween
//    Return true if tremolo is two-note cross-staff and beams between staves
//---------------------------------------------------------

bool Tremolo::crossStaffBeamBetween() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->crossStaffBeamBetween();
    } else {
        return m_tremoloSingleChord->crossStaffBeamBetween();
    }
}

DirectionV Tremolo::direction() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->direction();
    } else {
        return m_tremoloSingleChord->direction();
    }
}

void Tremolo::setDirection(DirectionV val)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setDirection(val);
    } else {
        m_tremoloSingleChord->setDirection(val);
    }
}

void Tremolo::setUserModified(DirectionV d, bool val)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setUserModified(d, val);
    } else {
        m_tremoloSingleChord->setUserModified(d, val);
    }
}

TDuration Tremolo::durationType() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->durationType();
    } else {
        return m_tremoloSingleChord->durationType();
    }
}

void Tremolo::setDurationType(TDuration d)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setDurationType(d);
    } else {
        m_tremoloSingleChord->setDurationType(d);
    }
    styleChanged();
}

int Tremolo::lines() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->lines();
    } else {
        return m_tremoloSingleChord->lines();
    }
}

bool Tremolo::up() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->up();
    } else {
        return m_tremoloSingleChord->up();
    }
}

void Tremolo::setUp(bool up)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setUp(up);
    } else {
        m_tremoloSingleChord->setUp(up);
    }
}

//---------------------------------------------------------
//   tremoloLen
//---------------------------------------------------------

Fraction Tremolo::tremoloLen() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->tremoloLen();
    } else {
        return m_tremoloSingleChord->tremoloLen();
    }
}

Chord* Tremolo::chord1() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->chord1();
    } else {
        return m_tremoloSingleChord->chord1();
    }
}

void Tremolo::setChord1(Chord* ch)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setChord1(ch);
    } else {
        m_tremoloSingleChord->setChord1(ch);
    }
}

Chord* Tremolo::chord2() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->chord2();
    } else {
        return m_tremoloSingleChord->chord2();
    }
}

void Tremolo::setChord2(Chord* ch)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setChord2(ch);
    } else {
        m_tremoloSingleChord->setChord2(ch);
    }
}

void Tremolo::setChords(Chord* c1, Chord* c2)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setChords(c1, c2);
    } else {
        m_tremoloSingleChord->setChords(c1, c2);
    }
}

//---------------------------------------------------------
//   setBeamPos
//---------------------------------------------------------

void Tremolo::setBeamPos(const PairF& bp)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setBeamPos(bp);
    } else {
        m_tremoloSingleChord->setBeamPos(bp);
    }
}

//---------------------------------------------------------
//   beamPos
//---------------------------------------------------------

PairF Tremolo::beamPos() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->beamPos();
    } else {
        return m_tremoloSingleChord->beamPos();
    }
}

//---------------------------------------------------------
//   userModified
//---------------------------------------------------------

bool Tremolo::userModified() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->userModified();
    } else {
        return m_tremoloSingleChord->userModified();
    }
}

//---------------------------------------------------------
//   setUserModified
//---------------------------------------------------------

void Tremolo::setUserModified(bool val)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setUserModified(val);
    } else {
        m_tremoloSingleChord->setUserModified(val);
    }
}

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void Tremolo::triggerLayout() const
{
    if (twoNotes()) {
        m_tremoloTwoChord->triggerLayout();
    } else {
        m_tremoloSingleChord->triggerLayout();
    }
}

bool Tremolo::needStartEditingAfterSelecting() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->needStartEditingAfterSelecting();
    } else {
        return m_tremoloSingleChord->needStartEditingAfterSelecting();
    }
}

int Tremolo::gripsCount() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->gripsCount();
    } else {
        return m_tremoloSingleChord->gripsCount();
    }
}

Grip Tremolo::initialEditModeGrip() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->initialEditModeGrip();
    } else {
        return m_tremoloSingleChord->initialEditModeGrip();
    }
}

Grip Tremolo::defaultGrip() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->defaultGrip();
    } else {
        return m_tremoloSingleChord->defaultGrip();
    }
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> Tremolo::gripsPositions(const EditData& ed) const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->gripsPositions(ed);
    } else {
        return m_tremoloSingleChord->gripsPositions(ed);
    }
}

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Tremolo::endEdit(EditData& ed)
{
    if (twoNotes()) {
        m_tremoloTwoChord->endEdit(ed);
    } else {
        m_tremoloSingleChord->endEdit(ed);
    }
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Tremolo::editDrag(EditData& ed)
{
    if (twoNotes()) {
        m_tremoloTwoChord->editDrag(ed);
    } else {
        m_tremoloSingleChord->editDrag(ed);
    }
}

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

TranslatableString Tremolo::subtypeUserName() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->subtypeUserName();
    } else {
        return m_tremoloSingleChord->subtypeUserName();
    }
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Tremolo::accessibleInfo() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->accessibleInfo();
    } else {
        return m_tremoloSingleChord->accessibleInfo();
    }
}

//---------------------------------------------------------
//   customStyleApplicable
//---------------------------------------------------------

bool Tremolo::customStyleApplicable() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->customStyleApplicable();
    } else {
        return m_tremoloSingleChord->customStyleApplicable();
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Tremolo::getProperty(Pid propertyId) const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->getProperty(propertyId);
    } else {
        return m_tremoloSingleChord->getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Tremolo::setProperty(Pid propertyId, const PropertyValue& val)
{
    if (twoNotes()) {
        return m_tremoloTwoChord->setProperty(propertyId, val);
    } else {
        return m_tremoloSingleChord->setProperty(propertyId, val);
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Tremolo::propertyDefault(Pid propertyId) const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->propertyDefault(propertyId);
    } else {
        return m_tremoloSingleChord->propertyDefault(propertyId);
    }
}

const ElementStyle* Tremolo::styledProperties() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->styledProperties();
    } else {
        return m_tremoloSingleChord->styledProperties();
    }
}

PropertyFlags* Tremolo::propertyFlagsList() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->propertyFlagsList();
    } else {
        return m_tremoloSingleChord->propertyFlagsList();
    }
}

PropertyFlags Tremolo::propertyFlags(Pid pid) const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->propertyFlags(pid);
    } else {
        return m_tremoloSingleChord->propertyFlags(pid);
    }
}

void Tremolo::setPropertyFlags(Pid pid, PropertyFlags f)
{
    if (twoNotes()) {
        return m_tremoloTwoChord->setPropertyFlags(pid, f);
    } else {
        return m_tremoloSingleChord->setPropertyFlags(pid, f);
    }
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Tremolo::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    if (twoNotes()) {
        m_tremoloTwoChord->scanElements(data, func, all);
    } else {
        m_tremoloSingleChord->scanElements(data, func, all);
    }
}

const std::vector<BeamSegment*>& Tremolo::beamSegments() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->beamSegments();
    } else {
        return m_tremoloSingleChord->beamSegments();
    }
}

std::vector<BeamSegment*>& Tremolo::beamSegments()
{
    if (twoNotes()) {
        return m_tremoloTwoChord->beamSegments();
    } else {
        return m_tremoloSingleChord->beamSegments();
    }
}

void Tremolo::clearBeamSegments()
{
    if (twoNotes()) {
        m_tremoloTwoChord->clearBeamSegments();
    } else {
        m_tremoloSingleChord->clearBeamSegments();
    }
}

std::shared_ptr<rendering::dev::BeamTremoloLayout> Tremolo::layoutInfo()
{
    if (twoNotes()) {
        return m_tremoloTwoChord->layoutInfo;
    } else {
        return m_tremoloSingleChord->layoutInfo;
    }
}

void Tremolo::setLayoutInfo(std::shared_ptr<rendering::dev::BeamTremoloLayout> info)
{
    if (twoNotes()) {
        m_tremoloTwoChord->layoutInfo = info;
    } else {
        m_tremoloSingleChord->layoutInfo = info;
    }
}
}
