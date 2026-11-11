/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "editlyrics.h"

#include "containers.h"
#include "log.h"

#include "../dom/chord.h"
#include "../dom/factory.h"
#include "../dom/lyrics.h"
#include "../dom/measure.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/utils.h"

using namespace mu::engraving;

EditLyrics::InitialState EditLyrics::initialState(Lyrics* lyrics)
{
    InitialState state;
    state.lyrics = lyrics;
    state.chordRest = lyrics->chordRest();
    state.segment = lyrics->segment();
    state.track = lyrics->track();
    state.verse = lyrics->verse();
    state.placement = lyrics->placement();
    state.placementFlags = lyrics->propertyFlags(Pid::PLACEMENT);
    state.fontStyle = lyrics->fontStyle();
    state.fontStyleFlags = lyrics->propertyFlags(Pid::FONT_STYLE);
    state.centering = lyrics->centerBetweenStaves();
    state.styleType = lyrics->textStyleType();
    state.yOffset = lyrics->offset().y();
    state.hasPrecedingRepeat = state.chordRest->hasPrecedingJumpItem();
    state.hasFollowingRepeat = state.chordRest->hasFollowingJumpItem();
    state.prevPartialDash = findPrevPartialLyricsLineDash(lyrics);
    return state;
}

bool EditLyrics::canNavigateToNextSyllable(const InitialState& state)
{
    track_idx_t toLyricTrack = state.track;
    return findNextSyllableSegment(state, toLyricTrack) || state.hasFollowingRepeat || state.hasPrecedingRepeat;
}

Lyrics* EditLyrics::navigateToNextSyllable(Transaction&, Score* score, const InitialState& state)
{
    ChordRest* initialCR = state.chordRest;
    const track_idx_t track = state.track;
    const int verse = state.verse;
    const PlacementV placement = state.placement;
    PartialLyricsLine* prevPartialLyricsLine = state.prevPartialDash;

    track_idx_t toLyricTrack = track;
    Segment* nextSegment = findNextSyllableSegment(state, toLyricTrack);

    // look for the lyrics we are moving from; may be the current lyrics or a previous one
    // we are extending with several dashes
    Lyrics* fromLyrics = nullptr;
    Segment* segment = state.segment;
    const Segment* curSeg = segment;
    while (segment && segmentsAreAdjacent(segment, curSeg)) {
        ChordRest* cr = toChordRest(segment->element(track));
        if (!cr) {
            segment = segment->prev1(SegmentType::ChordRest);
            continue;
        }
        fromLyrics = cr->lyrics(verse, placement);
        if (fromLyrics) {
            break;
        }
        segment = segment->prev1(SegmentType::ChordRest);
    }

    if (!nextSegment) {
        if (fromLyrics && state.hasFollowingRepeat) {
            // Allow dash with no end syllable if there is a repeat
            switch (fromLyrics->syllabic()) {
            case LyricsSyllabic::BEGIN:
            case LyricsSyllabic::MIDDLE:
                break;
            case LyricsSyllabic::SINGLE:
                fromLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::BEGIN));
                break;
            case LyricsSyllabic::END:
                fromLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::MIDDLE));
                break;
            }
            fromLyrics->undoChangeProperty(Pid::LYRIC_TICKS, Fraction(0, 1));
            score->setLayoutAll();
            return nullptr;
        }

        if (state.hasPrecedingRepeat) {
            // No from lyrics - create incoming partial dash
            score->undoAddElement(createPartialLyricsLine(score, state, false, Fraction(0, 1)));

            // Don't advance cursor
            Lyrics* toLyrics = createSyllable(initialCR, state, state.styleType, LyricsSyllabic::END);
            score->undoAddElement(toLyrics);
            return toLyrics;
        }
    }

    ChordRest* cr = nextSegment ? toChordRest(nextSegment->element(toLyricTrack)) : nullptr;
    Lyrics* toLyrics = cr ? cr->lyrics(verse, placement) : nullptr;

    if (!toLyrics && !cr) {
        return nullptr;
    }

    // If no lyrics in current track, check others
    if (!toLyrics) {
        const staff_idx_t staffIdx = track2staff(track);
        for (track_idx_t t = staff2track(staffIdx); t < staff2track(staffIdx + 1); ++t) {
            if (t == track) {
                continue;
            }
            cr = toChordRest(nextSegment->element(t));
            if (cr) {
                toLyrics = cr->lyrics(verse, placement);
                if (toLyrics) {
                    break;
                }
            }
        }
    }

    // This will be a partial dash at the start of a measure after a repeat
    // We don't want toLyrics
    toLyrics = state.hasPrecedingRepeat && !fromLyrics ? nullptr : toLyrics;

    // Make sure we end up with either the cr of toLyrics or cr on correct track
    cr = !toLyrics ? toChordRest(nextSegment->element(track)) : cr;

    // Disallow dashes between non-adjacent repeat sections eg. 1st volta -> 2nd volta
    // Instead, try to add partial dashes
    if (cr && fromLyrics && isInNonAdjacentRepeatSection(cr, fromLyrics, track)) {
        fromLyrics = nullptr;
    }

    const bool startIncomingPartialDash = state.hasPrecedingRepeat && !fromLyrics
                                          && !prevPartialLyricsLine && state.lyrics->xmlText().empty();

    const bool newLyrics = !toLyrics || startIncomingPartialDash;
    if (newLyrics) {
        /* Don't advance the cursor when starting an incoming partial dash after a repeat,
         * i.e. when there is no adjacent preceding syllable to dash from: */
        ChordRest* toLyricsChord = startIncomingPartialDash ? initialCR : cr;
        toLyrics = createSyllable(toLyricsChord, state, state.styleType, LyricsSyllabic::END);
    } else {
        // as we arrived at toLyrics by a dash, it cannot be initial or isolated
        if (toLyrics->syllabic() == LyricsSyllabic::BEGIN) {
            toLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::MIDDLE));
        } else if (toLyrics->syllabic() == LyricsSyllabic::SINGLE) {
            toLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::END));
        }
    }

    if (fromLyrics) {
        // as we moved away from fromLyrics by a dash,
        // it can have syll. dashes before and after but cannot be isolated or terminal
        switch (fromLyrics->syllabic()) {
        case LyricsSyllabic::BEGIN:
        case LyricsSyllabic::MIDDLE:
            break;
        case LyricsSyllabic::SINGLE:
            fromLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::BEGIN));
            break;
        case LyricsSyllabic::END:
            fromLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::MIDDLE));
            break;
        }
        // for the same reason, it cannot have a melisma
        fromLyrics->undoChangeProperty(Pid::LYRIC_TICKS, Fraction(0, 1));
    } else if (state.hasPrecedingRepeat && !prevPartialLyricsLine) {
        // No from lyrics - create incoming partial dash
        score->undoAddElement(createPartialLyricsLine(score, state, false, Fraction(0, 1)));
    } else if (prevPartialLyricsLine) {
        const Fraction tickDiff = cr->tick() - prevPartialLyricsLine->tick2();
        prevPartialLyricsLine->undoMoveEnd(tickDiff);
        score->undoAddElement(prevPartialLyricsLine);
        prevPartialLyricsLine->triggerLayout();
    }

    if (newLyrics) {
        score->undoAddElement(toLyrics);
    }

    return toLyrics;
}

EditLyrics::MelismaEnds EditLyrics::addMelisma(Transaction&, Score* score, const InitialState& state)
{
    Lyrics* lyrics = state.lyrics;
    ChordRest* initialCR = state.chordRest;
    const track_idx_t track = state.track;
    const int verse = state.verse;
    const PlacementV placement = state.placement;
    Segment* segment = state.segment;
    const Fraction endTick = segment->tick(); // a previous melisma cannot extend beyond this point

    // search next chord
    Segment* nextSegment = segment;
    while ((nextSegment = nextSegment->next1(SegmentType::ChordRest))) {
        EngravingItem* el = nextSegment->element(track);
        if (el && el->isChord()) {
            break;
        }
    }

    if (!segmentsAreAdjacent(segment, nextSegment)) {
        nextSegment = nullptr;
    }

    // look for the lyrics we are moving from; may be the current lyrics or a previous one
    // we are extending with several underscores
    const Segment* curSeg = segment;
    Lyrics* fromLyrics = nullptr;
    PartialLyricsLine* prevPartialLyricsLine = nullptr;
    while (segment && segmentsAreAdjacent(segment, curSeg)) {
        ChordRest* cr = toChordRest(segment->element(track));
        if (cr) {
            fromLyrics = cr->lyrics(verse, placement);
            if (fromLyrics) {
                break;
            }
            // Check if there is a partial melisma to extend
            prevPartialLyricsLine = findPartialMelisma(score, cr, verse, placement);
            if (prevPartialLyricsLine) {
                break;
            }
        }
        segment = segment->prev1(SegmentType::ChordRest);
    }

    // one-chord melisma?
    // if still at melisma initial chord and there is a valid next chord (if not,
    // there will be no melisma anyway), set a temporary melisma duration
    if (fromLyrics == lyrics && nextSegment) {
        lyrics->undoChangeProperty(Pid::LYRIC_TICKS, Lyrics::TEMP_MELISMA_TICKS);
        score->setLayoutAll();
    }

    if (!nextSegment) {
        if (fromLyrics) {
            switch (fromLyrics->syllabic()) {
            case LyricsSyllabic::SINGLE:
            case LyricsSyllabic::END:
                break;
            default:
                fromLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::END));
                break;
            }
            if (fromLyrics->segment()->tick() < endTick
                || (endTick + initialCR->actualTicks() == segment->measure()->endTick() && initialCR->hasFollowingJumpItem())) {
                Fraction ticks = std::max(endTick - fromLyrics->segment()->tick(), Lyrics::TEMP_MELISMA_TICKS);
                fromLyrics->undoChangeProperty(Pid::LYRIC_TICKS, ticks);
            }
        }

        if (state.hasPrecedingRepeat && !prevPartialLyricsLine) {
            // No from lyrics - create incoming partial melisma
            score->undoAddElement(createPartialLyricsLine(score, state, true, initialCR->ticks()));
        } else if (prevPartialLyricsLine) {
            const Fraction tickDiff = partialMelismaEndTick(score, segment, track, endTick) - prevPartialLyricsLine->tick2();
            prevPartialLyricsLine->undoMoveEnd(tickDiff);
        }

        score->setLayoutAll();
        return { nullptr, false, fromLyrics };
    }

    // if a place for a new lyrics has been found, create a lyrics there
    ChordRest* nextCR = toChordRest(nextSegment->element(track));

    // Disallow melisma lines between non-adjacent repeat sections eg. 1st volta -> 2nd volta
    // Instead, try to add partial melisma line
    if (nextCR && fromLyrics && isInNonAdjacentRepeatSection(nextCR, fromLyrics, track)) {
        fromLyrics = nullptr;
    }

    Lyrics* toLyrics = nextCR->lyrics(verse, placement);
    const bool newLyrics = (toLyrics == nullptr);
    if (!toLyrics) {
        toLyrics = createSyllable(nextCR, state, std::nullopt, LyricsSyllabic::SINGLE);
    }
    // as we arrived at toLyrics by an underscore, it cannot have syllabic dashes before
    else if (toLyrics->syllabic() == LyricsSyllabic::MIDDLE) {
        toLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::BEGIN));
    } else if (toLyrics->syllabic() == LyricsSyllabic::END) {
        toLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::SINGLE));
    }

    if (fromLyrics) {
        // as we moved away from fromLyrics by an underscore,
        // it can be isolated or terminal but cannot have dashes after
        switch (fromLyrics->syllabic()) {
        case LyricsSyllabic::SINGLE:
        case LyricsSyllabic::END:
            break;
        default:
            fromLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::END));
            break;
        }
        // for the same reason, if it has a melisma, this cannot extend beyond toLyrics
        if (fromLyrics->segment()->tick() < endTick) {
            fromLyrics->undoChangeProperty(Pid::LYRIC_TICKS, endTick - fromLyrics->segment()->tick());
        }
    } else if (state.hasPrecedingRepeat && !prevPartialLyricsLine) {
        // No from lyrics - create incoming partial melisma
        score->undoAddElement(createPartialLyricsLine(score, state, true, initialCR->ticks()));
    } else if (prevPartialLyricsLine) {
        const Fraction tickDiff = partialMelismaEndTick(score, segment, track, endTick) - prevPartialLyricsLine->tick2();
        prevPartialLyricsLine->undoMoveEnd(tickDiff);
        prevPartialLyricsLine->triggerLayout();
    }

    return { toLyrics, newLyrics, fromLyrics };
}

Lyrics* EditLyrics::addVerse(Transaction&, Score* score, const InitialState& state)
{
    Lyrics* lyrics = createLyrics(state.chordRest, state, state.verse + 1, std::nullopt);
    score->undoAddElement(lyrics);
    return lyrics;
}

bool EditLyrics::canNavigateToLyrics(const InitialState& state, bool back)
{
    return findNextLyricsSegment(state, back);
}

EditLyrics::NavigatedLyrics EditLyrics::navigateToLyrics(Transaction&, Score* score, const InitialState& state, bool back,
                                                         bool moveOnly)
{
    const track_idx_t track = state.track;
    const int verse = state.verse;
    const PlacementV placement = state.placement;

    Segment* nextSegment = findNextLyricsSegment(state, back);
    if (!nextSegment) {
        return {};
    }

    // look for the lyrics we are moving from; may be the current lyrics or a previous one
    // if we are skipping several chords with spaces
    Lyrics* fromLyrics = nullptr;
    if (!back) {
        Segment* segment = state.segment;
        while (segment) {
            ChordRest* cr = toChordRest(segment->element(track));
            if (cr) {
                fromLyrics = cr->lyrics(verse, placement);
                if (fromLyrics) {
                    break;
                }
            }
            segment = segment->prev1(SegmentType::ChordRest);
        }
    }

    ChordRest* cr = toChordRest(nextSegment->element(track));
    if (!cr) {
        LOGD("no next lyrics list: %s", nextSegment->element(track)->typeName());
        return {};
    }

    Lyrics* nextLyrics = cr->lyrics(verse, placement);
    const bool newLyrics = !nextLyrics;
    if (newLyrics) {
        nextLyrics = createSyllable(cr, state, state.styleType, LyricsSyllabic::SINGLE);
    }

    if (fromLyrics && !moveOnly) {
        switch (nextLyrics->syllabic()) {
        // as we arrived at nextLyrics by a [Space], it can be the beginning
        // of a multi-syllable, but cannot have syllabic dashes before
        case LyricsSyllabic::SINGLE:
        case LyricsSyllabic::BEGIN:
            break;
        case LyricsSyllabic::END:
            nextLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::SINGLE));
            break;
        case LyricsSyllabic::MIDDLE:
            nextLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::BEGIN));
            break;
        }
        // as we moved away from fromLyrics by a [Space], it can be
        // the end of a multi-syllable, but cannot have syllabic dashes after
        switch (fromLyrics->syllabic()) {
        case LyricsSyllabic::SINGLE:
        case LyricsSyllabic::END:
            break;
        case LyricsSyllabic::BEGIN:
            fromLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::SINGLE));
            break;
        case LyricsSyllabic::MIDDLE:
            fromLyrics->undoChangeProperty(Pid::SYLLABIC, int(LyricsSyllabic::END));
            break;
        }
        // for the same reason, it cannot have a melisma
        if (fromLyrics->separator() && !fromLyrics->separator()->isEndMelisma()) {
            fromLyrics->undoChangeProperty(Pid::LYRIC_TICKS, Fraction(0, 1));
        }
    }

    if (newLyrics) {
        score->undoAddElement(nextLyrics);
    }

    return { nextLyrics, newLyrics };
}

Lyrics* EditLyrics::navigateToVerse(Transaction&, Score* score, const InitialState& state, int verse)
{
    Lyrics* lyrics = state.chordRest->lyrics(verse, state.placement);
    if (!lyrics) {
        lyrics = createLyrics(state.chordRest, state, verse, state.styleType);
        score->undoAddElement(lyrics);
    }
    return lyrics;
}

Segment* EditLyrics::findNextSyllableSegment(const InitialState& state, track_idx_t& toLyricTrack)
{
    const track_idx_t track = state.track;
    const staff_idx_t staffIdx = track2staff(track);

    Segment* nextSegment = state.segment;
    while ((nextSegment = nextSegment->next1(SegmentType::ChordRest))) {
        EngravingItem* el = nextSegment->element(track);
        if (!el || !el->isChord()) {
            for (track_idx_t t = staff2track(staffIdx); t < staff2track(staffIdx + 1); ++t) {
                el = nextSegment->element(t);
                if (el && el->isChord() && toChord(el)->lyrics(state.verse, state.placement)) {
                    toLyricTrack = t;
                    break;
                }
            }
        }

        if (el && el->isChord()) {
            break;
        }
    }

    return segmentsAreAdjacent(state.segment, nextSegment) ? nextSegment : nullptr;
}

Segment* EditLyrics::findNextLyricsSegment(const InitialState& state, bool back)
{
    Segment* nextSegment = state.segment;
    while ((nextSegment = back ? nextSegment->prev1(SegmentType::ChordRest) : nextSegment->next1(SegmentType::ChordRest))) {
        EngravingItem* el = nextSegment->element(state.track);
        if (!el) {
            continue;
        }
        if (el->isChord()) {
            break;
        } else if (el->isRest() && toChordRest(el)->lyrics(state.verse, state.placement)) {
            break;
        }
    }
    return nextSegment;
}

Lyrics* EditLyrics::createLyrics(ChordRest* cr, const InitialState& state, int verse, std::optional<TextStyleType> styleType)
{
    Lyrics* lyrics = Factory::createLyrics(cr);
    lyrics->setTrack(state.track);
    lyrics->setVerse(verse);
    lyrics->setTextStyleType(styleType ? *styleType : (lyrics->isEven() ? TextStyleType::LYRICS_EVEN : TextStyleType::LYRICS_ODD));

    lyrics->setPlacement(state.placement);
    lyrics->setPropertyFlags(Pid::PLACEMENT, state.placementFlags);
    lyrics->setCenterBetweenStaves(state.centering);
    lyrics->setFontStyle(state.fontStyle);
    lyrics->setPropertyFlags(Pid::FONT_STYLE, state.fontStyleFlags);
    return lyrics;
}

Lyrics* EditLyrics::createSyllable(ChordRest* cr, const InitialState& state, std::optional<TextStyleType> styleType,
                                   LyricsSyllabic syllabic)
{
    Lyrics* lyrics = createLyrics(cr, state, state.verse, styleType);
    lyrics->setSyllabic(syllabic);
    lyrics->setOffset(PointF(0.0, state.yOffset));
    return lyrics;
}

PartialLyricsLine* EditLyrics::createPartialLyricsLine(Score* score, const InitialState& state, bool isEndMelisma,
                                                       const Fraction& ticks)
{
    const ChordRest* initialCR = state.chordRest;

    PartialLyricsLine* line = Factory::createPartialLyricsLine(score->dummy());
    line->setIsEndMelisma(isEndMelisma);
    line->setVerse(state.verse);
    line->setPlacement(state.placement);
    line->setPropertyFlags(Pid::PLACEMENT, state.placementFlags);
    line->setCenterBetweenStaves(state.centering);
    line->setTick(initialCR->tick());
    line->setTicks(ticks);
    line->setTrack(initialCR->track());
    line->setTrack2(initialCR->track());
    return line;
}

PartialLyricsLine* EditLyrics::findPartialMelisma(const Score* score, const ChordRest* cr, int verse, PlacementV placement)
{
    for (auto& spanner : score->spannerMap().findOverlapping(cr->tick().ticks(), cr->endTick().ticks())) {
        if (!spanner.value->isPartialLyricsLine() || spanner.value->staffIdx() != cr->staffIdx()) {
            continue;
        }
        PartialLyricsLine* lyricsLine = toPartialLyricsLine(spanner.value);
        if (lyricsLine->verse() != verse || lyricsLine->placement() != placement || !lyricsLine->isEndMelisma()) {
            continue;
        }
        return lyricsLine;
    }
    return nullptr;
}

bool EditLyrics::isInNonAdjacentRepeatSection(const ChordRest* toCr, const Lyrics* fromLyrics, track_idx_t track)
{
    Measure* toLyricsMeasure = toCr->measure();
    Measure* fromLyricsMeasure = fromLyrics->measure();

    if (toLyricsMeasure == fromLyricsMeasure || !fromLyricsMeasure->lastChordRest(track)->hasFollowingJumpItem()) {
        return false;
    }

    const std::vector<Measure*> previousRepeats = findPreviousRepeatMeasures(toLyricsMeasure);
    return !previousRepeats.empty() && !muse::contains(previousRepeats, fromLyricsMeasure);
}

Fraction EditLyrics::partialMelismaEndTick(const Score* score, Segment* segment, track_idx_t track, const Fraction& endTick)
{
    Segment* endSegment = segment;
    while (endSegment && endSegment->tick() < endTick) {
        endSegment = endSegment->nextCR(track, true);
    }
    EngravingItem* endSegmentElement = endSegment->element(track);
    if (endSegment->tick() == endTick && endSegmentElement && endSegmentElement->isChord()) {
        Segment* endChordSeg = endSegment;
        Chord* endChord = toChord(endSegmentElement);

        endSegment = endChordSeg->nextCR(track, false);

        if (!endSegment || endSegment->tick() > endChord->endTick()) {
            endSegment = endChordSeg;
            while (endSegment && endSegment->tick() < endChord->endTick()) {
                endSegment = endSegment->nextCR(muse::nidx, true);
            }
        }
    }
    return endSegment ? endSegment->tick() : score->endTick();
}
