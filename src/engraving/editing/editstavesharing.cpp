/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

namespace mu::engraving {
//-------------------------------------------------------------------
// EditStaveSharing
//-------------------------------------------------------------------

void EditStaveSharing::toggleStaveSharing(Score* score, bool on)
{
    score->undoChangeStyleVal(Sid::enableStaveSharing, on);

    if (on && score->sharedParts().empty()) {
        cmdCreateSharedStaves(score);
    }

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

void EditStaveSharing::cmdCreateSharedStaves(Score* score)
{
    StaveSharingGroups staveSharingGroups = computeGroups(score);

    createSharedParts(staveSharingGroups, score);
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

void EditStaveSharing::createSharedParts(const StaveSharingGroups& groups, Score* score)
{
    const KeyList& keyList = score->keyList();

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
                disconnectSharedPart(existingSharedPart, originPart);
                connectSharedPart(sharedPart, originPart);
            } else if (!existingSharedPart) {
                connectSharedPart(sharedPart, originPart);
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
    score->undoInsertStaff(staff, relStaffIdx);

    staff_idx_t absStaffIdx = track2staff(sharedPart->startTrack()) + relStaffIdx;
    score->adjustKeySigs(absStaffIdx, absStaffIdx + 1, keyList);
}

void EditStaveSharing::connectSharedPart(SharedPart* sharedPart, Part* originPart)
{
    Score* score = originPart->score();
    score->undo(new ConnectSharedPart(sharedPart, originPart));
    addStaffToSharedPart(sharedPart, score->keyList(), originPart->staff(0)->staffType());
}

void EditStaveSharing::disconnectSharedPart(SharedPart* sharedPart, Part* originPart)
{
    Score* score = originPart->score();
    score->undo(new DisconnectSharedPart(sharedPart, originPart));
    score->undoRemoveStaff(sharedPart->staves().back());
}

void EditStaveSharing::handleRemovePart(Part* part)
{
    Score* score = part->score();

    if (SharedPart* sharedPart = part->sharedPart()) {
        disconnectSharedPart(sharedPart, part);
        if (sharedPart->originParts().empty()) {
            score->cmdRemovePart(sharedPart);
        }
    } else if (part->isSharedPart()) {
        sharedPart = toSharedPart(part);
        std::vector<Part*> originParts = sharedPart->originParts();
        for (Part* originPart : originParts) {
            score->undo(new DisconnectSharedPart(sharedPart, originPart));
        }
    }
}

//-------------------------------------------------------------------
// Undo commands
//-------------------------------------------------------------------

void ConnectSharedPart::undo(EditData*)
{
    sharedPart->removeOriginPart(originPart);
}

void ConnectSharedPart::redo(EditData*)
{
    sharedPart->addOriginPart(originPart);
}

void DisconnectSharedPart::undo(EditData*)
{
    sharedPart->addOriginPart(originPart);
}

void DisconnectSharedPart::redo(EditData*)
{
    sharedPart->removeOriginPart(originPart);
}
}
