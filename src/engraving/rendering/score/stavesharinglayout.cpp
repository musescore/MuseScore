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
#include "dom/measure.h"
#include "dom/note.h"
#include "dom/part.h"
#include "dom/rest.h"
#include "dom/score.h"
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
                updateTrackMaps(sharedPart, ctx);
                updateNotation(sharedPart, ctx);
            }
        }
    }
}

void StaveSharingLayout::updateTrackMaps(SharedPart* p, StaveSharingContext& ctx)
{
    const SharedTrackMap& curTrackMap = p->trackMapAtTick(ctx.sTick);

    SharedTrackMap trackMap = computeTrackMap(p, ctx);
    if (trackMap != curTrackMap) {
        ctx.trackMapChanged = true;
    }

    p->removeMapsBetweenTicks(ctx.sTick, ctx.eTick);
    p->setTrackMapAtTick(trackMap, ctx.sTick);
}

SharedTrackMap StaveSharingLayout::computeTrackMap(SharedPart* p, StaveSharingContext& ctx)
{
    TrackGroup originTracks;
    originTracks.reserve(p->originParts().size());
    for (Part* op : p->originParts()) {
        originTracks.push_back(op->startTrack());
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
    track_idx_t curSharedTrack = p->startTrack();

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
            if (!notes1[i]->isExactUnison(notes2[i])) {
                return false;
            }
        }
    }

    for (Segment* segment : ctx.allSegments) {
        if (!checkAnnotationsForSameVoice(segment, prevTrack, nextTrack, ctx)) {
            return false;
        }
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
        for (Note* n1 : c1->notes()) {
            for (Note* n2 : c2->notes()) {
                if (n2->pitch() > n1->pitch()) {
                    return false;
                }

                if (!n2->isExactUnison(n1)) {
                    if (n2->pitch() == n1->pitch() || muse::contains(localUnisonNotes, n1)) {
                        return false;
                    }

                    continue;
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
        }
    }

    for (Segment* segment : ctx.allSegments) {
        if (!checkAnnotationsForSameVoice(segment, prevTrack, nextTrack, ctx)) {
            return false;
        }
    }

    for (Note* unisonNote : potentialUnisonNotes) {
        localUnisonNotes.insert(unisonNote);
    }

    return true;
}

bool StaveSharingLayout::checkAnnotationsForSameVoice(Segment* segment, track_idx_t prevTrack, track_idx_t nextTrack,
                                                      StaveSharingContext& ctx)
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

        for (auto i = range.first; i != range.second; ++i) {
            EngravingItem* nextItem = i->second;
            if (muse::contains(TEXTBASE_TYPES, type)) {
                if (toTextBase(item)->xmlText() != toTextBase(nextItem)->xmlText()) {
                    return false;
                }
            }
            // TODO: other types will probably need other checks
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

void StaveSharingLayout::updateNotation(SharedPart* p, StaveSharingContext& ctx)
{
    // 0. Compute segments to update (if possible update only last measure)
    computeSegmentsToUpdate(ctx);

    // 1. Disconnect the shared notation from the origin
    disconnectAll(p, ctx);

    // 2. Recompute shared notation and reconnect
    makeSharedNotation(p, ctx);

    // 3. Remove from shared notation any item that wasn't reconnected
    cleanup(p, ctx);
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

void StaveSharingLayout::disconnectAll(SharedPart* p, StaveSharingContext& ctx)
{
    track_idx_t startTrack = p->startTrack();
    track_idx_t endTrack = p->endTrack();

    for (Segment* seg : ctx.segmentsToUpdate) {
        for (EngravingItem* item : seg->annotations()) {
            if (item->track() >= startTrack && item->track() < endTrack) {
                EngravingItem::disconnectAllOriginItems(item);
            }
        }

        if (!seg->isChordRestType()) {
            continue;
        }

        for (track_idx_t track = p->startTrack(); track < p->endTrack(); ++track) {
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
                }
            }
        }
    }
}

void StaveSharingLayout::makeSharedNotation(SharedPart* p, StaveSharingContext& ctx)
{
    makeSharedChordRests(p, ctx);
    makeSharedAnnotations(p, ctx);
}

void StaveSharingLayout::makeSharedChordRests(SharedPart* p, StaveSharingContext& ctx)
{
    Score* score = ctx.score;

    const SharedTrackMap& trackMap = p->trackMapAtTick(ctx.sTick);

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
                if (n->pitch() == originNote->pitch()) {
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
        }
    }
}

void StaveSharingLayout::makeSharedAnnotations(SharedPart* p, StaveSharingContext& ctx)
{
    Score* score = ctx.score;

    const SharedTrackMap& trackMap = p->trackMapAtTick(ctx.sTick);
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

            track_idx_t sharedTrack = trackMap.at(originTrack);
            EngravingItem* sharedItem = seg->findAnnotation(originItem->type(), sharedTrack, sharedTrack);
            if (sharedItem && sharedItem->isTextBase()) {
                if (toTextBase(sharedItem)->xmlText() != toTextBase(originItem)->xmlText()) {
                    sharedItem = nullptr; // Not the one we are looking for
                }
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

    for (EngravingItem* sharedItem : sharedAnnotations) {
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

bool StaveSharingLayout::annotationRefersToBothVoices(EngravingItem* sharedAnnotation, StaveSharingContext& ctx)
{
}

void StaveSharingLayout::cleanup(SharedPart* p, StaveSharingContext& ctx)
{
    Score* score = ctx.score;

    track_idx_t startTrack = p->startTrack();
    track_idx_t endTrack = p->endTrack();

    for (Segment* seg : ctx.segmentsToUpdate) {
        std::vector<EngravingItem*> annotations = seg->annotations(); // Copy because we may remove elements
        for (EngravingItem* item : annotations) {
            if (item->track() >= startTrack && item->track() < endTrack) {
                if (item->originItems().empty()) {
                    score->undoRemoveElement(item);
                }
            }
        }

        if (!seg->isChordRestType()) {
            continue;
        }

        for (track_idx_t track = p->startTrack(); track < p->endTrack(); ++track) {
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
                std::vector<Note*> notes = c->notes();     // copy because may be removed
                for (Note* note : notes) {
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
}
}
