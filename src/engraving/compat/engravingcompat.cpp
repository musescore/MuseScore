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

#include "engraving/dom/beam.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/instrument.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pedal.h"
#include "engraving/dom/spanner.h"

using namespace mu::engraving;

namespace mu::engraving::compat {
void EngravingCompat::doPreLayoutCompatIfNeeded(MasterScore* score)
{
    if (score->mscVersion() >= 440) {
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
    for (Score* score : masterScore->scoreList()) {
        for (Part* part : score->parts()) {
            if (!part->instrument()->isVocalInstrument()) {
                continue;
            }
            for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
                if (!mb->isMeasure()) {
                    continue;
                }
                for (Segment& segment : toMeasure(mb)->segments()) {
                    if (!segment.isChordRestType()) {
                        continue;
                    }
                    for (EngravingItem* item : segment.annotations()) {
                        if (!item || !item->hasVoiceApplicationProperties()) {
                            continue;
                        }
                        if (item->getProperty(Pid::DIRECTION) == item->propertyDefault(Pid::DIRECTION)) {
                            item->setProperty(Pid::DIRECTION, DirectionV::DOWN);
                            item->setPropertyFlags(Pid::DIRECTION, PropertyFlags::UNSTYLED);
                        }
                    }
                }
            }
        }
        for (auto pair : score->spanner()) {
            Spanner* spanner = pair.second;
            if (!spanner->isHairpin()) {
                continue;
            }
            if (spanner->getProperty(Pid::DIRECTION) == spanner->propertyDefault(Pid::DIRECTION)) {
                spanner->setProperty(Pid::DIRECTION, DirectionV::DOWN);
                spanner->setPropertyFlags(Pid::DIRECTION, PropertyFlags::UNSTYLED);
            }
        }
    }
}

void EngravingCompat::doPostLayoutCompatIfNeeded(MasterScore* score)
{
    if (score->mscVersion() >= 440) {
        return;
    }

    bool needRelayout = false;

    if (relayoutUserModifiedCrossStaffBeams(score)) {
        needRelayout = true;
    }
    // As we progress, likely that more things will be done here

    if (needRelayout) {
        score->update();
    }
}

bool EngravingCompat::relayoutUserModifiedCrossStaffBeams(MasterScore* score)
{
    bool found = false;

    auto findBeam = [&found, score](ChordRest* cr) {
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
} // namespace mu::engraving::compat
