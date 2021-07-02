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

#include "chord.h"
#include "score.h"
#include "system.h"
#include "xml.h"
#include "staff.h"
#include "segment.h"
#include "undo.h"
#include "textedit.h"
#include "measure.h"
#include "draw/pen.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   searchNextLyrics
//---------------------------------------------------------

static Lyrics* searchNextLyrics(Segment* s, int staffIdx, int verse, Placement p)
{
    Lyrics* l = 0;
    while ((s = s->next1(SegmentType::ChordRest))) {
        int strack = staffIdx * VOICES;
        int etrack = strack + VOICES;
        // search through all tracks of current staff looking for a lyric in specified verse
        for (int track = strack; track < etrack; ++track) {
            ChordRest* cr = toChordRest(s->element(track));
            if (cr) {
                // cr with lyrics found, but does it have a syllable in specified verse?
                l = cr->lyrics(verse, p);
                if (l) {
                    break;
                }
            }
        }
        if (l) {
            break;
        }
    }
    return l;
}

//---------------------------------------------------------
//   LyricsLine
//---------------------------------------------------------

LyricsLine::LyricsLine(Score* s)
    : SLine(s, ElementFlag::NOT_SELECTABLE)
{
    setGenerated(true);             // no need to save it, as it can be re-generated
    setDiagonal(false);
    setLineWidth(score()->styleP(Sid::lyricsDashLineThickness));
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
    setLineWidth(score()->styleP(Sid::lyricsDashLineThickness));
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void LyricsLine::layout()
{
    bool tempMelismaTicks = (lyrics()->ticks() == Fraction::fromTicks(Lyrics::TEMP_MELISMA_TICKS));
    if (isEndMelisma()) {           // melisma
        setLineWidth(score()->styleP(Sid::lyricsLineThickness));
        // if lyrics has a temporary one-chord melisma, set to 0 ticks (just its own chord)
        if (tempMelismaTicks) {
            lyrics()->setTicks(Fraction(0, 1));
        }

        // Lyrics::_ticks points to the beginning of the last spanned segment,
        // but the line shall include it:
        // include the duration of this last segment in the melisma duration
        Segment* lyricsSegment   = lyrics()->segment();
        Fraction lyricsStartTick = lyricsSegment->tick();
        Fraction lyricsEndTick   = lyrics()->endTick();
        int lyricsTrack          = lyrics()->track();

        // find segment with tick >= endTick
        Segment* s = lyricsSegment;
        while (s && s->tick() < lyricsEndTick) {
            s = s->nextCR(lyricsTrack, true);
        }
        if (!s) {
            // user probably deleted measures at end of score, leaving this melisma too long
            // set s to last segment and reset lyricsEndTick to trigger FIXUP code below
            s = score()->lastSegment();
            lyricsEndTick = Fraction(-1, 1);
        }
        Element* se = s->element(lyricsTrack);
        // everything is OK if we have reached a chord at right tick on right track
        if (s->tick() == lyricsEndTick && se && se->type() == ElementType::CHORD) {
            // advance to next CR, or last segment if no next CR
            s = s->nextCR(lyricsTrack, true);
            if (!s) {
                s = score()->lastSegment();
            }
        } else {
            // FIXUP - lyrics tick count not valid
            // this happens if edits to score have removed the original end segment
            // so let's fix it here
            // s is already pointing to segment past endTick (or to last segment)
            // we should shorten the lyrics tick count to make this work
            Segment* ns = s;
            Segment* ps = s->prev1(SegmentType::ChordRest);
            while (ps && ps != lyricsSegment) {
                Element* pe = ps->element(lyricsTrack);
                // we're looking for an actual chord on this track
                if (pe && pe->type() == ElementType::CHORD) {
                    break;
                }
                s = ps;
                ps = ps->prev1(SegmentType::ChordRest);
            }
            if (!ps || ps == lyricsSegment) {
                // no valid previous CR, so try to lengthen melisma instead
                ps = ns;
                s = ps->nextCR(lyricsTrack, true);
                Element* e = s ? s->element(lyricsTrack) : nullptr;
                // check to make sure we have a chord
                if (!e || e->type() != ElementType::CHORD) {
                    // nothing to do but set ticks to 0
                    // this will result in melisma being deleted later
                    lyrics()->undoChangeProperty(Pid::LYRIC_TICKS, 0);
                    setTicks(Fraction(0, 1));
                    return;
                }
            }
            lyrics()->undoChangeProperty(Pid::LYRIC_TICKS, ps->tick() - lyricsStartTick);
        }
        // Spanner::computeEndElement() will actually ignore this value and use the (earlier) lyrics()->endTick() instead
        // still, for consistency with other lines, we should set the ticks for this to the computed (later) value
        if (s) {
            setTicks(s->tick() - lyricsStartTick);
        }
    } else {                                    // dash(es)
        _nextLyrics = searchNextLyrics(lyrics()->segment(), staffIdx(), lyrics()->no(), lyrics()->placement());
        setTick2(_nextLyrics ? _nextLyrics->segment()->tick() : tick());
    }
    if (ticks().isNotZero()) {                  // only do layout if some time span
        // do layout with non-0 duration
        if (tempMelismaTicks) {
            lyrics()->setTicks(Fraction::fromTicks(Lyrics::TEMP_MELISMA_TICKS));
        }
    }
}

//---------------------------------------------------------
//   layoutSystem
//---------------------------------------------------------

SpannerSegment* LyricsLine::layoutSystem(System* system)
{
    Fraction stick = system->firstMeasure()->tick();
    Fraction etick = system->lastMeasure()->endTick();

    LyricsLineSegment* lineSegm = toLyricsLineSegment(getNextLayoutSystemSegment(system, [this]() {
        return createLineSegment();
    }));

    SpannerSegmentType sst;
    if (tick() >= stick) {
        layout();
        if (ticks().isZero()) {                   // only do layout if some time span
            return nullptr;
        }
        SLine::layout();
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
        qreal len = p2.x() - p1.x();
        lineSegm->setPos(p1);
        lineSegm->setPos2(PointF(len, p2.y() - p1.y()));
    }
    break;
    case SpannerSegmentType::BEGIN: {
        System* s;
        PointF p1 = linePos(Grip::START, &s);
        lineSegm->setPos(p1);
        qreal x2 = system->bbox().right();
        lineSegm->setPos2(PointF(x2 - p1.x(), 0.0));
    }
    break;
    case SpannerSegmentType::MIDDLE: {
        bool leading = (anchor() == Anchor::SEGMENT || anchor() == Anchor::MEASURE);
        qreal x1 = system->firstNoteRestSegmentX(leading);
        qreal x2 = system->bbox().right();
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
        qreal x1 = system->firstNoteRestSegmentX(leading);
        qreal len = p2.x() - x1;
        lineSegm->setPos(PointF(p2.x() - len, p2.y()));
        lineSegm->setPos2(PointF(len, 0.0));
    }
    break;
    }
    lineSegm->layout();
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

LineSegment* LyricsLine::createLineSegment()
{
    LyricsLineSegment* seg = new LyricsLineSegment(this, score());
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

bool LyricsLine::setProperty(Pid propertyId, const QVariant& v)
{
    switch (propertyId) {
    case Pid::SPANNER_TICKS:
    {
        // if parent lyrics has a melisma, change its length too
        if (parent() && parent()->type() == ElementType::LYRICS
            && isEndMelisma()) {
            Fraction newTicks   = toLyrics(parent())->ticks() + v.value<Fraction>() - ticks();
            parent()->undoChangeProperty(Pid::LYRIC_TICKS, newTicks);
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

LyricsLineSegment::LyricsLineSegment(Spanner* sp, Score* s)
    : LineSegment(sp, s, ElementFlag::ON_STAFF | ElementFlag::NOT_SELECTABLE)
{
    setGenerated(true);
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void LyricsLineSegment::layout()
{
    ryoffset() = 0.0;

    bool endOfSystem       = false;
    bool isEndMelisma      = lyricsLine()->isEndMelisma();
    Lyrics* lyr               = 0;
    Lyrics* nextLyr           = 0;
    qreal fromX             = 0;
    qreal toX               = 0;                     // start and end point of intra-lyrics room
    qreal sp                = spatium();
    System* sys;

    if (lyricsLine()->ticks() <= Fraction(0, 1)) {     // if no span,
        _numOfDashes = 0;                 // nothing to draw
        return;                           // and do nothing
    }

    // HORIZONTAL POSITION
    // A) if line precedes a syllable, advance line end to right before the next syllable text
    // if not a melisma and there is a next syllable;
    if (!isEndMelisma && lyricsLine()->nextLyrics() && isSingleEndType()) {
        lyr         = nextLyr = lyricsLine()->nextLyrics();
        sys         = lyr->segment()->system();
        endOfSystem = (sys != system());
        // if next lyrics is on a different system, this line segment is at the end of its system:
        // do not adjust for next lyrics position
        if (sys && !endOfSystem) {
            qreal lyrX        = lyr->bbox().x();
            qreal lyrXp       = lyr->pagePos().x();
            qreal sysXp       = sys->pagePos().x();
            toX               = lyrXp - sysXp + lyrX;             // syst.rel. X pos.
            qreal offsetX     = toX - pos().x() - pos2().x() - score()->styleP(Sid::lyricsDashPad);
            //                    delta from current end pos.| ending padding
            rxpos2()          += offsetX;
        }
    }
    // B) if line follows a syllable, advance line start to after the syllable text
    lyr  = lyricsLine()->lyrics();
    sys  = lyr->segment()->system();
    if (sys && isSingleBeginType()) {
        qreal lyrX        = lyr->bbox().x();
        qreal lyrXp       = lyr->pagePos().x();
        qreal lyrW        = lyr->bbox().width();
        qreal sysXp       = sys->pagePos().x();
        fromX             = lyrXp - sysXp + lyrX + lyrW;
        //               syst.rel. X pos. | lyr.advance
        qreal offsetX     = fromX - pos().x();
        offsetX           += score()->styleP(isEndMelisma ? Sid::lyricsMelismaPad : Sid::lyricsDashPad);

        //               delta from curr.pos. | add initial padding
        rxpos()           += offsetX;
        rxpos2()          -= offsetX;
    }

    // VERTICAL POSITION: at the base line of the syllable text
    if (!isEndType()) {
        rypos() = lyr->ipos().y();
        ryoffset() = lyr->offset().y();
    } else {
        // use Y position of *next* syllable if there is one on same system
        Lyrics* nextLyr1 = searchNextLyrics(lyr->segment(), lyr->staffIdx(), lyr->no(), lyr->placement());
        if (nextLyr1 && nextLyr1->segment()->system() == system()) {
            rypos() = nextLyr1->ipos().y();
            ryoffset() = nextLyr1->offset().y();
        } else {
            rypos() = lyr->ipos().y();
            ryoffset() = lyr->offset().y();
        }
    }

    // MELISMA vs. DASHES
    if (isEndMelisma) {                   // melisma
        _numOfDashes = 1;
        rypos()      -= lyricsLine()->lineWidth() * .5;     // let the line 'sit on' the base line
        // if not final segment, shorten it
        if (isBeginType() || isMiddleType()) {
            rxpos2() -= score()->styleP(Sid::minNoteDistance) * mag();
        }
    } else {                              // dash(es)
        // set conventional dash Y pos
        rypos() -= MScore::pixelRatio * lyr->fontMetrics().xHeight() * score()->styleD(Sid::lyricsDashYposRatio);
        _dashLength = score()->styleP(Sid::lyricsDashMaxLength) * mag();      // and dash length
        qreal len         = pos2().x();
        qreal minDashLen  = score()->styleS(Sid::lyricsDashMinLength).val() * sp;
        qreal maxDashDist = score()->styleS(Sid::lyricsDashMaxDistance).val() * sp;
        if (len < minDashLen) {                                               // if no room for a dash
            // if at end of system or dash is forced
            if (endOfSystem || score()->styleB(Sid::lyricsDashForce)) {
                rxpos2()          = minDashLen;                               //     draw minimal dash
                _numOfDashes      = 1;
                _dashLength       = minDashLen;
            } else {                                                          //   if within system or dash not forced
                _numOfDashes = 0;                                             //     draw no dash
            }
        } else if (len < (maxDashDist * 1.5)) {                               // if no room for two dashes
            _numOfDashes = 1;                                                 //    draw one dash
            if (_dashLength > len) {                                          // if no room for a full dash
                _dashLength = len;                                            //    shorten it
            }
        } else {
            _numOfDashes = len / maxDashDist + 1;                             // draw several dashes
        }
        // adjust next lyrics horiz. position if too little a space forced to skip the dash
        if (_numOfDashes == 0 && nextLyr != nullptr && len > 0) {
            nextLyr->rxpos() -= (toX - fromX);
        }
    }

    // apply yoffset for staff type change (keeps lyrics lines aligned with lyrics)
    if (staffType()) {
        rypos() += staffType()->yoffset().val() * spatium();
    }

    // set bounding box
    RectF r = RectF(0.0, 0.0, pos2().x(), pos2().y()).normalized();
    qreal lw = lyricsLine()->lineWidth() * .5;
    setbbox(r.adjusted(-lw, -lw, lw, lw));
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void LyricsLineSegment::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
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
        qreal step  = pos2().x() / _numOfDashes;
        qreal x     = step * .5 - _dashLength * .5;
        for (int i = 0; i < _numOfDashes; i++, x += step) {
            painter->drawLine(PointF(x, 0.0), PointF(x + _dashLength, 0.0));
        }
    }
}
}
