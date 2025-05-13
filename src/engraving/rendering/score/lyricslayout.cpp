/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include "lyricslayout.h"

#include "dom/masterscore.h"
#include "dom/repeatlist.h"
#include "style/styledef.h"

#include "dom/chordrest.h"
#include "dom/lyrics.h"
#include "dom/measure.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/stafftype.h"
#include "dom/system.h"

#include "tlayout.h"
#include "autoplace.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

static Lyrics* searchNextLyrics(Segment* s, staff_idx_t staffIdx, int verse, PlacementV p)
{
    Lyrics* l = nullptr;
    const Segment* originalSeg = s;
    while ((s = s->next1(SegmentType::ChordRest))) {
        if (!segmentsAreAdjacentInRepeatStructure(originalSeg, s)) {
            return nullptr;
        }

        track_idx_t strack = staffIdx * VOICES;
        track_idx_t etrack = strack + VOICES;
        // search through all tracks of current staff looking for a lyric in specified verse
        for (track_idx_t track = strack; track < etrack; ++track) {
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

void LyricsLayout::layout(Lyrics* item, LayoutContext& ctx)
{
    if (!item->explicitParent()) {   // palette & clone trick
        item->setPos(PointF());
        TLayout::layoutBaseTextBase1(item, ctx);
        return;
    }

    Lyrics::LayoutData* ldata = item->mutldata();

    //
    // parse leading verse number and/or punctuation, so we can factor it into layout separately
    //
    bool hasNumber = false;   // _verseNumber;

    // find:
    // 1) string of numbers and non-word characters at start of syllable
    // 2) at least one other character (indicating start of actual lyric)
    // 3) string of non-word characters at end of syllable
    //QRegularExpression leadingPattern("(^[\\d\\W]+)([^\\d\\W]+)");

    const String text = item->plainText();
    String leading;
    String trailing;

    if (ctx.conf().styleB(Sid::lyricsAlignVerseNumber)) {
        size_t leadingIdx = 0;
        for (size_t i = 0; i < text.size(); ++i) {
            Char ch = text.at(i);
            if (ch.isLetter()) {
                leadingIdx = i;
                break;
            }
        }

        if (leadingIdx != 0) {
            leading = text.mid(0, leadingIdx);
        }

        size_t trailingIdx = text.size() - 1;
        for (int i = static_cast<int>(text.size() - 1); i >= 0; --i) {
            Char ch = text.at(i);
            if (ch.isLetter()) {
                trailingIdx = i;
                break;
            }
        }

        if (trailingIdx != text.size() - 1) {
            trailing = text.mid(trailingIdx + 1);
        }

        if (!leading.isEmpty() && leading.at(0).isDigit()) {
            hasNumber = true;
        }
    }

    createOrRemoveLyricsLine(item, ctx);

    if (item->isMelisma() || hasNumber) {
        // use the melisma style alignment setting
        if (item->isStyled(Pid::ALIGN)) {
            item->setAlign(ctx.conf().styleV(Sid::lyricsMelismaAlign).value<Align>());
        }
    } else {
        // use the text style alignment setting
        if (item->isStyled(Pid::ALIGN)) {
            item->setAlign(item->propertyDefault(Pid::ALIGN).value<Align>());
        }
    }

    PointF o(item->propertyDefault(Pid::OFFSET).value<PointF>());

    // Negate ChordRest offset
    ChordRest* cr = item->chordRest();
    double x = o.x() - cr->x();

    TLayout::layoutBaseTextBase1(item, ctx);
    TLayout::computeTextHighResShape(item, ldata);

    double centerAdjust = 0.0;
    double leftAdjust   = 0.0;

    if (ctx.conf().styleB(Sid::lyricsAlignVerseNumber)) {
        // Calculate leading and trailing parts widths. Lyrics
        // should have text layout to be able to do it correctly.
        DO_ASSERT(ldata->blocks.size() != 0);
        if (!leading.isEmpty() || !trailing.isEmpty()) {
//                   LOGD("create leading, trailing <%s> -- <%s><%s>", muPrintable(text), muPrintable(leading), muPrintable(trailing));
            const TextBlock& tb = ldata->blocks.at(0);

            const double leadingWidth = tb.xpos(leading.size(), item) - tb.boundingRect().x();
            const size_t trailingPos = text.size() - trailing.size();
            const double trailingWidth = tb.boundingRect().right() - tb.xpos(trailingPos, item);

            leftAdjust = leadingWidth;
            centerAdjust = leadingWidth - trailingWidth;
        }
    }

    double nominalWidth = item->symWidth(SymId::noteheadBlack);
    if (item->align() == AlignH::HCENTER) {
        //
        // center under notehead, not origin
        // however, lyrics that are melismas or have verse numbers will be forced to left alignment
        //
        // center under note head
        x += nominalWidth * .5 - centerAdjust * 0.5;
    } else if (item->align() == AlignH::LEFT) {
        // even for left aligned syllables, ignore leading verse numbers and/or punctuation
        x -= leftAdjust;
    } else if (item->align() == AlignH::RIGHT) {
        x += nominalWidth;
    }

    ldata->setPosX(x);

    if (item->ticks().isNotZero()) {
        // set melisma end
        ChordRest* ecr = ctx.mutDom().findCR(item->endTick(), item->track());
        if (ecr) {
            ecr->setMelismaEnd(true);
        }
    }
}

void LyricsLayout::layout(LyricsLine* item, LayoutContext& ctx)
{
    if (item->isEndMelisma()) {           // melisma
        item->setLineWidth(ctx.conf().styleS(Sid::lyricsLineThickness));
    } else { // dash(es)
        item->setNextLyrics(searchNextLyrics(item->lyrics()->segment(),
                                             item->staffIdx(),
                                             item->lyrics()->no(),
                                             item->lyrics()->placement()
                                             ));
        item->setTrack2(item->nextLyrics() ? item->nextLyrics()->track() : item->track());

        Fraction endTick = item->tick();
        const Measure* lyricsMeasure = item->lyrics()->segment()->measure();
        const Segment* endCRSeg = lyricsMeasure ? lyricsMeasure->last(SegmentType::ChordRest) : nullptr;

        const ChordRest* endCR = nullptr;
        if (endCRSeg && !endCRSeg->empty()) {
            for (EngravingItem* cr : endCRSeg->elist()) {
                if (cr && cr->isChordRest()) {
                    endCR = toChordRest(cr);
                    break;
                }
            }
        }

        if (item->nextLyrics()) {
            endTick = item->nextLyrics()->tick();
        } else if (endCR && endCR->hasFollowingJumpItem()) {
            endTick = endCR->tick();
        }

        item->setTick2(endTick);
    }
}

void LyricsLayout::layout(LyricsLineSegment* item, LayoutContext& ctx)
{
    UNUSED(ctx);

    assert(item->isPartialLyricsLineSegment() || item->lyrics());

    if (!item->isPartialLyricsLineSegment()) {
        item->ryoffset() = 0.0;
    }

    LyricsLineSegment::LayoutData* ldata = item->mutldata();
    ldata->clearDashes();

    if (item->lyricsLine()->isEndMelisma()) {
        layoutMelismaLine(item);
    } else {
        layoutDashes(item);
    }

    double halfLineWidth = item->absoluteFromSpatium(item->lineWidth());
    RectF rect(PointF(), item->pos2());
    rect.adjust(0.0, -halfLineWidth, 0.0, halfLineWidth);

    ldata->setShape(Shape(rect, item));
}

void LyricsLayout::layoutMelismaLine(LyricsLineSegment* item)
{
    const bool isPartialLyricsLine = item->isPartialLyricsLineSegment();
    LyricsLine* lyricsLine = item->lyricsLine();
    Lyrics* startLyrics = lyricsLine->lyrics();

    System* system = item->system();
    if (!system) {
        return;
    }
    const MStyle& style = item->style();

    double startX = lyricsLineStartX(item);
    double endX = lyricsLineEndX(item);

    double tolerance = 0.05 * item->spatium();
    if (endX - startX < style.styleMM(Sid::lyricsMelismaMinLength) - tolerance) {
        const Fraction ticks = isPartialLyricsLine ? lyricsLine->ticks() : startLyrics->ticks();
        if (style.styleB(Sid::lyricsMelismaForce) || ticks == Lyrics::TEMP_MELISMA_TICKS) {
            endX = startX + style.styleMM(Sid::lyricsMelismaMinLength);
        } else {
            endX = startX;
        }
    }

    adjustLyricsLineYOffset(item);

    double y = 0.0; // actual value is set later

    item->setPos(startX, y);
    item->setPos2(PointF(endX - startX, 0.0));

    item->mutldata()->addDash(LineF(PointF(), item->pos2()));
}

void LyricsLayout::layoutDashes(LyricsLineSegment* item)
{
    const bool isPartialLyricsLine = item->isPartialLyricsLineSegment();
    LyricsLine* lyricsLine = item->lyricsLine();

    ChordRest* endCR = lyricsLine->endElement()
                       && lyricsLine->endElement()->isChordRest() ? toChordRest(lyricsLine->endElement()) : nullptr;
    Lyrics* endLyrics = nullptr;
    if (endCR) {
        for (Lyrics* lyr : endCR->lyrics()) {
            if (lyr->no() == item->no()) {
                endLyrics = lyr;
                break;
            }
        }
    }

    // When the end element is a timetick segment rather than a chordrest, the start element should be a chord with a following repeat
    ChordRest* startCR = lyricsLine->startElement()
                         && lyricsLine->startElement()->isChordRest() ? toChordRest(lyricsLine->startElement()) : nullptr;
    bool hasFollowingJump = endCR ? endCR->hasFollowingJumpItem() : (startCR ? startCR->hasFollowingJumpItem() : false);

    if (!endLyrics && !isPartialLyricsLine && !hasFollowingJump) {
        return;
    }

    System* system = item->system();
    if (!system) {
        return;
    }
    const MStyle& style = item->style();

    double startX = lyricsLineStartX(item);
    double endX = 0.0;
    if (endCR) {
        endX = lyricsLineEndX(item, endLyrics);
    } else {
        endX = startCR ? startCR->measure()->endingXForOpenEndedLines() : endX;
    }

    adjustLyricsLineYOffset(item, endLyrics);

    double y = 0.0; // actual value is set later

    item->setPos(startX, y);
    item->setPos2(PointF(endX - startX, 0.0));

    bool isDashOnFirstSyllable = lyricsLine->tick2() == system->firstMeasure()->tick();
    double curLength = endX - startX;
    double dashMinLength = style.styleMM(Sid::lyricsDashMinLength);
    bool firstAndLastGapAreHalf = style.styleB(Sid::lyricsDashFirstAndLastGapAreHalf);
    bool forceDash = style.styleB(Sid::lyricsDashForce)
                     || (style.styleB(Sid::lyricsShowDashIfSyllableOnFirstNote) && isDashOnFirstSyllable);
    double maxDashDistance = style.styleMM(Sid::lyricsDashMaxDistance);
    int dashCount = firstAndLastGapAreHalf && curLength > maxDashDistance ? std::ceil(curLength / maxDashDistance)
                    : std::floor(curLength / maxDashDistance);

    if (curLength > dashMinLength || forceDash) {
        dashCount = std::max(dashCount, 1);
    }

    if (curLength < dashMinLength && dashCount > 0) {
        double diff = dashMinLength - curLength;
        if (isDashOnFirstSyllable) {
            startX -= diff;
        } else {
            startX -= 0.5 * diff;
            endX += 0.5 * diff;
        }
        item->setPos(startX, y);
        item->setPos2(PointF(endX - startX, 0.0));
        curLength = endX - startX;
    }

    double dashWidth = std::min(curLength, style.styleMM(Sid::lyricsDashMaxLength).val());

    LyricsDashSystemStart lyricsDashSystemStart = style.styleV(Sid::lyricsDashPosAtStartOfSystem).value<LyricsDashSystemStart>();
    bool dashesLeftAligned = lyricsDashSystemStart != LyricsDashSystemStart::STANDARD && !item->isSingleBeginType();
    double dashDist = curLength / (dashesLeftAligned || firstAndLastGapAreHalf ? dashCount : dashCount + 1);
    double xDash = 0.0;
    if (dashesLeftAligned) {
        for (int i = 0; i < dashCount; ++i) {
            item->mutldata()->addDash(LineF(PointF(xDash, 0.0), PointF(xDash + dashWidth, 0.0)));
            xDash += dashDist;
        }
    } else {
        for (int i = 0; i < dashCount; ++i) {
            if (firstAndLastGapAreHalf && i == 0) {
                xDash += 0.5 * dashDist;
            } else {
                xDash += dashDist;
            }
            item->mutldata()->addDash(LineF(PointF(xDash - 0.5 * dashWidth, 0.0), PointF(xDash + 0.5 * dashWidth, 0.0)));
        }
    }
}

Lyrics* LyricsLayout::findNextLyrics(const ChordRest* endChordRest, int verseNumber)
{
    if (!endChordRest) {
        return nullptr;
    }
    for (Segment* segment = endChordRest->segment()->next1(SegmentType::ChordRest); segment;
         segment = segment->next1(SegmentType::ChordRest)) {
        if (!segment->elementAt(endChordRest->track())) {
            continue;
        }
        ChordRest* nextCR = toChordRest(segment->elementAt(endChordRest->track()));
        for (Lyrics* lyr : nextCR->lyrics()) {
            if (lyr->no() == verseNumber) {
                return lyr;
            }
        }
    }

    return nullptr;
}

void LyricsLayout::createOrRemoveLyricsLine(Lyrics* item, LayoutContext& ctx)
{
    if (item->needRemoveInvalidSegments()) {
        item->removeInvalidSegments();
    }

    auto isEndMelisma = [item]() {
        return item->ticks().isNotZero();
    };

    Fraction lyricsLineTicks = Lyrics::TEMP_MELISMA_TICKS;

    // Update the end tick
    if (isEndMelisma()) {
        const track_idx_t track = item->track();

        const Segment* const startSegment = item->segment();
        const Fraction startTick = startSegment->tick();

        // if lyrics has a temporary one-chord melisma, interpret as 0 ticks (just its own chord)
        Fraction itemEndTick = item->ticks() == Lyrics::TEMP_MELISMA_TICKS ? startTick : item->endTick();
        Fraction endTick = itemEndTick;

        const Segment* endSegment = startSegment;
        while (endSegment && endSegment->tick() < endTick) {
            endSegment = endSegment->nextCR(track, true);
        }
        if (!endSegment) {
            // user probably deleted measures at end of score, leaving this melisma too long
            // set endSegment to last segment and reset lyricsEndTick to trigger FIXUP code below
            endSegment = ctx.dom().lastSegment();
            endTick = Fraction(-1, 1);
        }

        EngravingItem* endSegmentElement = endSegment->element(track);
        if (endSegment->tick() == endTick && endSegmentElement && endSegmentElement->type() == ElementType::CHORD) {
            // everything is OK if we have reached a chord at right tick on right track
            // advance to next CR after duration of note, or last segment if no next CR
            const Segment* endChordSeg = endSegment;
            const Chord* endChord = toChord(endSegmentElement);

            endSegment = endChordSeg->nextCR(track, false);

            if (!endSegment) {
                endSegment = endChordSeg;
                while (endSegment && endSegment->tick() < endChord->tick() + endChord->ticks()) {
                    endSegment = endSegment->nextCR(muse::nidx, true);
                }
            }
        } else {
            // FIXUP - lyrics tick count not valid
            // this happens if edits to score have removed the original end segment
            // so let's fix it here
            // endSegment is already pointing to segment past endTick (or to last segment)
            // we should shorten the lyrics tick count to make this work
            const Segment* ns = endSegment;
            const Segment* ps = endSegment->prev1(SegmentType::ChordRest);
            while (ps && ps != startSegment) {
                EngravingItem* pe = ps->element(track);
                // we're looking for an actual chord on this track
                if (pe && pe->type() == ElementType::CHORD) {
                    break;
                }
                endSegment = ps;
                ps = ps->prev1(SegmentType::ChordRest);
            }

            if (!ps || ps == startSegment) {
                // either there is no valid previous CR, or the previous CR is the one the lyric starts on
                // we don't want to make the melisma longer arbitrarily, but there is a possibility that the next
                // CR won't extend the melisma, so let's check it
                ps = ns;
                endSegment = ps->nextCR(track, false);
                EngravingItem* e = endSegment ? endSegment->element(track) : nullptr;

                // check to make sure we have a chord
                if (!e || e->type() != ElementType::CHORD || ps->tick() > itemEndTick) {
                    // nope, nothing to do but set ticks to 0
                    // this will result in no melisma
                    item->undoChangeProperty(Pid::LYRIC_TICKS, Fraction::fromTicks(0));
                } else {
                    item->undoChangeProperty(Pid::LYRIC_TICKS, ps->tick() - startTick);
                }
            } else {
                item->undoChangeProperty(Pid::LYRIC_TICKS, ps->tick() - startTick);
            }
        }

        if (endSegment) {
            // Lyrics::_ticks points to the beginning of the last spanned segment,
            // but the line shall include it:
            // include the duration of this last segment in the melisma duration
            lyricsLineTicks = endSegment->tick() - startTick;
        } else {
            lyricsLineTicks = item->score()->endTick() - startTick;
        }
    }

    ChordRest* cr = item->chordRest();

    if (isEndMelisma() || item->syllabic() == LyricsSyllabic::BEGIN || item->syllabic() == LyricsSyllabic::MIDDLE) {
        if (!item->separator()) {
            LyricsLine* separator = new LyricsLine(ctx.mutDom().dummyParent());
            separator->setTick(cr->tick());
            item->setSeparator(separator);
            ctx.mutDom().addUnmanagedSpanner(item->separator());
        }
        item->separator()->setParent(item);
        item->separator()->setTick(cr->tick());
        item->separator()->setTicks(lyricsLineTicks);
        item->separator()->setTrack(item->track());
        item->separator()->setTrack2(item->track());
        item->separator()->setVisible(item->visible());
    } else {
        if (item->separator()) {
            item->separator()->removeUnmanaged();
            delete item->separator();
            item->setSeparator(nullptr);
        }
    }
}

void LyricsLayout::computeVerticalPositions(System* system, LayoutContext& ctx)
{
    staff_idx_t nStaves = system->score()->nstaves();

    std::vector<staff_idx_t> visibleStaves;
    visibleStaves.reserve(system->staves().size());

    for (staff_idx_t staffIdx = 0; staffIdx < nStaves; ++staffIdx) {
        if (system->staff(staffIdx)->show()) {
            computeVerticalPositions(staffIdx, system, ctx);
        }
    }
}

void LyricsLayout::computeVerticalPositions(staff_idx_t staffIdx, System* system, LayoutContext& ctx)
{
    LyricsVersesMap lyricsVersesAbove;
    LyricsVersesMap lyricsVersesBelow;

    collectLyricsVerses(staffIdx, system, lyricsVersesAbove, lyricsVersesBelow);

    setDefaultPositions(staffIdx, lyricsVersesAbove, lyricsVersesBelow, ctx);

    checkCollisionsWithStaffElements(system, staffIdx, ctx, lyricsVersesAbove, lyricsVersesBelow);

    addToSkyline(system, staffIdx, ctx, lyricsVersesAbove, lyricsVersesBelow);
}

void LyricsLayout::collectLyricsVerses(staff_idx_t staffIdx, System* system, LyricsVersesMap& lyricsVersesAbove,
                                       LyricsVersesMap& lyricsVersesBelow)
{
    track_idx_t startTrack = staffIdx * VOICES;
    track_idx_t endTrack = startTrack + VOICES;

    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment& segment : toMeasure(mb)->segments()) {
            if (!segment.isChordRestType()) {
                continue;
            }
            for (track_idx_t track = startTrack; track < endTrack; ++track) {
                EngravingItem* element = segment.elementAt(track);
                if (!element) {
                    continue;
                }
                for (Lyrics* lyrics : toChordRest(element)->lyrics()) {
                    int verse = lyrics->no();
                    if (lyrics->placeAbove()) {
                        lyricsVersesAbove[verse].addLyrics(lyrics);
                    } else {
                        lyricsVersesBelow[verse].addLyrics(lyrics);
                    }
                }
            }
        }
    }

    for (SpannerSegment* spannerSegment : system->spannerSegments()) {
        if (spannerSegment->staffIdx() == staffIdx && spannerSegment->isLyricsLineSegment()) {
            if (muse::RealIsNull(spannerSegment->pos2().x())) {
                continue;
            }
            LyricsLineSegment* lyricsLineSegment = toLyricsLineSegment(spannerSegment);
            int verse = lyricsLineSegment->no();
            if (lyricsLineSegment->lyricsPlaceAbove()) {
                lyricsVersesAbove[verse].addLine(lyricsLineSegment);
            } else {
                lyricsVersesBelow[verse].addLine(lyricsLineSegment);
            }
        }
    }
}

void LyricsLayout::setDefaultPositions(staff_idx_t staffIdx, const LyricsVersesMap& lyricsVersesAbove,
                                       const LyricsVersesMap& lyricsVersesBelow,
                                       LayoutContext& ctx)
{
    double staffHeight = ctx.dom().staff(staffIdx)->staffHeight();
    double lyricsLineHeightFactor = ctx.conf().styleD(Sid::lyricsLineHeight);

    int maxVerseAbove = !lyricsVersesAbove.empty() ? lyricsVersesAbove.crbegin()->first : 0;
    for (auto& pair : lyricsVersesAbove) {
        int verse = pair.first;
        const LyricsVerse& lyricsVerse = pair.second;
        for (Lyrics* lyrics : lyricsVerse.lyrics()) {
            double y = -(maxVerseAbove - verse) * lyrics->lineSpacing() * lyricsLineHeightFactor;
            lyrics->setYRelativeToStaff(y);
        }
        for (LyricsLineSegment* lyricsLineSegment : lyricsVerse.lines()) {
            double y = -(maxVerseAbove - verse) * lyricsLineSegment->lineSpacing() * lyricsLineHeightFactor;
            lyricsLineSegment->move(PointF(0.0, y + lyricsLineSegment->baseLineShift()));
        }
    }

    for (auto& pair : lyricsVersesBelow) {
        int verse = pair.first;
        const LyricsVerse& lyricsVerse = pair.second;
        for (Lyrics* lyrics : lyricsVerse.lyrics()) {
            double y = staffHeight + verse * lyrics->lineSpacing() * lyricsLineHeightFactor;
            lyrics->setYRelativeToStaff(y);
        }
        for (LyricsLineSegment* lyricsLineSegment : lyricsVerse.lines()) {
            double y = staffHeight + verse * lyricsLineSegment->lineSpacing() * lyricsLineHeightFactor;
            lyricsLineSegment->move(PointF(0.0, y + lyricsLineSegment->baseLineShift()));
        }
    }
}

void LyricsLayout::checkCollisionsWithStaffElements(System* system, staff_idx_t staffIdx,  LayoutContext& ctx,
                                                    const LyricsVersesMap& lyricsVersesAbove,
                                                    const LyricsVersesMap& lyricsVersesBelow)
{
    SysStaff* systemStaff = system->staff(staffIdx);

    double lyricsMinDist = ctx.conf().styleMM(Sid::lyricsMinTopDistance);

    SkylineLine& staffSkylineNorth = systemStaff->skyline().north();
    SkylineLine& staffSkylineSouth = systemStaff->skyline().south();

    int maxVerseAbove = !lyricsVersesAbove.empty() ? lyricsVersesAbove.crbegin()->first : 0;
    int maxVerseBelow = !lyricsVersesBelow.empty() ? lyricsVersesBelow.crbegin()->first : 0;

    for (int verse = maxVerseAbove; verse >= 0; --verse) {
        if (lyricsVersesAbove.count(verse) == 0) {
            continue;
        }
        SkylineLine verseSkyline = createSkylineForVerse(verse, false, lyricsVersesAbove, system);
        double minDistance = -verseSkyline.minDistance(staffSkylineNorth);
        if (minDistance < lyricsMinDist) {
            double diff = lyricsMinDist - minDistance;
            moveThisVerseAndOuterOnes(verse, 0, true, -diff, lyricsVersesAbove);
        }
    }

    for (int verse = 0; verse <= maxVerseBelow; ++verse) {
        if (lyricsVersesBelow.count(verse) == 0) {
            continue;
        }
        SkylineLine verseSkyline = createSkylineForVerse(verse, true, lyricsVersesBelow, system);
        double minDistance = -staffSkylineSouth.minDistance(verseSkyline);
        if (minDistance < lyricsMinDist) {
            double diff = lyricsMinDist - minDistance;
            moveThisVerseAndOuterOnes(verse, maxVerseBelow, false, diff, lyricsVersesBelow);
        }
    }
}

SkylineLine LyricsLayout::createSkylineForVerse(int verse, bool north, const LyricsVersesMap& lyricsVerses, System* system)
{
    double systemX = system->pageX();

    SkylineLine lyricsSkyline(north);

    if (lyricsVerses.count(verse) > 0) {
        const LyricsVerse& lyricsVerse = lyricsVerses.at(verse);
        for (Lyrics* lyrics : lyricsVerse.lyrics()) {
            if (lyrics->addToSkyline()) {
                Shape lyricsShape = lyrics->highResShape().translated(PointF(lyrics->pageX() - systemX, lyrics->yRelativeToStaff()));
                lyricsSkyline.add(lyricsShape);
            }
        }
        for (LyricsLineSegment* lyricsLineSeg : lyricsVerse.lines()) {
            if (lyricsLineSeg->lyricsAddToSkyline()) {
                lyricsSkyline.add(lyricsLineSeg->shape().translate(lyricsLineSeg->pos()));
            }
        }
    }

    return lyricsSkyline;
}

void LyricsLayout::moveThisVerseAndOuterOnes(int verse, int lastVerse, bool above, double diff, const LyricsVersesMap& lyricsVerses)
{
    auto moveVerse = [&](int verse) {
        if (lyricsVerses.count(verse) > 0) {
            const LyricsVerse& lyricsVerse = lyricsVerses.at(verse);
            for (Lyrics* lyrics : lyricsVerse.lyrics()) {
                lyrics->move(PointF(0.0, diff));
            }
            for (LyricsLineSegment* lyricsLineSeg : lyricsVerse.lines()) {
                lyricsLineSeg->move(PointF(0.0, diff));
            }
        }
    };

    if (above) {
        for (int otherVerse = verse; otherVerse >= lastVerse; --otherVerse) {
            moveVerse(otherVerse);
        }
    } else {
        for (int otherVerse = verse; otherVerse <= lastVerse; ++otherVerse) {
            moveVerse(otherVerse);
        }
    }
}

void LyricsLayout::addToSkyline(System* system, staff_idx_t staffIdx, LayoutContext& ctx, const LyricsVersesMap& lyricsVersesAbove,
                                const LyricsVersesMap& lyricsVersesBelow)
{
    double systemX = system->pageX();
    // HACK: subtract minVerticalDistance here because it's added later during staff distance calculations. Needs a better solution.
    double lyricsVerticalPadding = ctx.conf().styleMM(Sid::lyricsMinBottomDistance) - ctx.conf().styleMM(Sid::minVerticalDistance);
    Skyline& skyline = system->staff(staffIdx)->skyline();
    for (auto& pair : lyricsVersesAbove) {
        const LyricsVerse& lyricsVerse = pair.second;
        for (Lyrics* lyrics : lyricsVerse.lyrics()) {
            if (lyrics->addToSkyline()) {
                Shape lyricsShape
                    = lyrics->highResShape().translated(PointF(lyrics->pageX() - systemX, lyrics->yRelativeToStaff()));
                skyline.north().add(lyricsShape.adjust(0.0, -lyricsVerticalPadding, 0.0, 0.0));
            }
        }
        for (LyricsLineSegment* lyricsLineSeg : lyricsVerse.lines()) {
            if (lyricsLineSeg->lyricsAddToSkyline()) {
                Shape lineShape = lyricsLineSeg->shape().translate(lyricsLineSeg->pos());
                skyline.north().add(lineShape.adjust(0.0, -lyricsVerticalPadding, 0.0, 0.0));
            }
        }
    }
    for (auto& pair : lyricsVersesBelow) {
        const LyricsVerse& lyricsVerse = pair.second;
        for (Lyrics* lyrics : lyricsVerse.lyrics()) {
            if (lyrics->addToSkyline()) {
                Shape lyricsShape
                    = lyrics->highResShape().translated(PointF(lyrics->pageX() - systemX, lyrics->yRelativeToStaff()));
                skyline.south().add(lyricsShape.adjust(0.0, 0.0, 0.0, lyricsVerticalPadding));
            }
        }
        for (LyricsLineSegment* lyricsLineSeg : lyricsVerse.lines()) {
            if (lyricsLineSeg->lyricsAddToSkyline()) {
                Shape lineShape = lyricsLineSeg->shape().translate(lyricsLineSeg->pos());
                skyline.south().add(lineShape.adjust(0.0, 0.0, 0.0, lyricsVerticalPadding));
            }
        }
    }
}

double LyricsLayout::lyricsLineStartX(const LyricsLineSegment* item)
{
    const System* system = item->system();
    const LyricsLine* lyricsLine = item->lyricsLine();
    const Lyrics* startLyrics = lyricsLine->lyrics();
    const MStyle& style = item->style();
    const bool melisma = lyricsLine->isEndMelisma();

    const LyricsDashSystemStart lyricsDashSystemStart = style.styleV(Sid::lyricsDashPosAtStartOfSystem).value<LyricsDashSystemStart>();
    const bool leading = melisma || (lyricsDashSystemStart != LyricsDashSystemStart::UNDER_FIRST_NOTE);

    // Full melisma or dashes at beginning of system
    if (!item->isSingleBeginType()) {
        return system->firstNoteRestSegmentX(leading);
    }

    // Partial melisma or dashes
    if (lyricsLine->isPartialLyricsLine()) {
        const Measure* measure = lyricsLine->findStartMeasure();
        return measure->firstNoteRestSegmentX(leading);
    }

    // Full melisma or dashes in middle of system
    const double pad = melisma ? style.styleMM(Sid::lyricsMelismaPad) : style.styleMM(Sid::lyricsDashPad);
    const double lyricsRightEdge = startLyrics->pageX() - system->pageX() + startLyrics->shape().right();
    return lyricsRightEdge + pad;
}

double LyricsLayout::lyricsLineEndX(const LyricsLineSegment* item, const Lyrics* endLyrics)
{
    const System* system = item->system();
    const LyricsLine* lyricsLine = item->lyricsLine();
    const ChordRest* endChordRest = toChordRest(lyricsLine->endElement());
    const double systemPageX = system->pageX();
    const MStyle& style = item->style();
    const bool melisma = lyricsLine->isEndMelisma();
    const bool hasFollowingJump = endChordRest->hasFollowingJumpItem();
    const bool hasPrecedingJump = endChordRest->hasPrecedingJumpItem();

    if (!melisma && !item->isPartialLyricsLineSegment() && !endLyrics && !hasFollowingJump) {
        return 0.0;
    }

    const bool dashesEndOfSystem = !melisma && lyricsLine->tick2() == system->endTick();

    // Full melisma or dashes at end of system
    if (!item->isSingleEndType() || dashesEndOfSystem || !lyricsLine->endElement()) {
        return system->endingXForOpenEndedLines();
    }

    // Partial dashes after a repeat
    if (!melisma && hasPrecedingJump && item->isPartialLyricsLineSegment()) {
        if (endLyrics) {
            const double lyricsLeftEdge = endLyrics->pageX() - systemPageX + endLyrics->ldata()->bbox().left();
            return lyricsLeftEdge - style.styleMM(Sid::lyricsDashPad);
        } else if (endChordRest->isChord()) {
            const Chord* endChord = toChord(endChordRest);
            const Note* note = endChord->up() ? endChord->downNote() : endChord->upNote();
            return note->pageX() - systemPageX + note->headWidth();
        }
    }

    // Full dashes or melisma before a repeat - possibly with a partial dash/repeat on the other side of the repeat
    if (endChordRest && hasFollowingJump && !lyricsLine->nextLyrics()) {
        Measure* endMeasure = lyricsLine->findEndMeasure();
        // check if there is a partial lyrics line in a following measure
        if (repeatHasPartialLyricLine(endMeasure) || endChordRest == lyricsLine->startElement()) {
            return endMeasure->endingXForOpenEndedLines();
        }
    }

    // Full or partial dashes in middle of system
    if (!melisma && endLyrics) {
        const double lyricsLeftEdge = endLyrics->pageX() - systemPageX + endLyrics->ldata()->bbox().left();
        return lyricsLeftEdge - style.styleMM(Sid::lyricsDashPad);
    }

    // Full or partial melisma in middle of system
    if (endChordRest->isChord()) {
        const Chord* endChord = toChord(endChordRest);
        const Note* note = endChord->up() ? endChord->downNote() : endChord->upNote();
        return note->pageX() - systemPageX + note->headWidth();
    }

    return endChordRest->pageX() - systemPageX + endChordRest->rightEdge();
}

void LyricsLayout::adjustLyricsLineYOffset(LyricsLineSegment* item, const Lyrics* endLyrics)
{
    const LyricsLine* lyricsLine = item->lyricsLine();
    ChordRest* endChordRest = lyricsLine->endElement()
                              && lyricsLine->endElement()->isChordRest() ? toChordRest(lyricsLine->endElement()) : nullptr;
    const Lyrics* startLyrics = lyricsLine->lyrics();
    const bool melisma = lyricsLine->isEndMelisma();

    // Partial melisma or dashes
    if (lyricsLine->isPartialLyricsLine()) {
        Lyrics* nextLyrics = findNextLyrics(endChordRest, item->no());
        item->ryoffset() = nextLyrics ? nextLyrics->offset().y() : item->offset().y();
        return;
    }

    if (item->isSingleBeginType()) {
        item->ryoffset() = startLyrics->offset().y();
        return;
    }

    if (melisma || !endLyrics) {
        Lyrics* nextLyrics = findNextLyrics(endChordRest, item->no());
        item->ryoffset() = nextLyrics ? nextLyrics->offset().y() : startLyrics->offset().y();
        return;
    }

    item->ryoffset() = endLyrics->offset().y();
}
