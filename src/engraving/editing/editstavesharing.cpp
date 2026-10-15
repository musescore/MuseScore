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
#include "dom/stavesharingchange.h"
#include "dom/system.h"

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

class AddStaveSharingChange : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, AddStaveSharingChange)

    SharedPart* m_sharedPart = nullptr;
    Fraction m_tick = Fraction();
    StaveSharingState m_staveSharingState = StaveSharingState();

public:
    AddStaveSharingChange(SharedPart* s, Fraction t, StaveSharingState m)
        : m_sharedPart(s), m_tick(t), m_staveSharingState(m) {}

    void undo() override
    {
        m_sharedPart->removeStaveSharingStateAtTick(m_tick);
    }

    void redo() override
    {
        m_sharedPart->setStaveSharingStateAtTick(m_staveSharingState, m_tick);
    }

    UNDO_TYPE(CommandType::AddStaveSharingChange)
    UNDO_NAME("Add stave sharing change")
};

class ChangeStaveSharingState : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeStaveSharingState)

    SharedPart* m_sharedPart = nullptr;
    Fraction m_tick = Fraction();
    StaveSharingState m_staveSharingState = StaveSharingState();

    void flip() override
    {
        std::optional<StaveSharingState> oldState = m_sharedPart->exactStaveSharingStateAtTick(m_tick);
        IF_ASSERT_FAILED(oldState) {
            return;
        }
        m_sharedPart->setStaveSharingStateAtTick(m_staveSharingState, m_tick);
        m_staveSharingState = *oldState;
    }

public:
    // Only for ticks with an existing state - use AddStaveSharingChange otherwise
    ChangeStaveSharingState(SharedPart* s, Fraction t, StaveSharingState m)
        : m_sharedPart(s), m_tick(t), m_staveSharingState(m) {}

    UNDO_TYPE(CommandType::ChangeStaveSharingState)
    UNDO_NAME("Change stave sharing state")
};

class ChangeStaveSharingIsReset : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, ChangeStaveSharingIsReset)

    SharedPart* m_sharedPart = nullptr;
    Fraction m_tick = Fraction();
    bool m_isReset = false;

    void flip() override
    {
        std::optional<StaveSharingState> state = m_sharedPart->exactStaveSharingStateAtTick(m_tick);
        IF_ASSERT_FAILED(state) {
            return;
        }
        bool oldIsReset = state->isReset();
        state->setIsReset(m_isReset);
        m_sharedPart->setStaveSharingStateAtTick(*state, m_tick);
        m_isReset = oldIsReset;
    }

public:
    // Only for ticks with an existing state
    ChangeStaveSharingIsReset(SharedPart* s, Fraction t, bool r)
        : m_sharedPart(s), m_tick(t), m_isReset(r) {}

    UNDO_TYPE(CommandType::ChangeStaveSharingIsReset)
    UNDO_NAME("Change stave sharing is reset")
};

class RemoveStaveSharingChange : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveStaveSharingChange)

    SharedPart* m_sharedPart = nullptr;
    Fraction m_tick = Fraction();
    StaveSharingState m_staveSharingState = StaveSharingState();

public:
    RemoveStaveSharingChange(SharedPart* s, Fraction t)
        : m_sharedPart(s), m_tick(t) {}

    void undo() override
    {
        m_sharedPart->setStaveSharingStateAtTick(m_staveSharingState, m_tick);
    }

    void redo() override
    {
        std::optional<StaveSharingState> oldState = m_sharedPart->exactStaveSharingStateAtTick(m_tick);
        IF_ASSERT_FAILED(oldState) {
            return;
        }
        m_staveSharingState = oldState.value();
        m_sharedPart->removeStaveSharingStateAtTick(m_tick);
    }

    UNDO_TYPE(CommandType::RemoveStaveSharingChange)
    UNDO_NAME("Remove stave sharing change")
};

void undoSetStaveSharingState(Transaction& tx, SharedPart* sharedPart, const Fraction& tick, const StaveSharingState& state)
{
    if (sharedPart->exactStaveSharingStateAtTick(tick)) {
        tx.push(new ChangeStaveSharingState(sharedPart, tick, state));
    } else {
        tx.push(new AddStaveSharingChange(sharedPart, tick, state));
    }
}
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
        if (part->nstaves() > 1 || part->isSharedPart()) {
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

void EditStaveSharing::addStaffToSharedPart(SharedPart* sharedPart, const Part* originPart, const KeyList& keyList,
                                            const StaffType* staffType)
{
    Staff* staff = Factory::createStaff(sharedPart);
    staff->setStaffType(Fraction(0, 1), *staffType);
    staff->setPart(sharedPart);

    Score* score = sharedPart->score();
    staff->setScore(score);

    staff_idx_t relStaffIdx = sharedPart->nstaves();
    staff->setDefaultClefType(originPart->instrument()->clefType(0));

    score->undoInsertStaff(staff, relStaffIdx);

    staff_idx_t absStaffIdx = track2staff(sharedPart->trackRange().startTrack) + relStaffIdx;
    score->adjustKeySigs(absStaffIdx, absStaffIdx + 1, keyList);
}

void EditStaveSharing::connectSharedPart(Transaction& tx, SharedPart* sharedPart, Part* originPart)
{
    Score* score = originPart->score();
    tx.push(new ConnectSharedPart(sharedPart, originPart));
    addStaffToSharedPart(sharedPart, originPart, score->keyList(), originPart->staff(0)->staffType());
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

void EditStaveSharing::addStaveSharingChange(Transaction& tx, Segment* seg, Part* part, bool reset)
{
    Score* score = seg ? seg->score() : nullptr;
    IF_ASSERT_FAILED(part && part->isSharedPart()) {
        return;
    }
    SharedPart* sharedPart = toSharedPart(part);

    // Use current state to initialise map
    StaveSharingState newStaveSharingState = sharedPart->staveSharingStateAtTick(seg->tick());
    newStaveSharingState.setUserTrackMap(newStaveSharingState.sharedTrackMap());
    newStaveSharingState.setIsReset(reset);

    undoSetStaveSharingState(tx, sharedPart, seg->tick(), newStaveSharingState);

    // When adding a stave sharing change, reset at the start of the next system if there is no user change there already
    const System* curSys = !reset ? seg->system() : nullptr;
    const Fraction nextSysTick = curSys ? curSys->endTick() : score->endTick();
    if (nextSysTick < score->endTick()) {
        std::optional<StaveSharingState> existingState = sharedPart->exactStaveSharingStateAtTick(nextSysTick);
        if (!existingState || !existingState->isUserModified()) {
            StaveSharingState resetState = existingState ? existingState.value() : newStaveSharingState;
            resetState.setUserTrackMap(resetState.sharedTrackMap());
            resetState.setIsReset(true);

            undoSetStaveSharingState(tx, sharedPart, nextSysTick, resetState);
        }
    }

    seg->triggerLayout();
}

void EditStaveSharing::setTrackMapping(Transaction& tx, Segment* seg, Part* part, track_idx_t originTrack, track_idx_t sharedTrack)
{
    IF_ASSERT_FAILED(part && part->isSharedPart()) {
        return;
    }
    SharedPart* sharedPart = toSharedPart(part);

    StaveSharingState newStaveSharingState = sharedPart->staveSharingStateAtTick(seg->tick());

    SharedTrackMap map = newStaveSharingState.userTrackMap().value_or(newStaveSharingState.sharedTrackMap());
    map[originTrack] = sharedTrack;
    newStaveSharingState.setUserTrackMap(map);
    newStaveSharingState.setIsReset(false);

    undoSetStaveSharingState(tx, sharedPart, seg->tick(), newStaveSharingState);

    seg->triggerLayout();
}

void EditStaveSharing::setIsReset(Transaction& tx, StaveSharingChange* staveSharingChange, bool reset)
{
    Part* part = staveSharingChange ? staveSharingChange->part() : nullptr;
    IF_ASSERT_FAILED(part && part->isSharedPart()) {
        return;
    }

    SharedPart* sharedPart = toSharedPart(part);
    const Fraction tick = staveSharingChange->tick();
    std::optional<StaveSharingState> state = sharedPart->exactStaveSharingStateAtTick(tick);
    IF_ASSERT_FAILED(state) {
        return;
    }

    if (state->isReset() == reset) {
        return;
    }

    tx.push(new ChangeStaveSharingIsReset(sharedPart, tick, reset));
    staveSharingChange->triggerLayout();
}

void EditStaveSharing::removeStaveSharingChange(Transaction& tx, StaveSharingChange* staveSharingChange)
{
    Part* part = staveSharingChange->part();
    IF_ASSERT_FAILED(part->isSharedPart()) {
        return;
    }

    SharedPart* sharedPart = toSharedPart(part);

    tx.push(new RemoveStaveSharingChange(sharedPart, staveSharingChange->tick()));
    staveSharingChange->triggerLayout();
}
