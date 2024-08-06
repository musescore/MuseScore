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
using namespace mu::engraving::rendering::stable;

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

    bool styleDidChange = false;
    if (item->isEven() && (item->textStyleType() != TextStyleType::LYRICS_EVEN)) {
        item->initTextStyleType(TextStyleType::LYRICS_EVEN, /* preserveDifferent */ true);
        styleDidChange = true;
    }
    if (!item->isEven() && (item->textStyleType() != TextStyleType::LYRICS_ODD)) {
        item->initTextStyleType(TextStyleType::LYRICS_ODD, /* preserveDifferent */ true);
        styleDidChange = true;
    }

    if (styleDidChange) {
        item->styleChanged();
    }

    ChordRest* cr = item->chordRest();
    if (item->needRemoveInvalidSegments()) {
        item->removeInvalidSegments();
    }
    if (item->ticks() > Fraction(0, 1) || item->syllabic() == LyricsSyllabic::BEGIN || item->syllabic() == LyricsSyllabic::MIDDLE) {
        if (!item->separator()) {
            LyricsLine* separator = new LyricsLine(ctx.mutDom().dummyParent());
            separator->setTick(cr->tick());
            item->setSeparator(separator);
            ctx.mutDom().addUnmanagedSpanner(item->separator());
        }
        item->separator()->setParent(item);
        item->separator()->setTick(cr->tick());
        // HACK separator should have non-zero length to get its layout
        // always triggered. A proper ticks length will be set later on the
        // separator layout.
        item->separator()->setTicks(Fraction::fromTicks(1));
        item->separator()->setTrack(item->track());
        item->separator()->setTrack2(item->track());
        item->separator()->setVisible(item->visible());
        // bbox().setWidth(bbox().width());  // ??
    } else {
        if (item->separator()) {
            item->separator()->removeUnmanaged();
            delete item->separator();
            item->setSeparator(nullptr);
        }
    }

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
    double x = o.x() - cr->x();

    TLayout::layoutBaseTextBase1(item, ctx);

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
    bool tempMelismaTicks = (item->lyrics()->ticks() == Lyrics::TEMP_MELISMA_TICKS);
    if (item->isEndMelisma()) {           // melisma
        item->setLineWidth(ctx.conf().styleMM(Sid::lyricsLineThickness));
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
        const Segment* s = lyricsSegment;
        while (s && s->tick() < lyricsEndTick) {
            s = s->nextCR(lyricsTrack, true);
        }
        if (!s) {
            // user probably deleted measures at end of score, leaving this melisma too long
            // set s to last segment and reset lyricsEndTick to trigger FIXUP code below
            s = ctx.dom().lastSegment();
            lyricsEndTick = Fraction(-1, 1);
        }
        EngravingItem* se = s->element(lyricsTrack);
        // everything is OK if we have reached a chord at right tick on right track
        if (s->tick() == lyricsEndTick && se && se->type() == ElementType::CHORD) {
            // advance to next CR, or last segment if no next CR
            s = s->nextCR(lyricsTrack, true);
            if (!s) {
                s = ctx.dom().lastSegment();
            }
        } else {
            // FIXUP - lyrics tick count not valid
            // this happens if edits to score have removed the original end segment
            // so let's fix it here
            // s is already pointing to segment past endTick (or to last segment)
            // we should shorten the lyrics tick count to make this work
            const Segment* ns = s;
            const Segment* ps = s->prev1(SegmentType::ChordRest);
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
                // either there is no valid previous CR, or the previous CR is the one the lyric starts on
                // we don't want to make the melisma longer arbitrarily, but there is a possibility that the next
                // CR won't extend the melisma, so let's check it
                ps = ns;
                s = ps->nextCR(lyricsTrack, true);
                EngravingItem* e = s ? s->element(lyricsTrack) : nullptr;
                // check to make sure we have a chord
                if (!e || e->type() != ElementType::CHORD || ps->tick() > item->tick() + item->ticks()) {
                    // nope, nothing to do but set ticks to 0
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
        item->setNextLyrics(searchNextLyrics(item->lyrics()->segment(),
                                             item->staffIdx(),
                                             item->lyrics()->no(),
                                             item->lyrics()->placement()
                                             ));

        item->setTick2(item->nextLyrics() ? item->nextLyrics()->segment()->tick() : item->tick());
    }
    if (item->ticks().isNotZero()) {                  // only do layout if some time span
        // do layout with non-0 duration
        if (tempMelismaTicks) {
            item->lyrics()->setTicks(Lyrics::TEMP_MELISMA_TICKS);
        }
    }
}
