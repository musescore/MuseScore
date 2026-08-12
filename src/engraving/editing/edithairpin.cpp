/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "edithairpin.h"

#include "../dom/anchors.h"
#include "../dom/chord.h"
#include "../dom/chordrest.h"
#include "../dom/dynamic.h"
#include "../dom/factory.h"
#include "../dom/hairpin.h"
#include "../dom/measure.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/select.h"

using namespace mu::engraving;

std::vector<Hairpin*> EditHairpin::addHairpins(Transaction& tx, Score* score, HairpinType type)
{
    std::vector<Hairpin*> hairpins;

    // add hairpin on each staff if possible
    const Selection& selection = score->selection();
    if (selection.isRange() && selection.staffStart() != selection.staffEnd() - 1) {
        for (staff_idx_t staffIdx = selection.staffStart(); staffIdx < selection.staffEnd(); ++staffIdx) {
            ChordRest* cr1 = selection.firstChordRest(staffIdx * VOICES);
            ChordRest* cr2 = selection.lastChordRest(staffIdx * VOICES);
            Hairpin* h = cr1 ? addHairpin(tx, score, type, cr1, cr2)
                         : addHairpin(tx, score, type, selection.tickStart(), selection.tickEnd(), staff2track(staffIdx));
            if (h) {
                hairpins.push_back(h);
            }
        }
    } else {
        // for single staff range selection, or single selection,
        // find start & end elements elements
        ChordRest* cr1 = nullptr;
        ChordRest* cr2 = nullptr;
        score->getSelectedStartEndChordRests(cr1, cr2);
        Hairpin* h = addHairpin(tx, score, type, cr1, cr2);
        if (h) {
            hairpins.push_back(h);
        }
    }

    for (Hairpin* hairpin : hairpins) {
        hairpin->setInitialTrackAndVoiceAssignment(hairpin->track(), /*curVoiceOnlyOverride*/ false);
    }

    return hairpins;
}

Hairpin* EditHairpin::addHairpin(Transaction& tx, Score* score, HairpinType type, ChordRest* cr1, ChordRest* cr2)
{
    if (!cr1) {
        return nullptr;
    }

    Hairpin* hairpin = Factory::createHairpin(score->dummy()->segment());
    hairpin->setHairpinType(type);
    if (type == HairpinType::CRESC_LINE) {
        hairpin->setBeginText(u"cresc.");
        hairpin->setContinueText(u"(cresc.)");
    } else if (type == HairpinType::DIM_LINE) {
        hairpin->setBeginText(u"dim.");
        hairpin->setContinueText(u"(dim.)");
    }

    addHairpin(tx, score, hairpin, cr1, cr2);

    return hairpin;
}

Hairpin* EditHairpin::addHairpin(Transaction&, Score* score, HairpinType type, Fraction sTick, Fraction eTick, track_idx_t track)
{
    Hairpin* hairpin = Factory::createHairpin(score->dummy()->segment());
    hairpin->setHairpinType(type);
    if (type == HairpinType::CRESC_LINE) {
        hairpin->setBeginText(u"cresc.");
        hairpin->setContinueText(u"(cresc.)");
    } else if (type == HairpinType::DIM_LINE) {
        hairpin->setBeginText(u"dim.");
        hairpin->setContinueText(u"(dim.)");
    }

    hairpin->setTrack(track);
    hairpin->setTick(sTick);
    hairpin->setTick2(eTick);

    score->undoAddElement(hairpin);

    return hairpin;
}

void EditHairpin::addHairpin(Transaction&, Score* score, Hairpin* hairpin, ChordRest* cr1, ChordRest* cr2)
{
    track_idx_t track = cr1->track();
    Fraction startTick = cr1->tick();

    Fraction endTick = startTick;
    if (score->selection().isRange()) {
        endTick = score->selection().tickEnd();
    } else {
        if (cr2 && cr2 != cr1) {
            endTick = cr2->endTick();
        } else {
            endTick = cr1->isChord() ? toChord(cr1)->endTickIncludingTied() : cr1->endTick();
        }
        const Segment* startSegment = cr2 ? cr2->segment() : cr1->segment();
        for (const Segment* segment = startSegment; segment && segment->tick() < endTick;
             segment = segment->next1(SegmentType::Duration)) {
            if (segment == startSegment) {
                continue;
            }
            if (segment->findAnnotation(ElementType::DYNAMIC, track, track)) {
                endTick = segment->tick();
                break;
            }
        }
    }

    hairpin->setTrack(track);
    hairpin->setTrack2(track);
    hairpin->setTick(startTick);
    hairpin->setTick2(endTick);

    score->undoAddElement(hairpin);
}

void EditHairpin::addHairpinToDynamic(Transaction&, Score* score, Hairpin* hairpin, Dynamic* dynamic)
{
    track_idx_t track = dynamic->track();
    hairpin->setTrack(track);
    hairpin->setTrack2(track);

    hairpin->setTick(dynamic->tick());

    Segment* dynamicSegment = dynamic->segment();

    Chord* startChord = nullptr;
    for (Segment* segment = dynamicSegment; segment; segment = segment->prev(SegmentType::ChordRest)) {
        EngravingItem* element = segment->element(track);
        if (element && element->isChord()) {
            startChord = toChord(element);
            break;
        }
    }

    Fraction endTick = startChord ? startChord->endTickIncludingTied() : dynamic->segment()->measure()->endTick();

    for (Segment* segment = dynamicSegment; segment && segment->tick() < endTick;
         segment = segment->next1(SegmentType::Duration)) {
        if (segment == dynamicSegment) {
            continue;
        }
        if (segment->findAnnotation(ElementType::DYNAMIC, track, track)) {
            endTick = segment->tick();
            break;
        }
    }

    hairpin->setTick2(endTick);

    hairpin->setVoiceAssignment(dynamic->voiceAssignment());

    score->undoAddElement(hairpin);
}

Hairpin* EditHairpin::addHairpinToDynamicOnGripDrag(Transaction&, Score* score, Dynamic* dynamic, bool isLeftGrip,
                                                    const muse::PointF& pos)
{
    const track_idx_t track = dynamic->track();
    staff_idx_t staffIndex = dynamic->staffIdx();
    Segment* seg = nullptr;
    constexpr double spacingFactor = 0.5;

    // Ensure time tick segments are created
    EditTimeTickAnchors::updateAnchors(dynamic);

    // Find segment of type ChordRest or TimeTick near cursor postion
    score->dragPosition(pos, &staffIndex, &seg, spacingFactor, /*allowTimeAnchor*/ true);

    const bool hasValidTick = seg && (isLeftGrip
                                      ? seg->tick() < dynamic->tick()
                                      : seg->tick() > dynamic->tick());
    if (!hasValidTick) {
        return nullptr;
    }

    Hairpin* hairpin = Factory::createHairpin(score->dummy()->segment());
    hairpin->setHairpinType(isLeftGrip ? HairpinType::DIM_HAIRPIN : HairpinType::CRESC_HAIRPIN);

    hairpin->setTrack(track);
    hairpin->setTrack2(track);

    if (isLeftGrip) {
        hairpin->setTick(seg->tick());
        hairpin->setTick2(dynamic->tick());
    } else {
        hairpin->setTick(dynamic->tick());
        hairpin->setTick2(seg->tick());
    }

    hairpin->setVoiceAssignment(dynamic->voiceAssignment());

    score->undoAddElement(hairpin);

    return hairpin;
}
