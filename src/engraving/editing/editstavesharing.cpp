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

#include <functional>
#include <optional>
#include <set>

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

class EditStaveSharingState : public UndoableCommand
{
protected:
    SharedPart* m_sharedPart = nullptr;
    Fraction m_tick = Fraction();

    EditStaveSharingState(SharedPart* s, const Fraction& t)
        : m_sharedPart(s), m_tick(t) {}

    std::optional<StaveSharingState> currentState() const
    {
        return m_sharedPart->exactStaveSharingStateAtTick(m_tick);
    }

    void setState(const StaveSharingState& state)
    {
        m_sharedPart->setStaveSharingStateAtTick(state, m_tick);
        triggerLayout();
    }

    void removeState()
    {
        m_sharedPart->removeStaveSharingStateAtTick(m_tick);
        triggerLayout();
    }

private:
    void triggerLayout()
    {
        const staff_idx_t staff = m_sharedPart->staves().front()->idx();
        const Fraction& tick2 = m_sharedPart->nextUserModifiedStaveSharingTick(m_tick);
        m_sharedPart->score()->setLayout(m_tick, tick2, staff, staff);
    }
};

class AddStaveSharingChange : public EditStaveSharingState
{
    OBJECT_ALLOCATOR(engraving, AddStaveSharingChange)

    StaveSharingState m_staveSharingState = StaveSharingState();

public:
    AddStaveSharingChange(SharedPart* s, const Fraction& t, const StaveSharingState& m)
        : EditStaveSharingState(s, t), m_staveSharingState(m) {}

    void undo() override
    {
        removeState();
    }

    void redo() override
    {
        setState(m_staveSharingState);
    }

    UNDO_TYPE(CommandType::AddStaveSharingChange)
    UNDO_NAME("Add stave sharing change")
};

class ChangeStaveSharingState : public EditStaveSharingState
{
    OBJECT_ALLOCATOR(engraving, ChangeStaveSharingState)

    StaveSharingState m_staveSharingState = StaveSharingState();

    void flip() override
    {
        std::optional<StaveSharingState> oldState = currentState();
        IF_ASSERT_FAILED(oldState) {
            return;
        }
        setState(m_staveSharingState);
        m_staveSharingState = *oldState;
    }

public:
    // Only for ticks with an existing state - use AddStaveSharingChange otherwise
    ChangeStaveSharingState(SharedPart* s, const Fraction& t, const StaveSharingState& m)
        : EditStaveSharingState(s, t), m_staveSharingState(m) {}

    UNDO_TYPE(CommandType::ChangeStaveSharingState)
    UNDO_NAME("Change stave sharing state")
};

class ChangeStaveSharingIsReset : public EditStaveSharingState
{
    OBJECT_ALLOCATOR(engraving, ChangeStaveSharingIsReset)

    bool m_isReset = false;

    void flip() override
    {
        std::optional<StaveSharingState> state = currentState();
        IF_ASSERT_FAILED(state) {
            return;
        }
        bool oldIsReset = state->isReset();
        state->setIsReset(m_isReset);
        setState(*state);
        m_isReset = oldIsReset;
    }

public:
    // Only for ticks with an existing state
    ChangeStaveSharingIsReset(SharedPart* s, const Fraction& t, bool r)
        : EditStaveSharingState(s, t), m_isReset(r) {}

    UNDO_TYPE(CommandType::ChangeStaveSharingIsReset)
    UNDO_NAME("Change stave sharing is reset")
};

class RemoveStaveSharingChange : public EditStaveSharingState
{
    OBJECT_ALLOCATOR(engraving, RemoveStaveSharingChange)

    StaveSharingState m_staveSharingState = StaveSharingState();

public:
    RemoveStaveSharingChange(SharedPart* s, const Fraction& t)
        : EditStaveSharingState(s, t) {}

    void undo() override
    {
        setState(m_staveSharingState);
    }

    void redo() override
    {
        std::optional<StaveSharingState> oldState = currentState();
        IF_ASSERT_FAILED(oldState) {
            return;
        }
        m_staveSharingState = *oldState;
        removeState();
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

const SharedTrackMap& storedOrComputedTrackMap(const StaveSharingState& state)
{
    const std::optional<SharedTrackMap>& userMap = state.userTrackMap();
    return userMap && !userMap->empty() ? *userMap : state.sharedTrackMap();
}
}

//-------------------------------------------------------------------
// EditStaveSharing
//-------------------------------------------------------------------

void EditStaveSharing::toggleStaveSharing(Transaction& tx, Score* score, bool on)
{
    score->undoChangeStyleVal(Sid::enableStaveSharing, on);

    if (on && score->sharedParts().empty()) {
        createSharedStaves(tx, score);
    }

    score->setBracketsAndBarlines();

    score->update();
}

void EditStaveSharing::createSharedStaves(Transaction& tx, Score* score)
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
        if (staffType->group() != StaffGroup::STANDARD && staffType->group() != StaffGroup::JIANPU) {
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
    const TrackRange originTracks = originPart->trackRange();

    remapUserTrackMaps(tx, score, [&originTracks](track_idx_t track) {
        return track >= originTracks.startTrack && track < originTracks.endTrack ? muse::nidx : track;
    });

    tx.push(new DisconnectSharedPart(sharedPart, originPart));
    score->undoRemoveStaff(sharedPart->staves().back());
}

void EditStaveSharing::disconnectFromCurrentSharedPart(Transaction& tx, Part* part)
{
    SharedPart* currentSharedPart = part->sharedPart();
    if (!currentSharedPart) {
        return;
    }

    disconnectSharedPart(tx, currentSharedPart, part);
    if (currentSharedPart->originParts().empty()) {
        part->score()->cmdRemovePart(currentSharedPart);
    }
}

SharedPart* EditStaveSharing::createSharedPart(Transaction& tx, const std::vector<Part*>& parts)
{
    IF_ASSERT_FAILED(!parts.empty()) {
        return nullptr;
    }

    Part* firstPart = parts.front();
    Score* score = firstPart->score();
    const size_t idx = muse::indexOf(score->parts(), firstPart);
    SharedPart* sharedPart = createSharedPart(score, idx, firstPart->instrument());

    changeSharedParts(tx, score, SharedPartsChange().join(parts, sharedPart));

    return sharedPart;
}

void EditStaveSharing::changeSharedParts(Transaction& tx, Score* score, const SharedPartsChange& change)
{
    const std::unordered_map<Part*, SharedPart*>& targets = change.targets();
    std::set<SharedPart*> leftSharedParts;

    // In score order, so the commands are pushed in the same order on every run
    const std::vector<Part*> parts = score->parts();
    for (Part* part : parts) {
        auto it = targets.find(part);
        if (it == targets.end()) {
            continue;
        }

        SharedPart* targetSharedPart = it->second;
        SharedPart* currentSharedPart = part->sharedPart();
        if (currentSharedPart == targetSharedPart) {
            continue;
        }

        if (currentSharedPart) {
            disconnectSharedPart(tx, currentSharedPart, part);
            leftSharedParts.insert(currentSharedPart);
        }

        if (targetSharedPart) {
            connectSharedPart(tx, targetSharedPart, part);
        }
    }

    // Only now: a shared part emptied above may have been joined later in this edit
    for (SharedPart* sharedPart : leftSharedParts) {
        if (sharedPart->originParts().empty()) {
            score->cmdRemovePart(sharedPart);
        }
    }

    score->setBracketsAndBarlines();
    score->update();
}

void EditStaveSharing::remapUserTrackMaps(Transaction& tx, Score* score, const std::function<track_idx_t(track_idx_t)>& remap)
{
    for (SharedPart* sharedPart : score->sharedParts()) {
        for (const auto& [tick, state] : sharedPart->staveSharingStatesBetweenTicks(Fraction(0, 1), score->endTick())) {
            if (!state.isUserModified()) {
                continue;
            }

            SharedTrackMap newMap;
            bool reset = false;
            for (const auto& [originTrack, sharedTrack] : state.userTrackMap().value()) {
                const track_idx_t newOriginTrack = remap(originTrack);
                const track_idx_t newSharedTrack = remap(sharedTrack);
                if (newOriginTrack == muse::nidx) {
                    continue;
                }
                if (newSharedTrack == muse::nidx) {
                    newMap.clear();
                    reset = true;
                    break;
                }
                newMap.emplace(newOriginTrack, newSharedTrack);
            }

            StaveSharingState newState = state;
            newState.setUserTrackMap(newMap);
            if (reset) {
                newState.setIsReset(true);
            }
            if (newState != state) {
                undoSetStaveSharingState(tx, sharedPart, tick, newState);
            }
        }
    }
}

void EditStaveSharing::disconnectBeforeRemovingPart(Transaction& tx, Part* part)
{
    if (part->sharedPart()) {
        disconnectFromCurrentSharedPart(tx, part);
    } else if (part->isSharedPart()) {
        SharedPart* sharedPart = toSharedPart(part);
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
}

void EditStaveSharing::addStaveSharingChangeToRange(Transaction& tx, Segment* startSeg, Segment* endSeg, staff_idx_t startStaff,
                                                    staff_idx_t endStaffIdx, bool reset)
{
    const Score* score = startSeg->score();
    const Fraction& startTick = startSeg->tick();

    for (staff_idx_t staffIdx = startStaff; staffIdx < endStaffIdx;) {
        Staff* staff = score->staff(staffIdx);
        Part* part = staff->part();
        if (!part->isSharedPart()) {
            staffIdx = track2staff(part->trackRange().endTrack);
            continue;
        }
        SharedPart* sharedPart = toSharedPart(part);

        // Create change
        StaveSharingState prevStaveSharingState = sharedPart->staveSharingStateAtTick(startTick);

        StaveSharingState newStaveSharingState = prevStaveSharingState;
        newStaveSharingState.setUserTrackMap(newStaveSharingState.sharedTrackMap());
        newStaveSharingState.setIsReset(reset);

        undoSetStaveSharingState(tx, sharedPart, startTick, newStaveSharingState);

        // No end state needed if the range runs to the end of the score
        std::optional<StaveSharingState> existingState = endSeg ? sharedPart->exactStaveSharingStateAtTick(endSeg->tick()) : std::nullopt;
        if (endSeg && (!existingState || !existingState->isUserModified())) {
            const Fraction& endTick = endSeg->tick();
            if (prevStaveSharingState.isUserModified() && !prevStaveSharingState.isReset()) {
                // Resume previous user sharing state
                undoSetStaveSharingState(tx, sharedPart, endTick, prevStaveSharingState);
            } else if (!reset) {
                // Create reset
                StaveSharingState nextState = existingState ? existingState.value() : newStaveSharingState;
                nextState.setUserTrackMap(nextState.sharedTrackMap());
                nextState.setIsReset(true);
                undoSetStaveSharingState(tx, sharedPart, endTick, nextState);
            }
        }

        // Next part
        staffIdx = track2staff(part->trackRange().endTrack);
    }
}

void EditStaveSharing::setTrackMapping(Transaction& tx, Segment* seg, Part* part, track_idx_t originTrack, track_idx_t sharedTrack)
{
    IF_ASSERT_FAILED(part && part->isSharedPart()) {
        return;
    }
    SharedPart* sharedPart = toSharedPart(part);

    StaveSharingState newStaveSharingState = sharedPart->staveSharingStateAtTick(seg->tick());

    SharedTrackMap map = storedOrComputedTrackMap(newStaveSharingState);
    map[originTrack] = sharedTrack;
    newStaveSharingState.setUserTrackMap(map);
    newStaveSharingState.setIsReset(false);

    undoSetStaveSharingState(tx, sharedPart, seg->tick(), newStaveSharingState);
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

    if (!reset && state->userTrackMap()->empty()) {
        state->setUserTrackMap(state->sharedTrackMap());
        state->setIsReset(false);
        undoSetStaveSharingState(tx, sharedPart, tick, *state);
        return;
    }

    tx.push(new ChangeStaveSharingIsReset(sharedPart, tick, reset));
}

void EditStaveSharing::removeStaveSharingChange(Transaction& tx, StaveSharingChange* staveSharingChange)
{
    Part* part = staveSharingChange ? staveSharingChange->part() : nullptr;
    IF_ASSERT_FAILED(part && part->isSharedPart()) {
        return;
    }

    SharedPart* sharedPart = toSharedPart(part);

    tx.push(new RemoveStaveSharingChange(sharedPart, staveSharingChange->tick()));
}

void EditStaveSharing::removeStaveSharingStatesOnRemoveTime(Transaction& tx, Score* score, const Fraction& startTick,
                                                            const Fraction& endTick)
{
    for (SharedPart* sharedPart : score->sharedParts()) {
        for (const auto& [tick, state] : sharedPart->staveSharingStatesBetweenTicks(startTick, endTick)) {
            if (state.isUserModified()) {
                tx.push(new RemoveStaveSharingChange(sharedPart, tick));
            }
        }
    }
}

StaveSharingStatesByPart EditStaveSharing::userStaveSharingStatesBetweenTicks(Score* score, const Fraction& startTick,
                                                                              const Fraction& endTick)
{
    StaveSharingStatesByPart result;

    for (SharedPart* sharedPart : score->sharedParts()) {
        for (const auto& [tick, state] : sharedPart->staveSharingStatesBetweenTicks(startTick, endTick)) {
            if (state.isUserModified()) {
                result[sharedPart].emplace(tick, state);
            }
        }
    }

    return result;
}

void EditStaveSharing::restoreStaveSharingStates(Transaction& tx, const StaveSharingStatesByPart& states)
{
    for (const auto& [sharedPart, statesByTick] : states) {
        for (const auto& [tick, state] : statesByTick) {
            undoSetStaveSharingState(tx, sharedPart, tick, state);
        }
    }
}
