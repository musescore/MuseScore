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

TremoloDispatcher::TremoloDispatcher(Chord* parent)
    : EngravingItem(ElementType::TREMOLO, parent, ElementFlag::MOVABLE)
{
    //initElementStyle(&TREMOLO_STYLE);
}

TremoloDispatcher::TremoloDispatcher(const TremoloDispatcher& t)
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

TremoloDispatcher::~TremoloDispatcher()
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

bool TremoloDispatcher::twoNotes() const
{
    DO_ASSERT(m_tremoloType != TremoloType::INVALID_TREMOLO);
    return m_tremoloType >= TremoloType::C8;
}

void TremoloDispatcher::setTrack(track_idx_t val)
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

void TremoloDispatcher::setTremoloType(TremoloType t)
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

void TremoloDispatcher::setParent(Chord* ch)
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

double TremoloDispatcher::chordMag() const
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

double TremoloDispatcher::minHeight() const
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

PointF TremoloDispatcher::chordBeamAnchor(const ChordRest* chord, ChordBeamAnchorType anchorType) const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->chordBeamAnchor(chord, anchorType);
    } else {
        UNREACHABLE;
        return PointF();
    }
}

double TremoloDispatcher::beamWidth() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->beamWidth();
    } else {
        UNREACHABLE;
        return 0;
    }
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

RectF TremoloDispatcher::drag(EditData& ed)
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

void TremoloDispatcher::spatiumChanged(double oldValue, double newValue)
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

void TremoloDispatcher::localSpatiumChanged(double oldValue, double newValue)
{
    if (twoNotes()) {
        m_tremoloTwoChord->localSpatiumChanged(oldValue, newValue);
    } else {
        m_tremoloSingleChord->localSpatiumChanged(oldValue, newValue);
    }
}

//---------------------------------------------------------
//   styleChanged
//    the scale of a staff changed
//---------------------------------------------------------

void TremoloDispatcher::styleChanged()
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

PainterPath TremoloDispatcher::basePath(double stretch) const
{
    if (twoNotes()) {
        UNREACHABLE;
        return PainterPath();
    } else {
        return m_tremoloSingleChord->basePath(stretch);
    }
}

const mu::draw::PainterPath& TremoloDispatcher::path() const
{
    if (twoNotes()) {
        UNREACHABLE;
        static mu::draw::PainterPath path;
        return path;
    } else {
        return m_tremoloSingleChord->path();
    }
}

void TremoloDispatcher::setPath(const mu::draw::PainterPath& p)
{
    if (twoNotes()) {
        UNREACHABLE;
    } else {
        m_tremoloSingleChord->setPath(p);
    }
}

const mu::PointF& TremoloDispatcher::startAnchor() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->startAnchor();
    } else {
        UNREACHABLE;
        static PointF p;
        return p;
    }
}

mu::PointF& TremoloDispatcher::startAnchor()
{
    if (twoNotes()) {
        return m_tremoloTwoChord->startAnchor();
    } else {
        UNREACHABLE;
        static PointF p;
        return p;
    }
}

void TremoloDispatcher::setStartAnchor(const mu::PointF& p)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setStartAnchor(p);
    } else {
        UNREACHABLE;
    }
}

const mu::PointF& TremoloDispatcher::endAnchor() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->endAnchor();
    } else {
        UNREACHABLE;
        static PointF p;
        return p;
    }
}

mu::PointF& TremoloDispatcher::endAnchor()
{
    if (twoNotes()) {
        return m_tremoloTwoChord->endAnchor();
    } else {
        UNREACHABLE;
        static PointF p;
        return p;
    }
}

void TremoloDispatcher::setEndAnchor(const mu::PointF& p)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setEndAnchor(p);
    } else {
        UNREACHABLE;
    }
}

//---------------------------------------------------------
//   computeShape
//---------------------------------------------------------

void TremoloDispatcher::computeShape()
{
    if (!twoNotes()) {
        m_tremoloSingleChord->computeShape();
        RectF bbox = m_tremoloSingleChord->ldata()->bbox();
        setbbox(bbox);
    }
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void TremoloDispatcher::reset()
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

PointF TremoloDispatcher::pagePos() const
{
    return EngravingItem::pagePos();
}

TremoloStyle TremoloDispatcher::tremoloStyle() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->tremoloStyle();
    } else {
        UNREACHABLE;
        return TremoloStyle::DEFAULT;
    }
}

void TremoloDispatcher::setTremoloStyle(TremoloStyle v)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setTremoloStyle(v);
    } else {
        UNREACHABLE;
    }
}

void TremoloDispatcher::setBeamFragment(const BeamFragment& bf)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setBeamFragment(bf);
    } else {
        UNREACHABLE;
    }
}

const BeamFragment& TremoloDispatcher::beamFragment() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->beamFragment();
    } else {
        UNREACHABLE;
        static BeamFragment f;
        return f;
    }
}

BeamFragment& TremoloDispatcher::beamFragment()
{
    if (twoNotes()) {
        return m_tremoloTwoChord->beamFragment();
    } else {
        UNREACHABLE;
        static BeamFragment f;
        return f;
    }
}

bool TremoloDispatcher::playTremolo() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->playTremolo();
    } else {
        return m_tremoloSingleChord->playTremolo();
    }
}

void TremoloDispatcher::setPlayTremolo(bool v)
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

bool TremoloDispatcher::crossStaffBeamBetween() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->crossStaffBeamBetween();
    } else {
        UNREACHABLE;
        return false;
    }
}

DirectionV TremoloDispatcher::direction() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->direction();
    } else {
        UNREACHABLE;
        return DirectionV::UP;
    }
}

void TremoloDispatcher::setDirection(DirectionV val)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setDirection(val);
    } else {
        UNREACHABLE;
    }
}

void TremoloDispatcher::setUserModified(DirectionV d, bool val)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setUserModified(d, val);
    } else {
        UNREACHABLE;
    }
}

TDuration TremoloDispatcher::durationType() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->durationType();
    } else {
        return m_tremoloSingleChord->durationType();
    }
}

void TremoloDispatcher::setDurationType(TDuration d)
{
    if (twoNotes()) {
        return m_tremoloTwoChord->setDurationType(d);
    } else {
        return m_tremoloSingleChord->setDurationType(d);
    }
    styleChanged();
}

int TremoloDispatcher::lines() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->lines();
    } else {
        return m_tremoloSingleChord->lines();
    }
}

bool TremoloDispatcher::up() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->up();
    } else {
        UNREACHABLE;
        return false;
    }
}

void TremoloDispatcher::setUp(bool up)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setUp(up);
    } else {
        UNREACHABLE;
    }
}

//---------------------------------------------------------
//   tremoloLen
//---------------------------------------------------------

Fraction TremoloDispatcher::tremoloLen() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->tremoloLen();
    } else {
        return m_tremoloSingleChord->tremoloLen();
    }
}

Chord* TremoloDispatcher::chord1() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->chord1();
    } else {
        return m_tremoloSingleChord->chord();
    }
}

void TremoloDispatcher::setChord1(Chord* ch)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setChord1(ch);
    } else {
        m_tremoloSingleChord->setParent(ch);
    }
}

Chord* TremoloDispatcher::chord2() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->chord2();
    } else {
        UNREACHABLE;
        return nullptr;
    }
}

void TremoloDispatcher::setChord2(Chord* ch)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setChord2(ch);
    } else {
        UNREACHABLE;
    }
}

void TremoloDispatcher::setChords(Chord* c1, Chord* c2)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setChords(c1, c2);
    } else {
        UNREACHABLE;
    }
}

//---------------------------------------------------------
//   setBeamPos
//---------------------------------------------------------

void TremoloDispatcher::setBeamPos(const PairF& bp)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setBeamPos(bp);
    } else {
        UNREACHABLE;
    }
}

//---------------------------------------------------------
//   beamPos
//---------------------------------------------------------

PairF TremoloDispatcher::beamPos() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->beamPos();
    } else {
        UNREACHABLE;
        return PairF();
    }
}

//---------------------------------------------------------
//   userModified
//---------------------------------------------------------

bool TremoloDispatcher::userModified() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->userModified();
    } else {
        UNREACHABLE;
        return false;
    }
}

//---------------------------------------------------------
//   setUserModified
//---------------------------------------------------------

void TremoloDispatcher::setUserModified(bool val)
{
    if (twoNotes()) {
        m_tremoloTwoChord->setUserModified(val);
    } else {
        UNREACHABLE;
    }
}

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void TremoloDispatcher::triggerLayout() const
{
    if (twoNotes()) {
        m_tremoloTwoChord->triggerLayout();
    } else {
        m_tremoloSingleChord->triggerLayout();
    }
}

bool TremoloDispatcher::needStartEditingAfterSelecting() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->needStartEditingAfterSelecting();
    } else {
        return m_tremoloSingleChord->needStartEditingAfterSelecting();
    }
}

int TremoloDispatcher::gripsCount() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->gripsCount();
    } else {
        return m_tremoloSingleChord->gripsCount();
    }
}

Grip TremoloDispatcher::initialEditModeGrip() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->initialEditModeGrip();
    } else {
        return m_tremoloSingleChord->initialEditModeGrip();
    }
}

Grip TremoloDispatcher::defaultGrip() const
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

std::vector<PointF> TremoloDispatcher::gripsPositions(const EditData& ed) const
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

void TremoloDispatcher::endEdit(EditData& ed)
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

void TremoloDispatcher::editDrag(EditData& ed)
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

TranslatableString TremoloDispatcher::subtypeUserName() const
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

String TremoloDispatcher::accessibleInfo() const
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

bool TremoloDispatcher::customStyleApplicable() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->customStyleApplicable();
    } else {
        UNREACHABLE;
        return false;
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue TremoloDispatcher::getProperty(Pid propertyId) const
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

bool TremoloDispatcher::setProperty(Pid propertyId, const PropertyValue& val)
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

PropertyValue TremoloDispatcher::propertyDefault(Pid propertyId) const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->propertyDefault(propertyId);
    } else {
        return m_tremoloSingleChord->propertyDefault(propertyId);
    }
}

const ElementStyle* TremoloDispatcher::styledProperties() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->styledProperties();
    } else {
        return m_tremoloSingleChord->styledProperties();
    }
}

PropertyFlags* TremoloDispatcher::propertyFlagsList() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->propertyFlagsList();
    } else {
        return m_tremoloSingleChord->propertyFlagsList();
    }
}

PropertyFlags TremoloDispatcher::propertyFlags(Pid pid) const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->propertyFlags(pid);
    } else {
        return m_tremoloSingleChord->propertyFlags(pid);
    }
}

void TremoloDispatcher::setPropertyFlags(Pid pid, PropertyFlags f)
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

void TremoloDispatcher::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    if (chord() && chord()->tremoloChordType() == TremoloChordType::TremoloSecondNote) {
        return;
    }
    EngravingItem::scanElements(data, func, all);

//    if (twoNotes()) {
//        m_tremoloTwoChord->scanElements(data, func, all);
//    } else {
//        m_tremoloSingleChord->scanElements(data, func, all);
//    }
}

const std::vector<BeamSegment*>& TremoloDispatcher::beamSegments() const
{
    if (twoNotes()) {
        return m_tremoloTwoChord->beamSegments();
    } else {
        UNREACHABLE;
        static std::vector<BeamSegment*> bs;
        return bs;
    }
}

std::vector<BeamSegment*>& TremoloDispatcher::beamSegments()
{
    if (twoNotes()) {
        return m_tremoloTwoChord->beamSegments();
    } else {
        UNREACHABLE;
        static std::vector<BeamSegment*> bs;
        return bs;
    }
}

void TremoloDispatcher::clearBeamSegments()
{
    if (twoNotes()) {
        m_tremoloTwoChord->clearBeamSegments();
    } else {
        UNREACHABLE;
    }
}

std::shared_ptr<rendering::dev::BeamTremoloLayout> TremoloDispatcher::layoutInfo()
{
    if (twoNotes()) {
        return m_tremoloTwoChord->layoutInfo;
    } else {
        UNREACHABLE;
        return nullptr;
    }
}

void TremoloDispatcher::setLayoutInfo(std::shared_ptr<rendering::dev::BeamTremoloLayout> info)
{
    if (twoNotes()) {
        m_tremoloTwoChord->layoutInfo = info;
    } else {
        UNREACHABLE;
    }
}
}
