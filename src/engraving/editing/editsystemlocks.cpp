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

#include "editsystemlocks.h"

#include "transaction/transaction.h"
#include "transaction/undoablecommand.h"

#include "../dom/measurebase.h"
#include "../dom/score.h"
#include "../dom/system.h"
#include "../dom/rangelock.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   AddSystemLock
//---------------------------------------------------------

class AddSystemLock : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, AddSystemLock)

    const RangeLock* m_systemLock;
public:
    AddSystemLock(const RangeLock* systemLock)
        : m_systemLock(systemLock) {}

    void undo() override
    {
        Score* score = m_systemLock->startMB()->score();
        score->removeSystemLock(m_systemLock);
    }

    void redo() override
    {
        Score* score = m_systemLock->startMB()->score();
        score->addSystemLock(m_systemLock);
    }

    void cleanup(bool wasDone) override
    {
        if (!wasDone) {
            delete m_systemLock;
            m_systemLock = nullptr;
        }
    }

    UNDO_NAME("AddSystemLock")

    std::vector<EngravingObject*> objectItems() const override
    {
        return { m_systemLock->startMB(), m_systemLock->endMB() };
    }
};

//---------------------------------------------------------
//   RemoveSystemLock
//---------------------------------------------------------

class RemoveSystemLock : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveSystemLock)

    const RangeLock* m_systemLock;
public:
    RemoveSystemLock(const RangeLock* systemLock)
        : m_systemLock(systemLock) {}

    void undo() override
    {
        Score* score = m_systemLock->startMB()->score();
        score->addSystemLock(m_systemLock);
    }

    void redo() override
    {
        Score* score = m_systemLock->startMB()->score();
        score->removeSystemLock(m_systemLock);
    }

    void cleanup(bool wasDone) override
    {
        if (wasDone) {
            delete m_systemLock;
            m_systemLock = nullptr;
        }
    }

    UNDO_NAME("RemoveSystemLock")
    std::vector<EngravingObject*> objectItems() const override
    {
        return { m_systemLock->startMB(), m_systemLock->endMB() };
    }
};

//---------------------------------------------------------
//   EditSystemLocks
//---------------------------------------------------------

void EditSystemLocks::undoAddSystemLock(Transaction& tx, Score* score, const RangeLock* lock)
{
    removeLayoutBreaksOnAddSystemLock(tx, score, lock);
    tx.push(new AddSystemLock(lock));
}

void EditSystemLocks::undoRemoveSystemLock(Transaction& tx, Score* score, const RangeLock* lock)
{
    tx.push(new RemoveSystemLock(lock));
}

void EditSystemLocks::undoRemoveAllLocks(Transaction& tx, Score* score)
{
    std::vector<const RangeLock*> allLocks = score->systemLocks()->allLocks(); // copy
    for (const RangeLock* lock : allLocks) {
        undoRemoveSystemLock(tx, score, lock);
    }
}

void EditSystemLocks::toggleSystemLock(Transaction& tx, Score* score, const std::vector<System*>& systems)
{
    bool unlockAll = true;
    for (const System* system : systems) {
        if (!system->isLocked()) {
            unlockAll = false;
            break;
        }
    }

    for (System* system : systems) {
        MeasureBase* startMeas = system->first();
        const RangeLock* currentLock = score->systemLocks()->lockStartingAt(startMeas);
        if (currentLock && unlockAll) {
            undoRemoveSystemLock(tx, score, currentLock);
            continue;
        } else if (!currentLock && !unlockAll) {
            RangeLock* newSystemLock = new RangeLock(startMeas, system->last());
            undoAddSystemLock(tx, score, newSystemLock);
        }
    }
}

void EditSystemLocks::toggleScoreLock(Transaction& tx, Score* score)
{
    bool unlockAll = true;
    for (const System* system : score->systems()) {
        const MeasureBase* first = system->first();
        if (!(first->isMeasure() || first->isHBox())) {
            continue;
        }
        if (!system->isLocked()) {
            unlockAll = false;
            break;
        }
    }

    for (System* system : score->systems()) {
        MeasureBase* startMeas = system->first();
        if (!(startMeas->isMeasure() || startMeas->isHBox())) {
            continue;
        }
        const RangeLock* currentLock = score->systemLocks()->lockStartingAt(startMeas);
        if (currentLock && unlockAll) {
            undoRemoveSystemLock(tx, score, currentLock);
            continue;
        } else if (!currentLock && !unlockAll) {
            RangeLock* newSystemLock = new RangeLock(startMeas, system->last());
            undoAddSystemLock(tx, score, newSystemLock);
        }
    }
}

void EditSystemLocks::addRemoveSystemLocks(Transaction& tx, Score* score, int interval, bool lock)
{
    MeasureBase* startMeasure = score->selection().startMeasureBase();
    MeasureBase* endMeasure = score->selection().endMeasureBase();
    if (!endMeasure) {
        endMeasure = score->lastMeasureMM();
    }

    if (!startMeasure || !endMeasure) {
        return;
    }

    if (lock) {
        for (const System* system : score->systems()) {
            if (system->last()->isBefore(startMeasure)) {
                continue;
            }
            if (system->first()->isAfter(endMeasure)) {
                break;
            }
            if (!system->isLocked()) {
                undoAddSystemLock(tx, score, new RangeLock(system->first(), system->last()));
            }
        }
        return;
    }

    std::vector<const RangeLock*> currentLocks = score->systemLocks()->locksContainedInRange(startMeasure, endMeasure);
    for (const RangeLock* l : currentLocks) {
        undoRemoveSystemLock(tx, score, l);
    }

    if (interval == 0) {
        return;
    }

    int count = 0;
    MeasureBase* lockStart = nullptr;
    for (MeasureBase* mb = startMeasure; mb; mb = mb->nextMM()) {
        if (count == 0) {
            lockStart = mb;
        }
        count++;
        if (count == interval || mb == endMeasure) {
            undoAddSystemLock(tx, score, new RangeLock(lockStart, mb));
            lockStart = nullptr;
            count = 0;
        }
        if (mb == endMeasure) {
            break;
        }
    }
}

void EditSystemLocks::makeIntoSystem(Transaction& tx, Score* score, MeasureBase* first, MeasureBase* last)
{
    const RangeLock* lockContainingfirst = score->systemLocks()->lockContaining(first);
    const RangeLock* lockContaininglast = score->systemLocks()->lockContaining(last);

    if (lockContainingfirst) {
        undoRemoveSystemLock(tx, score, lockContainingfirst);
        if (lockContainingfirst->startMB()->isBefore(first)) {
            MeasureBase* oneBeforeFirst = first->prevMM();
            RangeLock* newLockBefore = new RangeLock(lockContainingfirst->startMB(), oneBeforeFirst);
            undoAddSystemLock(tx, score, newLockBefore);
        }
    }

    if (lockContaininglast) {
        if (lockContaininglast != lockContainingfirst) {
            undoRemoveSystemLock(tx, score, lockContaininglast);
        }
        if (last->isBefore(lockContaininglast->endMB())) {
            MeasureBase* oneAfterLast = last->nextMM();
            RangeLock* newLockAfter = new RangeLock(oneAfterLast, lockContaininglast->endMB());
            undoAddSystemLock(tx, score, newLockAfter);
        }
    }

    std::vector<const RangeLock*> locksContainedInRange = score->systemLocks()->locksContainedInRange(first, last);
    for (const RangeLock* lock : locksContainedInRange) {
        if (lock != lockContainingfirst && lock != lockContaininglast) {
            undoRemoveSystemLock(tx, score, lock);
        }
    }

    RangeLock* newLock = new RangeLock(first, last);
    undoAddSystemLock(tx, score, newLock);
}

void EditSystemLocks::moveMeasureToPrevSystem(Transaction& tx, Score* score, MeasureBase* m)
{
    const System* prevSystem = m->prevNonVBoxSystem();
    if (!prevSystem) {
        return;
    }

    MeasureBase* prevSystemFirstMeas = prevSystem->first();

    const RangeLock* prevSystemLock = score->systemLocks()->lockStartingAt(prevSystemFirstMeas);
    if (prevSystemLock) {
        undoRemoveSystemLock(tx, score, prevSystemLock);
    }

    const System* curSystem = m->system();
    const RangeLock* curSystemLock = score->systemLocks()->lockStartingAt(curSystem->first());
    if (curSystemLock) {
        undoRemoveSystemLock(tx, score, curSystemLock);
        if (curSystemLock->endMB() != m) {
            MeasureBase* nextMB = m->nextMM();
            RangeLock* newLockOnCurSystem = new RangeLock(nextMB, curSystemLock->endMB());
            undoAddSystemLock(tx, score, newLockOnCurSystem);
        }
    }

    RangeLock* sysLock = new RangeLock(prevSystemFirstMeas, m);
    undoAddSystemLock(tx, score, sysLock);
}

void EditSystemLocks::moveMeasureToNextSystem(Transaction& tx, Score* score, MeasureBase* m)
{
    const System* curSystem = m->system();
    MeasureBase* startMeas = curSystem->first();
    bool refMeasureIsStartOfSystem = m == startMeas;

    const RangeLock* curLock = score->systemLocks()->lockStartingAt(startMeas);
    if (curLock) {
        undoRemoveSystemLock(tx, score, curLock);
    }

    if (!refMeasureIsStartOfSystem) {
        MeasureBase* prevMeas = m->prevMM();
        RangeLock* sysLock = new RangeLock(startMeas, prevMeas);
        undoAddSystemLock(tx, score, sysLock);
    }

    const System* nextSystem = m->nextNonVBoxSystem();
    if (!nextSystem) {
        return;
    }

    const RangeLock* nextSysLock = score->systemLocks()->lockStartingAt(nextSystem->first());
    if (nextSysLock) {
        undoRemoveSystemLock(tx, score, nextSysLock);
    }

    if (nextSysLock || refMeasureIsStartOfSystem) {
        RangeLock* newNextSysLock = new RangeLock(m, nextSystem->last());
        undoAddSystemLock(tx, score, newNextSysLock);
    }
}

void EditSystemLocks::applyLockToSelection(Transaction& tx, Score* score)
{
    MeasureBase* first = nullptr;
    MeasureBase* last = nullptr;

    if (score->selection().isRange()) {
        first = score->selection().startMeasureBase();
        last = score->selection().endMeasureBase();
    } else {
        for (EngravingItem* el : score->selection().elements()) {
            if (el->isSystemLockIndicator()) {
                const RangeLock* lock = toSystemLockIndicator(el)->systemLock();
                first = lock->startMB();
                last = lock->endMB();
                break;
            }
            MeasureBase* mb = el->findMeasureBase();
            if (!mb) {
                continue;
            }
            if (!first || mb->isBefore(first)) {
                first = mb;
            }
            if (!last || mb->isAfter(last)) {
                last = mb;
            }
        }
    }

    if (!first || !last) {
        return;
    }

    const RangeLock* lockOnLast = score->systemLocks()->lockContaining(last);
    if (lockOnLast && lockOnLast->endMB() == last) {
        undoRemoveSystemLock(tx, score, lockOnLast);
    } else if (first != last) {
        makeIntoSystem(tx, score, first, last);
    } else {
        makeIntoSystem(tx, score, first->system()->first(), last);
    }
}

void EditSystemLocks::removeSystemLocksOnAddLayoutBreak(Transaction& tx, Score* score, LayoutBreakType breakType,
                                                        const MeasureBase* measure)
{
    IF_ASSERT_FAILED(breakType != LayoutBreakType::NOBREAK) {
        return; // NOBREAK not allowed on locked measures
    }

    const RangeLock* lock = score->systemLocks()->lockContaining(measure);
    if (lock && (breakType == LayoutBreakType::LINE || measure != lock->endMB())) {
        undoRemoveSystemLock(tx, score, lock);
    }
}

void EditSystemLocks::removeLayoutBreaksOnAddSystemLock(Transaction&, Score* score, const RangeLock* lock)
{
    for (MeasureBase* mb = lock->startMB(); mb && mb->isBeforeOrEqual(lock->endMB()); mb = mb->nextMM()) {
        mb->undoSetBreak(false, LayoutBreakType::LINE);
        mb->undoSetBreak(false, LayoutBreakType::NOBREAK);
        if (mb != lock->endMB()) {
            mb->undoSetBreak(false, LayoutBreakType::SECTION);
            mb->undoSetBreak(false, LayoutBreakType::PAGE);
        }
    }
}

void EditSystemLocks::removeSystemLocksOnRemoveMeasures(Transaction& tx, Score* score, const MeasureBase* m1, const MeasureBase* m2)
{
    std::vector<const RangeLock*> allSysLocks = score->systemLocks()->allLocks();
    for (const RangeLock* lock : allSysLocks) {
        MeasureBase* lockStart = lock->startMB();
        MeasureBase* lockEnd = lock->endMB();
        bool lockStartIsInRange = lockStart->isAfterOrEqual(m1) && lockStart->isBeforeOrEqual(m2);
        bool lockEndIsInRange = lockEnd->isAfterOrEqual(m1) && lockEnd->isBeforeOrEqual(m2);
        if (lockStartIsInRange || lockEndIsInRange) {
            undoRemoveSystemLock(tx, score, lock);
        }
        if (lockStartIsInRange && !lockEndIsInRange) {
            MeasureBase* newLockStart = m2->nextMeasure();
            if (newLockStart) {
                undoAddSystemLock(tx, score, new RangeLock(newLockStart, lockEnd));
            }
        } else if (!lockStartIsInRange && lockEndIsInRange) {
            MeasureBase* newLockEnd = m1->prevMeasure();
            if (newLockEnd) {
                undoAddSystemLock(tx, score, new RangeLock(lockStart, newLockEnd));
            }
        }
    }
}

void EditSystemLocks::removeSystemLocksContainingMMRests(Transaction& tx, Score* score)
{
    std::vector<const RangeLock*> allLocks = score->systemLocks()->allLocks(); // copy
    for (const RangeLock* lock : allLocks) {
        for (MeasureBase* mb = lock->startMB(); mb; mb = mb->next()) {
            if (mb->isMeasure() && toMeasure(mb)->mmRest()) {
                undoRemoveSystemLock(tx, score, lock);
                break;
            }
            if (mb->isAfter(lock->endMB())) {
                break;
            }
        }
    }
}

void EditSystemLocks::updateSystemLocksOnCreateMMRests(Transaction& tx, Score* score, Measure* first, Measure* last)
{
    // NOTE: this must be done during layout as the mmRests get created.

    for (const RangeLock* lock : score->systemLocks()->locksContainedInRange(first, last)) {
        // These locks are inside the range of the mmRest so remove them
        undoRemoveSystemLock(tx, score, lock);
    }

    const RangeLock* lockOnFirst = score->systemLocks()->lockContaining(first);
    const RangeLock* lockOnLast = score->systemLocks()->lockContaining(last);

    if (lockOnFirst) {
        MeasureBase* startMB = lockOnFirst->startMB();
        MeasureBase* endMB = lockOnFirst->endMB();

        if (startMB->isBefore(first)) {
            if (endMB->isBeforeOrEqual(last)) {
                endMB = first->mmRest();
            } else {
                return;
            }
        } else {
            startMB = first->mmRest();
        }

        if (startMB != lockOnFirst->startMB() || endMB != lockOnFirst->endMB()) {
            undoRemoveSystemLock(tx, score, lockOnFirst);
            undoAddSystemLock(tx, score, new RangeLock(startMB, endMB));
        }
    }

    if (!lockOnLast || lockOnLast == lockOnFirst) {
        return;
    }

    MeasureBase* startMB = lockOnLast->startMB();
    MeasureBase* endMB = lockOnLast->endMB();
    assert(startMB->isAfter(first) && endMB->isAfter(last));

    undoRemoveSystemLock(tx, score, lockOnLast);
    startMB = last->nextMM();
    undoAddSystemLock(tx, score, new RangeLock(startMB, endMB));
}
