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
