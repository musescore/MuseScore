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

#include "ambitus.h"

#include "translation.h"

#include "accidental.h"
#include "chord.h"
#include "factory.h"
#include "measure.h"
#include "note.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "system.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

const Spatium Ambitus::LINEWIDTH_DEFAULT = Spatium(0.12);
//---------------------------------------------------------
//   Ambitus
//---------------------------------------------------------

Ambitus::Ambitus(Segment* parent)
    : EngravingItem(ElementType::AMBITUS, parent, ElementFlag::ON_STAFF)
{
    m_noteHeadGroup    = NOTEHEADGROUP_DEFAULT;
    m_noteHeadType     = NOTEHEADTYPE_DEFAULT;
    m_direction        = DIRECTION_DEFAULT;
    m_hasLine          = HASLINE_DEFAULT;
    m_lineWidth        = LINEWIDTH_DEFAULT;
    m_topPitch         = INVALID_PITCH;
    m_bottomPitch      = INVALID_PITCH;
    m_topTpc           = Tpc::TPC_INVALID;
    m_bottomTpc        = Tpc::TPC_INVALID;

    m_topAccidental = Factory::createAccidental(this, false);
    m_bottomAccidental = Factory::createAccidental(this, false);
    m_topAccidental->setParent(this);
    m_bottomAccidental->setParent(this);
}

Ambitus::Ambitus(const Ambitus& a)
    : EngravingItem(a)
{
    m_noteHeadGroup = a.m_noteHeadGroup;
    m_noteHeadType = a.m_noteHeadType;
    m_direction = a.m_direction;
    m_hasLine = a.m_hasLine;
    m_lineWidth = a.m_lineWidth;

    m_topAccidental = a.m_topAccidental->clone();
    m_topAccidental->setParent(this);

    m_bottomAccidental = a.m_bottomAccidental->clone();
    m_bottomAccidental->setParent(this);

    m_topPitch = a.m_topPitch;
    m_topTpc = a.m_topTpc;
    m_bottomPitch = a.m_bottomPitch;
    m_bottomTpc = a.m_bottomTpc;
}

Ambitus::~Ambitus()
{
    delete m_topAccidental;
    delete m_bottomAccidental;
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double Ambitus::mag() const
{
    return staff() ? staff()->staffMag(tick()) : 1.0;
}

//---------------------------------------------------------
//   initFrom
//---------------------------------------------------------

void Ambitus::initFrom(Ambitus* a)
{
    m_noteHeadGroup   = a->m_noteHeadGroup;
    m_noteHeadType    = a->m_noteHeadType;
    m_direction       = a->m_direction;
    m_hasLine         = a->m_hasLine;
    m_lineWidth       = a->m_lineWidth;
    m_topPitch        = a->m_topPitch;
    m_bottomPitch     = a->m_bottomPitch;
    m_topTpc          = a->m_topTpc;
    m_bottomTpc       = a->m_bottomTpc;
}

//---------------------------------------------------------
//   setTrack
//
//    when the Ambitus element is assigned a track,
//    initialize top and bottom 'notes' to top and bottom staff lines
//---------------------------------------------------------

void Ambitus::setTrack(track_idx_t t)
{
    Segment* segm  = segment();
    Staff* stf   = score()->staff(track2staff(t));

    EngravingItem::setTrack(t);
    // if not initialized and there is a segment and a staff,
    // initialize pitches and tpc's to first and last staff line
    // (for use in palettes)
    if (m_topPitch == INVALID_PITCH || m_topTpc == Tpc::TPC_INVALID
        || m_bottomPitch == INVALID_PITCH || m_bottomTpc == Tpc::TPC_INVALID) {
        if (segm && stf) {
            Ambitus::Ranges ranges = estimateRanges();
            m_topTpc = ranges.topTpc;
            m_bottomTpc = ranges.bottomTpc;
            m_topPitch = ranges.topPitch;
            m_bottomPitch = ranges.bottomPitch;

            m_topAccidental->setTrack(t);
            m_bottomAccidental->setTrack(t);
        }
//            else {
//                  _topPitch = _bottomPitch = INVALID_PITCH;
//                  _topTpc   = _bottomTpc   = Tpc::TPC_INVALID;
    }
}

//---------------------------------------------------------
//   setTop/BottomPitch
//
//    setting either pitch requires to adjust the corresponding tpc
//---------------------------------------------------------

void Ambitus::setTopPitch(int val, bool applyLogic)
{
    if (!applyLogic) {
        m_topPitch = val;
        return;
    }

    int deltaPitch = val - topPitch();
    // if deltaPitch is not an integer number of octaves, adjust tpc
    // (to avoid 'wild' tpc changes with octave changes)
    if (deltaPitch % PITCH_DELTA_OCTAVE != 0) {
        int newTpc = topTpc() + deltaPitch * TPC_DELTA_SEMITONE;
        // reduce newTpc into acceptable range via enharmonic
        while (newTpc < Tpc::TPC_MIN) {
            newTpc += TPC_DELTA_ENHARMONIC;
        }
        while (newTpc > Tpc::TPC_MAX) {
            newTpc -= TPC_DELTA_ENHARMONIC;
        }
        m_topTpc = newTpc;
    }
    m_topPitch = val;
    normalize();
}

void Ambitus::setBottomPitch(int val, bool applyLogic)
{
    if (!applyLogic) {
        m_bottomPitch = val;
        return;
    }

    int deltaPitch = val - bottomPitch();
    // if deltaPitch is not an integer number of octaves, adjust tpc
    // (to avoid 'wild' tpc changes with octave changes)
    if (deltaPitch % PITCH_DELTA_OCTAVE != 0) {
        int newTpc = bottomTpc() + deltaPitch * TPC_DELTA_SEMITONE;
        // reduce newTpc into acceptable range via enharmonic
        while (newTpc < Tpc::TPC_MIN) {
            newTpc += TPC_DELTA_ENHARMONIC;
        }
        while (newTpc > Tpc::TPC_MAX) {
            newTpc -= TPC_DELTA_ENHARMONIC;
        }
        m_bottomTpc = newTpc;
    }
    m_bottomPitch = val;
    normalize();
}

//---------------------------------------------------------
//   setTop/BottomTpc
//
//    setting either tpc requires to adjust the corresponding pitch
//    (but remaining in the same octave)
//---------------------------------------------------------

void Ambitus::setTopTpc(int val, bool applyLogic)
{
    if (!applyLogic) {
        m_topTpc = val;
        return;
    }

    int octave = topPitch() / PITCH_DELTA_OCTAVE;
    int deltaTpc = val - topTpc();
    // get new pitch according to tpc change
    int newPitch = topPitch() + deltaTpc * TPC_DELTA_SEMITONE;
    // reduce pitch to the same octave as original pitch
    newPitch = (octave * PITCH_DELTA_OCTAVE) + (newPitch % PITCH_DELTA_OCTAVE);
    m_topPitch = newPitch;
    m_topTpc = val;
    normalize();
}

void Ambitus::setBottomTpc(int val, bool applyLogic)
{
    if (!applyLogic) {
        m_bottomTpc = val;
        return;
    }

    int octave = bottomPitch() / PITCH_DELTA_OCTAVE;
    int deltaTpc = val - bottomTpc();
    // get new pitch according to tpc change
    int newPitch = bottomPitch() + deltaTpc * TPC_DELTA_SEMITONE;
    // reduce pitch to the same octave as original pitch
    newPitch = (octave * PITCH_DELTA_OCTAVE) + (newPitch % PITCH_DELTA_OCTAVE);
    m_bottomPitch = newPitch;
    m_bottomTpc = val;
    normalize();
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Ambitus::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    UNUSED(all);
    func(data, this);
    if (m_topAccidental->accidentalType() != AccidentalType::NONE) {
        func(data, m_topAccidental);
    }

    if (m_bottomAccidental->accidentalType() != AccidentalType::NONE) {
        func(data, m_bottomAccidental);
    }
}

//---------------------------------------------------------
//   noteHead
//---------------------------------------------------------

SymId Ambitus::noteHead() const
{
    int hg = 1;
    NoteHeadType ht  = NoteHeadType::HEAD_QUARTER;

    if (m_noteHeadType != NoteHeadType::HEAD_AUTO) {
        ht = m_noteHeadType;
    }

    SymId t = Note::noteHead(hg, m_noteHeadGroup, ht);
    if (t == SymId::noSym) {
        LOGD("invalid notehead %d/%d", int(m_noteHeadGroup), int(m_noteHeadType));
        t = Note::noteHead(0, NoteHeadGroup::HEAD_NORMAL, ht);
    }
    return t;
}

//---------------------------------------------------------
//   headWidth
//
//    returns the width of the notehead symbol
//---------------------------------------------------------

double Ambitus::headWidth() const
{
//      int head  = noteHead();
//      double val = symbols[score()->symIdx()][head].width(magS());
//      return val;
    return symWidth(noteHead());
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

PointF Ambitus::pagePos() const
{
    if (explicitParent() == 0) {
        return pos();
    }
    System* system = segment()->measure()->system();
    double yp = y();
    if (system) {
        yp += system->staff(staffIdx())->y() + system->y();
    }
    return PointF(pageX(), yp);
}

//---------------------------------------------------------
//   normalize
//
//    makes sure topPitch is not < bottomPitch
//---------------------------------------------------------

void Ambitus::normalize()
{
    if (m_topPitch < m_bottomPitch) {
        std::swap(m_topPitch, m_bottomPitch);
        std::swap(m_topTpc, m_bottomTpc);
    }
}

//---------------------------------------------------------
//   updateRange
//
//    scans the staff contents up to next section break to update the range pitches/tpc's
//---------------------------------------------------------

Ambitus::Ranges Ambitus::estimateRanges() const
{
    Ambitus::Ranges result;

    if (!segment()) {
        return result;
    }
    Chord* chord;
    track_idx_t firstTrack  = track();
    track_idx_t lastTrack   = firstTrack + VOICES - 1;
    int pitchTop    = -1000;
    int pitchBottom = 1000;
    int tpcTop      = 0;    // Initialized to prevent warning
    int tpcBottom   = 0;    // Initialized to prevent warning
    track_idx_t trk;
    Measure* meas     = segment()->measure();
    Segment* segm     = meas->findSegment(SegmentType::ChordRest, segment()->tick());
    bool stop     = meas->sectionBreak();
    while (segm) {
        // moved to another measure?
        if (segm->measure() != meas) {
            // if section break has been found, stop here
            if (stop) {
                break;
            }
            // update meas and stop condition
            meas = segm->measure();
            stop = meas->sectionBreak();
        }
        // scan all relevant tracks of this segment for chords
        for (trk = firstTrack; trk <= lastTrack; trk++) {
            EngravingItem* e = segm->element(trk);
            if (!e || !e->isChord()) {
                continue;
            }
            chord = toChord(e);
            // update pitch range (with associated tpc's)
            for (Note* n : chord->notes()) {
                if (!n->play()) {         // skip notes which are not to be played
                    continue;
                }
                int pitch = n->epitch();
                if (pitch > pitchTop) {
                    pitchTop = pitch;
                    tpcTop   = n->tpc();
                }
                if (pitch < pitchBottom) {
                    pitchBottom = pitch;
                    tpcBottom   = n->tpc();
                }
            }
        }
        segm = segm->nextCR();
    }

    if (pitchTop > -1000) {               // if something has been found, update this
        result.topPitch    = pitchTop;
        result.bottomPitch = pitchBottom;
        result.topTpc      = tpcTop;
        result.bottomTpc   = tpcBottom;
    }

    return result;
}

void Ambitus::remove(EngravingItem* e)
{
    if (e->type() == ElementType::ACCIDENTAL) {
        //! NOTE Do nothing (removing _topAccid or _bottomAccid)
        return;
    }

    EngravingItem::remove(e);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Ambitus::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::HEAD_GROUP:
        return int(noteHeadGroup());
    case Pid::HEAD_TYPE:
        return int(noteHeadType());
    case Pid::MIRROR_HEAD:
        return int(direction());
    case Pid::GHOST:                         // recycled property = _hasLine
        return hasLine();
    case Pid::LINE_WIDTH:
        return lineWidth();
    case Pid::TPC1:
        return topTpc();
    case Pid::FBPARENTHESIS1:                // recycled property = _bottomTpc
        return bottomTpc();
    case Pid::PITCH:
        return topPitch();
    case Pid::FBPARENTHESIS2:                // recycled property = _bottomPitch
        return bottomPitch();
    case Pid::FBPARENTHESIS3:                // recycled property = octave of _topPitch
        return topOctave();
    case Pid::FBPARENTHESIS4:                // recycled property = octave of _bottomPitch
        return bottomOctave();
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Ambitus::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::HEAD_GROUP:
        setNoteHeadGroup(v.value<NoteHeadGroup>());
        break;
    case Pid::HEAD_TYPE:
        setNoteHeadType(v.value<NoteHeadType>());
        break;
    case Pid::MIRROR_HEAD:
        setDirection(v.value<DirectionH>());
        break;
    case Pid::GHOST:                         // recycled property = _hasLine
        setHasLine(v.toBool());
        break;
    case Pid::LINE_WIDTH:
        setLineWidth(v.value<Spatium>());
        break;
    case Pid::TPC1:
        setTopTpc(v.toInt());
        break;
    case Pid::FBPARENTHESIS1:                // recycled property = _bottomTpc
        setBottomTpc(v.toInt());
        break;
    case Pid::PITCH:
        setTopPitch(v.toInt());
        break;
    case Pid::FBPARENTHESIS2:                // recycled property = _bottomPitch
        setBottomPitch(v.toInt());
        break;
    case Pid::FBPARENTHESIS3:                // recycled property = octave of _topPitch
        setTopPitch(topPitch() % 12 + (v.toInt() + 1) * 12);
        break;
    case Pid::FBPARENTHESIS4:                // recycled property = octave of _bottomPitch
        setBottomPitch(bottomPitch() % 12 + (v.toInt() + 1) * 12);
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

PropertyValue Ambitus::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::HEAD_GROUP:
        return int(NOTEHEADGROUP_DEFAULT);
    case Pid::HEAD_TYPE:
        return int(NOTEHEADTYPE_DEFAULT);
    case Pid::MIRROR_HEAD:
        return int(DIRECTION_DEFAULT);
    case Pid::GHOST:
        return HASLINE_DEFAULT;
    case Pid::LINE_WIDTH:
        return Spatium(LINEWIDTH_DEFAULT);
    case Pid::TPC1:
        return estimateRanges().topTpc;
    case Pid::FBPARENTHESIS1:
        return estimateRanges().bottomTpc;
    case Pid::PITCH:
        return estimateRanges().topPitch;
    case Pid::FBPARENTHESIS2:
        return estimateRanges().bottomPitch;
    case Pid::FBPARENTHESIS3:
        return int(estimateRanges().topPitch / 12) - 1;
    case Pid::FBPARENTHESIS4:
        return int(estimateRanges().bottomPitch / 12) - 1;
    default:
        return EngravingItem::propertyDefault(id);
    }
    //return QVariant();
}

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

EngravingItem* Ambitus::nextSegmentElement()
{
    return segment()->firstInNextSegments(staffIdx());
}

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

EngravingItem* Ambitus::prevSegmentElement()
{
    return segment()->lastInPrevSegments(staffIdx());
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Ambitus::accessibleInfo() const
{
    return EngravingItem::accessibleInfo() + u"; "
           + muse::mtrc("engraving", "Top pitch: %1; Bottom pitch: %2")
           .arg(tpc2name(topTpc(), NoteSpellingType::STANDARD, NoteCaseType::AUTO, false) + String::number(topOctave()),
                tpc2name(bottomTpc(), NoteSpellingType::STANDARD, NoteCaseType::AUTO, false) + String::number(bottomOctave()));
}

AccidentalType Ambitus::accidentalType(int tpc, Key key)
{
    AccidentalType accidType = AccidentalType::NONE;
    // if (13 <= (tpc - key) <= 19) there is no accidental)
    if (tpc - int(key) >= 13 && tpc - int(key) <= 19) {
        accidType = AccidentalType::NONE;
    } else {
        AccidentalVal accidVal = tpc2alter(tpc);
        accidType = Accidental::value2subtype(accidVal);
        if (accidType == AccidentalType::NONE) {
            accidType = AccidentalType::NATURAL;
        }
    }

    return accidType;
}

//! TODO Seems to need to be moved to utilities
int Ambitus::staffLine(int tpc, int pitch, ClefType clf)
{
    int line = absStep(tpc, pitch);
    line = relStep(line, clf);
    return line;
}
