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

#include "editsystemlocks.h"

#include "undo.h"

#include "../dom/measurebase.h"
#include "../dom/score.h"
#include "../dom/system.h"
#include "../dom/systemlock.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   AddSystemLock
//---------------------------------------------------------

class AddSystemLock : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, AddSystemLock)

    const SystemLock* m_systemLock;
public:
    AddSystemLock(const SystemLock* systemLock)
        : m_systemLock(systemLock) {}

    void undo(EditData*) override
    {
        Score* score = m_systemLock->startMB()->score();
        score->removeSystemLock(m_systemLock);
    }

    void redo(EditData*) override
    {
        Score* score = m_systemLock->startMB()->score();
        score->addSystemLock(m_systemLock);
    }

    void cleanup(bool undo) override
    {
        if (!undo) {
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

class RemoveSystemLock : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, RemoveSystemLock)

    const SystemLock* m_systemLock;
public:
    RemoveSystemLock(const SystemLock* systemLock)
        : m_systemLock(systemLock) {}

    void undo(EditData*) override
    {
        Score* score = m_systemLock->startMB()->score();
        score->addSystemLock(m_systemLock);
    }

    void redo(EditData*) override
    {
        Score* score = m_systemLock->startMB()->score();
        score->removeSystemLock(m_systemLock);
    }

    void cleanup(bool undo) override
    {
        if (undo) {
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

void EditSystemLocks::undoAddSystemLock(Score* score, const SystemLock* lock)
{
    removeLayoutBreaksOnAddSystemLock(score, lock);
    score->undo(new AddSystemLock(lock));
}

void EditSystemLocks::undoRemoveSystemLock(Score* score, const SystemLock* lock)
{
    score->undo(new RemoveSystemLock(lock));
}

void EditSystemLocks::undoRemoveAllLocks(Score* score)
{
    std::vector<const SystemLock*> allLocks = score->systemLocks()->allLocks(); // copy
    for (const SystemLock* lock : allLocks) {
        undoRemoveSystemLock(score, lock);
    }
}

void EditSystemLocks::toggleSystemLock(Score* score, const std::vector<System*>& systems)
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
        const SystemLock* currentLock = score->systemLocks()->lockStartingAt(startMeas);
        if (currentLock && unlockAll) {
            undoRemoveSystemLock(score, currentLock);
            continue;
        } else if (!currentLock && !unlockAll) {
            SystemLock* newSystemLock = new SystemLock(startMeas, system->last());
            undoAddSystemLock(score, newSystemLock);
        }
    }
}

void EditSystemLocks::toggleScoreLock(Score* score)
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
        const SystemLock* currentLock = score->systemLocks()->lockStartingAt(startMeas);
        if (currentLock && unlockAll) {
            undoRemoveSystemLock(score, currentLock);
            continue;
        } else if (!currentLock && !unlockAll) {
            SystemLock* newSystemLock = new SystemLock(startMeas, system->last());
            undoAddSystemLock(score, newSystemLock);
        }
    }
}

void EditSystemLocks::addRemoveSystemLocks(Score* score, int interval, bool lock)
{
    const bool mmrests = score->style().styleB(Sid::createMultiMeasureRests);

    MeasureBase* startMeasure = score->selection().startMeasureBase();
    MeasureBase* endMeasure = score->selection().endMeasureBase();
    if (!endMeasure) {
        endMeasure = mmrests ? score->lastMeasureMM() : score->lastMeasure();
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
                undoAddSystemLock(score, new SystemLock(system->first(), system->last()));
            }
        }
        return;
    }

    std::vector<const SystemLock*> currentLocks = score->systemLocks()->locksContainedInRange(startMeasure, endMeasure);
    for (const SystemLock* l : currentLocks) {
        undoRemoveSystemLock(score, l);
    }

    if (interval == 0) {
        return;
    }

    int count = 0;
    MeasureBase* lockStart = nullptr;
    for (MeasureBase* mb = startMeasure; mb; mb = mmrests ? mb->nextMM() : mb->next()) {
        if (count == 0) {
            lockStart = mb;
        }
        count++;
        if (count == interval || mb == endMeasure) {
            undoAddSystemLock(score, new SystemLock(lockStart, mb));
            lockStart = nullptr;
            count = 0;
        }
        if (mb == endMeasure) {
            break;
        }
    }
}

void EditSystemLocks::makeIntoSystem(Score* score, MeasureBase* first, MeasureBase* last)
{
    bool mmrests = score->style().styleB(Sid::createMultiMeasureRests);

    const SystemLock* lockContainingfirst = score->systemLocks()->lockContaining(first);
    const SystemLock* lockContaininglast = score->systemLocks()->lockContaining(last);

    if (lockContainingfirst) {
        undoRemoveSystemLock(score, lockContainingfirst);
        if (lockContainingfirst->startMB()->isBefore(first)) {
            MeasureBase* oneBeforeFirst = mmrests ? first->prevMM() : first->prev();
            SystemLock* newLockBefore = new SystemLock(lockContainingfirst->startMB(), oneBeforeFirst);
            undoAddSystemLock(score, newLockBefore);
        }
    }

    if (lockContaininglast) {
        if (lockContaininglast != lockContainingfirst) {
            undoRemoveSystemLock(score, lockContaininglast);
        }
        if (last->isBefore(lockContaininglast->endMB())) {
            MeasureBase* oneAfterLast = mmrests ? last->nextMM() : last->next();
            SystemLock* newLockAfter = new SystemLock(oneAfterLast, lockContaininglast->endMB());
            undoAddSystemLock(score, newLockAfter);
        }
    }

    std::vector<const SystemLock*> locksContainedInRange = score->systemLocks()->locksContainedInRange(first, last);
    for (const SystemLock* lock : locksContainedInRange) {
        if (lock != lockContainingfirst && lock != lockContaininglast) {
            undoRemoveSystemLock(score, lock);
        }
    }

    SystemLock* newLock = new SystemLock(first, last);
    undoAddSystemLock(score, newLock);
}

void EditSystemLocks::moveMeasureToPrevSystem(Score* score, MeasureBase* m)
{
    const System* prevSystem = m->prevNonVBoxSystem();
    if (!prevSystem) {
        return;
    }

    MeasureBase* prevSystemFirstMeas = prevSystem->first();

    const SystemLock* prevSystemLock = score->systemLocks()->lockStartingAt(prevSystemFirstMeas);
    if (prevSystemLock) {
        undoRemoveSystemLock(score, prevSystemLock);
    }

    const System* curSystem = m->system();
    const SystemLock* curSystemLock = score->systemLocks()->lockStartingAt(curSystem->first());
    if (curSystemLock) {
        undoRemoveSystemLock(score, curSystemLock);
        if (curSystemLock->endMB() != m) {
            const bool mmrests = score->style().styleB(Sid::createMultiMeasureRests);
            MeasureBase* nextMB = mmrests ? m->nextMM() : m->next();
            SystemLock* newLockOnCurSystem = new SystemLock(nextMB, curSystemLock->endMB());
            undoAddSystemLock(score, newLockOnCurSystem);
        }
    }

    SystemLock* sysLock = new SystemLock(prevSystemFirstMeas, m);
    undoAddSystemLock(score, sysLock);
}

void EditSystemLocks::moveMeasureToNextSystem(Score* score, MeasureBase* m)
{
    const System* curSystem = m->system();
    MeasureBase* startMeas = curSystem->first();
    bool refMeasureIsStartOfSystem = m == startMeas;

    const SystemLock* curLock = score->systemLocks()->lockStartingAt(startMeas);
    if (curLock) {
        undoRemoveSystemLock(score, curLock);
    }

    if (!refMeasureIsStartOfSystem) {
        bool mmrests = score->style().styleB(Sid::createMultiMeasureRests);
        MeasureBase* prevMeas = mmrests ? m->prevMM() : m->prev();
        SystemLock* sysLock = new SystemLock(startMeas, prevMeas);
        undoAddSystemLock(score, sysLock);
    }

    const System* nextSystem = m->nextNonVBoxSystem();
    if (!nextSystem) {
        return;
    }

    const SystemLock* nextSysLock = score->systemLocks()->lockStartingAt(nextSystem->first());
    if (nextSysLock) {
        undoRemoveSystemLock(score, nextSysLock);
    }

    if (nextSysLock || refMeasureIsStartOfSystem) {
        SystemLock* newNextSysLock = new SystemLock(m, nextSystem->last());
        undoAddSystemLock(score, newNextSysLock);
    }
}

void EditSystemLocks::applyLockToSelection(Score* score)
{
    MeasureBase* first = nullptr;
    MeasureBase* last = nullptr;

    if (score->selection().isRange()) {
        first = score->selection().startMeasureBase();
        last = score->selection().endMeasureBase();
    } else {
        for (EngravingItem* el : score->selection().elements()) {
            if (el->isSystemLockIndicator()) {
                const SystemLock* lock = toSystemLockIndicator(el)->systemLock();
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

    const SystemLock* lockOnLast = score->systemLocks()->lockContaining(last);
    if (lockOnLast && lockOnLast->endMB() == last) {
        undoRemoveSystemLock(score, lockOnLast);
    } else if (first != last) {
        makeIntoSystem(score, first, last);
    } else {
        makeIntoSystem(score, first->system()->first(), last);
    }
}

void EditSystemLocks::removeSystemLocksOnAddLayoutBreak(Score* score, LayoutBreakType breakType, const MeasureBase* measure)
{
    IF_ASSERT_FAILED(breakType != LayoutBreakType::NOBREAK) {
        return; // NOBREAK not allowed on locked measures
    }

    const SystemLock* lock = score->systemLocks()->lockContaining(measure);
    if (lock && (breakType == LayoutBreakType::LINE || measure != lock->endMB())) {
        undoRemoveSystemLock(score, lock);
    }
}

void EditSystemLocks::removeLayoutBreaksOnAddSystemLock(Score* score, const SystemLock* lock)
{
    bool mmrests = score->style().styleB(Sid::createMultiMeasureRests);
    for (MeasureBase* mb = lock->startMB(); mb && mb->isBeforeOrEqual(lock->endMB()); mb = mmrests ? mb->nextMM() : mb->next()) {
        mb->undoSetBreak(false, LayoutBreakType::LINE);
        mb->undoSetBreak(false, LayoutBreakType::NOBREAK);
        if (mb != lock->endMB()) {
            mb->undoSetBreak(false, LayoutBreakType::SECTION);
            mb->undoSetBreak(false, LayoutBreakType::PAGE);
        }
    }
}

void EditSystemLocks::removeSystemLocksOnRemoveMeasures(Score* score, const MeasureBase* m1, const MeasureBase* m2)
{
    std::vector<const SystemLock*> allSysLocks = score->systemLocks()->allLocks();
    for (const SystemLock* lock : allSysLocks) {
        MeasureBase* lockStart = lock->startMB();
        MeasureBase* lockEnd = lock->endMB();
        bool lockStartIsInRange = lockStart->isAfterOrEqual(m1) && lockStart->isBeforeOrEqual(m2);
        bool lockEndIsInRange = lockEnd->isAfterOrEqual(m1) && lockEnd->isBeforeOrEqual(m2);
        if (lockStartIsInRange || lockEndIsInRange) {
            undoRemoveSystemLock(score, lock);
        }
        if (lockStartIsInRange && !lockEndIsInRange) {
            MeasureBase* newLockStart = m2->nextMeasure();
            if (newLockStart) {
                undoAddSystemLock(score, new SystemLock(newLockStart, lockEnd));
            }
        } else if (!lockStartIsInRange && lockEndIsInRange) {
            MeasureBase* newLockEnd = m1->prevMeasure();
            if (newLockEnd) {
                undoAddSystemLock(score, new SystemLock(lockStart, newLockEnd));
            }
        }
    }
}

void EditSystemLocks::removeSystemLocksContainingMMRests(Score* score)
{
    std::vector<const SystemLock*> allLocks = score->systemLocks()->allLocks(); // copy
    for (const SystemLock* lock : allLocks) {
        for (MeasureBase* mb = lock->startMB(); mb; mb = mb->next()) {
            if (mb->isMeasure() && toMeasure(mb)->mmRest()) {
                undoRemoveSystemLock(score, lock);
                break;
            }
            if (mb->isAfter(lock->endMB())) {
                break;
            }
        }
    }
}

void EditSystemLocks::updateSystemLocksOnCreateMMRests(Score* score, Measure* first, Measure* last)
{
    // NOTE: this must be done during layout as the mmRests get created.

    for (const SystemLock* lock : score->systemLocks()->locksContainedInRange(first, last)) {
        // These locks are inside the range of the mmRest so remove them
        undoRemoveSystemLock(score, lock);
    }

    const SystemLock* lockOnFirst = score->systemLocks()->lockContaining(first);
    const SystemLock* lockOnLast = score->systemLocks()->lockContaining(last);

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
            undoRemoveSystemLock(score, lockOnFirst);
            undoAddSystemLock(score, new SystemLock(startMB, endMB));
        }
    }

    if (!lockOnLast || lockOnLast == lockOnFirst) {
        return;
    }

    MeasureBase* startMB = lockOnLast->startMB();
    MeasureBase* endMB = lockOnLast->endMB();
    assert(startMB->isAfter(first) && endMB->isAfter(last));

    undoRemoveSystemLock(score, lockOnLast);
    startMB = last->nextMM();
    undoAddSystemLock(score, new SystemLock(startMB, endMB));
}
