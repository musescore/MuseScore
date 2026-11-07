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

#include "editslur.h"

#include "../dom/barline.h"
#include "../dom/chord.h"
#include "../dom/factory.h"
#include "../dom/measure.h"
#include "../dom/note.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/select.h"
#include "../dom/slur.h"
#include "../dom/utils.h"

using namespace mu::engraving;

std::vector<EditSlur::AddedSlur> EditSlur::addSlurs(Transaction& tx, Score* score, const Slur* slurTemplate)
{
    std::vector<AddedSlur> addedSlurs;
    const auto addSlurBetween = [&](EngravingItem* firstItem, EngravingItem* secondItem) {
        const AddedSlur added = addSlur(tx, score, firstItem, secondItem, slurTemplate);
        if (added.slur) {
            addedSlurs.push_back(added);
        }
    };

    ChordRest* firstChordRest = nullptr;
    ChordRest* secondChordRest = nullptr;
    const auto& sel = score->selection();
    auto el = sel.uniqueElements();

    if (sel.isRange()) {
        track_idx_t startTrack = staff2track(sel.staffStart());
        track_idx_t endTrack = staff2track(sel.staffEnd());
        for (track_idx_t track = startTrack; track < endTrack; ++track) {
            firstChordRest = nullptr;
            secondChordRest = nullptr;
            for (EngravingItem* e : el) {
                if (e->track() != track) {
                    continue;
                }
                if (e->isNote()) {
                    e = toNote(e)->chord();
                }
                if (!e->isChord()) {
                    continue;
                }
                ChordRest* cr = toChordRest(e);
                if (!firstChordRest || firstChordRest->tick() > cr->tick()) {
                    firstChordRest = cr;
                }
                if (!secondChordRest || secondChordRest->tick() < cr->tick()) {
                    secondChordRest = cr;
                }
            }

            bool firstCrTrill = firstChordRest && firstChordRest->isChord() && toChord(firstChordRest)->isTrillCueNote();
            bool secondCrTrill = secondChordRest && secondChordRest->isChord() && toChord(secondChordRest)->isTrillCueNote();

            if (firstChordRest && (firstChordRest != secondChordRest)
                && !(firstCrTrill || secondCrTrill)) {
                addSlurBetween(firstChordRest, secondChordRest);
            }
        }
    } else if (sel.isSingle()) {
        if (sel.element()->isNote() && !toNote(sel.element())->isTrillCueNote()) {
            addSlurBetween(toNote(sel.element())->chord(), nullptr);
        } else if (sel.element()->isSlurSegment() && slurTemplate && slurTemplate->isHammerOnPullOff()) {
            Slur* slur = toSlurSegment(sel.element())->slur();
            addSlurBetween(slur->startElement(), slur->endElement());
        }
    } else {
        EngravingItem* firstItem = nullptr;
        EngravingItem* secondItem = nullptr;
        for (EngravingItem* e : el) {
            if (e->isNote()) {
                e = toNote(e)->chord();
            }

            Measure* meas = e->findMeasure();
            bool header = meas && meas->header() && e->tick() == meas->tick();

            if (!e->isChord() && !e->isBarLine() && !header) {
                continue;
            }

            if (!firstItem || e->isBefore(firstItem)) {
                firstItem = e;
            }
            if (!secondItem || secondItem->isBefore(e)) {
                secondItem = e;
            }
        }

        bool firstCrTrill = firstItem && firstItem->isChord() && toChord(firstItem)->isTrillCueNote();
        bool secondCrTrill = secondItem && secondItem->isChord() && toChord(secondItem)->isTrillCueNote();

        if (firstItem && firstItem == secondItem) {
            // A single chord is slurred as a single note is
            if (firstItem->isChord() && !firstCrTrill) {
                addSlurBetween(firstItem, nullptr);
            }
        } else if (firstItem && secondItem && (firstItem->isChordRest() || secondItem->isChordRest())
                   && !(firstCrTrill || secondCrTrill)) {
            addSlurBetween(firstItem, secondItem);
        }
    }

    return addedSlurs;
}

EditSlur::AddedSlur EditSlur::addSlur(Transaction&, Score* score, EngravingItem* firstItem, EngravingItem* secondItem,
                                      const Slur* slurTemplate)
{
    ChordRest* firstChordRest = nullptr;
    ChordRest* secondChordRest = nullptr;

    Measure* meas = firstItem ? firstItem->findMeasure() : nullptr;
    bool header = meas && meas->header() && firstItem->tick() == meas->tick() && !firstItem->isChordRest();

    if (firstItem && secondItem
        && ((firstItem->isBarLine() != secondItem->isBarLine()) || (header && secondItem->isChordRest()))) {
        const bool outgoing = firstItem->isChordRest();
        const EngravingItem* otherElement = outgoing ? secondItem : firstItem;
        ChordRest* cr = outgoing ? toChordRest(firstItem) : toChordRest(secondItem);

        // Check that cr and otherElement are part of the same repeat section
        Segment* seg1 = toSegment(firstItem->findAncestor(ElementType::SEGMENT));
        Segment* seg2 = toSegment(secondItem->findAncestor(ElementType::SEGMENT));

        bool differentRepeatSegments = segmentsAreInDifferentRepeatSegments(seg1, seg2);
        bool validBarline = isValidBarLineForRepeatSection(seg1, seg2);

        if (!cr || (!header && (differentRepeatSegments && !validBarline))) {
            return {};
        }

        Slur* partialSlur = slurTemplate ? slurTemplate->clone() : Factory::createSlur(score->dummy());
        if (outgoing) {
            partialSlur->undoSetOutgoing(true);
            firstChordRest = toChordRest(cr);
            const Measure* endMeas = otherElement->findMeasure();
            ChordRest* endCr = endMeas->lastChordRest(otherElement->track());
            secondChordRest = endCr;
        } else {
            partialSlur->undoSetIncoming(true);
            secondChordRest = toChordRest(cr);
            const Measure* startMeas = otherElement->findMeasure();
            if (otherElement->isBarLine()) {
                if (toBarLine(otherElement)->barLineType() == BarLineType::END_REPEAT) {
                    startMeas = startMeas->nextMeasure();
                }
            }
            ChordRest* startCr = startMeas->firstChordRest(otherElement->track());
            firstChordRest = startCr;
        }
        slurTemplate = partialSlur;
    } else {
        firstChordRest = toChordRest(firstItem);
        secondChordRest = toChordRest(secondItem);
    }

    const bool toNextChord = !secondItem;
    Slur* slur = firstChordRest->slur(secondChordRest);
    if (!slur || slur->slurDirection() != DirectionV::AUTO) {
        return { score->addSlur(firstChordRest, secondChordRest, slurTemplate), false, toNextChord };
    } else if (slurTemplate && slurTemplate->isHammerOnPullOff()) {
        // Replace existing slur with HOPO
        score->undoRemoveElement(slur);
        return { score->addSlur(firstChordRest, secondChordRest, slurTemplate), true, toNextChord };
    }

    return { slur, false, toNextChord };
}
