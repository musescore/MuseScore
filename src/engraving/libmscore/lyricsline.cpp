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

#include "layout/tlayout.h"

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
    setLineWidth(score()->styleMM(Sid::lyricsDashLineThickness));
    setAnchor(Spanner::Anchor::SEGMENT);
    _nextLyrics = 0;
}

LyricsLine::LyricsLine(const LyricsLine& g)
    : SLine(g)
{
    _nextLyrics = 0;
}

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void LyricsLine::styleChanged()
{
    setLineWidth(score()->styleMM(Sid::lyricsDashLineThickness));
}

//---------------------------------------------------------
<<<<<<< HEAD
<<<<<<< HEAD
=======
//   layout
//---------------------------------------------------------

void LyricsLine::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

//---------------------------------------------------------
>>>>>>> 4f8a1b6dd0... [engraving] replaced item->layout() to TLayout::layout
=======
>>>>>>> 11610ff2b5... [engraving] removed item->layout method
//   layoutSystem
//---------------------------------------------------------

SpannerSegment* LyricsLine::layoutSystem(System* system)
{
    LayoutContext ctx(score());

    Fraction stick = system->firstMeasure()->tick();
    Fraction etick = system->lastMeasure()->endTick();

    LyricsLineSegment* lineSegm = toLyricsLineSegment(getNextLayoutSystemSegment(system, [this](System* parent) {
        return createLineSegment(parent);
    }));

    SpannerSegmentType sst;
    if (tick() >= stick) {
        v0::TLayout::layout(this, ctx);
        if (ticks().isZero() && isEndMelisma()) { // only do layout if some time span
            // dash lines still need to be laid out, though
            return nullptr;
        }

        v0::TLayout::layoutLine(this, ctx);
        //
        // this is the first call to layoutSystem,
        // processing the first line segment
        //
        computeStartElement();
        computeEndElement();
        sst = tick2() <= etick ? SpannerSegmentType::SINGLE : SpannerSegmentType::BEGIN;
    } else if (tick() < stick && tick2() > etick) {
        sst = SpannerSegmentType::MIDDLE;
    } else {
        //
        // this is the last call to layoutSystem
        // processing the last line segment
        //
        sst = SpannerSegmentType::END;
    }
    lineSegm->setSpannerSegmentType(sst);

    switch (sst) {
    case SpannerSegmentType::SINGLE: {
        System* s;
        PointF p1 = linePos(Grip::START, &s);
        PointF p2 = linePos(Grip::END,   &s);
        double len = p2.x() - p1.x();
        lineSegm->setPos(p1);
        lineSegm->setPos2(PointF(len, p2.y() - p1.y()));
    }
    break;
    case SpannerSegmentType::BEGIN: {
        System* s;
        PointF p1 = linePos(Grip::START, &s);
        lineSegm->setPos(p1);
        double x2 = system->lastNoteRestSegmentX(true);
        lineSegm->setPos2(PointF(x2 - p1.x(), 0.0));
    }
    break;
    case SpannerSegmentType::MIDDLE: {
        bool leading = (anchor() == Anchor::SEGMENT || anchor() == Anchor::MEASURE);
        double x1 = system->firstNoteRestSegmentX(leading);
        double x2 = system->lastNoteRestSegmentX(true);
        System* s;
        PointF p1 = linePos(Grip::START, &s);
        lineSegm->setPos(PointF(x1, p1.y()));
        lineSegm->setPos2(PointF(x2 - x1, 0.0));
    }
    break;
    case SpannerSegmentType::END: {
        System* s;
        PointF p2 = linePos(Grip::END, &s);
        bool leading = (anchor() == Anchor::SEGMENT || anchor() == Anchor::MEASURE);
        double x1 = system->firstNoteRestSegmentX(leading);
        double len = p2.x() - x1;
        lineSegm->setPos(PointF(p2.x() - len, p2.y()));
        lineSegm->setPos2(PointF(len, 0.0));
    }
    break;
    }

    v0::TLayout::layout(lineSegm, ctx);

    // if temp melisma extend the first line segment to be
    // after the lyrics syllable (otherwise the melisma segment
    // will be too short).
    const bool tempMelismaTicks = (lyrics()->ticks() == Fraction::fromTicks(Lyrics::TEMP_MELISMA_TICKS));
    if (tempMelismaTicks && spannerSegments().size() > 0 && spannerSegments().front() == lineSegm) {
        lineSegm->rxpos2() += lyrics()->width();
    }
    // avoid backwards melisma
    if (lineSegm->pos2().x() < 0) {
        lineSegm->rxpos2() = 0;
    }
    return lineSegm;
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
    triggerLayoutAll();
    return true;
}

//=========================================================
//   LyricsLineSegment
//=========================================================

LyricsLineSegment::LyricsLineSegment(LyricsLine* sp, System* parent)
    : LineSegment(ElementType::LYRICSLINE_SEGMENT, sp, parent, ElementFlag::ON_STAFF | ElementFlag::NOT_SELECTABLE)
{
    setGenerated(true);
}

//---------------------------------------------------------
<<<<<<< HEAD
<<<<<<< HEAD
=======
//   layout
//---------------------------------------------------------

void LyricsLineSegment::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

//---------------------------------------------------------
>>>>>>> 4f8a1b6dd0... [engraving] replaced item->layout() to TLayout::layout
=======
>>>>>>> 11610ff2b5... [engraving] removed item->layout method
//   draw
//---------------------------------------------------------

void LyricsLineSegment::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;
    if (_numOfDashes < 1) {               // nothing to draw
        return;
    }

    Pen pen(lyricsLine()->lyrics()->curColor());
    pen.setWidthF(lyricsLine()->lineWidth());
    pen.setCapStyle(PenCapStyle::FlatCap);
    painter->setPen(pen);
    if (lyricsLine()->isEndMelisma()) {               // melisma
        painter->drawLine(PointF(), pos2());
    } else {                                          // dash(es)
        double step  = pos2().x() / _numOfDashes;
        double x     = step * .5 - _dashLength * .5;
        for (int i = 0; i < _numOfDashes; i++, x += step) {
            painter->drawLine(PointF(x, 0.0), PointF(x + _dashLength, 0.0));
        }
    }
}
}
