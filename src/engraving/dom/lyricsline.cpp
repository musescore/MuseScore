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

#include "lyrics.h"

#include "chord.h"
#include "chordrest.h"
#include "factory.h"
#include "measure.h"
#include "navigate.h"
#include "note.h"
#include "score.h"
#include "segment.h"
#include "system.h"
#include "utils.h"

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
    setLineWidth(style().styleS(Sid::lyricsDashLineThickness));
    setAnchor(Spanner::Anchor::SEGMENT);
    m_nextLyrics = 0;
}

LyricsLine::LyricsLine(const ElementType& type, EngravingItem* parent, ElementFlags f)
    : SLine(type, parent, f)
{
    setGenerated(true);             // no need to save it, as it can be re-generated
    setDiagonal(false);
    setLineWidth(style().styleS(Sid::lyricsDashLineThickness));
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
    setLineWidth(style().styleS(Sid::lyricsDashLineThickness));
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

void LyricsLine::doComputeEndElement()
{
    if (!isEndMelisma()) {
        Spanner::doComputeEndElement();
        return;
    }

    setEndElement(score()->findChordRestEndingBeforeTickInTrack(tick2(), track()));

    if (!endElement()) {
        setEndElement(score()->findChordRestEndingBeforeTickInStaff(tick2(), track2staff(track())));
    }
}

//=========================================================
//   LyricsLineSegment
//=========================================================

LyricsLineSegment::LyricsLineSegment(LyricsLine* sp, System* parent)
    : LineSegment(ElementType::LYRICSLINE_SEGMENT, sp, parent, ElementFlag::ON_STAFF | ElementFlag::NOT_SELECTABLE)
{
    setGenerated(true);
}

LyricsLineSegment::LyricsLineSegment(const ElementType& type, LyricsLine* sp, System* parent, ElementFlags f)
    : LineSegment(type, sp, parent, f)
{
    setGenerated(true);
}

double LyricsLineSegment::baseLineShift() const
{
    if (lyricsLine()->isEndMelisma()) {
        return -0.5 * absoluteFromSpatium(lineWidth());
    }

    Lyrics* segLyrics = lyrics();
    return -style().styleD(Sid::lyricsDashYposRatio) * segLyrics->fontMetrics().xHeight();
}

//=========================================================
//   PartialLyricsLine
//=========================================================
PartialLyricsLine::PartialLyricsLine(EngravingItem* parent)
    : LyricsLine(ElementType::PARTIAL_LYRICSLINE, parent)
{
    setGenerated(false);
}

PartialLyricsLine::PartialLyricsLine(const PartialLyricsLine& other)
    : LyricsLine(other)
{
    m_isEndMelisma = other.m_isEndMelisma;
}

LineSegment* PartialLyricsLine::createLineSegment(System* parent)
{
    PartialLyricsLineSegment* seg = new PartialLyricsLineSegment(this, parent);
    seg->setTrack(track());
    seg->setColor(color());
    return seg;
}

PropertyValue PartialLyricsLine::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::VERSE:
        return _no;
    default:
        return LyricsLine::getProperty(propertyId);
    }
}

bool PartialLyricsLine::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::VERSE:
        setNo(val.toInt());
        break;
    default:
        return LyricsLine::setProperty(propertyId, val);
    }

    triggerLayout();

    return true;
}

PropertyValue PartialLyricsLine::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::VERSE:
        return 0;
    default:
        return LyricsLine::propertyDefault(propertyId);
    }
}

Sid PartialLyricsLine::getPropertyStyle(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::PLACEMENT:
        return Sid::lyricsPlacement;
    default:
        return LyricsLine::getPropertyStyle(propertyId);
    }
}

void PartialLyricsLine::doComputeEndElement()
{
    LyricsLine::doComputeEndElement();

    if (startElement() && endElement() && startElement()->tick() > endElement()->tick()) {
        setEndElement(startElement());
    }
}

//=========================================================
//   PartialLyricsLineSegment
//=========================================================

static const ElementStyle partialLyricsLineSegmentElementStyle {
    { Sid::lyricsPlacement, Pid::PLACEMENT },
    { Sid::lyricsPosBelow, Pid::OFFSET },
    { Sid::lyricsMinTopDistance, Pid::MIN_DISTANCE },
};

PartialLyricsLineSegment::PartialLyricsLineSegment(PartialLyricsLine* line, System* parent)
    : LyricsLineSegment(ElementType::PARTIAL_LYRICSLINE_SEGMENT, line, parent, ElementFlag::ON_STAFF)
{
    setGenerated(false);
    initElementStyle(&partialLyricsLineSegmentElementStyle);
}

double PartialLyricsLineSegment::lineSpacing() const
{
    const Lyrics* lyrics = lyricsLine()->findAdjacentLyricsOrDefault();

    if (!lyrics) {
        return 0.0;
    }

    return lyrics->lineSpacing();
}

double PartialLyricsLineSegment::baseLineShift() const
{
    if (lyricsLine()->isEndMelisma()) {
        return -0.5 * absoluteFromSpatium(lineWidth());
    }

    const Lyrics* lyrics = lyricsLine()->findAdjacentLyricsOrDefault();
    if (!lyrics) {
        return 0.0;
    }

    return -style().styleD(Sid::lyricsDashYposRatio) * lyrics->fontMetrics().xHeight();
}

EngravingItem* PartialLyricsLineSegment::propertyDelegate(Pid pid)
{
    switch (pid) {
    case Pid::VERSE:
        return lyricsLine();
    default:
        return LyricsLineSegment::propertyDelegate(pid);
    }
}

Lyrics* PartialLyricsLine::findLyricsInPreviousRepeatSeg() const
{
    const std::vector<Measure*> measures = findPreviousRepeatMeasures(findStartMeasure());

    for (const Measure* measure : measures) {
        Lyrics* prev = lastLyricsInMeasure(measure->last(SegmentType::ChordRest), staffIdx(), no(), placement());

        if (!prev) {
            continue;
        }

        return prev;
    }

    return nullptr;
}

Lyrics* PartialLyricsLine::findAdjacentLyricsOrDefault() const
{
    Lyrics* prev = findLyricsInPreviousRepeatSeg();
    if (prev) {
        return prev;
    }

    Lyrics* next = nextLyrics();
    if (next) {
        return next;
    }

    // If there are no adjacent lyrics, create dummy lyrics using the odd lyrics text style to get font information
    Lyrics* dummyLyr = Factory::createLyrics(toChordRest(score()->dummy()->chord()));
    dummyLyr->setTextStyleType(TextStyleType::LYRICS_ODD);
    return dummyLyr;
}
}
