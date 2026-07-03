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

#include <vector>

namespace mu::engraving {
class Instrument;
class KeyList;
class Part;
class Score;
class SharedPart;
class StaffType;
class Transaction;

using StaveSharingGroup = std::vector<Part*>;
using StaveSharingGroups = std::vector<StaveSharingGroup>;

class EditStaveSharing
{
public:
    static void toggleStaveSharing(Transaction& tx, Score* score, bool on);
    static void handleRemovePart(Transaction& tx, Part* part);

private:
    static void cmdCreateSharedStaves(Transaction& tx, Score* score);
    static void cmdRemoveSharedStaves(Score* score);

    static StaveSharingGroups computeGroups(Score* score);
    static void createSharedParts(Transaction& tx, const StaveSharingGroups& groups, Score* score);
    static SharedPart* createSharedPart(Score* score, size_t idx, const Instrument* instr);
    static void addStaffToSharedPart(SharedPart* sharedPart, const KeyList& keyList, const StaffType* staffType);

    static void connectSharedPart(Transaction& tx, SharedPart* sharedPart, Part* originPart);
    static void disconnectSharedPart(Transaction& tx, SharedPart* sharedPart, Part* originPart);
};
}
