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

/* TO DO:
- XML export

NICE-TO-HAVE TODO:
- draggable handles of glissando segments
- re-attachable glissando extrema (with [Shift]+arrows, use SlurSegment::edit()
      and SlurSegment::changeAnchor() in slur.cpp as models)
*/

#include "glissando.h"

#include <cmath>
#include <algorithm>

#include "types/typesconv.h"

#include "chord.h"
#include "harppedaldiagram.h"
#include "measure.h"
#include "note.h"
#include "part.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "system.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
static const ElementStyle glissandoElementStyle {
    { Sid::glissandoFontFace,    Pid::FONT_FACE },
    { Sid::glissandoFontSize,    Pid::FONT_SIZE },
    { Sid::glissandoFontStyle,   Pid::FONT_STYLE },
    { Sid::glissandoLineWidth,   Pid::LINE_WIDTH },
    { Sid::glissandoShowText,    Pid::GLISS_SHOW_TEXT },
    { Sid::glissandoText,        Pid::GLISS_TEXT },
    { Sid::glissandoStyle,       Pid::GLISS_STYLE },
    { Sid::glissandoLineStyle,   Pid::LINE_STYLE },
    { Sid::glissandoDashLineLen, Pid::DASH_LINE_LEN },
    { Sid::glissandoDashGapLen,  Pid::DASH_GAP_LEN },
    { Sid::glissandoType,        Pid::GLISS_TYPE }
};

//=========================================================
//   GlissandoSegment
//=========================================================

GlissandoSegment::GlissandoSegment(Glissando* sp, System* parent)
    : LineSegment(ElementType::GLISSANDO_SEGMENT, sp, parent, ElementFlag::MOVABLE)
{
}

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

EngravingItem* GlissandoSegment::propertyDelegate(Pid pid)
{
    switch (pid) {
    case Pid::GLISS_TYPE:
    case Pid::GLISS_TEXT:
    case Pid::GLISS_SHOW_TEXT:
    case Pid::GLISS_STYLE:
    case Pid::GLISS_SHIFT:
    case Pid::GLISS_EASEIN:
    case Pid::GLISS_EASEOUT:
    case Pid::FONT_FACE:
    case Pid::FONT_SIZE:
    case Pid::FONT_STYLE:
    case Pid::LINE_WIDTH:
        return glissando();
    default:
        return LineSegment::propertyDelegate(pid);
    }
}

//=========================================================
//   Glissando
//=========================================================

Glissando::Glissando(EngravingItem* parent)
    : SLine(ElementType::GLISSANDO, parent, ElementFlag::MOVABLE)
{
    initElementStyle(&glissandoElementStyle);

    static const std::array<Pid, 5> propertiesToInitialise {
        Pid::GLISS_SHIFT,
        Pid::GLISS_EASEIN,
        Pid::GLISS_EASEOUT,
        Pid::DIAGONAL,
        Pid::ANCHOR
    };

    for (const Pid& pid : propertiesToInitialise) {
        resetProperty(pid);
    }
}

Glissando::Glissando(const Glissando& g)
    : SLine(g)
{
    _text           = g._text;
    _fontFace       = g._fontFace;
    _fontSize       = g._fontSize;
    _glissandoType  = g._glissandoType;
    _glissandoStyle = g._glissandoStyle;
    _glissandoShift = g._glissandoShift;
    _easeIn         = g._easeIn;
    _easeOut        = g._easeOut;
    _showText       = g._showText;
    _fontStyle      = g._fontStyle;
    m_isHarpGliss   = g.m_isHarpGliss;
}

const TranslatableString& Glissando::glissandoTypeName() const
{
    return TConv::userName(glissandoType());
}

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Glissando::createLineSegment(System* parent)
{
    GlissandoSegment* seg = new GlissandoSegment(this, parent);
    seg->setTrack(track());
    seg->setColor(color());
    return seg;
}

Sid Glissando::getPropertyStyle(Pid id) const
{
    if (id == Pid::GLISS_STYLE) {
        return isHarpGliss().value_or(false) ? Sid::glissandoStyleHarp : Sid::glissandoStyle;
    }

    return SLine::getPropertyStyle(id);
}

bool Glissando::pitchSteps(const Spanner* spanner, std::vector<int>& pitchOffsets)
{
    if (!spanner->endElement()->isNote()) {
        return false;
    }
    const Glissando* glissando = toGlissando(spanner);
    if (!glissando->playSpanner()) {
        return false;
    }
    GlissandoStyle glissandoStyle = glissando->glissandoStyle();
    if (glissandoStyle == GlissandoStyle::PORTAMENTO) {
        return false;
    }
    // only consider glissando connected to NOTE.
    const Note* noteStart = toNote(spanner->startElement());
    const Note* noteEnd = toNote(spanner->endElement());
    int pitchStart = noteStart->ppitch();
    int pitchEnd = noteEnd->ppitch();
    if (pitchEnd == pitchStart) {
        return false;
    }
    int direction = pitchEnd > pitchStart ? 1 : -1;
    pitchOffsets.clear();
    if (glissandoStyle == GlissandoStyle::DIATONIC) {
        // Obey harp pedal diagrams if on a harp staff
        if (glissando->isHarpGliss().value_or(false)) {
            HarpPedalDiagram* hd = spanner->part()->currentHarpDiagram(spanner->tick());
            std::set<int> playableTpcs = hd ? hd->playableTpcs() : std::set<int>({ 14, 16, 18, 13, 15, 17, 19 });
            std::vector<int> playablePitches;
            for (int t : playableTpcs) {
                playablePitches.push_back(tpc2pitch(t) % PITCH_DELTA_OCTAVE);
            }

            // Push starting note, then check for enharmonic on the next string.  If there is an enharmonic, 0 will be pushed back twice
            pitchOffsets.push_back(0);
            int en = noteStart->tpc() + TPC_DELTA_ENHARMONIC * -direction;
            // Harp pedalling will only have 1 flat or sharp
            if (en >= TPC_F_B && en <= TPC_B_S && playableTpcs.find(en) != playableTpcs.end()) {
                pitchOffsets.push_back(0);
            }

            for (int p = pitchStart + direction; p != pitchEnd; p += direction) {
                // Count times pitch occurs in harp pedalling - this accounts for enharmonics
                int pitchOccurrences = std::count(playablePitches.begin(), playablePitches.end(), p % PITCH_DELTA_OCTAVE);
                if (pitchOccurrences > 0) {
                    pitchOffsets.insert(pitchOffsets.end(), pitchOccurrences, p - pitchStart);
                }
            }

            // Check for enharmonic at end, in correct direction
            en = noteEnd->tpc() + TPC_DELTA_ENHARMONIC * direction;
            if (en >= TPC_F_B && en <= TPC_B_S && playableTpcs.find(en) != playableTpcs.end()) {
                pitchOffsets.push_back(pitchEnd - pitchStart);
            }

            return pitchOffsets.size() > 0;
        }

        // Regular diatonic mode
        int lineStart = noteStart->line();
        // scale obeying accidentals
        for (int line = lineStart, pitch = pitchStart; (direction == 1) ? (pitch < pitchEnd) : (pitch > pitchEnd); line -= direction) {
            int halfSteps = chromaticPitchSteps(noteStart, noteEnd, lineStart - line);
            pitch = pitchStart + halfSteps;
            if ((direction == 1) ? (pitch < pitchEnd) : (pitch > pitchEnd)) {
                pitchOffsets.push_back(halfSteps);
            }
        }
        return pitchOffsets.size() > 0;
    }
    if (glissandoStyle == GlissandoStyle::CHROMATIC) {
        for (int pitch = pitchStart; pitch != pitchEnd; pitch += direction) {
            pitchOffsets.push_back(pitch - pitchStart);
        }
        return true;
    }
    static const std::vector<bool> whiteNotes = { true, false, true, false, true, true, false, true, false, true, false, true };
    int Cnote = 60;   // pitch of middle C
    bool notePick = glissandoStyle == GlissandoStyle::WHITE_KEYS;
    for (int pitch = pitchStart; pitch != pitchEnd; pitch += direction) {
        int idx = ((pitch - Cnote) + 1200) % 12;
        if (whiteNotes[idx] == notePick) {
            pitchOffsets.push_back(pitch - pitchStart);
        }
    }
    return true;
}

//---------------------------------------------------------
//   STATIC FUNCTIONS: guessInitialNote
//
//    Used while reading old scores (either 1.x or transitional 2.0) to determine (guess!)
//    the glissando initial note from its final chord. Returns the top note of previous chord
//    of the same instrument, preferring the chord in the same track as chord, if it exists.
//
//    CANNOT be called if the final chord and/or its segment do not exist yet in the score
//
//    Parameter:  chord: the chord this glissando ends into
//    Returns:    the top note in a suitable previous chord or nullptr if none found.
//---------------------------------------------------------

Note* Glissando::guessInitialNote(Chord* chord)
{
    switch (chord->noteType()) {
//            case NoteType::INVALID:
//                  return 0;
    // for grace notes before, previous chord is previous chord of parent chord
    case NoteType::ACCIACCATURA:
    case NoteType::APPOGGIATURA:
    case NoteType::GRACE4:
    case NoteType::GRACE16:
    case NoteType::GRACE32:
        // move unto parent chord and proceed to standard case
        if (chord->explicitParent() && chord->explicitParent()->isChord()) {
            chord = toChord(chord->explicitParent());
        } else {
            return 0;
        }
        break;
    // for grace notes after, return top note of parent chord
    case NoteType::GRACE8_AFTER:
    case NoteType::GRACE16_AFTER:
    case NoteType::GRACE32_AFTER:
        if (chord->explicitParent() && chord->explicitParent()->isChord()) {
            return toChord(chord->explicitParent())->upNote();
        } else {                                // no parent or parent is not a chord?
            return nullptr;
        }
    case NoteType::NORMAL:
    {
        // if chord has grace notes before, the last one is the previous note
        std::vector<Chord*> graces = chord->graceNotesBefore();
        if (graces.size() > 0) {
            return graces.back()->upNote();
        }
    }
    break;                                      // else process to standard case
    default:
        break;
    }

    // standard case (NORMAL or grace before chord)

    // if parent not a segment, can't locate a target note
    if (!chord->explicitParent()->isSegment()) {
        return 0;
    }

    track_idx_t chordTrack = chord->track();
    Segment* segm = chord->segment();
    Part* part = chord->part();
    if (segm != nullptr) {
        segm = segm->prev1();
    }
    while (segm) {
        // if previous segment is a ChordRest segment
        if (segm->segmentType() == SegmentType::ChordRest) {
            Chord* target = nullptr;
            // look for a Chord in the same track
            if (segm->element(chordTrack) && segm->element(chordTrack)->isChord()) {
                target = toChord(segm->element(chordTrack));
            } else {                 // if no same track, look for other chords in the same instrument
                for (EngravingItem* currChord : segm->elist()) {
                    if (currChord && currChord->isChord() && toChord(currChord)->part() == part) {
                        target = toChord(currChord);
                        break;
                    }
                }
            }
            // if we found a target previous chord
            if (target) {
                // if chord has grace notes after, the last one is the previous note
                std::vector<Chord*> graces = target->graceNotesAfter();
                if (graces.size() > 0) {
                    return graces.back()->upNote();
                }
                return target->upNote();              // if no grace after, return top note
            }
        }
        segm = segm->prev1();
    }
    LOGD("no first note for glissando found");
    return 0;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Glissando::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::GLISS_TYPE:
        return int(glissandoType());
    case Pid::GLISS_TEXT:
        return text();
    case Pid::GLISS_SHOW_TEXT:
        return showText();
    case Pid::GLISS_STYLE:
        return glissandoStyle();
    case Pid::GLISS_SHIFT:
        return glissandoShift();
    case Pid::GLISS_EASEIN:
        return easeIn();
    case Pid::GLISS_EASEOUT:
        return easeOut();
    case Pid::FONT_FACE:
        return _fontFace;
    case Pid::FONT_SIZE:
        return _fontSize;
    case Pid::FONT_STYLE:
        return int(_fontStyle);
    default:
        break;
    }
    return SLine::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Glissando::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::GLISS_TYPE:
        setGlissandoType(GlissandoType(v.toInt()));
        break;
    case Pid::GLISS_TEXT:
        setText(v.value<String>());
        break;
    case Pid::GLISS_SHOW_TEXT:
        setShowText(v.toBool());
        break;
    case Pid::GLISS_STYLE:
    {
        // Make sure harp glisses can only be diatonic and chromatic
        GlissandoStyle glissStyle = v.value<GlissandoStyle>();
        if (isHarpGliss().value_or(false) && (glissStyle != GlissandoStyle::DIATONIC && glissStyle != GlissandoStyle::CHROMATIC)) {
            glissStyle = GlissandoStyle::DIATONIC;
        }
        setGlissandoStyle(glissStyle);
        break;
    }
    case Pid::GLISS_SHIFT:
        setGlissandoShift(v.toBool());
        break;
    case Pid::GLISS_EASEIN:
        setEaseIn(v.toInt());
        break;
    case Pid::GLISS_EASEOUT:
        setEaseOut(v.toInt());
        break;
    case Pid::FONT_FACE:
        setFontFace(v.value<String>());
        break;
    case Pid::FONT_SIZE:
        setFontSize(v.toReal());
        break;
    case Pid::FONT_STYLE:
        setFontStyle(FontStyle(v.toInt()));
        break;
    default:
        if (!SLine::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Glissando::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::GLISS_TYPE:
        return style().styleV(Sid::glissandoType);
    case Pid::GLISS_SHOW_TEXT:
        return true;
    case Pid::GLISS_STYLE:
        return style().styleV(getPropertyStyle(propertyId));
    case Pid::GLISS_SHIFT:
        return false;
    case Pid::GLISS_EASEIN:
    case Pid::GLISS_EASEOUT:
        return 0;
    case Pid::GLISS_TEXT:
        return style().styleV(Sid::glissandoText);
    case Pid::DIAGONAL:
        return true;
    case Pid::ANCHOR:
        return int(Spanner::Anchor::NOTE);
    default:
        break;
    }
    return SLine::propertyDefault(propertyId);
}
}
