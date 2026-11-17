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
#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include "../dom/sharedpart.h"
#include "../types/types.h"

namespace mu::engraving {
class Instrument;
class KeyList;
class Part;
class Score;
class Segment;
class SharedPart;
class StaffType;
class StaveSharingChange;
class Transaction;

using StaveSharingGroup = std::vector<Part*>;
using StaveSharingGroups = std::vector<StaveSharingGroup>;

class SharedPartsChange
{
public:
    SharedPartsChange& join(const std::vector<Part*>& parts, SharedPart* sharedPart)
    {
        for (Part* part : parts) {
            m_targets[part] = sharedPart;
        }
        return *this;
    }

    SharedPartsChange& leave(const std::vector<Part*>& parts)
    {
        for (Part* part : parts) {
            m_targets[part] = nullptr;
        }
        return *this;
    }

    bool empty() const { return m_targets.empty(); }

    //! nullptr: the part leaves its current shared part
    const std::unordered_map<Part*, SharedPart*>& targets() const { return m_targets; }

private:
    std::unordered_map<Part*, SharedPart*> m_targets;
};

class EditStaveSharing
{
public:
    static void toggleStaveSharing(Transaction& tx, Score* score, bool on);
    static void disconnectBeforeRemovingPart(Transaction& tx, Part* part);

    static SharedPart* createSharedPart(Transaction& tx, const std::vector<Part*>& parts);
    //! Applies the change, then removes the shared parts left empty
    static void changeSharedParts(Transaction& tx, Score* score, const SharedPartsChange& change);
    //! Applies remap to every custom voice map; nidx for an origin track drops that entry, for a shared track resets the map to the default
    static void remapUserTrackMaps(Transaction& tx, Score* score, const std::function<track_idx_t(track_idx_t)>& remap);

    static void addStaveSharingChange(Transaction& tx, Segment* seg, Part* part, bool reset);
    static void addStaveSharingChangeToRange(Transaction& tx, Segment* startSeg, Segment* endSeg, staff_idx_t startStaff,
                                             staff_idx_t endStaffIdx, bool reset);
    static void removeStaveSharingChange(Transaction& tx, StaveSharingChange* staveSharingChange);
    static void setIsReset(Transaction& tx, StaveSharingChange* staveSharingChange, bool reset);

    static void setTrackMapping(Transaction& tx, Segment* seg, Part* part, track_idx_t originTrack, track_idx_t sharedTrack);

    static void removeStaveSharingStatesOnRemoveTime(Transaction& tx, Score* score, const Fraction& startTick, const Fraction& endTick);
    static StaveSharingStatesByPart userStaveSharingStatesBetweenTicks(Score* score, const Fraction& startTick, const Fraction& endTick);
    static void restoreStaveSharingStates(Transaction& tx, const StaveSharingStatesByPart& states);

private:
    static void createSharedStaves(Transaction& tx, Score* score);

    static StaveSharingGroups computeGroups(Score* score);
    static void createSharedParts(Transaction& tx, const StaveSharingGroups& groups, Score* score);
    static SharedPart* createSharedPart(Score* score, size_t idx, const Instrument* instr);
    static void addStaffToSharedPart(SharedPart* sharedPart, const Part* originPart, const KeyList& keyList, const StaffType* staffType);

    static void connectSharedPart(Transaction& tx, SharedPart* sharedPart, Part* originPart);
    static void disconnectSharedPart(Transaction& tx, SharedPart* sharedPart, Part* originPart);
    static void disconnectFromCurrentSharedPart(Transaction& tx, Part* part);
};
}
