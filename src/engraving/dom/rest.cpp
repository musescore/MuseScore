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

#include "rest.h"

#include <cmath>
#include <set>

#include "containers.h"
#include "translation.h"

#include "actionicon.h"
#include "articulation.h"
#include "chord.h"
#include "deadslapped.h"
#include "factory.h"
#include "measure.h"
#include "measurerepeat.h"
#include "note.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "stafftype.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//    Rest
//--------------------------------------------------------

Rest::Rest(Segment* parent)
    : Rest(ElementType::REST, parent)
{
}

Rest::Rest(const ElementType& type, Segment* parent)
    : ChordRest(type, parent)
{
    m_beamMode  = BeamMode::NONE;
}

Rest::Rest(Segment* parent, const TDuration& d)
    : Rest(ElementType::REST, parent, d)
{
}

Rest::Rest(const ElementType& type, Segment* parent, const TDuration& d)
    : ChordRest(type, parent)
{
    m_beamMode  = BeamMode::NONE;
    setDurationType(d);
    if (d.fraction().isValid()) {
        setTicks(d.fraction());
    }
}

Rest::Rest(const Rest& r, bool link)
    : ChordRest(r, link)
{
    if (link) {
        score()->undo(new Link(this, const_cast<Rest*>(&r)));
        setAutoplace(true);
    }
    m_gap      = r.m_gap;
    m_dotline  = r.m_dotline;
    for (NoteDot* dot : r.m_dots) {
        add(Factory::copyNoteDot(*dot));
    }

    if (r.m_deadSlapped) {
        DeadSlapped* ndc = Factory::copyDeadSlapped(*r.m_deadSlapped);
        add(ndc);
        if (link) {
            score()->undo(new Link(ndc, r.m_deadSlapped));
        }
    }
}

void Rest::hack_toRestType()
{
    hack_setType(ElementType::REST);
}

//---------------------------------------------------------
//   setOffset, overridden from EngravingItem
//    (- raster vertical position in spatium units) -> no
//    - half rests and whole rests outside the staff are
//      replaced by special symbols with ledger lines
//---------------------------------------------------------

void Rest::setOffset(const PointF& o)
{
    double _spatium = spatium();
    int line = lrint(o.y() / _spatium);

    //! NOTE We need to find out why this is being done here.
    //! We rewrite sym in the layout (we get from the Rest::getSymbol method )

    LayoutData* ldata = mutldata();

    if (ldata->sym == SymId::restWhole && (line <= -2 || line >= 3)) {
        ldata->sym = SymId::restWholeLegerLine;
    } else if (ldata->sym == SymId::restWholeLegerLine && (line > -2 && line < 4)) {
        ldata->sym = SymId::restWhole;
    } else if (ldata->sym == SymId::restHalf && (line <= -3 || line >= 3)) {
        ldata->sym = SymId::restHalfLegerLine;
    } else if (ldata->sym == SymId::restHalfLegerLine && (line > -3 && line < 3)) {
        ldata->sym = SymId::restHalf;
    }

    EngravingItem::setOffset(o);
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

RectF Rest::drag(EditData& ed)
{
    // don't allow drag for Measure Rests, because they can't be easily laid out in correct position while dragging
    if (measure() && durationType().type() == DurationType::V_MEASURE) {
        return RectF();
    }

    PointF s(ed.delta);
    RectF r(pageBoundingRect());

    // Limit horizontal drag range
    static const double xDragRange = spatium() * 5;
    if (std::fabs(s.x()) > xDragRange) {
        s.rx() = xDragRange * (s.x() < 0 ? -1.0 : 1.0);
    }
    setOffset(PointF(s.x(), s.y()));

    renderer()->layoutItem(this);

    score()->rebuildBspTree();
    return pageBoundingRect().united(r);
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Rest::acceptDrop(EditData& data) const
{
    EngravingItem* e = data.dropElement;
    ElementType type = e->type();
    if ((type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_AUTO)
        || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_NONE)
        || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_BREAK_LEFT)
        || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_BREAK_INNER_8TH)
        || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_BREAK_INNER_16TH)
        || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_JOIN)
        || (type == ElementType::FERMATA)
        || (type == ElementType::CLEF)
        || (type == ElementType::KEYSIG)
        || (type == ElementType::TIMESIG)
        || (type == ElementType::SYSTEM_TEXT)
        || (type == ElementType::TRIPLET_FEEL)
        || (type == ElementType::STAFF_TEXT)
        || (type == ElementType::PLAYTECH_ANNOTATION)
        || (type == ElementType::CAPO)
        || (type == ElementType::BAR_LINE)
        || (type == ElementType::BREATH)
        || (type == ElementType::CHORD)
        || (type == ElementType::NOTE)
        || (type == ElementType::STAFF_STATE)
        || (type == ElementType::INSTRUMENT_CHANGE)
        || (type == ElementType::DYNAMIC)
        || (type == ElementType::EXPRESSION)
        || (type == ElementType::HARMONY)
        || (type == ElementType::TEMPO_TEXT)
        || (type == ElementType::REHEARSAL_MARK)
        || (type == ElementType::FRET_DIAGRAM)
        || (type == ElementType::TREMOLOBAR)
        || (type == ElementType::IMAGE)
        || (type == ElementType::SYMBOL)
        || (type == ElementType::HARP_DIAGRAM)
        || (type == ElementType::MEASURE_REPEAT && durationType().type() == DurationType::V_MEASURE)
        ) {
        return true;
    }

    if (type == ElementType::STRING_TUNINGS) {
        staff_idx_t staffIdx = 0;
        Segment* seg = nullptr;
        if (!score()->pos2measure(data.pos, &staffIdx, 0, &seg, 0)) {
            return false;
        }

        return measure()->canAddStringTunings(staffIdx);
    }

    // prevent 'hanging' slurs, avoid crash on tie
    static const std::set<ElementType> ignoredTypes {
        ElementType::SLUR,
        ElementType::HAMMER_ON_PULL_OFF,
        ElementType::TIE,
        ElementType::GLISSANDO
    };

    return e->isSpanner() && !muse::contains(ignoredTypes, type);
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* Rest::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    switch (e->type()) {
    case ElementType::ARTICULATION:
    {
        Articulation* a = toArticulation(e);
        if (!a->isFermata() || !score()->toggleArticulation(this, a)) {
            delete e;
            e = 0;
        }
    }
        return e;

    case ElementType::CHORD: {
        Chord* c = toChord(e);
        Note* n  = c->upNote();
        DirectionV dir = c->stemDirection();
        NoteVal nval;
        nval.pitch = n->pitch();
        nval.headGroup = n->headGroup();
        Fraction d = score()->inputState().ticks();
        if (!d.isZero()) {
            Segment* seg = score()->setNoteRest(segment(), track(), nval, d, dir);
            if (seg) {
                ChordRest* cr = toChordRest(seg->element(track()));
                if (cr) {
                    score()->nextInputPos(cr, false);
                }
            }
        }
        delete e;
    }
    break;
    case ElementType::MEASURE_REPEAT: {
        int numMeasures = toMeasureRepeat(e)->numMeasures();
        delete e;
        if (durationType().type() == DurationType::V_MEASURE) {
            score()->cmdAddMeasureRepeat(measure(), numMeasures, staffIdx());
        }
        break;
    }
    case ElementType::STRING_TUNINGS:
        return measure()->drop(data);

    default:
        return ChordRest::drop(data);
    }
    return 0;
}

//---------------------------------------------------------
//   getSymbol
//---------------------------------------------------------

SymId Rest::getSymbol(DurationType type, int line, int lines) const
{
    switch (type) {
    case DurationType::V_LONG:
        return SymId::restLonga;
    case DurationType::V_BREVE:
        return SymId::restDoubleWhole;
    case DurationType::V_MEASURE:
        if (ticks() >= Fraction(2, 1)) {
            return SymId::restDoubleWhole;
        }
    // fall through
    case DurationType::V_WHOLE:
        return (line < 0 || line >= lines) ? SymId::restWholeLegerLine : SymId::restWhole;
    case DurationType::V_HALF:
        return (line < 0 || line >= lines) ? SymId::restHalfLegerLine : SymId::restHalf;
    case DurationType::V_QUARTER:
        return SymId::restQuarter;
    case DurationType::V_EIGHTH:
        return SymId::rest8th;
    case DurationType::V_16TH:
        return SymId::rest16th;
    case DurationType::V_32ND:
        return SymId::rest32nd;
    case DurationType::V_64TH:
        return SymId::rest64th;
    case DurationType::V_128TH:
        return SymId::rest128th;
    case DurationType::V_256TH:
        return SymId::rest256th;
    case DurationType::V_512TH:
        return SymId::rest512th;
    case DurationType::V_1024TH:
        return SymId::rest1024th;
    default:
        LOGD("unknown rest type %d", int(type));
        return SymId::restQuarter;
    }
}

double Rest::symWidthNoLedgerLines(LayoutData* ldata) const
{
    if (ldata->sym == SymId::restHalfLegerLine) {
        return symWidth(SymId::restHalf);
    }
    if (ldata->sym == SymId::restWholeLegerLine) {
        return symWidth(SymId::restWhole);
    }
    if (ldata->sym == SymId::restDoubleWholeLegerLine) {
        return symWidth(SymId::restDoubleWhole);
    }
    return symWidth(ldata->sym);
}

//---------------------------------------------------------
//   checkDots
//---------------------------------------------------------

void Rest::checkDots()
{
    int n = dots() - int(m_dots.size());
    for (int i = 0; i < n; ++i) {
        NoteDot* dot = Factory::createNoteDot(this);
        dot->setParent(this);
        dot->setVisible(visible());
        score()->undoAddElement(dot);
    }
    if (n < 0) {
        for (int i = 0; i < -n; ++i) {
            score()->undoRemoveElement(m_dots.back());
        }
    }
}

//---------------------------------------------------------
//   dot
//---------------------------------------------------------

NoteDot* Rest::dot(int n)
{
    checkDots();
    return m_dots[n];
}

const std::vector<NoteDot*>& Rest::dotList() const
{
    return m_dots;
}

//---------------------------------------------------------
//   getDotline
//---------------------------------------------------------

int Rest::getDotline(DurationType durationType)
{
    int dl = -1;
    switch (durationType) {
    case DurationType::V_64TH:
    case DurationType::V_32ND:
        dl = -3;
        break;
    case DurationType::V_1024TH:
    case DurationType::V_512TH:
    case DurationType::V_256TH:
    case DurationType::V_128TH:
        dl = -5;
        break;
    case DurationType::V_MEASURE:
    case DurationType::V_WHOLE:
        dl = 1;
        break;
    default:
        dl = -1;
        break;
    }
    return dl;
}

bool Rest::isWholeRest() const
{
    TDuration durType = durationType();
    return durType == DurationType::V_WHOLE
           || (durType == DurationType::V_MEASURE && measure() && measure()->ticks() < Fraction(2, 1));
}

bool Rest::isBreveRest() const
{
    TDuration durType = durationType();
    return durType == DurationType::V_BREVE
           || (durType == DurationType::V_MEASURE && measure() && measure()->ticks() >= Fraction(2, 1));
}

//---------------------------------------------------------
//   upPos
//---------------------------------------------------------

double Rest::upPos() const
{
    return symBbox(ldata()->sym()).y();
}

//---------------------------------------------------------
//   downPos
//---------------------------------------------------------

double Rest::downPos() const
{
    return symBbox(ldata()->sym()).y() + symHeight(ldata()->sym());
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Rest::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    ChordRest::scanElements(data, func, all);
    for (EngravingItem* e : el()) {
        e->scanElements(data, func, all);
    }
    for (NoteDot* dot : m_dots) {
        dot->scanElements(data, func, all);
    }
    if (!isGap()) {
        func(data, this);
    }
}

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Rest::setTrack(track_idx_t val)
{
    ChordRest::setTrack(val);
    for (NoteDot* dot : m_dots) {
        dot->setTrack(val);
    }
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double Rest::mag() const
{
    double m = staff() ? staff()->staffMag(this) : 1.0;
    return m * intrinsicMag();
}

//---------------------------------------------------------
//   intrinsicMag
//   returns the INTRINSIC mag of the rest (i.e. NOT scaled
//   by staff size)
//---------------------------------------------------------

double Rest::intrinsicMag() const
{
    double m = 1.0;
    if (isSmall()) {
        m *= style().styleD(Sid::smallNoteMag);
    }
    return m;
}

//---------------------------------------------------------
//   rightEdge
//---------------------------------------------------------

double Rest::rightEdge() const
{
    return x() + width();
}

double Rest::centerX() const
{
    SymId sym = ldata()->sym();
    RectF bbox = symBbox(sym);
    return bbox.left() + bbox.width() / 2;
}

//---------------------------------------------------------
//   accent
//---------------------------------------------------------

bool Rest::accent()
{
    return voice() >= 2 && isSmall();
}

//---------------------------------------------------------
//   setAccent
//---------------------------------------------------------

void Rest::setAccent(bool flag)
{
    undoChangeProperty(Pid::SMALL, flag);
    if (voice() % 2 == 0) {
        if (flag) {
            double yOffset = -(ldata()->bbox().bottom());
            if (durationType() >= DurationType::V_HALF) {
                yOffset -= staff()->spatium(tick()) * 0.5;
            }
            mutldata()->moveY(yOffset);
        }
    }
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Rest::accessibleInfo() const
{
    String voice = muse::mtrc("engraving", "Voice: %1").arg(track() % VOICES + 1);
    return muse::mtrc("engraving", "%1; Duration: %2; %3").arg(EngravingItem::accessibleInfo(), durationUserName(), voice);
}

//---------------------------------------------------------
//   screenReaderInfo
//---------------------------------------------------------

String Rest::screenReaderInfo() const
{
    Measure* m = measure();
    bool voices = m ? m->hasVoices(staffIdx()) : false;
    String voice = voices ? (u"; " + muse::mtrc("engraving", "Voice: %1").arg(track() % VOICES + 1)) : u"";
    String crossStaff;
    if (staffMove() < 0) {
        crossStaff = u"; " + muse::mtrc("engraving", "Cross-staff above");
    } else if (staffMove() > 0) {
        crossStaff = u"; " + muse::mtrc("engraving", "Cross-staff below");
    }
    return String(u"%1 %2%3%4").arg(EngravingItem::accessibleInfo(), durationUserName(), crossStaff, voice);
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Rest::add(EngravingItem* e)
{
    if (e->explicitParent() != this) {
        e->setParent(this);
    }
    e->setTrack(track());

    switch (e->type()) {
    case ElementType::NOTEDOT:
        m_dots.push_back(toNoteDot(e));
        e->added();
        break;
    case ElementType::DEAD_SLAPPED:
        m_deadSlapped = toDeadSlapped(e);
        e->added();
        break;
    default:
        ChordRest::add(e);
        break;
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Rest::remove(EngravingItem* e)
{
    switch (e->type()) {
    case ElementType::NOTEDOT:
        m_dots.pop_back();
        e->removed();
        break;
    case ElementType::DEAD_SLAPPED:
        m_deadSlapped = nullptr;
    // fallthrough
    case ElementType::SYMBOL:
    case ElementType::IMAGE:
        if (!removeEl(e)) {
            LOGD("Rest::remove(): cannot find %s", e->typeName());
        } else {
            e->removed();
        }
        break;
    default:
        ChordRest::remove(e);
        break;
    }
}

//---------------------------------------------------------
//   localSpatiumChanged
//---------------------------------------------------------

void Rest::localSpatiumChanged(double oldValue, double newValue)
{
    ChordRest::localSpatiumChanged(oldValue, newValue);
    for (EngravingItem* e : m_dots) {
        e->localSpatiumChanged(oldValue, newValue);
    }
    for (EngravingItem* e : el()) {
        e->localSpatiumChanged(oldValue, newValue);
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Rest::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::GAP:
        return false;
    case Pid::ALIGN_WITH_OTHER_RESTS:
        return true;
    default:
        return ChordRest::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Rest::resetProperty(Pid id)
{
    setProperty(id, propertyDefault(id));
    return;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Rest::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::GAP:
        return m_gap;
    case Pid::ALIGN_WITH_OTHER_RESTS:
        return alignWithOtherRests();
    default:
        return ChordRest::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Rest::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::GAP:
        m_gap = v.toBool();
        break;
    case Pid::VISIBLE:
        setVisible(v.toBool());
        break;
    case Pid::OFFSET:
        score()->addRefresh(canvasBoundingRect());
        setOffset(v.value<PointF>());

        renderer()->layoutItem(this);

        score()->addRefresh(canvasBoundingRect());
        if (measure() && durationType().type() == DurationType::V_MEASURE) {
            measure()->triggerLayout();
        }
        break;
    case Pid::ALIGN_WITH_OTHER_RESTS:
        setAlignWithOtherRests(v.toBool());
        break;
    default:
        return ChordRest::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   undoChangeDotsVisible
//---------------------------------------------------------

void Rest::undoChangeDotsVisible(bool v)
{
    for (NoteDot* dot : m_dots) {
        dot->undoChangeProperty(Pid::VISIBLE, v);
    }
}

//---------------------------------------------------------
//   nextElement
//---------------------------------------------------------

EngravingItem* Rest::nextElement()
{
    return ChordRest::nextElement();
}

//---------------------------------------------------------
//   prevElement
//---------------------------------------------------------

EngravingItem* Rest::prevElement()
{
    return ChordRest::prevElement();
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Rest::editDrag(EditData& editData)
{
    Segment* seg = segment();

    if (editData.modifiers & ShiftModifier) {
        const Spatium deltaSp = Spatium(editData.delta.x() / spatium());
        seg->undoChangeProperty(Pid::LEADING_SPACE, seg->extraLeadingSpace() + deltaSp);
    } else {
        setOffset(offset() + editData.evtDelta);
    }
    triggerLayout();
}

//---------------------------------------------------------
//   Rest::shouldNotBeDrawn
//    in tab staff, do not draw rests (except mmrests)
//    if rests are off OR if dur. symbols are on
//    also measures covered by MeasureRepeat show no rests
//---------------------------------------------------------

bool Rest::shouldNotBeDrawn() const
{
    const StaffType* st = staff() ? staff()->staffTypeForElement(this) : nullptr;
    if (generated()) {
        return true;
    }

    if (st && st->isTabStaff() && (!st->showRests() || st->genDurations())
        && (!measure() || !measure()->isMMRest())) {
        return true;
    }

    if (measure() && measure()->measureRepeatCount(staffIdx())) {
        return true;
    }

    return false;
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------
Sid Rest::getPropertyStyle(Pid pid) const
{
    return ChordRest::getPropertyStyle(pid);
}
}
