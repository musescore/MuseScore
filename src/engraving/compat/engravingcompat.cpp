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
#include "engraving/dom/box.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pedal.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/staff.h"

using namespace mu::engraving;

namespace mu::engraving::compat {
void EngravingCompat::doPreLayoutCompatIfNeeded(MasterScore* score)
{
    int mscVersion = score->mscVersion();

    if (mscVersion < 460) {
        resetMarkerLeftFontSize(score);
        resetRestVerticalOffsets(score);
        adjustVBoxDistances(score);
    }

    if (mscVersion < 440) {
        correctPedalEndPoints(score);
        migrateDynamicPosOnVocalStaves(score);
        if (mscVersion >= 420) {
            undoStaffTextExcludeFromPart(score);
        }
    }
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

void EngravingCompat::resetRestVerticalOffsets(MasterScore* masterScore)
{
    for (Score* score : masterScore->scoreList()) {
        for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* measure = toMeasure(mb);
            for (staff_idx_t staff = 0; staff < score->nstaves(); ++staff) {
                if (!measure->hasVoices(staff)) {
                    continue;
                }
                track_idx_t startTrack = staff2track(staff);
                track_idx_t endTrack = startTrack + VOICES;
                for (Segment& seg : measure->segments()) {
                    if (!seg.isChordRestType()) {
                        continue;
                    }
                    for (track_idx_t track = startTrack; track < endTrack; ++track) {
                        EngravingItem* item = seg.element(track);
                        if (item && item->isRest()) {
                            item->resetProperty(Pid::OFFSET);
                        }
                    }
                }
            }
        }
    }
}

void EngravingCompat::adjustVBoxDistances(MasterScore* masterScore)
{
    for (Score* score : masterScore->scoreList()) {
        for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
            MeasureBase* nextmb = mb->next();
            if (mb->isVBoxBase()) {
                VBox* vbox = static_cast<VBox*>(mb);
                vbox->setProperty(Pid::PADDING_TO_NOTATION_ABOVE, Spatium()); // Because pre-4.6 these didn't exist
                vbox->setProperty(Pid::PADDING_TO_NOTATION_BELOW, Spatium());
                if (nextmb && nextmb->isVBoxBase()) {
                    VBox* first = static_cast<VBox*>(mb);
                    VBox* second = static_cast<VBox*>(nextmb);
                    first->setBottomGap(first->bottomGap() + second->topGap()); // Because pre-4.6 these used to be added
                }
            }
        }
    }
}

void EngravingCompat::doPostLayoutCompatIfNeeded(MasterScore* score)
{
    bool needRelayout = false;

    int mscVersion = score->mscVersion();

    if (mscVersion < 460) {
        needRelayout |= resetHookHeightSign(score);
    }

    if (mscVersion < 440) {
        needRelayout |= relayoutUserModifiedCrossStaffBeams(score);
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

bool EngravingCompat::resetHookHeightSign(MasterScore* masterScore)
{
    bool needRelayout = false;

    for (Score* score : masterScore->scoreList()) {
        for (auto pair : score->spanner()) {
            Spanner* spanner = pair.second;
            if (spanner->isTextLineBase()) {
                for (SpannerSegment* spannerSeg : spanner->spannerSegments()) {
                    TextLineBaseSegment* textLineSeg = static_cast<TextLineBaseSegment*>(spannerSeg);
                    if (textLineSeg->placeBelow()) {
                        if (!textLineSeg->isStyled(Pid::BEGIN_HOOK_HEIGHT)) {
                            Spatium beginHookHeight = textLineSeg->getProperty(Pid::BEGIN_HOOK_HEIGHT).value<Spatium>();
                            textLineSeg->setProperty(Pid::BEGIN_HOOK_HEIGHT, -beginHookHeight);
                            spanner->triggerLayout();
                            needRelayout = true;
                        }
                        if (!textLineSeg->isStyled(Pid::END_HOOK_HEIGHT)) {
                            Spatium endHookHeight = textLineSeg->getProperty(Pid::END_HOOK_HEIGHT).value<Spatium>();
                            textLineSeg->setProperty(Pid::END_HOOK_HEIGHT, -endHookHeight);
                            spanner->triggerLayout();
                            needRelayout = true;
                        }
                    }
                }
            }
        }
    }

    return needRelayout;
}
} // namespace mu::engraving::compat
