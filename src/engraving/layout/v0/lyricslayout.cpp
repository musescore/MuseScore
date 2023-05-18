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
#include "lyricslayout.h"

#include "style/styledef.h"

#include "libmscore/chordrest.h"
#include "libmscore/lyrics.h"
#include "libmscore/measure.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/stafftype.h"
#include "libmscore/system.h"

using namespace mu;
using namespace mu::engraving;
using namespace mu::engraving::layout::v0;

static Lyrics* searchNextLyrics(Segment* s, staff_idx_t staffIdx, int verse, PlacementV p)
{
    Lyrics* l = 0;
    while ((s = s->next1(SegmentType::ChordRest))) {
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

void LyricsLayout::layout(Lyrics* item, LayoutContext&)
{
    if (!item->explicitParent()) {   // palette & clone trick
        item->setPos(PointF());
        item->TextBase::layout1();
        return;
    }

    //
    // parse leading verse number and/or punctuation, so we can factor it into layout separately
    //
    bool hasNumber     = false;   // _verseNumber;

    // find:
    // 1) string of numbers and non-word characters at start of syllable
    // 2) at least one other character (indicating start of actual lyric)
    // 3) string of non-word characters at end of syllable
    //QRegularExpression leadingPattern("(^[\\d\\W]+)([^\\d\\W]+)");

    const String text = item->plainText();
    String leading;
    String trailing;

    if (item->score()->styleB(Sid::lyricsAlignVerseNumber)) {
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

    bool styleDidChange = false;
    if (item->isEven() && !item->_even) {
        item->initTextStyleType(TextStyleType::LYRICS_EVEN, /* preserveDifferent */ true);
        item->_even = true;
        styleDidChange = true;
    }
    if (!item->isEven() && item->_even) {
        item->initTextStyleType(TextStyleType::LYRICS_ODD, /* preserveDifferent */ true);
        item->_even = false;
        styleDidChange = true;
    }

    if (styleDidChange) {
        item->styleChanged();
    }

    ChordRest* cr = item->chordRest();
    if (item->_removeInvalidSegments) {
        item->removeInvalidSegments();
    } else if (item->_ticks > Fraction(0, 1) || item->_syllabic == LyricsSyllabic::BEGIN || item->_syllabic == LyricsSyllabic::MIDDLE) {
        if (!item->_separator) {
            item->_separator = new LyricsLine(item->score()->dummy());
            item->_separator->setTick(cr->tick());
            item->score()->addUnmanagedSpanner(item->_separator);
        }
        item->_separator->setParent(item);
        item->_separator->setTick(cr->tick());
        // HACK separator should have non-zero length to get its layout
        // always triggered. A proper ticks length will be set later on the
        // separator layout.
        item->_separator->setTicks(Fraction::fromTicks(1));
        item->_separator->setTrack(item->track());
        item->_separator->setTrack2(item->track());
        item->_separator->setVisible(item->visible());
        // bbox().setWidth(bbox().width());  // ??
    } else {
        if (item->_separator) {
            item->_separator->removeUnmanaged();
            delete item->_separator;
            item->_separator = 0;
        }
    }

    if (item->isMelisma() || hasNumber) {
        // use the melisma style alignment setting
        if (item->isStyled(Pid::ALIGN)) {
            item->setAlign(item->score()->styleV(Sid::lyricsMelismaAlign).value<Align>());
        }
    } else {
        // use the text style alignment setting
        if (item->isStyled(Pid::ALIGN)) {
            item->setAlign(item->propertyDefault(Pid::ALIGN).value<Align>());
        }
    }

    PointF o(item->propertyDefault(Pid::OFFSET).value<PointF>());
    item->setPosX(o.x());
    double x = item->pos().x();
    item->TextBase::layout1();

    double centerAdjust = 0.0;
    double leftAdjust   = 0.0;

    if (item->score()->styleB(Sid::lyricsAlignVerseNumber)) {
        // Calculate leading and trailing parts widths. Lyrics
        // should have text layout to be able to do it correctly.
        DO_ASSERT(item->rows() != 0);
        if (!leading.isEmpty() || !trailing.isEmpty()) {
//                   LOGD("create leading, trailing <%s> -- <%s><%s>", muPrintable(text), muPrintable(leading), muPrintable(trailing));
            const TextBlock& tb = item->textBlock(0);

            const double leadingWidth = tb.xpos(leading.size(), item) - tb.boundingRect().x();
            const size_t trailingPos = text.size() - trailing.size();
            const double trailingWidth = tb.boundingRect().right() - tb.xpos(trailingPos, item);

            leftAdjust = leadingWidth;
            centerAdjust = leadingWidth - trailingWidth;
        }
    }

    if (item->align() == AlignH::HCENTER) {
        //
        // center under notehead, not origin
        // however, lyrics that are melismas or have verse numbers will be forced to left alignment
        //
        // center under note head
        double nominalWidth = item->symWidth(SymId::noteheadBlack);
        x += nominalWidth * .5 - cr->x() - centerAdjust * 0.5;
    } else if (!(item->align() == AlignH::RIGHT)) {
        // even for left aligned syllables, ignore leading verse numbers and/or punctuation
        x -= leftAdjust;
    }

    item->setPosX(x);

    if (item->_ticks.isNotZero()) {
        // set melisma end
        ChordRest* ecr = item->score()->findCR(item->endTick(), item->track());
        if (ecr) {
            ecr->setMelismaEnd(true);
        }
    }
}

void LyricsLayout::layout(LyricsLine* item, LayoutContext&)
{
    bool tempMelismaTicks = (item->lyrics()->ticks() == Fraction::fromTicks(Lyrics::TEMP_MELISMA_TICKS));
    if (item->isEndMelisma()) {           // melisma
        item->setLineWidth(item->score()->styleMM(Sid::lyricsLineThickness));
        // if lyrics has a temporary one-chord melisma, set to 0 ticks (just its own chord)
        if (tempMelismaTicks) {
            item->lyrics()->setTicks(Fraction(0, 1));
        }

        // Lyrics::_ticks points to the beginning of the last spanned segment,
        // but the line shall include it:
        // include the duration of this last segment in the melisma duration
        Segment* lyricsSegment   = item->lyrics()->segment();
        Fraction lyricsStartTick = lyricsSegment->tick();
        Fraction lyricsEndTick   = item->lyrics()->endTick();
        track_idx_t lyricsTrack  = item->lyrics()->track();

        // find segment with tick >= endTick
        Segment* s = lyricsSegment;
        while (s && s->tick() < lyricsEndTick) {
            s = s->nextCR(lyricsTrack, true);
        }
        if (!s) {
            // user probably deleted measures at end of score, leaving this melisma too long
            // set s to last segment and reset lyricsEndTick to trigger FIXUP code below
            s = item->score()->lastSegment();
            lyricsEndTick = Fraction(-1, 1);
        }
        EngravingItem* se = s->element(lyricsTrack);
        // everything is OK if we have reached a chord at right tick on right track
        if (s->tick() == lyricsEndTick && se && se->type() == ElementType::CHORD) {
            // advance to next CR, or last segment if no next CR
            s = s->nextCR(lyricsTrack, true);
            if (!s) {
                s = item->score()->lastSegment();
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
                EngravingItem* pe = ps->element(lyricsTrack);
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
                EngravingItem* e = s ? s->element(lyricsTrack) : nullptr;
                // check to make sure we have a chord
                if (!e || e->type() != ElementType::CHORD) {
                    // nothing to do but set ticks to 0
                    // this will result in melisma being deleted later
                    item->lyrics()->undoChangeProperty(Pid::LYRIC_TICKS, Fraction::fromTicks(0));
                    item->setTicks(Fraction(0, 1));
                    return;
                }
            }
            item->lyrics()->undoChangeProperty(Pid::LYRIC_TICKS, ps->tick() - lyricsStartTick);
        }
        // Spanner::computeEndElement() will actually ignore this value and use the (earlier) lyrics()->endTick() instead
        // still, for consistency with other lines, we should set the ticks for this to the computed (later) value
        if (s) {
            item->setTicks(s->tick() - lyricsStartTick);
        }
    } else {                                    // dash(es)
        item->_nextLyrics = searchNextLyrics(item->lyrics()->segment(),
                                             item->staffIdx(),
                                             item->lyrics()->no(),
                                             item->lyrics()->placement()
                                             );

        item->setTick2(item->_nextLyrics ? item->_nextLyrics->segment()->tick() : item->tick());
    }
    if (item->ticks().isNotZero()) {                  // only do layout if some time span
        // do layout with non-0 duration
        if (tempMelismaTicks) {
            item->lyrics()->setTicks(Fraction::fromTicks(Lyrics::TEMP_MELISMA_TICKS));
        }
    }
}

void LyricsLayout::layout(LyricsLineSegment* item, LayoutContext&)
{
    item->ryoffset() = 0.0;

    bool endOfSystem       = false;
    bool isEndMelisma      = item->lyricsLine()->isEndMelisma();
    Lyrics* lyr               = 0;
    Lyrics* nextLyr           = 0;
    double fromX             = 0;
    double toX               = 0;                     // start and end point of intra-lyrics room
    double sp                = item->spatium();
    System* sys;

    if (item->lyricsLine()->ticks() <= Fraction(0, 1)) {     // if no span,
        item->_numOfDashes = 0;                 // nothing to draw
        return;                           // and do nothing
    }

    // HORIZONTAL POSITION
    // A) if line precedes a syllable, advance line end to right before the next syllable text
    // if not a melisma and there is a next syllable;
    if (!isEndMelisma && item->lyricsLine()->nextLyrics() && item->isSingleEndType()) {
        lyr         = nextLyr = item->lyricsLine()->nextLyrics();
        sys         = lyr->segment()->system();
        endOfSystem = (sys != item->system());
        // if next lyrics is on a different system, this line segment is at the end of its system:
        // do not adjust for next lyrics position
        if (sys && !endOfSystem) {
            double lyrX        = lyr->bbox().x();
            double lyrXp       = lyr->pagePos().x();
            double sysXp       = sys->pagePos().x();
            toX               = lyrXp - sysXp + lyrX;             // syst.rel. X pos.
            double offsetX     = toX - item->pos().x() - item->pos2().x() - item->score()->styleMM(Sid::lyricsDashPad);
            //                    delta from current end pos.| ending padding
            item->rxpos2()          += offsetX;
        }
    }
    // B) if line follows a syllable, advance line start to after the syllable text
    lyr  = item->lyricsLine()->lyrics();
    sys  = lyr->segment()->system();
    if (sys && item->isSingleBeginType()) {
        double lyrX        = lyr->bbox().x();
        double lyrXp       = lyr->pagePos().x();
        double lyrW        = lyr->bbox().width();
        double sysXp       = sys->pagePos().x();
        fromX             = lyrXp - sysXp + lyrX + lyrW;
        //               syst.rel. X pos. | lyr.advance
        double offsetX     = fromX - item->pos().x();
        offsetX           += item->score()->styleMM(isEndMelisma ? Sid::lyricsMelismaPad : Sid::lyricsDashPad);

        //               delta from curr.pos. | add initial padding
        item->movePosX(offsetX);
        item->rxpos2()          -= offsetX;
    }

    // VERTICAL POSITION: at the base line of the syllable text
    if (!item->isEndType()) {
        item->setPosY(lyr->ipos().y());
        item->ryoffset() = lyr->offset().y();
    } else {
        // use Y position of *next* syllable if there is one on same system
        Lyrics* nextLyr1 = searchNextLyrics(lyr->segment(), lyr->staffIdx(), lyr->no(), lyr->placement());
        if (nextLyr1 && nextLyr1->segment()->system() == item->system()) {
            item->setPosY(nextLyr1->ipos().y());
            item->ryoffset() = nextLyr1->offset().y();
        } else {
            item->setPosY(lyr->ipos().y());
            item->ryoffset() = lyr->offset().y();
        }
    }

    // MELISMA vs. DASHES
    const double minMelismaLen = 1 * sp; // TODO: style setting
    const double minDashLen  = item->score()->styleS(Sid::lyricsDashMinLength).val() * sp;
    const double maxDashDist = item->score()->styleS(Sid::lyricsDashMaxDistance).val() * sp;
    double len = item->pos2().rx();
    if (isEndMelisma) {                   // melisma
        if (len < minMelismaLen) { // Omit the extender line if too short
            item->_numOfDashes = 0;
        } else {
            item->_numOfDashes = 1;
        }
        item->movePosY(-item->lyricsLine()->lineWidth() * .5);     // let the line 'sit on' the base line
        // if not final segment, shorten it (why? -AS)
        /*
        if (isBeginType() || isMiddleType()) {
            rxpos2() -= score()->styleP(Sid::minNoteDistance) * mag();
        }
        */
    } else {                              // dash(es)
        // set conventional dash Y pos
        item->movePosY(-lyr->fontMetrics().xHeight() * item->score()->styleD(Sid::lyricsDashYposRatio));
        item->_dashLength = item->score()->styleMM(Sid::lyricsDashMaxLength) * item->mag();      // and dash length
        if (len < minDashLen) {                                               // if no room for a dash
            // if at end of system or dash is forced
            if (endOfSystem || item->score()->styleB(Sid::lyricsDashForce)) {
                item->rxpos2()          = minDashLen;                               //     draw minimal dash
                item->_numOfDashes      = 1;
                item->_dashLength       = minDashLen;
            } else {                                                          //   if within system or dash not forced
                item->_numOfDashes = 0;                                             //     draw no dash
            }
        } else if (len < (maxDashDist * 1.5)) {                               // if no room for two dashes
            item->_numOfDashes = 1;                                                 //    draw one dash
            if (item->_dashLength > len) {                                          // if no room for a full dash
                item->_dashLength = len;                                            //    shorten it
            }
        } else {
            item->_numOfDashes = len / maxDashDist + 1;                             // draw several dashes
        }
        // adjust next lyrics horiz. position if too little a space forced to skip the dash
        if (item->_numOfDashes == 0 && nextLyr != nullptr && len > 0) {
            nextLyr->movePosX(-(toX - fromX));
        }
    }

    // apply yoffset for staff type change (keeps lyrics lines aligned with lyrics)
    if (item->staffType()) {
        item->movePosY(item->staffType()->yoffset().val() * item->spatium());
    }

    // set bounding box
    RectF r = RectF(0.0, 0.0, item->pos2().x(), item->pos2().y()).normalized();
    double lw = item->lyricsLine()->lineWidth() * .5;
    item->setbbox(r.adjusted(-lw, -lw, lw, lw));
    if (item->system() && lyr->addToSkyline()) {
        item->system()->staff(lyr->staffIdx())->skyline().add(item->shape().translate(item->pos()));
    }
}

//---------------------------------------------------------
//   findLyricsMaxY
//---------------------------------------------------------

static double findLyricsMaxY(const MStyle& style, Segment& s, staff_idx_t staffIdx)
{
    double yMax = 0.0;
    if (!s.isChordRestType()) {
        return yMax;
    }

    double lyricsMinTopDistance = style.styleMM(Sid::lyricsMinTopDistance);

    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
        ChordRest* cr = s.cr(staffIdx * VOICES + voice);
        if (cr && !cr->lyrics().empty()) {
            SkylineLine sk(true);

            for (Lyrics* l : cr->lyrics()) {
                if (l->autoplace() && l->placeBelow()) {
                    double yOff = l->offset().y();
                    PointF offset = l->pos() + cr->pos() + s.pos() + s.measure()->pos();
                    RectF r = l->bbox().translated(offset);
                    r.translate(0.0, -yOff);
                    sk.add(r.x(), r.top(), r.width());
                }
            }
            SysStaff* ss = s.measure()->system()->staff(staffIdx);
            for (Lyrics* l : cr->lyrics()) {
                if (l->autoplace() && l->placeBelow()) {
                    double y = ss->skyline().south().minDistance(sk);
                    if (y > -lyricsMinTopDistance) {
                        yMax = std::max(yMax, y + lyricsMinTopDistance);
                    }
                }
            }
        }
    }
    return yMax;
}

//---------------------------------------------------------
//   findLyricsMinY
//---------------------------------------------------------

static double findLyricsMinY(const MStyle& style, Segment& s, staff_idx_t staffIdx)
{
    double yMin = 0.0;
    if (!s.isChordRestType()) {
        return yMin;
    }
    double lyricsMinTopDistance = style.styleMM(Sid::lyricsMinTopDistance);
    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
        ChordRest* cr = s.cr(staffIdx * VOICES + voice);
        if (cr && !cr->lyrics().empty()) {
            SkylineLine sk(false);

            for (Lyrics* l : cr->lyrics()) {
                if (l->autoplace() && l->placeAbove()) {
                    double yOff = l->offset().y();
                    RectF r = l->bbox().translated(l->pos() + cr->pos() + s.pos() + s.measure()->pos());
                    r.translate(0.0, -yOff);
                    sk.add(r.x(), r.bottom(), r.width());
                }
            }
            SysStaff* ss = s.measure()->system()->staff(staffIdx);
            for (Lyrics* l : cr->lyrics()) {
                if (l->autoplace() && l->placeAbove()) {
                    double y = sk.minDistance(ss->skyline().north());
                    if (y > -lyricsMinTopDistance) {
                        yMin = std::min(yMin, -y - lyricsMinTopDistance);
                    }
                }
            }
        }
    }
    return yMin;
}

static double findLyricsMaxY(const MStyle& style, Measure* m, staff_idx_t staffIdx)
{
    double yMax = 0.0;
    for (Segment& s : m->segments()) {
        yMax = std::max(yMax, findLyricsMaxY(style, s, staffIdx));
    }
    return yMax;
}

static double findLyricsMinY(const MStyle& style, Measure* m, staff_idx_t staffIdx)
{
    double yMin = 0.0;
    for (Segment& s : m->segments()) {
        yMin = std::min(yMin, findLyricsMinY(style, s, staffIdx));
    }
    return yMin;
}

//---------------------------------------------------------
//   applyLyricsMax
//---------------------------------------------------------

static void applyLyricsMax(const MStyle& style, Segment& s, staff_idx_t staffIdx, double yMax)
{
    if (!s.isChordRestType()) {
        return;
    }
    Skyline& sk = s.measure()->system()->staff(staffIdx)->skyline();
    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
        ChordRest* cr = s.cr(staffIdx * VOICES + voice);
        if (cr && !cr->lyrics().empty()) {
            double lyricsMinBottomDistance = style.styleMM(Sid::lyricsMinBottomDistance);
            for (Lyrics* l : cr->lyrics()) {
                if (l->autoplace() && l->placeBelow()) {
                    l->movePosY(yMax - l->propertyDefault(Pid::OFFSET).value<PointF>().y());
                    if (l->addToSkyline()) {
                        PointF offset = l->pos() + cr->pos() + s.pos() + s.measure()->pos();
                        sk.add(l->bbox().translated(offset).adjusted(0.0, 0.0, 0.0, lyricsMinBottomDistance));
                    }
                }
            }
        }
    }
}

static void applyLyricsMax(const MStyle& style, Measure* m, staff_idx_t staffIdx, double yMax)
{
    for (Segment& s : m->segments()) {
        applyLyricsMax(style, s, staffIdx, yMax);
    }
}

//---------------------------------------------------------
//   applyLyricsMin
//---------------------------------------------------------

static void applyLyricsMin(ChordRest* cr, staff_idx_t staffIdx, double yMin)
{
    Skyline& sk = cr->measure()->system()->staff(staffIdx)->skyline();
    for (Lyrics* l : cr->lyrics()) {
        if (l->autoplace() && l->placeAbove()) {
            l->movePosY(yMin - l->propertyDefault(Pid::OFFSET).value<PointF>().y());
            if (l->addToSkyline()) {
                PointF offset = l->pos() + cr->pos() + cr->segment()->pos() + cr->segment()->measure()->pos();
                sk.add(l->bbox().translated(offset));
            }
        }
    }
}

static void applyLyricsMin(Measure* m, staff_idx_t staffIdx, double yMin)
{
    for (Segment& s : m->segments()) {
        if (s.isChordRestType()) {
            for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
                ChordRest* cr = s.cr(staffIdx * VOICES + voice);
                if (cr) {
                    applyLyricsMin(cr, staffIdx, yMin);
                }
            }
        }
    }
}

//---------------------------------------------------------
//   layoutLyrics
//
//    vertical align lyrics
//
//---------------------------------------------------------

void LyricsLayout::layoutLyrics(const LayoutOptions& options, const Score* score, System* system)
{
    std::vector<staff_idx_t> visibleStaves;
    for (staff_idx_t staffIdx = system->firstVisibleStaff(); staffIdx < score->nstaves();
         staffIdx = system->nextVisibleStaff(staffIdx)) {
        visibleStaves.push_back(staffIdx);
    }

    //int nAbove[nstaves()];
    std::vector<staff_idx_t> VnAbove(score->nstaves());

    for (staff_idx_t staffIdx : visibleStaves) {
        VnAbove[staffIdx] = 0;
        for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* m = toMeasure(mb);
            for (Segment& s : m->segments()) {
                if (s.isChordRestType()) {
                    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
                        ChordRest* cr = s.cr(staffIdx * VOICES + voice);
                        if (cr) {
                            staff_idx_t nA = 0;
                            for (Lyrics* l : cr->lyrics()) {
                                // user adjusted offset can possibly change placement
                                if (l->offsetChanged() != OffsetChange::NONE) {
                                    PlacementV p = l->placement();
                                    l->rebaseOffset();
                                    if (l->placement() != p) {
                                        l->undoResetProperty(Pid::AUTOPLACE);
                                        //l->undoResetProperty(Pid::OFFSET);
                                        //l->layout();
                                    }
                                }
                                l->setOffsetChanged(false);
                                if (l->placeAbove()) {
                                    ++nA;
                                }
                            }
                            VnAbove[staffIdx] = std::max(VnAbove[staffIdx], nA);
                        }
                    }
                }
            }
        }
    }

    for (staff_idx_t staffIdx : visibleStaves) {
        for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* m = toMeasure(mb);
            for (Segment& s : m->segments()) {
                if (s.isChordRestType()) {
                    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
                        ChordRest* cr = s.cr(staffIdx * VOICES + voice);
                        if (cr) {
                            for (Lyrics* l : cr->lyrics()) {
                                l->layout2(static_cast<int>(VnAbove[staffIdx]));
                            }
                        }
                    }
                }
            }
        }
    }

    switch (options.verticalAlignRange) {
    case VerticalAlignRange::MEASURE:
        for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* m = toMeasure(mb);
            for (staff_idx_t staffIdx : visibleStaves) {
                double yMax = findLyricsMaxY(score->style(), m, staffIdx);
                applyLyricsMax(score->style(), m, staffIdx, yMax);
            }
        }
        break;
    case VerticalAlignRange::SYSTEM:
        for (staff_idx_t staffIdx : visibleStaves) {
            double yMax = 0.0;
            double yMin = 0.0;
            for (MeasureBase* mb : system->measures()) {
                if (!mb->isMeasure()) {
                    continue;
                }
                yMax = std::max<double>(yMax, findLyricsMaxY(score->style(), toMeasure(mb), staffIdx));
                yMin = std::min(yMin, findLyricsMinY(score->style(), toMeasure(mb), staffIdx));
            }
            for (MeasureBase* mb : system->measures()) {
                if (!mb->isMeasure()) {
                    continue;
                }
                applyLyricsMax(score->style(), toMeasure(mb), staffIdx, yMax);
                applyLyricsMin(toMeasure(mb), staffIdx, yMin);
            }
        }
        break;
    case VerticalAlignRange::SEGMENT:
        for (MeasureBase* mb : system->measures()) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* m = toMeasure(mb);
            for (staff_idx_t staffIdx : visibleStaves) {
                for (Segment& s : m->segments()) {
                    double yMax = findLyricsMaxY(score->style(), s, staffIdx);
                    applyLyricsMax(score->style(), s, staffIdx, yMax);
                }
            }
        }
        break;
    }
}
