/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <vector>

#include "dom/beam.h"
#include "dom/box.h"
#include "dom/chord.h"
#include "dom/hammeronpulloff.h"
#include "dom/harmony.h"
#include "dom/instrument.h"
#include "dom/lyrics.h"
#include "dom/marker.h"
#include "dom/masterscore.h"
#include "dom/measure.h"
#include "dom/note.h"
#include "dom/part.h"
#include "dom/pedal.h"
#include "dom/spanner.h"
#include "dom/staff.h"
#include "dom/system.h"
#include "dom/tapping.h"
#include "editing/editchord.h"
#include "rw/compat/compatutils.h"

using namespace mu::engraving;

namespace mu::engraving::compat {
void EngravingCompat::doPreLayoutCompatIfNeeded(MasterScore* score)
{
    int mscVersion = score->mscVersion();

    if (mscVersion < 470) {
        pre470TextCompat(score);
        migrateNoteParens(score);
    }

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
                vbox->setPropertyFlags(Pid::PADDING_TO_NOTATION_ABOVE, PropertyFlags::UNSTYLED);
                vbox->setProperty(Pid::PADDING_TO_NOTATION_BELOW, Spatium());
                vbox->setPropertyFlags(Pid::PADDING_TO_NOTATION_BELOW, PropertyFlags::UNSTYLED);
                if (nextmb && nextmb->isVBoxBase()) {
                    VBox* first = static_cast<VBox*>(mb);
                    VBox* second = static_cast<VBox*>(nextmb);
                    if (first->bottomGap() > 0_sp && second->topGap() > 0_sp) {
                        first->setProperty(Pid::BOTTOM_GAP, first->bottomGap() + second->topGap()); // Because pre-4.6 these used to be added
                        first->setPropertyFlags(Pid::BOTTOM_GAP, PropertyFlags::UNSTYLED);
                    }
                }
            }
        }
    }
}

static constexpr std::array<ElementType, 6> OFFSET_UNIT_CONVERT_TYPES = {
    ElementType::FINGERING,
    ElementType::HAMMER_ON_PULL_OFF_TEXT,
    ElementType::LYRICS,
    ElementType::TAPPING
};

void EngravingCompat::pre470TextCompat(MasterScore* masterScore)
{
    auto doCompat = [](EngravingItem* item) {
        if (!item->isTextBase()) {
            return;
        }

        TextBase* text = toTextBase(item);

        if (!text->isStyled(Pid::FRAME_ROUND)) {
            text->setFrameRound(compat::CompatUtils::convertPre470FrameRadius(text->frameRound().val()));
        }

        // Text offset could have been in mm for these types prior to 4.7
        // Convert actual distance to spatium
        if (muse::contains(OFFSET_UNIT_CONVERT_TYPES, item->type()) && !item->sizeIsSpatiumDependent()) {
            PointF offset = item->offset();
            double spatium = item->style().spatium();
            offset /= spatium;
            offset *= DPMM;

            item->setProperty(Pid::OFFSET, offset);
        }

        // Staff text, system text, and harp pedal diagrams are the only types which are attached to notes and weren't already
        // placed with their left edges / centres / right edges aligned to the left / centre / right of the notehead
        if (text->positionRelativeToNoteheadRest()
            && (text->isStaffText() || text->isSystemText() || text->isHarpPedalDiagram())) {
            double mag = item->staff() ? item->staff()->staffMag(item) : 1.0;
            double xAdj = item->symWidth(SymId::noteheadBlack) * mag;

            switch (text->position()) {
            case AlignH::HCENTER:
                text->setProperty(Pid::OFFSET, PointF(text->offset().x() - xAdj / 2, text->offset().y()));
                break;
            case AlignH::RIGHT:
                text->setProperty(Pid::OFFSET, PointF(text->offset().x() - xAdj, text->offset().y()));
                break;
            default:
                break;
            }
        }
    };

    for (Score* score : masterScore->scoreList()) {
        score->scanElements(doCompat);
    }
}

void EngravingCompat::migrateNoteParens(MasterScore* masterScore)
{
    for (Score* score : masterScore->scoreList()) {
        score->scanElements(CompatUtils::doMigrateNoteParens);
    }
}

static void doMigrateOffset500(EngravingItem* item)
{
    if (item->offset().isNull()
        || (!item->isTextBase() && !item->isSpanner() && !item->isSpannerSegment()) || !item->hasVoiceAssignmentProperties()) {
        return;
    }

    // In versions <5 any adjustment to offset meant we couldn't centre items between staves
    item->setProperty(Pid::CENTER_BETWEEN_STAVES, AutoOnOff::OFF);
    if (item->isStyled(Pid::CENTER_BETWEEN_STAVES)) {
        item->setPropertyFlags(Pid::CENTER_BETWEEN_STAVES, PropertyFlags::UNSTYLED);
    }

    item->setProperty(Pid::OFFSET, CompatUtils::getAdjustedOffset(item, item->offset()));
}

void EngravingCompat::migrateOffset500(MasterScore* masterScore)
{
    for (Score* score : masterScore->scoreList()) {
        for (Spanner* sp : score->spannerList()) {
            if (!sp->hasVoiceAssignmentProperties()) {
                continue;
            }
            sp->setPlacementBasedOnVoiceAssignment(sp->style().styleV(Sid::dynamicsHairpinVoiceBasedPlacement).value<DirectionV>());
            doMigrateOffset500(sp);
            for (SpannerSegment* seg : sp->spannerSegments()) {
                doMigrateOffset500(seg);
            }
        }

        score->scanElements(doMigrateOffset500);
    }
}

void EngravingCompat::doPostLayoutCompatIfNeeded(MasterScore* score)
{
    bool needRelayout = false;

    int mscVersion = score->mscVersion();

    if (mscVersion < 470) {
        needRelayout |= setLyricLineVisibility(score);
    }

    if (mscVersion < 440) {
        needRelayout |= relayoutUserModifiedCrossStaffBeams(score);
    }

    if (mscVersion < 500) {
        migrateOffset500(score);
        AlignmentMigration500::migrateSnappedItemAlignment(score);
        AlignmentMigration500::migrateSameItemTypeAlignment(score);
        AlignmentMigration500::migrateHopoLetterAlignment(score);
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

bool EngravingCompat::setLyricLineVisibility(MasterScore* masterScore)
{
    bool needRelayout = false;
    for (Score* score : masterScore->scoreList()) {
        for (Spanner* sp : score->unmanagedSpanners()) {
            if (!sp->isLyricsLine()) {
                continue;
            }

            LyricsLine* ll = toLyricsLine(sp);
            if (!ll->lyrics() || ll->visible() == ll->lyrics()->visible()) {
                continue;
            }

            ll->setVisible(ll->lyrics()->visible());
            needRelayout = true;
        }
    }

    return needRelayout;
}

// Migrate item centring & alignment. Before 5.0 offsets to one item would move the whole aligned group
// From 5.0, offsets are applied after centring & alignment which means items can be individually offset from their group
// Replicate the appearance of <5.0 scores by giving all items in a centred or aligned group the same offset
// These functions collect the same groups of items as are collected in AlignmentLayout & SystemLayout, then set their offset to the highest in the group

bool AlignmentMigration500::rowItemIsAbove(const EngravingItem* item)
{
    return item->isArticulationFamily() ? toArticulation(item)->ldata()->up() : item->placeAbove();
}

void AlignmentMigration500::alignItemOffsetGroup(const std::vector<EngravingItem*>& group)
{
    if (group.size() < 2) {
        return;
    }

    bool above = rowItemIsAbove(group.front());
    bool outermostYOffsetFound = false;
    double outermostYOffset = 0.0;
    for (const EngravingItem* item : group) {
        double y = item->offset().y();
        if (muse::RealIsNull(y)) {
            continue;
        }
        if (!outermostYOffsetFound || (above ? y < outermostYOffset : y > outermostYOffset)) {
            outermostYOffset = y;
            outermostYOffsetFound = true;
        }
    }

    if (!outermostYOffsetFound) {
        return;
    }

    for (EngravingItem* item : group) {
        if (!muse::RealIsEqual(item->offset().y(), outermostYOffset)) {
            item->setProperty(Pid::OFFSET, PointF(item->offset().x(), outermostYOffset));
        }
    }
}

void AlignmentMigration500::scanConnectedItemsInSnappingChain(EngravingItem* item, const System* system, std::set<EngravingItem*>& visited,
                                                              std::vector<EngravingItem*>& group)
{
    if (muse::contains(visited, item)) {
        return;
    }
    visited.insert(item);
    group.push_back(item);

    EngravingItem* before = item->ldata()->itemSnappedBefore();
    while (before && before->findAncestor(ElementType::SYSTEM) == system && !muse::contains(visited, before)) {
        visited.insert(before);
        group.push_back(before);
        before = before->ldata()->itemSnappedBefore();
    }

    EngravingItem* after = item->ldata()->itemSnappedAfter();
    while (after && after->findAncestor(ElementType::SYSTEM) == system && !muse::contains(visited, after)) {
        visited.insert(after);
        group.push_back(after);
        after = after->ldata()->itemSnappedAfter();
    }
}

void AlignmentMigration500::migrateSnappedItemAlignment(MasterScore* masterScore)
{
    // For items aligned by AlignmentLayout::alignItemsWithTheirSnappingChain
    // Used for dynamics, expressions, hairpins, tempo text and gradual tempo change lines
    for (Score* score : masterScore->scoreList()) {
        std::vector<EngravingItem*> candidates;
        score->scanElements([&candidates](EngravingItem* item) {
            if (item && (item->isDynamic()
                         || item->isExpression()
                         || item->isHairpinSegment()
                         || item->isTempoText()
                         || item->isGradualTempoChangeSegment())) {
                candidates.push_back(item);
            }
        });

        std::set<EngravingItem*> visited;
        for (EngravingItem* item : candidates) {
            if (muse::contains(visited, item)) {
                continue;
            }
            const System* system = toSystem(item->findAncestor(ElementType::SYSTEM));
            std::vector<EngravingItem*> group;
            scanConnectedItemsInSnappingChain(item, system, visited, group);
            alignItemOffsetGroup(group);
        }
    }
}

void AlignmentMigration500::migrateSameItemTypeAlignment(MasterScore* masterScore)
{
    // For items aligned by AlignmentLayout::alignItemsForSystem
    // Used for harmony, fret diagrams and sticking
    using RowKey = std::tuple<const System*, staff_idx_t, bool>;

    for (Score* score : masterScore->scoreList()) {
        std::map<RowKey, std::vector<EngravingItem*> > harmonyGroups;
        std::map<RowKey, std::vector<EngravingItem*> > harmonyOnFretDiagramGroups;
        std::map<RowKey, std::vector<EngravingItem*> > fretGroups;
        std::map<RowKey, std::vector<EngravingItem*> > stickingGroups;

        bool alignChordSymbols = score->style().styleB(Sid::verticallyAlignChordSymbols);

        score->scanElements([&](EngravingItem* item) {
            if (!item || !item->addToSkyline() || item->excludeVerticalAlign()) {
                return;
            }
            bool isHarmony = alignChordSymbols && item->isHarmony();
            bool isFretDiagram = alignChordSymbols && item->isFretDiagram();
            bool isSticking = item->isSticking();
            if (!isHarmony && !isFretDiagram && !isSticking) {
                return;
            }

            const System* system = toSystem(item->findAncestor(ElementType::SYSTEM));
            if (!system) {
                return;
            }
            RowKey key(system, item->staffIdx(), item->placeAbove());
            if (isHarmony && !toHarmony(item)->getParentFretDiagram()) {
                harmonyGroups[key].push_back(item);
            } else if (isHarmony && toHarmony(item)->getParentFretDiagram()) {
                harmonyOnFretDiagramGroups[key].push_back(item);
            } else if (isFretDiagram) {
                fretGroups[key].push_back(item);
            } else {
                stickingGroups[key].push_back(item);
            }
        });

        for (auto& pair : harmonyGroups) {
            alignItemOffsetGroup(pair.second);
        }
        for (auto& pair : harmonyOnFretDiagramGroups) {
            alignItemOffsetGroup(pair.second);
        }
        for (auto& pair : fretGroups) {
            alignItemOffsetGroup(pair.second);
        }
        for (auto& pair : stickingGroups) {
            alignItemOffsetGroup(pair.second);
        }
    }
}

void AlignmentMigration500::migrateHopoLetterAlignment(MasterScore* masterScore)
{
    // For items aligned by AlignmentLayout::alignHopoLetters
    for (Score* score : masterScore->scoreList()) {
        const bool alignLettersTab = score->style().styleB(Sid::hopoAlignLettersTabStaves);
        const bool alignLettersStd = score->style().styleB(Sid::hopoAlignLettersStandardStaves);

        for (Spanner* spanner : score->spannerList()) {
            if (!spanner->isHammerOnPullOff()) {
                continue;
            }
            HammerOnPullOff* hopo = toHammerOnPullOff(spanner);
            const Staff* staff = hopo->staff();
            const StaffType* staffType = staff ? staff->staffType(hopo->tick()) : nullptr;
            if (!staffType) {
                continue;
            }
            bool alignLetters = staffType->isTabStaff() ? alignLettersTab : alignLettersStd;
            if (!alignLetters) {
                continue;
            }

            for (SpannerSegment* seg : hopo->spannerSegments()) {
                if (!seg->system()) {
                    continue;
                }

                std::vector<EngravingItem*> group;
                for (HammerOnPullOffText* text : toHammerOnPullOffSegment(seg)->hopoText()) {
                    group.push_back(text);
                }
                if (group.empty()) {
                    continue;
                }

                bool above = rowItemIsAbove(group.front());
                bool itemsSamePlacement = true;
                for (EngravingItem* item : group) {
                    if (rowItemIsAbove(item) != above) {
                        itemsSamePlacement = false; // Don't try to align items split above/below
                    }
                }

                if (!itemsSamePlacement) {
                    continue;
                }

                if (hopo->startElement() && hopo->startElement()->isChordRest()) {
                    ChordRest* scr = toChordRest(hopo->startElement());
                    if (scr->isChord()) {
                        for (Articulation* art : toChord(scr)->articulations()) {
                            if (art->isTapping() && toTapping(art)->text() && rowItemIsAbove(art) == above) {
                                group.push_back(art);
                            }
                        }
                    }
                }

                alignItemOffsetGroup(group);
            }
        }
    }
}
} // namespace mu::engraving::compat
