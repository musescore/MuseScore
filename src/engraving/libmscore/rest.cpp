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

#include "rest.h"

#include <cmath>
#include <set>

#include "containers.h"
#include "style/style.h"
#include "rw/xml.h"

#include "factory.h"
#include "score.h"
#include "utils.h"
#include "tuplet.h"
#include "stafftext.h"
#include "articulation.h"
#include "chord.h"
#include "note.h"
#include "measure.h"
#include "measurerepeat.h"
#include "undo.h"
#include "staff.h"
#include "harmony.h"
#include "segment.h"
#include "stafftype.h"
#include "actionicon.h"
#include "image.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
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
    _beamMode  = BeamMode::NONE;
    m_sym      = SymId::restQuarter;
}

Rest::Rest(Segment* parent, const TDuration& d)
    : Rest(ElementType::REST, parent, d)
{
}

Rest::Rest(const ElementType& type, Segment* parent, const TDuration& d)
    : ChordRest(type, parent)
{
    _beamMode  = BeamMode::NONE;
    m_sym      = SymId::restQuarter;
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
    m_sym      = r.m_sym;
    m_dotline  = r.m_dotline;
    for (NoteDot* dot : r.m_dots) {
        add(Factory::copyNoteDot(*dot));
    }
}

void Rest::hack_toRestType()
{
    hack_setType(ElementType::REST);
}

//---------------------------------------------------------
//   Rest::draw
//---------------------------------------------------------

void Rest::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    if (shouldNotBeDrawn()) {
        return;
    }
    painter->setPen(curColor());
    drawSymbol(m_sym, painter);
}

//---------------------------------------------------------
//   setOffset, overridden from EngravingItem
//    (- raster vertical position in spatium units) -> no
//    - half rests and whole rests outside the staff are
//      replaced by special symbols with ledger lines
//---------------------------------------------------------

void Rest::setOffset(const mu::PointF& o)
{
    qreal _spatium = spatium();
    int line = lrint(o.y() / _spatium);

    if (m_sym == SymId::restWhole && (line <= -2 || line >= 3)) {
        m_sym = SymId::restWholeLegerLine;
    } else if (m_sym == SymId::restWholeLegerLine && (line > -2 && line < 4)) {
        m_sym = SymId::restWhole;
    } else if (m_sym == SymId::restHalf && (line <= -3 || line >= 3)) {
        m_sym = SymId::restHalfLegerLine;
    } else if (m_sym == SymId::restHalfLegerLine && (line > -3 && line < 3)) {
        m_sym = SymId::restHalf;
    }

    EngravingItem::setOffset(o);
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

mu::RectF Rest::drag(EditData& ed)
{
    // don't allow drag for Measure Rests, because they can't be easily laid out in correct position while dragging
    if (measure() && durationType().type() == DurationType::V_MEASURE) {
        return RectF();
    }

    PointF s(ed.delta);
    RectF r(abbox());

    // Limit horizontal drag range
    static const qreal xDragRange = spatium() * 5;
    if (fabs(s.x()) > xDragRange) {
        s.rx() = xDragRange * (s.x() < 0 ? -1.0 : 1.0);
    }
    setOffset(PointF(s.x(), s.y()));
    layout();
    score()->rebuildBspTree();
    return abbox().united(r);
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Rest::acceptDrop(EditData& data) const
{
    EngravingItem* e = data.dropElement;
    ElementType type = e->type();
    if (
        (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_START)
        || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_MID)
        || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_NONE)
        || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_BEGIN_32)
        || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_BEGIN_64)
        || (type == ElementType::ACTION_ICON && toActionIcon(e)->actionType() == ActionIconType::BEAM_AUTO)
        || (type == ElementType::FERMATA)
        || (type == ElementType::CLEF)
        || (type == ElementType::KEYSIG)
        || (type == ElementType::TIMESIG)
        || (type == ElementType::SYSTEM_TEXT)
        || (type == ElementType::STAFF_TEXT)
        || (type == ElementType::PLAYTECH_ANNOTATION)
        || (type == ElementType::BAR_LINE)
        || (type == ElementType::BREATH)
        || (type == ElementType::CHORD)
        || (type == ElementType::NOTE)
        || (type == ElementType::STAFF_STATE)
        || (type == ElementType::INSTRUMENT_CHANGE)
        || (type == ElementType::DYNAMIC)
        || (type == ElementType::HARMONY)
        || (type == ElementType::TEMPO_TEXT)
        || (type == ElementType::REHEARSAL_MARK)
        || (type == ElementType::FRET_DIAGRAM)
        || (type == ElementType::TREMOLOBAR)
        || (type == ElementType::IMAGE)
        || (type == ElementType::SYMBOL)
        || (type == ElementType::MEASURE_REPEAT && durationType().type() == DurationType::V_MEASURE)
        ) {
        return true;
    }

    // prevent 'hanging' slurs, avoid crash on tie
    static const std::set<ElementType> ignoredTypes {
        ElementType::SLUR,
        ElementType::TIE,
        ElementType::GLISSANDO
    };

    return e->isSpanner() && !mu::contains(ignoredTypes, type);
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
    case ElementType::SYMBOL:
    case ElementType::IMAGE:
        e->setParent(this);
        score()->undoAddElement(e);
        return e;

    default:
        return ChordRest::drop(data);
    }
    return 0;
}

//---------------------------------------------------------
//   getSymbol
//---------------------------------------------------------

SymId Rest::getSymbol(DurationType type, int line, int lines, int* yoffset)
{
    *yoffset = 2;
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
        *yoffset = 1;
        return (line <= -2 || line >= (lines - 1)) ? SymId::restWholeLegerLine : SymId::restWhole;
    case DurationType::V_HALF:
        return (line <= -3 || line >= (lines - 2)) ? SymId::restHalfLegerLine : SymId::restHalf;
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
        qDebug("unknown rest type %d", int(type));
        return SymId::restQuarter;
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Rest::layout()
{
    if (m_gap) {
        return;
    }
    for (EngravingItem* e : el()) {
        e->layout();
    }
    qreal _spatium = spatium();

    rxpos() = 0.0;
    const StaffType* stt = staffType();
    if (stt && stt->isTabStaff()) {
        // if rests are shown and note values are shown as duration symbols
        if (stt->showRests() && stt->genDurations()) {
            DurationType type = durationType().type();
            int dots = durationType().dots();
            // if rest is whole measure, convert into actual type and dot values
            if (type == DurationType::V_MEASURE && measure()) {
                Fraction ticks = measure()->ticks();
                TDuration dur  = TDuration(ticks).type();
                type           = dur.type();
                dots           = dur.dots();
            }
            // symbol needed; if not exist, create, if exists, update duration
            if (!_tabDur) {
                _tabDur = new TabDurationSymbol(this, stt, type, dots);
            } else {
                _tabDur->setDuration(type, dots, stt);
            }
            _tabDur->setParent(this);
// needed?        _tabDur->setTrack(track());
            _tabDur->layout();
            setbbox(_tabDur->bbox());
            setPos(0.0, 0.0);                   // no rest is drawn: reset any position might be set for it
            return;
        }
        // if no rests or no duration symbols, delete any dur. symbol and chain into standard staff mngmt
        // this is to ensure horiz space is reserved for rest, even if they are not displayed
        // Rest::draw() will skip their drawing, if not needed
        if (_tabDur) {
            delete _tabDur;
            _tabDur = 0;
        }
    }

    m_dotline = Rest::getDotline(durationType().type());

    qreal yOff       = offset().y();
    const Staff* stf = staff();
    const StaffType* st = stf ? stf->staffTypeForElement(this) : 0;
    qreal lineDist = st ? st->lineDistance().val() : 1.0;
    int userLine   = yOff == 0.0 ? 0 : lrint(yOff / (lineDist * _spatium));
    int lines      = st ? st->lines() : 5;
    int lineOffset = computeLineOffset(lines);

    int yo;
    m_sym = getSymbol(durationType().type(), lineOffset / 2 + userLine, lines, &yo);
    rypos() = (qreal(yo) + qreal(lineOffset) * .5) * lineDist * _spatium;
    if (!shouldNotBeDrawn()) {
        setbbox(symBbox(m_sym));
    }
    layoutDots();
}

//---------------------------------------------------------
//   layoutDots
//---------------------------------------------------------

void Rest::layoutDots()
{
    checkDots();
    qreal x = symWidth(m_sym) + score()->styleMM(Sid::dotNoteDistance) * mag();
    qreal dx = score()->styleMM(Sid::dotDotDistance) * mag();
    qreal y = m_dotline * spatium() * .5;
    for (NoteDot* dot : m_dots) {
        dot->layout();
        dot->setPos(x, y);
        x += dx;
    }
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
    default:
        dl = -1;
        break;
    }
    return dl;
}

//---------------------------------------------------------
//   computeLineOffset
//---------------------------------------------------------

int Rest::computeLineOffset(int lines)
{
    Segment* s = segment();
    bool offsetVoices = s && measure() && (voice() > 0 || measure()->hasVoices(staffIdx(), tick(), actualTicks()));
    if (offsetVoices && voice() == 0) {
        // do not offset voice 1 rest if there exists a matching invisible rest in voice 2;
        EngravingItem* e = s->element(track() + 1);
        if (e && e->isRest() && !e->visible() && !toRest(e)->isGap()) {
            Rest* r = toRest(e);
            if (r->globalTicks() == globalTicks()) {
                offsetVoices = false;
            }
        }
    }

    if (offsetVoices && voice() < 2) {
        // in slash notation voices 1 and 2 are not offset outside the staff
        // if the staff contains slash notation then only offset rests in voices 3 and 4
        track_idx_t baseTrack = staffIdx() * VOICES;
        for (voice_idx_t v = 0; v < VOICES; ++v) {
            EngravingItem* e = s->element(baseTrack + v);
            if (e && e->isChord() && toChord(e)->slash()) {
                offsetVoices = false;
                break;
            }
        }
    }

    if (offsetVoices && staff()->mergeMatchingRests()) {
        // automatically merge matching rests if nothing in any other voice
        // this is not always the right thing to do do, but is useful in choral music
        // and can be enabled via a staff property
        bool matchFound = false;
        track_idx_t baseTrack = staffIdx() * VOICES;
        for (voice_idx_t v = 0; v < VOICES; ++v) {
            if (v == voice()) {
                continue;
            }
            EngravingItem* e = s->element(baseTrack + v);
            // try to find match in any other voice
            if (e) {
                if (e->type() == ElementType::REST) {
                    Rest* r = toRest(e);
                    if (r->globalTicks() == globalTicks()) {
                        matchFound = true;
                        continue;
                    }
                }
                // no match found; no sense looking for anything else
                matchFound = false;
                break;
            }
        }
        if (matchFound) {
            offsetVoices = false;
        }
    }

    int lineOffset    = 0;
    int assumedCenter = 4;
    int actualCenter  = (lines - 1);
    int centerDiff    = actualCenter - assumedCenter;

    if (offsetVoices) {
        // move rests in a multi voice context
        bool up = (voice() == 0) || (voice() == 2);         // TODO: use style values

        // Calculate extra offset to move rests above the highest resp. below the lowest note
        // of this segment (for measure rests, of the whole measure) in all opposite voices.
        // Ignore stems and articulations, because which multi-voice they are at the opposite end.
        int upOffset = up ? 1 : 0;
        int line = up ? 10 : -10;

        // For compatibility reasons apply automatic collision avoidance only if y-offset is unchanged
        if (qFuzzyIsNull(offset().y()) && autoplace()) {
            track_idx_t firstTrack = staffIdx() * 4;
            int extraOffsetForFewLines = lines < 5 ? 2 : 0;
            bool isMeasureRest = durationType().type() == DurationType::V_MEASURE;
            Segment* seg = isMeasureRest ? measure()->first() : s;
            while (seg) {
                for (const track_idx_t& track : { firstTrack + upOffset, firstTrack + 2 + upOffset }) {
                    EngravingItem* e = seg->element(track);
                    if (e && e->isChord()) {
                        Chord* chord = toChord(e);
                        StaffGroup staffGroup = staff()->staffType(chord->tick())->group();
                        for (Note* note : chord->notes()) {
                            int nline = staffGroup == StaffGroup::TAB
                                        ? note->string() * 2
                                        : note->line();
                            nline = nline - centerDiff;
                            if (up && nline <= line) {
                                line = nline - extraOffsetForFewLines;
                                if (note->accidentalType() != AccidentalType::NONE) {
                                    line--;
                                }
                            } else if (!up && nline >= line) {
                                line = nline + extraOffsetForFewLines;
                                if (note->accidentalType() != AccidentalType::NONE) {
                                    line++;
                                }
                            }
                        }
                    }
                }
                seg = isMeasureRest ? seg->next() : nullptr;
            }
        }

        switch (durationType().type()) {
        case DurationType::V_LONG:
            lineOffset = up ? -3 : 5;
            lineOffset += up ? (line < 5 ? line - 5 : 0) : (line > 5 ? line - 5 : 0);
            break;
        case DurationType::V_BREVE:
            lineOffset = up ? -3 : 5;
            lineOffset += up ? (line < 3 ? line - 3 : 0) : (line > 5 ? line - 5 : 0);
            break;
        case DurationType::V_MEASURE:
            if (ticks() >= Fraction(2, 1)) {     // breve symbol
                lineOffset = up ? -3 : 5;
                lineOffset += up ? (line < 3 ? line - 3 : 0) : (line > 5 ? line - 4 : 0);
            } else {
                lineOffset = up ? -4 : 6;                   // whole symbol
                lineOffset += up ? (line < 3 ? line - 2 : 0) : (line > 6 ? line - 5 : 0);
            }
            break;
        case DurationType::V_WHOLE:
            lineOffset = up ? -4 : 6;
            lineOffset += up ? (line < 3 ? line - 2 : 0) : (line > 6 ? line - 5 : 0);
            break;
        case DurationType::V_HALF:
            lineOffset = up ? -4 : 4;
            lineOffset += up ? (line < 2 ? line - 3 : 0) : (line > 5 ? line - 4 : 0);
            break;
        case DurationType::V_QUARTER:
            lineOffset = up ? -4 : 4;
            lineOffset += up ? (line < 5 ? line - 4 : 0) : (line > 3 ? line - 3 : 0);
            break;
        case DurationType::V_EIGHTH:
            lineOffset = up ? -4 : 4;
            lineOffset += up ? (line < 4 ? line - 4 : 0) : (line > 4 ? line - 4 : 0);
            break;
        case DurationType::V_16TH:
            lineOffset = up ? -6 : 4;
            lineOffset += up ? (line < 4 ? line - 4 : 0) : (line > 4 ? line - 4 : 0);
            break;
        case DurationType::V_32ND:
            lineOffset = up ? -6 : 6;
            lineOffset += up ? (line < 4 ? line - 4 : 0) : (line > 4 ? line - 4 : 0);
            break;
        case DurationType::V_64TH:
            lineOffset = up ? -8 : 6;
            lineOffset += up ? (line < 4 ? line - 4 : 0) : (line > 4 ? line - 4 : 0);
            break;
        case DurationType::V_128TH:
            lineOffset = up ? -8 : 8;
            lineOffset += up ? (line < 4 ? line - 4 : 0) : (line > 4 ? line - 4 : 0);
            break;
        case DurationType::V_1024TH:
        case DurationType::V_512TH:
        case DurationType::V_256TH:
            lineOffset = up ? -10 : 6;
            lineOffset += up ? (line < 4 ? line - 4 : 0) : (line > 4 ? line - 4 : 0);
            break;
        default:
            break;
        }

        // adjust offsets for staves with other than five lines
        if (lines != 5) {
            lineOffset += centerDiff;
            if (centerDiff & 1) {
                // round to line
                if (lines == 2 && staff() && staff()->lineDistance(tick()) < 2.0) {
                    // leave alone
                } else if (lines <= 6) {
                    lineOffset += lineOffset > 0 ? -1 : 1;              // round inward
                } else {
                    lineOffset += lineOffset > 0 ? 1 : -1;              // round outward
                }
            }
        }
    } else {
        // Gould says to center rests on middle line or space
        // but subjectively, many rests look strange centered on a space
        // so we do it for 2-line staves only
        if (centerDiff & 1 && lines != 2) {
            centerDiff += 1;        // round down
        }
        lineOffset = centerDiff;
        switch (durationType().type()) {
        case DurationType::V_LONG:
        case DurationType::V_BREVE:
        case DurationType::V_MEASURE:
        case DurationType::V_WHOLE:
            if (lineOffset & 1) {
                lineOffset += 1;                // always round to nearest line
            } else if (lines <= 3) {
                lineOffset += 2;                // special case - move down for 1-line or 3-line staff
            }
            break;
        case DurationType::V_HALF:
            if (lineOffset & 1) {
                lineOffset += 1;                // always round to nearest line
            }
            break;
        default:
            break;
        }
    }
    // DEBUG: subtract this off only to be added back in layout()?
    // that would throw off calculation of when ledger lines are needed
    //if (staff())
    //      lineOffset -= staff()->staffType()->stepOffset();
    return lineOffset;
}

//---------------------------------------------------------
//   upPos
//---------------------------------------------------------

qreal Rest::upPos() const
{
    return symBbox(m_sym).y();
}

//---------------------------------------------------------
//   downPos
//---------------------------------------------------------

qreal Rest::downPos() const
{
    return symBbox(m_sym).y() + symHeight(m_sym);
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

qreal Rest::mag() const
{
    qreal m = staff() ? staff()->staffMag(this) : 1.0;
    if (isSmall()) {
        m *= score()->styleD(Sid::smallNoteMag);
    }
    return m;
}

//---------------------------------------------------------
//   upLine
//---------------------------------------------------------

int Rest::upLine() const
{
    qreal _spatium = spatium();
    return lrint((pos().y() + bbox().top() + _spatium) * 2 / _spatium);
}

//---------------------------------------------------------
//   downLine
//---------------------------------------------------------

int Rest::downLine() const
{
    qreal _spatium = spatium();
    return lrint((pos().y() + bbox().top() + _spatium) * 2 / _spatium);
}

//---------------------------------------------------------
//   stemPos
//    point to connect stem
//---------------------------------------------------------

PointF Rest::stemPos() const
{
    return pagePos();
}

//---------------------------------------------------------
//   stemPosBeam
//    return stem position of note on beam side
//    return canvas coordinates
//---------------------------------------------------------

PointF Rest::stemPosBeam() const
{
    PointF p(pagePos());
    if (_up) {
        p.ry() += bbox().top() + spatium() * 1.5;
    } else {
        p.ry() += bbox().bottom() - spatium() * 1.5;
    }
    return p;
}

//---------------------------------------------------------
//   stemPosX
//---------------------------------------------------------

qreal Rest::stemPosX() const
{
    if (_up) {
        return bbox().right();
    } else {
        return bbox().left();
    }
}

//---------------------------------------------------------
//   rightEdge
//---------------------------------------------------------

qreal Rest::rightEdge() const
{
    return x() + width();
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
            qreal yOffset = -(bbox().bottom());
            if (durationType() >= DurationType::V_HALF) {
                yOffset -= staff()->spatium(tick()) * 0.5;
            }
            rypos() += yOffset;
        }
    }
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Rest::accessibleInfo() const
{
    QString voice = QObject::tr("Voice: %1").arg(QString::number(track() % VOICES + 1));
    return QObject::tr("%1; Duration: %2; %3").arg(EngravingItem::accessibleInfo(), durationUserName(), voice);
}

//---------------------------------------------------------
//   screenReaderInfo
//---------------------------------------------------------

QString Rest::screenReaderInfo() const
{
    Measure* m = measure();
    bool voices = m ? m->hasVoices(staffIdx()) : false;
    QString voice = voices ? QObject::tr("Voice: %1").arg(QString::number(track() % VOICES + 1)) : "";
    return QString("%1 %2 %3").arg(EngravingItem::accessibleInfo(), durationUserName(), voice);
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
    case ElementType::SYMBOL:
    case ElementType::IMAGE:
        el().push_back(e);
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
    case ElementType::SYMBOL:
    case ElementType::IMAGE:
        if (!el().remove(e)) {
            qDebug("Rest::remove(): cannot find %s", e->typeName());
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
//   Rest::write
//---------------------------------------------------------

void Rest::write(XmlWriter& xml) const
{
    if (m_gap) {
        return;
    }
    writeBeam(xml);
    xml.startObject(this);
    writeStyledProperties(xml);
    ChordRest::writeProperties(xml);
    el().write(xml);
    bool write_dots = false;
    for (NoteDot* dot : m_dots) {
        if (!dot->offset().isNull() || !dot->visible() || dot->color() != engravingConfiguration()->defaultColor()
            || dot->visible() != visible()) {
            write_dots = true;
            break;
        }
    }
    if (write_dots) {
        for (NoteDot* dot: m_dots) {
            dot->write(xml);
        }
    }
    xml.endObject();
}

//---------------------------------------------------------
//   Rest::read
//---------------------------------------------------------

void Rest::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "Symbol") {
            Symbol* s = new Symbol(this);
            s->setTrack(track());
            s->read(e);
            add(s);
        } else if (tag == "Image") {
            if (MScore::noImages) {
                e.skipCurrentElement();
            } else {
                Image* image = new Image(this);
                image->setTrack(track());
                image->read(e);
                add(image);
            }
        } else if (tag == "NoteDot") {
            NoteDot* dot = Factory::createNoteDot(this);
            dot->read(e);
            add(dot);
        } else if (readStyledProperty(e, tag)) {
        } else if (ChordRest::readProperties(e)) {
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   localSpatiumChanged
//---------------------------------------------------------

void Rest::localSpatiumChanged(qreal oldValue, qreal newValue)
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
        triggerLayout();
        break;
    case Pid::VISIBLE:
        setVisible(v.toBool());
        triggerLayout();
        break;
    case Pid::OFFSET:
        score()->addRefresh(canvasBoundingRect());
        setOffset(v.value<PointF>());
        layout();
        score()->addRefresh(canvasBoundingRect());
        if (measure() && durationType().type() == DurationType::V_MEASURE) {
            measure()->triggerLayout();
        }
        triggerLayout();
        break;
    default:
        return ChordRest::setProperty(propertyId, v);
    }
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
//   shape
//---------------------------------------------------------

Shape Rest::shape() const
{
    Shape shape;
    if (!m_gap) {
        shape.add(ChordRest::shape());
        shape.add(bbox(), this);
        for (NoteDot* dot : m_dots) {
            shape.add(symBbox(SymId::augmentationDot).translated(dot->pos()));
        }
    }
    for (EngravingItem* e : el()) {
        if (e->addToSkyline()) {
            shape.add(e->shape().translated(e->pos()));
        }
    }
    return shape;
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Rest::editDrag(EditData& editData)
{
    Segment* seg = segment();

    if (editData.modifiers & Qt::ShiftModifier) {
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
    if (generated()
        || (st && st->isTabStaff() && (!st->showRests() || st->genDurations())
            && (!measure() || !measure()->isMMRest()))
        || (measure() && measure()->measureRepeatCount(staffIdx()))) {
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
