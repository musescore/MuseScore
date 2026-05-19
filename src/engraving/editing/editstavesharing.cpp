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

#include "editstavesharing.h"

#include "dom/factory.h"
#include "dom/instrtemplate.h"
#include "dom/part.h"
#include "dom/score.h"
#include "dom/sharedpart.h"
#include "dom/staff.h"

#include "transaction/transaction.h"
#include "transaction/undoablecommand.h"

using namespace mu::engraving;

namespace {
//-------------------------------------------------------------------
// Undo commands
//-------------------------------------------------------------------

class ConnectSharedPart : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, ConnectSharedPart)

    SharedPart* sharedPart = nullptr;
    Part* originPart = nullptr;

public:
    ConnectSharedPart(SharedPart* s, Part* o)
        : sharedPart(s), originPart(o) {}

    void undo() override
    {
        sharedPart->removeOriginPart(originPart);
    }

    void redo() override
    {
        sharedPart->addOriginPart(originPart);
    }

    UNDO_TYPE(CommandType::ConnectSharedPart)
    UNDO_NAME("Connect shared part")
};

class DisconnectSharedPart : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, DisconnectSharedPart)

    SharedPart* sharedPart = nullptr;
    Part* originPart = nullptr;

public:
    DisconnectSharedPart(SharedPart* s, Part* o)
        : sharedPart(s), originPart(o) {}

    void undo() override
    {
        sharedPart->addOriginPart(originPart);
    }

    void redo() override
    {
        sharedPart->removeOriginPart(originPart);
    }

    UNDO_TYPE(CommandType::DisconnectSharedPart)
    UNDO_NAME("Disconnect shared part")
};
}

//-------------------------------------------------------------------
// EditStaveSharing
//-------------------------------------------------------------------

void EditStaveSharing::toggleStaveSharing(Transaction& tx, Score* score, bool on)
{
    score->undoChangeStyleVal(Sid::enableStaveSharing, on);

    if (on && score->sharedParts().empty()) {
        cmdCreateSharedStaves(tx, score);
    }

    score->setBracketsAndBarlines();

    score->update();
}

void EditStaveSharing::cmdRemoveSharedStaves(Score* score)
{
    std::vector<Part*> parts = score->parts(); // COPY because we're about to remove elements

    for (Part* part : parts) {
        if (part->isSharedPart()) {
            score->cmdRemovePart(part);
        }
    }
}

void EditStaveSharing::cmdCreateSharedStaves(Transaction& tx, Score* score)
{
    StaveSharingGroups staveSharingGroups = computeGroups(score);

    createSharedParts(tx, staveSharingGroups, score);
}

StaveSharingGroups EditStaveSharing::computeGroups(Score* score)
{
    const std::vector<Part*>& parts = score->parts();

    StaveSharingGroups staveSharingGroups;

    auto excludePart = [](Part* part) {
        if (part->instruments().size() > 1 || part->nstaves() > 1 || part->isSharedPart()) {
            return true;
        }

        Staff* firstStaff = part->staves().front();
        const StaffType* staffType = firstStaff->staffType();
        if (staffType->group() != StaffGroup::STANDARD) {
            return true;
        }

        return false;
    };

    for (size_t partIdx = 0; partIdx < parts.size();) {
        Part* part = parts[partIdx];
        if (excludePart(part)) {
            ++partIdx;
            continue;
        }

        Instrument* instr = part->instrument();
        String instrGroup = instr->group();
        if (instrGroup != "woodwinds" && instrGroup != "brass") { // TODO: styles
            ++partIdx;
            continue;
        }

        StaveSharingGroup staveSharingGroup;

        size_t nextPartIdx = partIdx;
        while (nextPartIdx < parts.size()) {
            Part* nextPart = parts[nextPartIdx];
            if (excludePart(nextPart)) {
                break;
            }
            Instrument* nextInstr = nextPart->instrument();
            if (nextPart != part && nextInstr->id() != instr->id()) {
                break;
            }

            staveSharingGroup.push_back(nextPart);
            ++nextPartIdx;
        }

        if (staveSharingGroup.size() > 1) {
            staveSharingGroups.push_back(staveSharingGroup);
        }

        partIdx = nextPartIdx;
    }

    return staveSharingGroups;
}

void EditStaveSharing::createSharedParts(Transaction& tx, const StaveSharingGroups& groups, Score* score)
{
    for (const StaveSharingGroup& group : groups) {
        SharedPart* sharedPart = nullptr;
        for (Part* part : group) {
            if (part->sharedPart() && !sharedPart) {
                sharedPart = part->sharedPart();
                break;
            }
        }

        Part* firstPart = group.front();
        const Instrument* instr = firstPart->instrument();

        if (!sharedPart) {
            size_t idx = muse::indexOf(score->parts(), firstPart);
            sharedPart = createSharedPart(score, idx, instr);
        }

        for (Part* originPart : group) {
            SharedPart* existingSharedPart = originPart->sharedPart();
            if (existingSharedPart && existingSharedPart != sharedPart) {
                disconnectSharedPart(tx, existingSharedPart, originPart);
                connectSharedPart(tx, sharedPart, originPart);
            } else if (!existingSharedPart) {
                connectSharedPart(tx, sharedPart, originPart);
            }
        }
    }
}

SharedPart* EditStaveSharing::createSharedPart(Score* score, size_t idx, const Instrument* instr)
{
    SharedPart* sharedPart = new SharedPart(score);
    score->undoInsertPart(sharedPart, idx);

    Instrument* instrument = new Instrument(*instr);
    InstrumentLabel& label = instrument->instrumentLabel();
    label.setAllowGroupName(false);
    label.setNumber(0);
    label.setShowNumberLong(false);
    label.setShowNumberShort(false);

    sharedPart->setInstrument(instrument);

    return sharedPart;
}

void EditStaveSharing::addStaffToSharedPart(SharedPart* sharedPart, const KeyList& keyList, const StaffType* staffType)
{
    Staff* staff = Factory::createStaff(sharedPart);
    staff->setStaffType(Fraction(0, 1), *staffType);
    staff->setPart(sharedPart);

    Score* score = sharedPart->score();
    staff->setScore(score);

    staff_idx_t relStaffIdx = sharedPart->nstaves();
    staff->setDefaultClefType(sharedPart->instrument()->clefType(relStaffIdx));

    score->undoInsertStaff(staff, relStaffIdx);

    staff_idx_t absStaffIdx = track2staff(sharedPart->trackRange().startTrack) + relStaffIdx;
    score->adjustKeySigs(absStaffIdx, absStaffIdx + 1, keyList);
}

void EditStaveSharing::connectSharedPart(Transaction& tx, SharedPart* sharedPart, Part* originPart)
{
    Score* score = originPart->score();
    tx.push(new ConnectSharedPart(sharedPart, originPart));
    addStaffToSharedPart(sharedPart, score->keyList(), originPart->staff(0)->staffType());
}

void EditStaveSharing::disconnectSharedPart(Transaction& tx, SharedPart* sharedPart, Part* originPart)
{
    Score* score = originPart->score();
    tx.push(new DisconnectSharedPart(sharedPart, originPart));
    score->undoRemoveStaff(sharedPart->staves().back());
}

void EditStaveSharing::handleRemovePart(Transaction& tx, Part* part)
{
    Score* score = part->score();

    if (SharedPart* sharedPart = part->sharedPart()) {
        disconnectSharedPart(tx, sharedPart, part);
        if (sharedPart->originParts().empty()) {
            score->cmdRemovePart(sharedPart);
        }
    } else if (part->isSharedPart()) {
        sharedPart = toSharedPart(part);
        std::vector<Part*> originParts = sharedPart->originParts();
        for (Part* originPart : originParts) {
            tx.push(new DisconnectSharedPart(sharedPart, originPart));
        }
    }
}
