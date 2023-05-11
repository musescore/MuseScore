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

#include "draw/fontmetrics.h"
#include "draw/types/pen.h"
#include "style/style.h"
#include "layout/tlayout.h"
#include "types/typesconv.h"
#include "iengravingfont.h"

#include "chord.h"
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

namespace mu::engraving {
static const ElementStyle glissandoElementStyle {
    { Sid::glissandoFontFace,  Pid::FONT_FACE },
    { Sid::glissandoFontSize,  Pid::FONT_SIZE },
    { Sid::glissandoFontStyle, Pid::FONT_STYLE },
    { Sid::glissandoLineWidth, Pid::LINE_WIDTH },
    { Sid::glissandoText,      Pid::GLISS_TEXT },
};

//=========================================================
//   GlissandoSegment
//=========================================================

GlissandoSegment::GlissandoSegment(Glissando* sp, System* parent)
    : LineSegment(ElementType::GLISSANDO_SEGMENT, sp, parent, ElementFlag::MOVABLE)
{
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void GlissandoSegment::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void GlissandoSegment::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;

    if (pos2().x() <= 0) {
        return;
    }

    painter->save();
    double _spatium = spatium();

    Pen pen(curColor(visible(), glissando()->lineColor()));
    pen.setWidthF(glissando()->lineWidth());
    pen.setCapStyle(PenCapStyle::RoundCap);
    painter->setPen(pen);

    // rotate painter so that the line become horizontal
    double w     = pos2().x();
    double h     = pos2().y();
    double l     = sqrt(w * w + h * h);
    double wi    = asin(-h / l) * 180.0 / M_PI;
    painter->rotate(-wi);

    if (glissando()->glissandoType() == GlissandoType::STRAIGHT) {
        painter->drawLine(LineF(0.0, 0.0, l, 0.0));
    } else if (glissando()->glissandoType() == GlissandoType::WAVY) {
        RectF b = symBbox(SymId::wiggleTrill);
        double a  = symAdvance(SymId::wiggleTrill);
        int n    = static_cast<int>(l / a);          // always round down (truncate) to avoid overlap
        double x  = (l - n * a) * 0.5;     // centre line in available space
        SymIdList ids;
        for (int i = 0; i < n; ++i) {
            ids.push_back(SymId::wiggleTrill);
        }

        score()->engravingFont()->draw(ids, painter, magS(), PointF(x, -(b.y() + b.height() * 0.5)));
    }

    if (glissando()->showText()) {
        mu::draw::Font f(glissando()->fontFace(), draw::Font::Type::Unknown);
        f.setPointSizeF(glissando()->fontSize() * _spatium / SPATIUM20);
        f.setBold(glissando()->fontStyle() & FontStyle::Bold);
        f.setItalic(glissando()->fontStyle() & FontStyle::Italic);
        f.setUnderline(glissando()->fontStyle() & FontStyle::Underline);
        f.setStrike(glissando()->fontStyle() & FontStyle::Strike);
        mu::draw::FontMetrics fm(f);
        RectF r = fm.boundingRect(glissando()->text());

        // if text longer than available space, skip it
        if (r.width() < l) {
            double yOffset = r.height() + r.y();             // find text descender height
            // raise text slightly above line and slightly more with WAVY than with STRAIGHT
            yOffset += _spatium * (glissando()->glissandoType() == GlissandoType::WAVY ? 0.4 : 0.1);

            mu::draw::Font scaledFont(f);
            scaledFont.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
            painter->setFont(scaledFont);

            double x = (l - r.width()) * 0.5;
            painter->drawText(PointF(x, -yOffset), glissando()->text());
        }
    }
    painter->restore();
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
    case Pid::GLISS_EASEIN:
    case Pid::GLISS_EASEOUT:
    case Pid::PLAY:
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
    setAnchor(Spanner::Anchor::NOTE);
    setDiagonal(true);

    initElementStyle(&glissandoElementStyle);

    resetProperty(Pid::GLISS_SHOW_TEXT);
    resetProperty(Pid::PLAY);
    resetProperty(Pid::GLISS_STYLE);
    resetProperty(Pid::GLISS_TYPE);
    resetProperty(Pid::GLISS_TEXT);
    resetProperty(Pid::GLISS_EASEIN);
    resetProperty(Pid::GLISS_EASEOUT);
}

Glissando::Glissando(const Glissando& g)
    : SLine(g)
{
    _text           = g._text;
    _fontFace       = g._fontFace;
    _fontSize       = g._fontSize;
    _glissandoType  = g._glissandoType;
    _glissandoStyle = g._glissandoStyle;
    _easeIn         = g._easeIn;
    _easeOut        = g._easeOut;
    _showText       = g._showText;
    _playGlissando  = g._playGlissando;
    _fontStyle      = g._fontStyle;
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

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Glissando::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

void Glissando::addLineAttachPoints()
{
    auto seg = toGlissandoSegment(frontSegment());
    Note* startNote = nullptr;
    Note* endNote = nullptr;
    if (startElement() && startElement()->isNote()) {
        startNote = toNote(startElement());
    }
    if (endElement() && endElement()->isNote()) {
        endNote = toNote(endElement());
    }
    if (!seg || !startNote || !endNote || (startNote->findMeasure() != endNote->findMeasure())) {
        return;
    }
    double startX = seg->ipos().x();
    double endX = seg->pos2().x() + seg->ipos().x(); // because pos2 is relative to ipos
    // Here we don't pass y() because its value is unreliable during the first stages of layout.
    // The y() is irrelevant anyway for horizontal spacing.
    startNote->addLineAttachPoint(PointF(startX, 0.0), this);
    endNote->addLineAttachPoint(PointF(endX, 0.0), this);
}

bool Glissando::pitchSteps(const Spanner* spanner, std::vector<int>& pitchOffsets)
{
    if (!spanner->endElement()->isNote()) {
        return false;
    }
    const Glissando* glissando = toGlissando(spanner);
    if (!glissando->playGlissando()) {
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
    static std::vector<bool> whiteNotes = { true, false, true, false, true, true, false, true, false, true, false, true };
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
//   STATIC FUNCTIONS: guessFinalNote
//
//    Used while dropping a glissando on a note to determine (guess!) the glissando final
//    note from its initial chord.
//    Returns the top note of next chord of the same instrument,
//    preferring the chord in the same track as chord, if it exists.
//
//    Parameter:  chord: the chord this glissando start from
//    Returns:    the top note in a suitable following chord or nullptr if none found
//---------------------------------------------------------

Note* Glissando::guessFinalNote(Chord* chord, Note* startNote)
{
    switch (chord->noteType()) {
//            case NoteType::INVALID:
//                  return nullptr;
    // for grace notes before, return top note of parent chord
    // TODO : if the grace-before is not the LAST ONE, this still returns the main note
    //    which is probably not correct; however a glissando between two grace notes
    //    probably makes little sense.
    case NoteType::ACCIACCATURA:
    case NoteType::APPOGGIATURA:
    case NoteType::GRACE4:
    case NoteType::GRACE16:
    case NoteType::GRACE32:
        if (chord->explicitParent() && chord->explicitParent()->isChord()) {
            return toChord(chord->explicitParent())->upNote();
        } else {                                // no parent or parent is not a chord?
            return nullptr;
        }
    // for grace notes after, next chord is next chord of parent chord
    // TODO : same note as case above!
    case NoteType::GRACE8_AFTER:
    case NoteType::GRACE16_AFTER:
    case NoteType::GRACE32_AFTER:
        // move unto parent chord and proceed to standard case
        if (chord->explicitParent() && chord->explicitParent()->isChord()) {
            chord = toChord(chord->explicitParent());
        } else {
            return 0;
        }
        break;
    case NoteType::NORMAL:
    {
        // if chord has grace notes after, the first one is the next note
        std::vector<Chord*> graces = chord->graceNotesAfter();
        if (graces.size() > 0) {
            return graces.front()->upNote();
        }
    }
    break;
    default:
        break;
    }

    // standard case (NORMAL or grace after chord)

    // if parent not a segment, can't locate a target note
    if (!chord->explicitParent()->isSegment()) {
        return 0;
    }

    // look for first ChordRest segment after initial note is elapsed
    Segment* segm = chord->score()->tick2rightSegment(chord->tick() + chord->actualTicks());
    track_idx_t chordTrack = chord->track();
    Part* part = chord->part();
    while (segm) {
        // if next segment is a ChordRest segment
        if (segm->segmentType() == SegmentType::ChordRest) {
            Chord* target = nullptr;

            // look for a Chord in the same track
            if (segm->element(chordTrack) && segm->element(chordTrack)->isChord()) {
                target = toChord(segm->element(chordTrack));
            } else {                  // if no same track, look for other chords in the same instrument
                for (EngravingItem* currChord : segm->elist()) {
                    if (currChord && currChord->isChord() && toChord(currChord)->part() == part) {
                        target = toChord(currChord);
                        break;
                    }
                }
            }

            // if we found a target next chord
            if (target) {
                // if chord has grace notes before, the first one is the next note
                std::vector<Chord*> graces = target->graceNotesBefore();
                if (graces.size() > 0) {
                    return graces.front()->upNote();
                }
                // normal case: try to return the note in the next chord that is in the
                // same position as the start note relative to the end chord
                auto startNoteIter = find(chord->notes().begin(), chord->notes().end(), startNote);
                int startNoteIdx = std::distance(chord->notes().begin(), startNoteIter);
                int endNoteIdx = std::min(startNoteIdx, int(target->notes().size()) - 1);
                return target->notes().at(endNoteIdx);
            }
        }
        segm = segm->next1();
    }
    LOGD("no second note for glissando found");
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
    case Pid::GLISS_EASEIN:
        return easeIn();
    case Pid::GLISS_EASEOUT:
        return easeOut();
    case Pid::PLAY:
        return bool(playGlissando());
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
        setGlissandoStyle(v.value<GlissandoStyle>());
        break;
    case Pid::GLISS_EASEIN:
        setEaseIn(v.toInt());
        break;
    case Pid::GLISS_EASEOUT:
        setEaseOut(v.toInt());
        break;
    case Pid::PLAY:
        setPlayGlissando(v.toBool());
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
    triggerLayoutAll();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Glissando::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::GLISS_TYPE:
        return int(GlissandoType::STRAIGHT);
    case Pid::GLISS_SHOW_TEXT:
        return true;
    case Pid::GLISS_STYLE:
        return GlissandoStyle::CHROMATIC;
    case Pid::GLISS_EASEIN:
    case Pid::GLISS_EASEOUT:
        return 0;
    case Pid::PLAY:
        return true;
    default:
        break;
    }
    return SLine::propertyDefault(propertyId);
}
}
