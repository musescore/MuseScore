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

#include "engravingcompat.h"

#include "dom/marker.h"
#include "dom/system.h"
#include "engraving/dom/beam.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pedal.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/staff.h"
#include "engraving/rendering/score/harmonylayout.h"

using namespace mu::engraving;

namespace mu::engraving::compat {
void EngravingCompat::doPreLayoutCompatIfNeeded(MasterScore* score)
{
    if (score->mscVersion() >= 440) {
        resetMarkerLeftFontSize(score);
        return;
    }

    correctPedalEndPoints(score);

    if (score->mscVersion() >= 420) {
        undoStaffTextExcludeFromPart(score);
    }

    migrateDynamicPosOnVocalStaves(score);
}

void EngravingCompat::correctPedalEndPoints(MasterScore* score)
{
    // Pedal lines ending with 45° hook used to be hacked to end before their actual end duration.
    // Hack is now removed, so we need to correct them to preserve engraving result. (M.S.)
    for (auto pair : score->spanner()) {
        Spanner* spanner = pair.second;
        if (spanner->isPedal() && toPedal(spanner)->endHookType() == HookType::HOOK_45) {
            ChordRest* endCR = score->findChordRestEndingBeforeTickInStaff(spanner->tick2(), track2staff(spanner->track()));
            if (endCR) {
                for (EngravingObject* item : spanner->linkList()) {
                    toSpanner(item)->setTick2(endCR->tick());
                }
            }
        }
    }
}

void EngravingCompat::undoStaffTextExcludeFromPart(MasterScore* masterScore)
{
    for (Score* score : masterScore->scoreList()) {
        for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
            if (!mb->isMeasure()) {
                continue;
            }
            for (Segment& segment : toMeasure(mb)->segments()) {
                if (!segment.isChordRestType()) {
                    continue;
                }
                for (EngravingItem* item : segment.annotations()) {
                    if (!item || !item->isStaffText()) {
                        continue;
                    }
                    if (item->excludeFromOtherParts()) {
                        item->undoChangeProperty(Pid::EXCLUDE_FROM_OTHER_PARTS, false);
                        for (EngravingObject* linkedItem : item->linkList()) {
                            if (linkedItem == item && !linkedItem->score()->isMaster()) {
                                toEngravingItem(item)->setAppearanceLinkedToMaster(false);
                            } else if (linkedItem != item) {
                                linkedItem->undoChangeProperty(Pid::VISIBLE, false);
                            }
                        }
                    }
                }
            }
        }
    }
}

void EngravingCompat::migrateDynamicPosOnVocalStaves(MasterScore* masterScore)
{
    auto migrateVoiceAssignmentAndPosition = [masterScore](EngravingItem* item) {
        if (item->voice() != 0) {
            item->setProperty(Pid::VOICE_ASSIGNMENT, VoiceAssignment::CURRENT_VOICE_ONLY);
        }
        // Migrate position on vocal staves (to match old default, which used to be below)
        Staff* staff = item->staff();
        Part* part = staff ? staff->part() : nullptr;
        Instrument* instrument = part ? part->instrument() : nullptr;
        const bool isVocalInstrument = instrument && instrument->isVocalInstrument();
        const bool directionIsDefault = item->getProperty(Pid::DIRECTION) == item->propertyDefault(Pid::DIRECTION);
        const PlacementV defaultPlacement = masterScore->style().styleV(item->getPropertyStyle(Pid::PLACEMENT)).value<PlacementV>();
        const bool defaultIsBelow = defaultPlacement == PlacementV::BELOW;

        if (isVocalInstrument && directionIsDefault && defaultIsBelow) {
            item->setProperty(Pid::DIRECTION, DirectionV::DOWN);
            item->setPropertyFlags(Pid::DIRECTION, PropertyFlags::UNSTYLED);
        }
    };

    for (Score* score : masterScore->scoreList()) {
        for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
            if (!mb->isMeasure()) {
                continue;
            }
            for (Segment& segment : toMeasure(mb)->segments()) {
                if (!segment.isChordRestType()) {
                    continue;
                }
                for (EngravingItem* item : segment.annotations()) {
                    if (item && item->hasVoiceAssignmentProperties()) {
                        migrateVoiceAssignmentAndPosition(item);
                    }
                }
            }
        }

        for (auto pair : score->spanner()) {
            Spanner* spanner = pair.second;
            if (spanner->isHairpin()) {
                migrateVoiceAssignmentAndPosition(spanner);
            }
        }
    }
}

void EngravingCompat::resetMarkerLeftFontSize(MasterScore* masterScore)
{
    // Reset the new incorrect 4.4.0 - 4.4.2 default size of 11 to the previous correct size of 18
    const double INCORRECT_DEFAULT_SIZE = 11.0;
    const double CORRECT_DEFAULT_SIZE = 18.0;
    bool needsAdjustMarkerSize = masterScore->mscoreVersion().contains(u"4.4") && masterScore->mscoreVersion() != u"4.4.3";
    if (!needsAdjustMarkerSize || masterScore->style().styleD(Sid::repeatLeftFontSize) != INCORRECT_DEFAULT_SIZE) {
        return;
    }
    masterScore->style().set(Sid::repeatLeftFontSize, CORRECT_DEFAULT_SIZE);

    for (Score* score : masterScore->scoreList()) {
        for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* meas = toMeasure(mb);
            for (EngravingItem* item : meas->el()) {
                if (!item->isMarker()) {
                    continue;
                }
                Marker* marker = toMarker(item);
                if (marker->textStyleType() != TextStyleType::REPEAT_LEFT || marker->size() != INCORRECT_DEFAULT_SIZE) {
                    continue;
                }
                marker->setSize(CORRECT_DEFAULT_SIZE);
            }
        }
    }
}

void EngravingCompat::doPostLayoutCompatIfNeeded(MasterScore* score)
{
    bool needRelayout = false;

    if (relayoutUserModifiedCrossStaffBeams(score)) {
        needRelayout = true;
    }

    if (migrateChordSymbolAlignment(score)) {
        needRelayout = true;
    }

    if (needRelayout) {
        score->update();
    }
}

bool EngravingCompat::relayoutUserModifiedCrossStaffBeams(MasterScore* score)
{
    if (score->mscVersion() >= 440) {
        return false;
    }
    bool found = false;

    auto findBeam = [&found](ChordRest* cr) {
        Beam* beam = cr->beam();
        if (beam && beam->userModified() && beam->cross() && beam->elements().front() == cr) {
            found = true;
            beam->triggerLayout();
        }
    };

    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment& seg : toMeasure(mb)->segments()) {
            if (!seg.isChordRestType()) {
                continue;
            }
            for (EngravingItem* item : seg.elist()) {
                if (!item) {
                    continue;
                }
                findBeam(toChordRest(item));
                if (item->isChord()) {
                    for (Chord* grace : toChord(item)->graceNotes()) {
                        findBeam(grace);
                    }
                }
            }
        }
    }

    return found;
}

bool EngravingCompat::migrateChordSymbolAlignment(MasterScore* score)
{
    if (score->mscVersion() >= 460) {
        return false;
    }
    // Use maxChordShiftAbove/Below with the old algorithm to decide which chord symbols to
    // Exclude from vertical alignment
    bool needsRelayout = false;
    const double maxShiftAbove = score->style().styleMM(Sid::maxChordShiftAbove);
    const double maxShiftBelow = score->style().styleMM(Sid::maxChordShiftBelow);
    const double maxFretShiftAbove = score->style().styleMM(Sid::maxFretShiftAbove);
    const double maxFretShiftBelow = score->style().styleMM(Sid::maxFretShiftBelow);
    for (System* sys : score->systems()) {
        // Get segment list for system
        std::vector<Segment*> sl;
        Measure* firstMeas = sys->firstMeasure();
        if (!firstMeas) {
            continue;
        }
        for (Segment* s = firstMeas->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
            if (s->system() != sys) {
                break;
            }
            if (s->annotations().empty()) {
                continue;
            }
            sl.push_back(s);
        }

        // Harmony
        needsRelayout |= rendering::score::HarmonyLayout::alignHarmonies(sl, true, maxShiftAbove, maxShiftBelow);
        // Fret diagrams
        needsRelayout |= rendering::score::HarmonyLayout::alignHarmonies(sl, false, maxFretShiftAbove, maxFretShiftBelow);
    }

    return needsRelayout;
}
} // namespace mu::engraving::compat
