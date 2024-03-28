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

#include "lyrics.h"

#include "draw/types/pen.h"

#include "chord.h"
#include "chordrest.h"
#include "measure.h"
#include "score.h"
#include "segment.h"
#include "stafftype.h"
#include "system.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   LyricsLine
//---------------------------------------------------------

LyricsLine::LyricsLine(EngravingItem* parent)
    : SLine(ElementType::LYRICSLINE, parent, ElementFlag::NOT_SELECTABLE)
{
    setGenerated(true);             // no need to save it, as it can be re-generated
    setDiagonal(false);
    setLineWidth(style().styleMM(Sid::lyricsDashLineThickness));
    setAnchor(Spanner::Anchor::SEGMENT);
    m_nextLyrics = 0;
}

LyricsLine::LyricsLine(const LyricsLine& g)
    : SLine(g)
{
    m_nextLyrics = 0;
}

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void LyricsLine::styleChanged()
{
    setLineWidth(style().styleMM(Sid::lyricsDashLineThickness));
}

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* LyricsLine::createLineSegment(System* parent)
{
    LyricsLineSegment* seg = new LyricsLineSegment(this, parent);
    seg->setTrack(track());
    seg->setColor(color());
    return seg;
}

//---------------------------------------------------------
//   removeUnmanaged
//    same as Spanner::removeUnmanaged(), but in addition, remove from hosting Lyrics
//---------------------------------------------------------

void LyricsLine::removeUnmanaged()
{
    Spanner::removeUnmanaged();
    if (lyrics()) {
        lyrics()->remove(this);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool LyricsLine::setProperty(Pid propertyId, const engraving::PropertyValue& v)
{
    switch (propertyId) {
    case Pid::SPANNER_TICKS:
    {
        // if parent lyrics has a melisma, change its length too
        if (explicitParent() && explicitParent()->type() == ElementType::LYRICS
            && isEndMelisma()) {
            Fraction newTicks   = toLyrics(explicitParent())->ticks() + v.value<Fraction>() - ticks();
            explicitParent()->undoChangeProperty(Pid::LYRIC_TICKS, newTicks);
        }
        setTicks(v.value<Fraction>());
    }
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

PointF LyricsLine::linePos(Grip grip, System** system) const
{
    if (grip == Grip::START) {
        return PointF(); // Start is computed elsewhere
    }

    EngravingItem* endEl = endElement();
    ChordRest* endCr = endEl && endEl->isChordRest() ? toChordRest(endEl) : nullptr;
    if (!endCr) {
        return PointF();
    }

    if (endCr->track() != track()) {
        EngravingItem* cr = endCr->segment()->elementAt(track());
        if (cr) {
            endCr = toChordRest(cr);
        }
    }

    Segment* endSeg = endCr->segment();
    *system = endSeg->measure()->system();
    double x = endSeg->x() + endSeg->measure()->x();
    if (endCr) {
        x += endCr->isChord() ? toChord(endCr)->rightEdge() : endCr->width();
    }

    return PointF(x, 0.0);
}

void LyricsLine::doComputeEndElement()
{
    if (!isEndMelisma()) {
        Spanner::doComputeEndElement();
        return;
    }

    // TODO: review this hack
    // lyrics endTick should already indicate the segment we want
    // except for TEMP_MELISMA_TICKS case
    Lyrics* l = lyrics();
    Fraction tick = (l->ticks() == Lyrics::TEMP_MELISMA_TICKS) ? l->tick() : l->endTick();
    Segment* s = score()->tick2segment(tick, true, SegmentType::ChordRest);
    if (!s) {
        LOGD("%s no end segment for tick %d", typeName(), tick.ticks());
        return;
    }
    voice_idx_t t = trackZeroVoice(track2());
    // take the first chordrest we can find;
    // linePos will substitute one in current voice if available
    for (voice_idx_t v = 0; v < VOICES; ++v) {
        setEndElement(s->element(t + v));
        if (endElement()) {
            break;
        }
    }
    return;
}

//=========================================================
//   LyricsLineSegment
//=========================================================

LyricsLineSegment::LyricsLineSegment(LyricsLine* sp, System* parent)
    : LineSegment(ElementType::LYRICSLINE_SEGMENT, sp, parent, ElementFlag::ON_STAFF | ElementFlag::NOT_SELECTABLE)
{
    setGenerated(true);
}
}
