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
        if (!twoChord) {
            twoChord = new TremoloTwoChord(*t.twoChord);
            twoChord->setDispatcher(this);
        }
    } else {
        if (!singleChord) {
            singleChord = new TremoloSingleChord(*t.singleChord);
            singleChord->setDispatcher(this);
        }
    }

    styleChanged();
}

TremoloDispatcher::~TremoloDispatcher()
{
    if (twoChord) {
        twoChord->setParent(nullptr);
    }

    if (singleChord) {
        singleChord->setParent(nullptr);
    }

    delete twoChord;
    delete singleChord;
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
        twoChord->setTrack(val);
    } else {
        singleChord->setTrack(val);
    }
}

void TremoloDispatcher::setTremoloType(TremoloType t)
{
    m_tremoloType = t;

    if (twoNotes()) {
        if (!twoChord) {
            twoChord = new TremoloTwoChord(toChord(this->parent()));
            twoChord->setDispatcher(this);
            twoChord->setTrack(track());
        }
        twoChord->setTremoloType(t);
    } else {
        if (!singleChord) {
            singleChord = new TremoloSingleChord(toChord(this->parent()));
            singleChord->setDispatcher(this);
            singleChord->setTrack(track());
        }
        singleChord->setTremoloType(t);
    }

    styleChanged();
}

void TremoloDispatcher::setParent(Chord* ch)
{
    EngravingItem::setParent(ch);
}

void TremoloDispatcher::setParentInternal(EngravingObject* p)
{
    EngravingItem::setParentInternal(p);

    if (twoChord) {
        twoChord->setParent(p);
    }

    if (singleChord) {
        singleChord->setParent(p);
    }
}

EngravingItem::LayoutData* TremoloDispatcher::createLayoutData() const
{
    if (twoNotes()) {
        return twoChord->createLayoutData();
    } else {
        return singleChord->createLayoutData();
    }
}

const EngravingItem::LayoutData* TremoloDispatcher::ldataInternal() const
{
    if (twoNotes()) {
        return twoChord ? twoChord->ldataInternal() : nullptr;
    } else {
        return singleChord ? singleChord->ldataInternal() : nullptr;
    }
}

EngravingItem::LayoutData* TremoloDispatcher::mutldataInternal()
{
    if (twoNotes()) {
        return twoChord->mutldataInternal();
    } else {
        return singleChord->mutldataInternal();
    }
}

//---------------------------------------------------------
//   chordMag
//---------------------------------------------------------

double TremoloDispatcher::chordMag() const
{
    if (twoNotes()) {
        return twoChord->chordMag();
    } else {
        return singleChord->chordMag();
    }
}

//---------------------------------------------------------
//   minHeight
//---------------------------------------------------------

double TremoloDispatcher::minHeight() const
{
    if (twoNotes()) {
        return twoChord->minHeight();
    } else {
        return singleChord->minHeight();
    }
}

//---------------------------------------------------------
//   chordBeamAnchor
//---------------------------------------------------------

PointF TremoloDispatcher::chordBeamAnchor(const ChordRest* chord, ChordBeamAnchorType anchorType) const
{
    if (twoNotes()) {
        return twoChord->chordBeamAnchor(chord, anchorType);
    } else {
        UNREACHABLE;
        return PointF();
    }
}

double TremoloDispatcher::beamWidth() const
{
    if (twoNotes()) {
        return twoChord->beamWidth();
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
        return twoChord->drag(ed);
    } else {
        return singleChord->drag(ed);
    }
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void TremoloDispatcher::spatiumChanged(double oldValue, double newValue)
{
    if (twoNotes()) {
        twoChord->spatiumChanged(oldValue, newValue);
    } else {
        singleChord->spatiumChanged(oldValue, newValue);
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
        twoChord->localSpatiumChanged(oldValue, newValue);
    } else {
        singleChord->localSpatiumChanged(oldValue, newValue);
    }
}

//---------------------------------------------------------
//   styleChanged
//    the scale of a staff changed
//---------------------------------------------------------

void TremoloDispatcher::styleChanged()
{
    if (twoNotes()) {
        twoChord->styleChanged();
    } else {
        singleChord->styleChanged();
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
        return twoChord->basePath(stretch);
    } else {
        return singleChord->basePath(stretch);
    }
}

const mu::draw::PainterPath& TremoloDispatcher::path() const
{
    if (twoNotes()) {
        return twoChord->path();
    } else {
        return singleChord->path();
    }
}

void TremoloDispatcher::setPath(const mu::draw::PainterPath& p)
{
    if (twoNotes()) {
        twoChord->setPath(p);
    } else {
        singleChord->setPath(p);
    }
}

const mu::PointF& TremoloDispatcher::startAnchor() const
{
    if (twoNotes()) {
        return twoChord->startAnchor();
    } else {
        UNREACHABLE;
        static PointF p;
        return p;
    }
}

mu::PointF& TremoloDispatcher::startAnchor()
{
    if (twoNotes()) {
        return twoChord->startAnchor();
    } else {
        UNREACHABLE;
        static PointF p;
        return p;
    }
}

void TremoloDispatcher::setStartAnchor(const mu::PointF& p)
{
    if (twoNotes()) {
        twoChord->setStartAnchor(p);
    } else {
        UNREACHABLE;
    }
}

const mu::PointF& TremoloDispatcher::endAnchor() const
{
    if (twoNotes()) {
        return twoChord->endAnchor();
    } else {
        UNREACHABLE;
        static PointF p;
        return p;
    }
}

mu::PointF& TremoloDispatcher::endAnchor()
{
    if (twoNotes()) {
        return twoChord->endAnchor();
    } else {
        UNREACHABLE;
        static PointF p;
        return p;
    }
}

void TremoloDispatcher::setEndAnchor(const mu::PointF& p)
{
    if (twoNotes()) {
        twoChord->setEndAnchor(p);
    } else {
        UNREACHABLE;
    }
}

//---------------------------------------------------------
//   computeShape
//---------------------------------------------------------

void TremoloDispatcher::computeShape()
{
    if (twoNotes()) {
        //! NOTE used for palette
        twoChord->computeShape();
    } else {
        singleChord->computeShape();
    }
}

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void TremoloDispatcher::reset()
{
    if (twoNotes()) {
        twoChord->reset();
    } else {
        singleChord->reset();
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
        return twoChord->tremoloStyle();
    } else {
        UNREACHABLE;
        return TremoloStyle::DEFAULT;
    }
}

void TremoloDispatcher::setTremoloStyle(TremoloStyle v)
{
    if (twoNotes()) {
        twoChord->setTremoloStyle(v);
    } else {
        UNREACHABLE;
    }
}

void TremoloDispatcher::setBeamFragment(const BeamFragment& bf)
{
    if (twoNotes()) {
        twoChord->setBeamFragment(bf);
    } else {
        UNREACHABLE;
    }
}

const BeamFragment& TremoloDispatcher::beamFragment() const
{
    if (twoNotes()) {
        return twoChord->beamFragment();
    } else {
        UNREACHABLE;
        static BeamFragment f;
        return f;
    }
}

BeamFragment& TremoloDispatcher::beamFragment()
{
    if (twoNotes()) {
        return twoChord->beamFragment();
    } else {
        UNREACHABLE;
        static BeamFragment f;
        return f;
    }
}

bool TremoloDispatcher::playTremolo() const
{
    if (twoNotes()) {
        return twoChord->playTremolo();
    } else {
        return singleChord->playTremolo();
    }
}

void TremoloDispatcher::setPlayTremolo(bool v)
{
    if (twoNotes()) {
        twoChord->setPlayTremolo(v);
    } else {
        singleChord->setPlayTremolo(v);
    }
}

//---------------------------------------------------------
//   crossStaffBeamBetween
//    Return true if tremolo is two-note cross-staff and beams between staves
//---------------------------------------------------------

bool TremoloDispatcher::crossStaffBeamBetween() const
{
    if (twoNotes()) {
        return twoChord->crossStaffBeamBetween();
    } else {
        UNREACHABLE;
        return false;
    }
}

DirectionV TremoloDispatcher::direction() const
{
    if (twoNotes()) {
        return twoChord->direction();
    } else {
        UNREACHABLE;
        return DirectionV::UP;
    }
}

void TremoloDispatcher::setDirection(DirectionV val)
{
    if (twoNotes()) {
        twoChord->setDirection(val);
    } else {
        UNREACHABLE;
    }
}

void TremoloDispatcher::setUserModified(DirectionV d, bool val)
{
    if (twoNotes()) {
        twoChord->setUserModified(d, val);
    } else {
        UNREACHABLE;
    }
}

TDuration TremoloDispatcher::durationType() const
{
    if (twoNotes()) {
        return twoChord->durationType();
    } else {
        return singleChord->durationType();
    }
}

void TremoloDispatcher::setDurationType(TDuration d)
{
    if (twoNotes()) {
        twoChord->setDurationType(d);
    } else {
        singleChord->setDurationType(d);
    }
}

int TremoloDispatcher::lines() const
{
    if (twoNotes()) {
        return twoChord->lines();
    } else {
        return singleChord->lines();
    }
}

bool TremoloDispatcher::up() const
{
    if (twoNotes()) {
        return twoChord->up();
    } else {
        UNREACHABLE;
        return false;
    }
}

void TremoloDispatcher::setUp(bool up)
{
    if (twoNotes()) {
        twoChord->setUp(up);
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
        return twoChord->tremoloLen();
    } else {
        return singleChord->tremoloLen();
    }
}

Chord* TremoloDispatcher::chord1() const
{
    if (twoNotes()) {
        return twoChord->chord1();
    } else {
        return singleChord->chord();
    }
}

void TremoloDispatcher::setChord1(Chord* ch)
{
    if (twoNotes()) {
        twoChord->setChord1(ch);
    } else {
        singleChord->setParent(ch);
    }
}

Chord* TremoloDispatcher::chord2() const
{
    if (twoNotes()) {
        return twoChord->chord2();
    } else {
        UNREACHABLE;
        return nullptr;
    }
}

void TremoloDispatcher::setChord2(Chord* ch)
{
    if (twoNotes()) {
        twoChord->setChord2(ch);
    } else {
        UNREACHABLE;
    }
}

void TremoloDispatcher::setChords(Chord* c1, Chord* c2)
{
    if (twoNotes()) {
        twoChord->setChords(c1, c2);
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
        twoChord->setBeamPos(bp);
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
        return twoChord->beamPos();
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
        return twoChord->userModified();
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
        twoChord->setUserModified(val);
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
        twoChord->triggerLayout();
    } else {
        singleChord->triggerLayout();
    }
}

bool TremoloDispatcher::needStartEditingAfterSelecting() const
{
    if (twoNotes()) {
        return twoChord->needStartEditingAfterSelecting();
    } else {
        return singleChord->needStartEditingAfterSelecting();
    }
}

int TremoloDispatcher::gripsCount() const
{
    if (twoNotes()) {
        return twoChord->gripsCount();
    } else {
        return singleChord->gripsCount();
    }
}

Grip TremoloDispatcher::initialEditModeGrip() const
{
    if (twoNotes()) {
        return twoChord->initialEditModeGrip();
    } else {
        return singleChord->initialEditModeGrip();
    }
}

Grip TremoloDispatcher::defaultGrip() const
{
    if (twoNotes()) {
        return twoChord->defaultGrip();
    } else {
        return singleChord->defaultGrip();
    }
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<PointF> TremoloDispatcher::gripsPositions(const EditData& ed) const
{
    if (twoNotes()) {
        return twoChord->gripsPositions(ed);
    } else {
        return singleChord->gripsPositions(ed);
    }
}

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void TremoloDispatcher::endEdit(EditData& ed)
{
    if (twoNotes()) {
        twoChord->endEdit(ed);
    } else {
        singleChord->endEdit(ed);
    }
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void TremoloDispatcher::editDrag(EditData& ed)
{
    if (twoNotes()) {
        twoChord->editDrag(ed);
    } else {
        singleChord->editDrag(ed);
    }
}

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

TranslatableString TremoloDispatcher::subtypeUserName() const
{
    if (twoNotes()) {
        return twoChord->subtypeUserName();
    } else {
        return singleChord->subtypeUserName();
    }
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String TremoloDispatcher::accessibleInfo() const
{
    if (twoNotes()) {
        return twoChord->accessibleInfo();
    } else {
        return singleChord->accessibleInfo();
    }
}

//---------------------------------------------------------
//   customStyleApplicable
//---------------------------------------------------------

bool TremoloDispatcher::customStyleApplicable() const
{
    if (twoNotes()) {
        return twoChord->customStyleApplicable();
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
        return twoChord->getProperty(propertyId);
    } else {
        return singleChord->getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool TremoloDispatcher::setProperty(Pid propertyId, const PropertyValue& val)
{
    EngravingItem::setProperty(propertyId, val);

    if (twoNotes()) {
        return twoChord->setProperty(propertyId, val);
    } else {
        return singleChord->setProperty(propertyId, val);
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue TremoloDispatcher::propertyDefault(Pid propertyId) const
{
    if (twoNotes()) {
        return twoChord->propertyDefault(propertyId);
    } else {
        return singleChord->propertyDefault(propertyId);
    }
}

void TremoloDispatcher::setColor(const mu::draw::Color& c)
{
    EngravingItem::setColor(c);
    if (twoNotes()) {
        twoChord->setColor(c);
    } else {
        singleChord->setColor(c);
    }
}

mu::draw::Color TremoloDispatcher::color() const
{
    if (twoNotes()) {
        return twoChord->color();
    } else {
        return singleChord->color();
    }
}

const ElementStyle* TremoloDispatcher::styledProperties() const
{
    if (twoNotes()) {
        return twoChord->styledProperties();
    } else {
        return singleChord->styledProperties();
    }
}

PropertyFlags* TremoloDispatcher::propertyFlagsList() const
{
    if (twoNotes()) {
        return twoChord->propertyFlagsList();
    } else {
        return singleChord->propertyFlagsList();
    }
}

PropertyFlags TremoloDispatcher::propertyFlags(Pid pid) const
{
    if (twoNotes()) {
        return twoChord->propertyFlags(pid);
    } else {
        return singleChord->propertyFlags(pid);
    }
}

void TremoloDispatcher::setPropertyFlags(Pid pid, PropertyFlags f)
{
    if (twoNotes()) {
        return twoChord->setPropertyFlags(pid, f);
    } else {
        return singleChord->setPropertyFlags(pid, f);
    }
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void TremoloDispatcher::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    if (chord() && chord()->tremoloChordType() == TremoloChordType::TremoloSecondChord) {
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
        return twoChord->beamSegments();
    } else {
        UNREACHABLE;
        static std::vector<BeamSegment*> bs;
        return bs;
    }
}

std::vector<BeamSegment*>& TremoloDispatcher::beamSegments()
{
    if (twoNotes()) {
        return twoChord->beamSegments();
    } else {
        UNREACHABLE;
        static std::vector<BeamSegment*> bs;
        return bs;
    }
}

void TremoloDispatcher::clearBeamSegments()
{
    if (twoNotes()) {
        twoChord->clearBeamSegments();
    } else {
        UNREACHABLE;
    }
}

std::shared_ptr<rendering::dev::BeamTremoloLayout> TremoloDispatcher::layoutInfo()
{
    if (twoNotes()) {
        return twoChord->layoutInfo();
    } else {
        UNREACHABLE;
        return nullptr;
    }
}

void TremoloDispatcher::setLayoutInfo(std::shared_ptr<rendering::dev::BeamTremoloLayout> info)
{
    if (twoNotes()) {
        twoChord->setLayoutInfo(info);
    } else {
        UNREACHABLE;
    }
}
}
