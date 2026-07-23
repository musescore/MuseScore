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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "stavesharinglayout.h"

#include "measurelayout.h"
#include "systemheaderlayout.h"

#include "dom/chord.h"
#include "dom/factory.h"
#include "dom/measure.h"
#include "dom/note.h"
#include "dom/part.h"
#include "dom/rest.h"
#include "dom/score.h"
#include "dom/staff.h"
#include "dom/system.h"

using namespace mu::engraving;

namespace mu::engraving::rendering::score {
void StaveSharingLayout::updateStaveSharingForFullSystem(MeasureBase* firstMB, MeasureBase* lastMB, LayoutContext& ctx)
{
    StaveSharingContext ssctx(firstMB, lastMB, ctx);
    if (ssctx.crSegments.empty()) {
        return;
    }

    ssctx.updateForLastAdded = false;

    updateStaveSharing(ssctx);
}

void StaveSharingLayout::updateStaveSharingForLastAddedMeasure(System* system, LayoutContext& ctx)
{
    if (!system->last()->isMeasure()) {
        return;
    }

    StaveSharingContext ssctx(system->first(), system->last(), ctx);
    if (ssctx.crSegments.empty()) {
        return;
    }

    ssctx.updateForLastAdded = true;

    updateStaveSharing(ssctx);

    if (ssctx.trackMapChanged) {
        SystemHeaderLayout::updateSystemHeaderWidth(system, ctx);

        for (MeasureBase* mb = system->first(); mb && mb != system->last(); mb = mb->next()) {
            if (mb->isMeasure()) {
                // TODO: only relayout shared staves
                toMeasure(mb)->mutldata()->setNeedLayout(true);
                MeasureLayout::layoutMeasure(mb, ctx);
            }
        }
    }
}

void StaveSharingLayout::updateStaveSharing(StaveSharingContext& ctx)
{
    for (Part* p : ctx.layoutCtx.dom().parts()) {
        if (p->isSharedPart() && p->show() && toSharedPart(p)->enabled()) {
            SharedPart* sharedPart = toSharedPart(p);
            if (sharedPart->show() && sharedPart->enabled()) {
                ctx.curSharedPart = sharedPart;
                updateTrackMaps(ctx);
                updateNotation(ctx);
            }
        }
    }
}

void StaveSharingLayout::updateTrackMaps(StaveSharingContext& ctx)
{
    const SharedTrackMap& curTrackMap = ctx.curSharedPart->trackMapAtTick(ctx.sTick);

    SharedTrackMap trackMap = computeTrackMap(ctx);
    if (trackMap != curTrackMap) {
        ctx.trackMapChanged = true;
    }

    ctx.curSharedPart->removeMapsBetweenTicks(ctx.sTick, ctx.eTick);
    ctx.curSharedPart->setTrackMapAtTick(trackMap, ctx.sTick);
}

SharedTrackMap StaveSharingLayout::computeTrackMap(StaveSharingContext& ctx)
{
    SharedPart* p = ctx.curSharedPart;

    TrackGroup originTracks;
    originTracks.reserve(p->originParts().size());
    for (Part* op : p->originParts()) {
        originTracks.push_back(op->trackRange().startTrack);
    }

    std::unordered_set<track_idx_t> processedTracks;

    std::map<track_idx_t, TrackGroup> unisonsMap;
    for (size_t i = 0; i < originTracks.size(); ++i) {
        track_idx_t track1 = originTracks[i];
        if (muse::contains(processedTracks, track1)) {
            continue;
        }

        for (size_t j = i + 1; j < originTracks.size(); ++j) {
            track_idx_t track2 = originTracks[j];
            if (muse::contains(processedTracks, track2)) {
                continue;
            }
            if (isUnison(track1, track2, ctx)) {
                processedTracks.insert(track1);
                processedTracks.insert(track2);

                if (!muse::contains(unisonsMap, track1)) {
                    unisonsMap[track1] = { track1, track2 };
                } else {
                    unisonsMap.at(track1).push_back(track2);
                }
            }
        }
    }

    std::map<track_idx_t, TrackGroup> sameVoiceMap;
    for (size_t i = 0; i < originTracks.size(); ++i) {
        track_idx_t track1 = originTracks[i];
        if (muse::contains(processedTracks, track1)) {
            continue;
        }
        processedTracks.insert(track1);

        TrackGroup curTrackGroup = { track1 };
        std::unordered_set<Note*> localUnisonNotes;

        track_idx_t lastInThisVoice = track1;
        for (size_t j = i + 1; j < originTracks.size(); ++j) {
            track_idx_t track2 = originTracks[j];
            if (muse::contains(processedTracks, track2)) {
                continue;
            }
            if (canGoToSameVoice(lastInThisVoice, track2, ctx, curTrackGroup, localUnisonNotes)) {
                processedTracks.insert(track2);
                curTrackGroup.push_back(track2);
                lastInThisVoice = track2;
            } else if (curTrackGroup.size() > 1) {
                break;
            }
        }

        sameVoiceMap.emplace(track1, curTrackGroup);
    }

    SharedTrackMap trackMap;

    std::unordered_set<track_idx_t> assignedTracks;
    track_idx_t lastAssignedTrack = muse::nidx;
    track_idx_t curSharedTrack = p->trackRange().startTrack;

    for (track_idx_t track : originTracks) {
        if (muse::contains(assignedTracks, track)) {
            continue;
        }

        bool canGoOnThisStave = track2voice(curSharedTrack) == 0
                                || lastAssignedTrack == muse::nidx
                                || canGoToSameStave(lastAssignedTrack, track, ctx);
        if (!canGoOnThisStave) {
            curSharedTrack = trackZeroVoice(curSharedTrack + VOICES);
        }

        if (muse::contains(unisonsMap, track)) {
            for (track_idx_t unisonTrack : unisonsMap[track]) {
                trackMap[unisonTrack] = curSharedTrack;
                assignedTracks.insert(unisonTrack);
                lastAssignedTrack = unisonTrack;
            }
        } else {
            for (track_idx_t sharedVoiceTrack : sameVoiceMap[track]) {
                trackMap[sharedVoiceTrack] = curSharedTrack;
                assignedTracks.insert(sharedVoiceTrack);
                lastAssignedTrack = sharedVoiceTrack;
            }
        }

        track2voice(curSharedTrack) == 0 ? ++curSharedTrack : curSharedTrack = trackZeroVoice(curSharedTrack + VOICES);
    }

    return trackMap;
}

bool StaveSharingLayout::isEmpty(track_idx_t track, StaveSharingContext& ctx)
{
    for (Segment* seg : ctx.crSegments) {
        ChordRest* cr = toChordRest(seg->element(track));
        if (cr && !(cr->isRest() && toRest(cr)->isFullMeasureRest())) {
            return false;
        }
    }

    return true;
}

bool StaveSharingLayout::isUnison(track_idx_t prevTrack, track_idx_t nextTrack, StaveSharingContext& ctx)
{
    for (Segment* segment : ctx.crSegments) {
        ChordRest* cr1 = toChordRest(segment->element(prevTrack));
        ChordRest* cr2 = toChordRest(segment->element(nextTrack));
        if (bool(cr1) != bool(cr2)) {
            return false;
        }

        if (!cr1) {
            continue;
        }

        if (cr1->isChord() != cr2->isChord()) {
            return false;
        }

        if (cr1->durationType() != cr2->durationType() || cr1->actualTicks() != cr2->actualTicks()) {
            return false;
        }

        if (!cr1->isChord()) {
            continue;
        }

        Chord* c1 = toChord(cr1);
        Chord* c2 = toChord(cr2);
        const std::vector<Note*> notes1 = c1->notes();
        const std::vector<Note*> notes2 = c2->notes();
        if (notes1.size() != notes2.size()) {
            return false;
        }

        for (size_t i = 0; i < notes1.size(); ++i) {
            Note* n1 = notes1[i];
            Note* n2 = notes2[i];

            if (!n1->isExactUnison(n2)) {
                return false;
            }

            if (!checkNoteSpannersForUnison(n1, n2)) {
                return false;
            }
        }

        if (!checkArticulationsForSameVoice(c1, c2)) {
            return false;
        }
    }

    for (Segment* segment : ctx.allSegments) {
        if (!checkAnnotationsForSameVoice(segment, prevTrack, nextTrack)) {
            return false;
        }
    }

    if (!checkSpannersForSameVoice(prevTrack, nextTrack, ctx)) {
        return false;
    }

    return true;
}

bool StaveSharingLayout::canGoToSameVoice(track_idx_t prevTrack, track_idx_t nextTrack, StaveSharingContext& ctx,
                                          const TrackGroup& curTrackGroup, std::unordered_set<Note*>& localUnisonNotes)
{
    std::vector<Note*> potentialUnisonNotes;

    for (Segment* segment : ctx.crSegments) {
        ChordRest* cr1 = toChordRest(segment->element(prevTrack));
        ChordRest* cr2 = toChordRest(segment->element(nextTrack));
        if (bool(cr1) != bool(cr2)) {
            return false;
        }

        if (!cr1) {
            continue;
        }

        if (cr1->isChord() != cr2->isChord()) {
            return false;
        }

        if (cr1->durationType() != cr2->durationType() || cr1->actualTicks() != cr2->actualTicks()) {
            return false;
        }

        if (!cr1->isChord()) {
            continue;
        }

        Chord* c1 = toChord(cr1);
        Chord* c2 = toChord(cr2);

        const std::vector<Note*>& notes1 = c1->notes();
        const std::vector<Note*>& notes2 = c2->notes();
        if (notes1.size() != notes2.size()) {
            return false;
        }

        for (size_t i = 0; i < notes1.size(); ++i) {
            Note* n1 = notes1[i];
            Note* n2 = notes2[i];

            if (n2->pitch() > n1->pitch()) {
                return false;
            }

            if (!n2->isExactUnison(n1)) {
                if (n2->pitch() == n1->pitch() || muse::contains(localUnisonNotes, n1)) {
                    return false;
                }

                continue;
            }

            if (!checkNoteSpannersForUnison(n1, n2)) {
                return false;
            }

            for (track_idx_t track : curTrackGroup) {
                if (track == prevTrack) {
                    continue;
                }
                if (ChordRest* cr = toChordRest(segment->element(track)); cr && cr->isChord()) {
                    for (Note* n : toChord(cr)->notes()) {
                        if (!n2->isExactUnison(n)) {
                            return false;
                        }
                    }
                }
            }

            potentialUnisonNotes.push_back(n2);
        }

        if (!checkArticulationsForSameVoice(c1, c2)) {
            return false;
        }
    }

    for (Segment* segment : ctx.allSegments) {
        if (!checkAnnotationsForSameVoice(segment, prevTrack, nextTrack)) {
            return false;
        }
    }

    if (!checkSpannersForSameVoice(prevTrack, nextTrack, ctx)) {
        return false;
    }

    for (Note* unisonNote : potentialUnisonNotes) {
        localUnisonNotes.insert(unisonNote);
    }

    return true;
}

bool StaveSharingLayout::checkAnnotationsForSameVoice(Segment* segment, track_idx_t prevTrack, track_idx_t nextTrack)
{
    std::multimap<ElementType, EngravingItem*> annotationsOnPrevTrack;
    std::multimap<ElementType, EngravingItem*> annotationsOnNextTrack;

    for (EngravingItem* annotation : segment->annotations()) {
        if (annotation->track() == prevTrack) {
            annotationsOnPrevTrack.insert({ annotation->type(), annotation });
        } else if (annotation->track() == nextTrack) {
            annotationsOnNextTrack.insert({ annotation->type(), annotation });
        }
    }

    if (annotationsOnPrevTrack.size() != annotationsOnNextTrack.size()) {
        return false;
    }

    for (auto [type, item] : annotationsOnPrevTrack) {
        auto range = annotationsOnNextTrack.equal_range(type);
        if (range.first == range.second) {
            // No annotation of this type exists in nextTrack
            return false;
        }

        if (muse::contains(TEXTBASE_TYPES, type)) {
            EngravingItem* matchingAnnotation = nullptr;
            for (auto i = range.first; i != range.second; ++i) {
                EngravingItem* nextItem = i->second;
                if (nextItem->type() == type && toTextBase(item)->xmlText() == toTextBase(nextItem)->xmlText()) {
                    matchingAnnotation = nextItem;
                    break;
                }
            }

            if (!matchingAnnotation) {
                return false;
            }
        }

        // TODO: other types will probably need other checks
    }

    return true;
}

bool StaveSharingLayout::checkNoteSpannersForUnison(const Note* note1, const Note* note2)
{
    const Tie* tieBack1 = note1->tieBack();
    const Tie* tieBack2 = note2->tieBack();
    if (bool(tieBack1) != bool(tieBack2)) {
        return false;
    }
    if (tieBack1 && bool(tieBack1->startElement()) != bool(tieBack2->startElement())) {
        return false;
    }

    const Tie* tieFor1 = note1->tieFor();
    const Tie* tieFor2 = note2->tieFor();
    if (bool(tieFor1) != bool(tieFor2)) {
        return false;
    }
    if (tieFor1 && bool(tieFor1->endElement()) != bool(tieFor2->endElement())) {
        return false;
    }

    const std::vector<Spanner*>& spannerBack1 = note1->spannerBack();
    const std::vector<Spanner*>& spannerBack2 = note2->spannerBack();
    if (spannerBack1.size() != spannerBack2.size()) {
        return false;
    }

    for (Spanner* sp1 : spannerBack1) {
        auto i = std::find_if(spannerBack2.begin(), spannerBack2.end(), [sp1](Spanner* sp2) { return sp2->type() == sp1->type(); });
        if (i == spannerBack2.end()) {
            return false;
        }

        Spanner* sp2 = *i;
        if (bool(sp1->startElement()) != bool(sp2->startElement())) {
            return false;
        }
    }

    const std::vector<Spanner*>& spannerFor1 = note1->spannerFor();
    const std::vector<Spanner*>& spannerFor2 = note2->spannerFor();
    if (spannerFor1.size() != spannerFor2.size()) {
        return false;
    }

    for (Spanner* sp1 : spannerFor1) {
        auto i = std::find_if(spannerFor2.begin(), spannerFor2.end(), [sp1](Spanner* sp2) { return sp2->type() == sp1->type(); });
        if (i == spannerFor2.end()) {
            return false;
        }

        Spanner* sp2 = *i;
        if (bool(sp1->endElement()) != bool(sp2->endElement())) {
            return false;
        }
    }

    return true;
}

bool StaveSharingLayout::checkSpannersForSameVoice(track_idx_t prevTrack, track_idx_t nextTrack, StaveSharingContext& ctx)
{
    std::multimap<ElementType, Spanner*> spannersOnPrevTrack;
    std::multimap<ElementType, Spanner*> spannersOnNextTrack;

    for (Spanner* spanner : ctx.overlappingSpanners) {
        if (spanner->track() == prevTrack) {
            spannersOnPrevTrack.insert({ spanner->type(), spanner });
        } else if (spanner->track() == nextTrack) {
            spannersOnNextTrack.insert({ spanner->type(), spanner });
        }
    }

    if (spannersOnPrevTrack.size() != spannersOnNextTrack.size()) {
        return false;
    }

    for (auto [type, spanner] : spannersOnPrevTrack) {
        auto range = spannersOnNextTrack.equal_range(type);
        if (range.first == range.second) {
            // No spanner of this type exists in nextTrack
            return false;
        }

        Spanner* matchingSpanner = nullptr;
        for (auto i = range.first; i != range.second; ++i) {
            Spanner* nextSpanner = i->second;
            if (nextSpanner->tick() == spanner->tick() && nextSpanner->ticks() == spanner->ticks()) {
                matchingSpanner = nextSpanner;
                break;
            }
        }

        if (!matchingSpanner) {
            return false;
        }
    }

    return true;
}

bool StaveSharingLayout::checkArticulationsForSameVoice(Chord* chord1, Chord* chord2)
{
    const std::vector<Articulation*>& articulations1 = chord1->articulations();
    const std::vector<Articulation*>& articulations2 = chord2->articulations();

    if (articulations1.size() != articulations2.size()) {
        return false;
    }

    for (size_t i = 0; i < articulations1.size(); ++i) {
        Articulation* art1 = articulations1[i];
        Articulation* art2 = articulations2[i];
        if (art1->subtype() != art2->subtype()) {
            return false;
        }
    }

    return true;
}

bool StaveSharingLayout::canGoToSameStave(track_idx_t prevTrack, track_idx_t nextTrack,
                                          StaveSharingContext& ctx)
{
    if (ctx.layoutCtx.conf().styleB(Sid::allowVoiceCrossing)) {
        return true;
    }

    for (Segment* segment : ctx.crSegments) {
        ChordRest* cr1 = toChordRest(segment->element(prevTrack));
        ChordRest* cr2 = toChordRest(segment->element(nextTrack));
        if (cr1 && cr2 && cr1->isChord() && cr2->isChord()) {
            Chord* c1 = toChord(cr1);
            Chord* c2 = toChord(cr2);
            if (c2->upNote()->pitch() > c1->downNote()->pitch()) {
                return false;
            }
        }
    }

    return true;
}

void StaveSharingLayout::updateNotation(StaveSharingContext& ctx)
{
    // 0. Compute segments to update (if possible update only last measure)
    computeSegmentsToUpdate(ctx);

    // 1. Disconnect the shared notation from the origin
    disconnectAll(ctx);

    // 2. Recompute shared notation and reconnect
    makeSharedNotation(ctx);

    // 3. Remove from shared notation any item that wasn't reconnected
    cleanup(ctx);
}

void StaveSharingLayout::computeSegmentsToUpdate(StaveSharingContext& ctx)
{
    if (!ctx.updateForLastAdded || ctx.trackMapChanged) {
        ctx.segmentsToUpdate = ctx.allSegments;
        ctx.crSegmentsToUpdate = ctx.crSegments;
        return;
    }

    Measure* lastMeasure = ctx.crSegments.back()->measure();
    for (Segment& seg : lastMeasure->segments()) {
        ctx.segmentsToUpdate.push_back(&seg);
        if (seg.isChordRestType()) {
            ctx.crSegmentsToUpdate.push_back(&seg);
        }
    }
}

void StaveSharingLayout::disconnectAll(StaveSharingContext& ctx)
{
    SharedPart* p = ctx.curSharedPart;

    const TrackRange range = p->trackRange();

    ctx.oldStaveSharingLabels.clear();
    ctx.updatedStaveSharingLabels.clear();

    for (Segment* seg : ctx.segmentsToUpdate) {
        for (EngravingItem* item : seg->annotations()) {
            if (!item->systemFlag() && item->track() >= range.startTrack && item->track() < range.endTrack) {
                EngravingItem::disconnectAllOriginItems(item);

                if (item->isStaveSharingLabel()) {
                    ctx.oldStaveSharingLabels.push_back(toStaveSharingLabel(item));
                }
            }
        }

        if (!seg->isChordRestType()) {
            continue;
        }

        for (track_idx_t track = range.startTrack; track < range.endTrack; ++track) {
            ChordRest* cr = toChordRest(seg->element(track));
            if (!cr) {
                continue;
            }

            for (DurationElement* de = cr; de;) {
                Tuplet* tuplet = de->tuplet();
                if (!tuplet) {
                    break;
                }

                if (de == tuplet->elements().front()) {
                    EngravingItem::disconnectAllOriginItems(tuplet);
                }
                de = tuplet;
            }

            if (cr->isRest()) {
                EngravingItem::disconnectAllOriginItems(cr);
            }

            if (cr->isChord()) {
                for (Note* note : toChord(cr)->notes()) {
                    EngravingItem::disconnectAllOriginItems(note);

                    if (Tie* tieFor = note->tieFor(); tieFor && !tieFor->endNote()) { // if it does have endNote it will be disconnected when we process that note
                        EngravingItem::disconnectAllOriginItems(tieFor);
                    }

                    for (Spanner* spannerFor : note->spannerFor()) {
                        if (!spannerFor->endElement()) {
                            EngravingItem::disconnectAllOriginItems(spannerFor); // if it does have endElement it will be disconnected when we process that note
                        }
                    }

                    if (Tie* tieBack = note->tieBack()) {
                        EngravingItem::disconnectAllOriginItems(tieBack);
                    }

                    for (Spanner* spannerBack : note->spannerBack()) {
                        EngravingItem::disconnectAllOriginItems(spannerBack);
                    }
                }

                for (Articulation* art : toChord(cr)->articulations()) {
                    EngravingItem::disconnectAllOriginItems(art);
                }
            }
        }
    }

    for (Spanner* spanner : ctx.overlappingSpanners) {
        if (!spanner->systemFlag() && spanner->track() >= range.startTrack && spanner->track() < range.endTrack) {
            EngravingItem::disconnectAllOriginItems(spanner);
        }
    }
}

void StaveSharingLayout::makeSharedNotation(StaveSharingContext& ctx)
{
    makeSharedChordRests(ctx);
    makeSharedAnnotations(ctx);
    makeSharedSpanners(ctx);
    makeStaveSharingLabels(ctx);
}

void StaveSharingLayout::makeSharedChordRests(StaveSharingContext& ctx)
{
    Score* score = ctx.score;

    const SharedTrackMap& trackMap = ctx.curSharedPart->trackMapAtTick(ctx.sTick);

    ctx.sharedUnisonNotes.clear();

    for (Segment* seg : ctx.crSegmentsToUpdate) {
        for (const auto& [originTrack, sharedTrack] : trackMap) {
            ChordRest* originCR = toChordRest(seg->element(originTrack));
            if (!originCR) {
                continue;
            }

            ChordRest* sharedCR = toChordRest(seg->element(sharedTrack));
            if (sharedCR) {
                bool sameType = sharedCR->type() == originCR->type();
                bool sameDuration = sharedCR->durationType() == originCR->durationType()
                                    && sharedCR->actualTicks() == originCR->actualTicks();
                if (!(sameType && sameDuration)) {
                    score->undoRemoveElement(sharedCR);
                    sharedCR = nullptr;
                }
            }

            bool barRestInSecVoice = originCR->isRest() && toRest(originCR)->isFullMeasureRest() && track2voice(sharedTrack) != 0;
            if (barRestInSecVoice) {
                continue;
            }

            if (!sharedCR) {
                sharedCR = toChordRest(originCR->clone());
                sharedCR->setTrack(sharedTrack);
                sharedCR->setParent(seg);
                score->undoAddElement(sharedCR);
            }

            DurationElement* sharedDE = sharedCR;
            for (DurationElement* originDE = originCR; originDE;) {
                Tuplet* originTuplet = originDE->tuplet();
                if (!originTuplet) {
                    break;
                }

                Tuplet* sharedTuplet = nullptr;
                if (originDE == originTuplet->elements().front()) {
                    sharedTuplet = sharedDE->tuplet();
                    if (!sharedTuplet) {
                        sharedTuplet = toTuplet(originTuplet->clone());
                        sharedTuplet->setTrack(sharedTrack);
                        sharedTuplet->setParent(originTuplet->measure());
                        score->undoAddElement(sharedTuplet);

                        EngravingItem::connectSharedItem(sharedTuplet, originTuplet);
                    }
                } else {
                    sharedTuplet = toTuplet(originTuplet->sharedItem());
                }

                IF_ASSERT_FAILED(sharedTuplet) {
                    break;
                }

                sharedTuplet->add(sharedDE);

                originDE = originTuplet;
                sharedDE = sharedTuplet;
            }

            if (originCR->isRest()) {
                EngravingItem::connectSharedItem(sharedCR, originCR);
            }

            if (!originCR->isChord()) {
                continue;
            }

            Chord* originChord = toChord(originCR);
            Note* originNote = originChord->upNote();
            Chord* sharedChord = toChord(sharedCR);
            Note* sharedNote = nullptr;
            for (Note* n : sharedChord->notes()) {
                if (n->isExactUnison(originNote)) {
                    sharedNote = n;
                    break;
                }
            }

            if (!sharedNote) {
                sharedNote = originNote->clone();
                sharedNote->setTrack(sharedTrack);
                sharedNote->setParent(sharedChord);
                score->undoAddElement(sharedNote);
            }

            EngravingItem::connectSharedItem(sharedNote, originNote);

            if (sharedNote->originItems().size() > 1) {
                ctx.sharedUnisonNotes.insert(sharedNote);
            }

            makeSharedTiesAndNoteSpanners(originNote, sharedNote);

            makeSharedArticulations(originChord, sharedChord);
        }
    }
}

void StaveSharingLayout::makeSharedArticulations(Chord* originChord, Chord* sharedChord)
{
    Score* score = originChord->score();

    for (Articulation* originArt : originChord->articulations()) {
        Articulation* sharedArt = nullptr;
        for (Articulation* possibleSharedArt : sharedChord->articulations()) {
            if (possibleSharedArt->subtype() == originArt->subtype()) {
                sharedArt = possibleSharedArt;
                break;
            }
        }

        if (!sharedArt) {
            sharedArt = originArt->clone();
            sharedArt->setTrack(sharedChord->track());
            sharedArt->setParent(sharedChord);

            score->undoAddElement(sharedArt);
        }

        EngravingItem::connectSharedItem(sharedArt, originArt);
    }
}

void StaveSharingLayout::makeSharedTiesAndNoteSpanners(Note* originNote, Note* sharedNote)
{
    Score* score = originNote->score();

    Tie* tieBack = originNote->tieBack();
    if (tieBack) {
        Tie* sharedTieBack = sharedNote->tieBack();
        if (!sharedTieBack) {
            sharedTieBack = toTie(tieBack->clone());
            Note* sharedStartNote = tieBack->startNote() ? toNote(tieBack->startNote()->sharedItem()) : nullptr;
            sharedTieBack->setNoteSpan(sharedStartNote, sharedNote);

            score->undoAddElement(sharedTieBack);
        }

        EngravingItem::connectSharedItem(sharedTieBack, tieBack);
    }

    Tie* tieFor = originNote->tieFor();
    if (tieFor && !tieFor->endNote()) { // If it does have an end note we add it when we process the end note
        Tie* sharedTieFor = sharedNote->tieFor();
        if (!sharedTieFor) {
            sharedTieFor = toTie(tieFor->clone());
            sharedTieFor->setNoteSpan(sharedNote, nullptr);

            score->undoAddElement(sharedTieFor);
        }

        EngravingItem::connectSharedItem(sharedTieFor, tieFor);
    }

    const std::vector<Spanner*>& originSpannerBack = originNote->spannerBack();
    const std::vector<Spanner*>& sharedSpannerBack = sharedNote->spannerBack();

    for (Spanner* spanner : originSpannerBack) {
        Spanner* sharedSpanner = nullptr;
        for (Spanner* sp : sharedSpannerBack) {
            if (sp && sp->type() == spanner->type()) {
                sharedSpanner = sp;
                break;
            }
        }

        if (!sharedSpanner) {
            sharedSpanner = toSpanner(spanner->clone());
            Note* startNote = spanner->startElement() ? toNote(spanner->startElement()->sharedItem()) : nullptr;
            sharedSpanner->setNoteSpan(startNote, sharedNote);

            score->undoAddElement(sharedSpanner);
        }

        EngravingItem::connectSharedItem(sharedSpanner, spanner);
    }

    for (Spanner* spanner : originNote->spannerFor()) {
        if (spanner->endElement()) { // If it does have an end note we add it when we process the end note
            continue;
        }

        Spanner* sharedSpanner = nullptr;
        for (Spanner* sp : sharedNote->spannerFor()) {
            if (sp->type() == spanner->type()) {
                sharedSpanner = sp;
                break;
            }
        }

        if (!sharedSpanner) {
            sharedSpanner = toSpanner(spanner->clone());
            sharedSpanner->setTrack(sharedNote->track());
            sharedSpanner->setStartElement(sharedNote);

            score->undoAddElement(sharedSpanner);
        }

        EngravingItem::connectSharedItem(sharedSpanner, spanner);
    }
}

void StaveSharingLayout::makeSharedAnnotations(StaveSharingContext& ctx)
{
    Score* score = ctx.score;

    const SharedTrackMap& trackMap = ctx.curSharedPart->trackMapAtTick(ctx.sTick);
    track_idx_t startOriginTrack = trackMap.begin()->first;
    track_idx_t endOriginTrack = trackMap.rbegin()->first;

    std::vector<EngravingItem*> sharedAnnotations;

    for (Segment* seg : ctx.segmentsToUpdate) {
        std::vector<EngravingItem*> annotations = seg->annotations(); // Copy because we are about to add
        for (EngravingItem* originItem : annotations) {
            track_idx_t originTrack = originItem->track();
            if (originTrack < startOriginTrack || originTrack > endOriginTrack) {
                continue;
            }

            IF_ASSERT_FAILED(muse::contains(trackMap, originTrack)) {
                continue;
            }

            if (originItem->isFermata()) {
                EngravingItem* fermataOriginBaseItem = seg->element(originTrack);
                if (fermataOriginBaseItem && !fermataOriginBaseItem->sharedItem()) {
                    continue;
                }
            }

            track_idx_t sharedTrack = trackMap.at(originTrack);
            EngravingItem* sharedItem = nullptr;
            for (EngravingItem* item : seg->annotations()) {
                if (item->track() != sharedTrack || item->type() != originItem->type()) {
                    continue;
                }

                if (item->isTextBase() && toTextBase(item)->xmlText() != toTextBase(originItem)->xmlText()) {
                    continue;
                }

                sharedItem = item;
                break;
            }

            if (!sharedItem) {
                sharedItem = originItem->clone();
                sharedItem->setTrack(sharedTrack);
                sharedItem->setParent(seg);
                score->undoAddElement(sharedItem);
            }

            EngravingItem::connectSharedItem(sharedItem, originItem);

            sharedAnnotations.push_back(sharedItem);
        }
    }

    manageVoicePropertyAndTrackForSharedItems(sharedAnnotations, startOriginTrack, endOriginTrack, trackMap);
}

void StaveSharingLayout::makeSharedSpanners(StaveSharingContext& ctx)
{
    const SharedTrackMap& trackMap = ctx.curSharedPart->trackMapAtTick(ctx.sTick);
    track_idx_t startOriginTrack = trackMap.begin()->first;
    track_idx_t endOriginTrack = trackMap.rbegin()->first;

    std::vector<EngravingItem*> sharedSpanners;

    std::vector<Spanner*> overlappingSpanners = ctx.overlappingSpanners; // copy because we may add
    for (Spanner* spanner : overlappingSpanners) {
        if (spanner->track() < startOriginTrack || spanner->track() > endOriginTrack) {
            continue;
        }

        track_idx_t originTrack = spanner->track();
        IF_ASSERT_FAILED(muse::contains(trackMap, originTrack)) {
            continue;
        }

        track_idx_t sharedTrack = trackMap.at(originTrack);

        Spanner* sharedSpanner = nullptr;
        for (Spanner* possibleSharedSpanner : ctx.overlappingSpanners) {
            if (possibleSharedSpanner->track() == sharedTrack && possibleSharedSpanner->type() == spanner->type()
                && possibleSharedSpanner->tick() == spanner->tick() && possibleSharedSpanner->ticks() == spanner->ticks()) {
                sharedSpanner = possibleSharedSpanner;
                break;
            }
        }

        if (!sharedSpanner) {
            sharedSpanner = toSpanner(spanner->clone());
            sharedSpanner->setTrack(sharedTrack);
            sharedSpanner->setTrack2(sharedTrack);
            sharedSpanner->setTick(spanner->tick());
            sharedSpanner->setTicks(spanner->ticks());
            sharedSpanner->setStartElement(nullptr);
            sharedSpanner->setEndElement(nullptr);
            sharedSpanner->reset();

            ctx.score->undoAddElement(sharedSpanner);
            ctx.overlappingSpanners.push_back(sharedSpanner);
        }

        EngravingItem::connectSharedItem(sharedSpanner, spanner);

        sharedSpanners.push_back(sharedSpanner);
    }

    manageVoicePropertyAndTrackForSharedItems(sharedSpanners, startOriginTrack, endOriginTrack, trackMap);
}

void StaveSharingLayout::makeStaveSharingLabels(StaveSharingContext& ctx)
{
    const SharedTrackMap& trackMap = ctx.curSharedPart->trackMapAtTick(ctx.sTick);

    std::vector<EngravingItem*> updatedStaveSharingLabels;

    for (Note* unisonNote : ctx.sharedUnisonNotes) {
        if (!unisonNoteNeedsLabel(unisonNote)) {
            continue;
        }

        String text = formatUnisonLabel(unisonNote, trackMap, ctx);
        Segment* segment = unisonNote->chord()->segment();

        StaveSharingLabel* label = nullptr;
        for (EngravingItem* item : segment->annotations()) {
            if (item->isStaveSharingLabel() && item->track() == unisonNote->track() && toStaveSharingLabel(item)->xmlText() == text) {
                label = toStaveSharingLabel(item);
                break;
            }
        }

        if (!label) {
            label = Factory::createStaveSharingLabel(segment);
            label->setParent(segment);
            label->setTrack(unisonNote->track());
            label->setXmlText(text);

            ctx.score->undoAddElement(label, /*addToLinkedStaves*/ false);
        }

        updatedStaveSharingLabels.push_back(label);
    }

    const TrackRange curSharedPartTrackRange = ctx.curSharedPart->trackRange();
    manageVoicePropertyAndTrackForSharedItems(updatedStaveSharingLabels, curSharedPartTrackRange.startTrack,
                                              curSharedPartTrackRange.endTrack, trackMap);

    ctx.updatedStaveSharingLabels.reserve(updatedStaveSharingLabels.size());
    for (EngravingItem* el : updatedStaveSharingLabels) {
        ctx.updatedStaveSharingLabels.push_back(toStaveSharingLabel(el));
    }
}

bool StaveSharingLayout::unisonNoteNeedsLabel(Note* unisonNote)
{
    std::vector<track_idx_t> originTracksOfThisNote;
    for (EngravingItem* originNote : unisonNote->originItems()) {
        originTracksOfThisNote.push_back(originNote->track());
    }

    Note* prevNote = nullptr;
    track_idx_t curTrack = unisonNote->track();
    Segment* curSegment = unisonNote->chord()->segment();
    for (Segment* seg = curSegment->prev1(SegmentType::ChordRest); seg; seg = seg->prev1(SegmentType::ChordRest)) {
        if (EngravingItem* el = seg->element(curTrack); el && el->isChord()) {
            for (Note* note : toChord(el)->notes()) {
                if (!note->originItems().empty()) {
                    prevNote = note;
                    break;
                }
            }
        }
        if (prevNote) {
            break;
        }
    }

    if (!prevNote) {
        return true;
    }

    std::vector<track_idx_t> originTracksOfPrevNote;
    for (EngravingItem* originNote : prevNote->originItems()) {
        originTracksOfPrevNote.push_back(originNote->track());
    }

    return originTracksOfPrevNote != originTracksOfThisNote;
}

String StaveSharingLayout::formatUnisonLabel(Note* unisonNote, const SharedTrackMap& trackMap, const StaveSharingContext& ctx)
{
    String result;

    size_t originUnisonsCount = unisonNote->originItems().size();
    result += u"a " + String::number(originUnisonsCount);
    bool trailingDot = false; // TODO: style
    if (trailingDot) {
        result += '.';
    }

    track_idx_t curTrack = unisonNote->track();
    staff_idx_t curStaff = track2staff(curTrack);

    std::vector<track_idx_t> originTracks;
    originTracks.reserve(originUnisonsCount);
    std::vector<track_idx_t> tracksMappedToThisStave;
    tracksMappedToThisStave.reserve(originUnisonsCount);

    for (auto [originTrack, sharedTrack] : trackMap) {
        if (track2staff(sharedTrack) == curStaff) {
            tracksMappedToThisStave.push_back(originTrack);
        }
        if (sharedTrack == curTrack) {
            originTracks.push_back(originTrack);
        }
    }

    if (tracksMappedToThisStave.size() <= originUnisonsCount) {
        return result;
    }

    String prefix;
    for (track_idx_t originTrack : originTracks) {
        if (!prefix.empty()) {
            prefix += '.';
        }
        Instrument* originInstrument = ctx.score->staff(track2staff(originTrack))->part()->instrument();
        prefix += String::number(originInstrument->number());
    }
    prefix += ' ';

    result.prepend(prefix);

    return result;
}

void StaveSharingLayout::manageVoicePropertyAndTrackForSharedItems(const std::vector<EngravingItem*>& sharedItems,
                                                                   track_idx_t startOriginTrack, track_idx_t endOriginTrack,
                                                                   const SharedTrackMap& trackMap)
{
    for (EngravingItem* sharedItem : sharedItems) {
        bool refersToVoice2 = track2voice(sharedItem->track()) != 0;
        bool voice2isUsed = refersToVoice2;
        if (!voice2isUsed) {
            track_idx_t trackOfVoice2 = trackZeroVoice(sharedItem->track()) + 1;
            for (track_idx_t originTrack = startOriginTrack; originTrack <= endOriginTrack; originTrack += VOICES) {
                if (trackMap.contains(originTrack) && trackMap.at(originTrack) == trackOfVoice2) {
                    voice2isUsed = true;
                    break;
                }
            }
        }

        if (voice2isUsed) {
            if (sharedItem->hasVoiceAssignmentProperties()) {
                sharedItem->setProperty(Pid::VOICE_ASSIGNMENT, VoiceAssignment::CURRENT_VOICE_ONLY);
                sharedItem->setPlacementBasedOnVoiceAssignment(sharedItem->propertyDefault(Pid::DIRECTION).value<DirectionV>());
            } else {
                if (refersToVoice2) {
                    sharedItem->setPlacement(PlacementV::BELOW);
                } else {
                    sharedItem->setPlacement(PlacementV::ABOVE);
                }
            }
        } else {
            if (sharedItem->hasVoiceAssignmentProperties()) {
                sharedItem->setProperty(Pid::VOICE_ASSIGNMENT, VoiceAssignment::ALL_VOICE_IN_INSTRUMENT);
                sharedItem->setPlacementBasedOnVoiceAssignment(sharedItem->propertyDefault(Pid::DIRECTION).value<DirectionV>());
            } else {
                sharedItem->resetProperty(Pid::PLACEMENT);
            }
        }
    }
}

void StaveSharingLayout::cleanup(StaveSharingContext& ctx)
{
    SharedPart* p = ctx.curSharedPart;

    Score* score = ctx.score;

    const TrackRange range = p->trackRange();

    for (Segment* seg : ctx.segmentsToUpdate) {
        std::vector<EngravingItem*> annotations = seg->annotations(); // Copy because we may remove elements
        for (EngravingItem* item : annotations) {
            if (!item->systemFlag() && !item->isStaveSharingLabel() && item->track() >= range.startTrack && item->track() < range.endTrack
                && item->originItems().empty()) {
                score->undoRemoveElement(item);
            }
        }

        if (!seg->isChordRestType()) {
            continue;
        }

        for (track_idx_t track = range.startTrack; track < range.endTrack; ++track) {
            ChordRest* cr = toChordRest(seg->element(track));
            if (!cr) {
                continue;
            }

            for (Tuplet* tuplet = cr->tuplet(); tuplet; tuplet = tuplet->tuplet()) {
                if (tuplet->originItems().empty()) {
                    score->undoRemoveElement(tuplet);
                }
            }

            if (cr->isRest()) {
                bool barRestOnFirstVoice = toRest(cr)->isFullMeasureRest() && cr->voice() == 0;
                if (barRestOnFirstVoice) {
                    continue;
                }
                if (cr->originItems().empty()) {
                    score->undoRemoveElement(cr);
                }
            }

            if (cr->isChord()) {
                Chord* c = toChord(cr);

                std::vector<Articulation*> articulations = c->articulations(); // copy because may be removed
                for (Articulation* art : articulations) {
                    if (art->originItems().empty()) {
                        score->undoRemoveElement(art);
                    }
                }

                std::vector<Note*> notes = c->notes();     // copy because may be removed
                for (Note* note : notes) {
                    if (Tie* tieBack = note->tieBack(); tieBack && tieBack->originItems().empty()) {
                        score->undoRemoveElement(tieBack);
                    }

                    for (Spanner* sp : note->spannerBack()) {
                        if (sp->originItems().empty()) {
                            score->undoRemoveElement(sp);
                        }
                    }

                    if (Tie* tieFor = note->tieFor(); tieFor && !tieFor->endElement() && tieFor->originItems().empty()) {
                        score->undoRemoveElement(tieFor);
                    }

                    for (Spanner* sp : note->spannerFor()) {
                        if (!sp->endElement() && sp->originItems().empty()) {
                            score->undoRemoveElement(sp);
                        }
                    }

                    if (note->originItems().empty()) {
                        score->undoRemoveElement(note);
                    }
                }
                if (c->notes().empty()) {
                    score->undoRemoveElement(c);
                }
            }
        }
    }

    for (Spanner* spanner : ctx.overlappingSpanners) {
        if (!spanner->systemFlag() && spanner->track() >= range.startTrack && spanner->track() < range.endTrack
            && spanner->originItems().empty()) {
            score->undoRemoveElement(spanner);
        }
    }

    for (StaveSharingLabel* oldLabel : ctx.oldStaveSharingLabels) {
        if (!muse::contains(ctx.updatedStaveSharingLabels, oldLabel)) {
            ctx.score->undoRemoveElement(oldLabel);
        }
    }
}

StaveSharingLayout::StaveSharingContext::StaveSharingContext(MeasureBase* first, MeasureBase* last, LayoutContext& ctx)
    : layoutCtx(ctx)
{
    for (MeasureBase* mb = first; mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }

        for (Segment& seg : toMeasure(mb)->segments()) {
            allSegments.push_back(&seg);
            if (seg.isChordRestType()) {
                crSegments.push_back(&seg);
            }
        }

        if (mb == last) {
            break;
        }
    }

    if (crSegments.empty()) {
        return;
    }

    sTick = crSegments.front()->tick();
    eTick = crSegments.back()->measure()->endTick();
    score = crSegments.front()->score();

    auto spanners = score->spannerMap().findOverlapping(sTick.ticks(), eTick.ticks());
    for (auto& i : spanners) {
        overlappingSpanners.push_back(i.value);
    }
}
}
