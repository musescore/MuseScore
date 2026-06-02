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

#include "editpagelocks.h"

#include "transaction/transaction.h"
#include "transaction/undoablecommand.h"

#include "../dom/measurebase.h"
#include "../dom/page.h"
#include "../dom/score.h"
#include "../dom/rangelock.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   AddPageLock
//---------------------------------------------------------

class AddPageLock : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, AddPageLock)

    const RangeLock* m_pageLock;
public:
    AddPageLock(const RangeLock* pageLock)
        : m_pageLock(pageLock) {}

    void undo(EditData*) override
    {
        Score* score = m_pageLock->startMB()->score();
        score->removePageLock(m_pageLock);
    }

    void redo(EditData*) override
    {
        Score* score = m_pageLock->startMB()->score();
        score->addPageLock(m_pageLock);
    }

    void cleanup(bool undo) override
    {
        if (!undo) {
            delete m_pageLock;
            m_pageLock = nullptr;
        }
    }

    UNDO_NAME("AddPageLock")

    std::vector<EngravingObject*> objectItems() const override
    {
        return { m_pageLock->startMB(), m_pageLock->endMB() };
    }
};

//---------------------------------------------------------
//   RemovePageLock
//---------------------------------------------------------

class RemovePageLock : public UndoableCommand
{
    OBJECT_ALLOCATOR(engraving, RemovePageLock)

    const RangeLock* m_pageLock;
public:
    RemovePageLock(const RangeLock* pageLock)
        : m_pageLock(pageLock) {}

    void undo(EditData*) override
    {
        Score* score = m_pageLock->startMB()->score();
        score->addPageLock(m_pageLock);
    }

    void redo(EditData*) override
    {
        Score* score = m_pageLock->startMB()->score();
        score->removePageLock(m_pageLock);
    }

    void cleanup(bool undo) override
    {
        if (undo) {
            delete m_pageLock;
            m_pageLock = nullptr;
        }
    }

    UNDO_NAME("RemovePageLock")
    std::vector<EngravingObject*> objectItems() const override
    {
        return { m_pageLock->startMB(), m_pageLock->endMB() };
    }
};

//---------------------------------------------------------
//   EditPageLocks
//---------------------------------------------------------

void EditPageLocks::undoAddPageLock(Transaction& tx, Score* score, const RangeLock* lock)
{
    removeLayoutBreaksOnAddPageLock(tx, score, lock);
    score->undo(new AddPageLock(lock));
}

void EditPageLocks::undoRemovePageLock(Transaction& tx, Score* score, const RangeLock* lock)
{
    tx.push(new RemovePageLock(lock));
}

void EditPageLocks::undoRemoveAllLocks(Transaction& tx, Score* score)
{
    std::vector<const RangeLock*> allLocks = score->pageLocks()->allLocks(); // copy
    for (const RangeLock* lock : allLocks) {
        undoRemovePageLock(tx, score, lock);
    }
}

void EditPageLocks::togglePageLock(Transaction& tx, Score* score, const std::vector<Page*>& pages)
{
    bool unlockAll = true;
    for (const Page* page : pages) {
        if (!page->isLocked()) {
            unlockAll = false;
            break;
        }
    }

    for (Page* page : pages) {
        MeasureBase* startMeas = page->firstMeasureBase();
        const RangeLock* currentLock = score->pageLocks()->lockStartingAt(startMeas);
        if (currentLock && unlockAll) {
            undoRemovePageLock(tx, score, currentLock);
            continue;
        } else if (!currentLock && !unlockAll) {
            RangeLock* newPageLock = new RangeLock(startMeas, page->lastMeasureBase());
            undoAddPageLock(tx, score, newPageLock);
        }
    }
}

void EditPageLocks::toggleScoreLock(Transaction& tx, Score* score)
{
    bool unlockAll = true;
    for (const Page* page : score->pages()) {
        if (!page->isLocked()) {
            unlockAll = false;
            break;
        }
    }

    for (Page* page : score->pages()) {
        MeasureBase* startMeas = page->firstMeasureBase();
        const RangeLock* currentLock = score->pageLocks()->lockStartingAt(startMeas);
        if (currentLock && unlockAll) {
            undoRemovePageLock(tx, score, currentLock);
            continue;
        } else if (!currentLock && !unlockAll) {
            RangeLock* newPageLock = new RangeLock(startMeas, page->lastMeasureBase());
            undoAddPageLock(tx, score, newPageLock);
        }
    }
}

void EditPageLocks::addRemovePageLocks(Transaction& tx, Score* score, int interval, bool lock)
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
        for (const Page* page : score->pages()) {
            if (page->lastMeasureBase()->isBefore(startMeasure)) {
                continue;
            }
            if (page->firstMeasureBase()->isAfter(endMeasure)) {
                break;
            }
            if (!page->isLocked()) {
                undoAddPageLock(tx, score, new RangeLock(page->firstMeasureBase(), page->lastMeasureBase()));
            }
        }
        return;
    }

    std::vector<const RangeLock*> currentLocks = score->pageLocks()->locksContainedInRange(startMeasure, endMeasure);
    for (const RangeLock* l : currentLocks) {
        undoRemovePageLock(tx, score, l);
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
            undoAddPageLock(tx, score, new RangeLock(lockStart, mb));
            lockStart = nullptr;
            count = 0;
        }
        if (mb == endMeasure) {
            break;
        }
    }
}

void EditPageLocks::makeIntoPage(Transaction& tx, Score* score, MeasureBase* first, MeasureBase* last)
{
    const RangeLock* lockContainingfirst = score->pageLocks()->lockContaining(first);
    const RangeLock* lockContaininglast = score->pageLocks()->lockContaining(last);

    if (lockContainingfirst) {
        undoRemovePageLock(tx, score, lockContainingfirst);
        if (lockContainingfirst->startMB()->isBefore(first)) {
            MeasureBase* oneBeforeFirst = first->prevMM();
            RangeLock* newLockBefore = new RangeLock(lockContainingfirst->startMB(), oneBeforeFirst);
            undoAddPageLock(tx, score, newLockBefore);
        }
    }

    if (lockContaininglast) {
        if (lockContaininglast != lockContainingfirst) {
            undoRemovePageLock(tx, score, lockContaininglast);
        }
        if (last->isBefore(lockContaininglast->endMB())) {
            MeasureBase* oneAfterLast = last->nextMM();
            RangeLock* newLockAfter = new RangeLock(oneAfterLast, lockContaininglast->endMB());
            undoAddPageLock(tx, score, newLockAfter);
        }
    }

    std::vector<const RangeLock*> locksContainedInRange = score->pageLocks()->locksContainedInRange(first, last);
    for (const RangeLock* lock : locksContainedInRange) {
        if (lock != lockContainingfirst && lock != lockContaininglast) {
            undoRemovePageLock(tx, score, lock);
        }
    }

    RangeLock* newLock = new RangeLock(first, last);
    undoAddPageLock(tx, score, newLock);
}

void EditPageLocks::moveMeasureToPrevPage(Transaction& tx, Score* score, MeasureBase* m)
{
    const Page* prevPage = m->prevPage();
    if (!prevPage) {
        return;
    }

    MeasureBase* prevPageFirstMeas = prevPage->firstMeasureBase();

    const RangeLock* prevPageLock = score->pageLocks()->lockStartingAt(prevPageFirstMeas);
    if (prevPageLock) {
        undoRemovePageLock(tx, score, prevPageLock);
    }

    const Page* curPage = m->page();
    const RangeLock* curPageLock = score->pageLocks()->lockStartingAt(curPage->firstMeasureBase());
    if (curPageLock) {
        undoRemovePageLock(tx, score, curPageLock);
        if (curPageLock->endMB() != m) {
            MeasureBase* nextMB = m->nextMM();
            RangeLock* newLockOnCurPage = new RangeLock(nextMB, curPageLock->endMB());
            undoAddPageLock(tx, score, newLockOnCurPage);
        }
    }

    RangeLock* pageLock = new RangeLock(prevPageFirstMeas, m);
    undoAddPageLock(tx, score, pageLock);
}

void EditPageLocks::moveMeasureToNextPage(Transaction& tx, Score* score, MeasureBase* m)
{
    const Page* curPage = m->page();
    MeasureBase* startMeas = curPage->firstMeasureBase();
    bool refMeasureIsStartOfPage = m == startMeas;

    const RangeLock* curLock = score->pageLocks()->lockStartingAt(startMeas);
    if (curLock) {
        undoRemovePageLock(tx, score, curLock);
    }

    if (!refMeasureIsStartOfPage) {
        MeasureBase* prevMeas = m->prevMM();
        RangeLock* pageLock = new RangeLock(startMeas, prevMeas);
        undoAddPageLock(tx, score, pageLock);
    }

    const Page* nextPage = m->nextPage();
    if (!nextPage) {
        return;
    }

    const RangeLock* nextPageLock = score->pageLocks()->lockStartingAt(nextPage->firstMeasureBase());
    if (nextPageLock) {
        undoRemovePageLock(tx, score, nextPageLock);
    }

    if (nextPageLock || refMeasureIsStartOfPage) {
        RangeLock* newNextPageLock = new RangeLock(m, nextPage->lastMeasureBase());
        undoAddPageLock(tx, score, newNextPageLock);
    }
}

void EditPageLocks::applyLockToSelection(Transaction& tx, Score* score)
{
    MeasureBase* first = nullptr;
    MeasureBase* last = nullptr;

    if (score->selection().isRange()) {
        first = score->selection().startMeasureBase();
        last = score->selection().endMeasureBase();
    } else {
        for (EngravingItem* el : score->selection().elements()) {
            if (el->isPageLockIndicator()) {
                const RangeLock* lock = toPageLockIndicator(el)->pageLock();
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

    const RangeLock* lockOnLast = score->pageLocks()->lockContaining(last);
    if (lockOnLast && lockOnLast->endMB() == last) {
        undoRemovePageLock(tx, score, lockOnLast);
    } else if (first != last) {
        makeIntoPage(tx, score, first, last);
    } else {
        makeIntoPage(tx, score, first->page()->firstMeasureBase(), last);
    }
}

void EditPageLocks::removePageLocksOnAddLayoutBreak(Transaction& tx, Score* score, LayoutBreakType breakType, const MeasureBase* measure)
{
    IF_ASSERT_FAILED(breakType != LayoutBreakType::NOBREAK) {
        return; // NOBREAK not allowed on locked measures
    }

    const RangeLock* lock = score->pageLocks()->lockContaining(measure);
    if (lock && (breakType == LayoutBreakType::PAGE || measure != lock->endMB())) {
        undoRemovePageLock(tx, score, lock);
    }
}

void EditPageLocks::removeLayoutBreaksOnAddPageLock(Transaction&, Score* score, const RangeLock* lock)
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

void EditPageLocks::removePageLocksOnRemoveMeasures(Transaction& tx, Score* score, const MeasureBase* m1, const MeasureBase* m2)
{
    std::vector<const RangeLock*> allPageLocks = score->pageLocks()->allLocks();
    for (const RangeLock* lock : allPageLocks) {
        MeasureBase* lockStart = lock->startMB();
        MeasureBase* lockEnd = lock->endMB();
        bool lockStartIsInRange = lockStart->isAfterOrEqual(m1) && lockStart->isBeforeOrEqual(m2);
        bool lockEndIsInRange = lockEnd->isAfterOrEqual(m1) && lockEnd->isBeforeOrEqual(m2);
        if (lockStartIsInRange || lockEndIsInRange) {
            undoRemovePageLock(tx, score, lock);
        }
        if (lockStartIsInRange && !lockEndIsInRange) {
            MeasureBase* newLockStart = m2->nextMeasure();
            if (newLockStart) {
                undoAddPageLock(tx, score, new RangeLock(newLockStart, lockEnd));
            }
        } else if (!lockStartIsInRange && lockEndIsInRange) {
            MeasureBase* newLockEnd = m1->prevMeasure();
            if (newLockEnd) {
                undoAddPageLock(tx, score, new RangeLock(lockStart, newLockEnd));
            }
        }
    }
}

void EditPageLocks::removePageLocksContainingMMRests(Transaction& tx, Score* score)
{
    std::vector<const RangeLock*> allLocks = score->pageLocks()->allLocks(); // copy
    for (const RangeLock* lock : allLocks) {
        for (MeasureBase* mb = lock->startMB(); mb; mb = mb->next()) {
            if (mb->isMeasure() && toMeasure(mb)->mmRest()) {
                undoRemovePageLock(tx, score, lock);
                break;
            }
            if (mb->isAfter(lock->endMB())) {
                break;
            }
        }
    }
}

void EditPageLocks::updatePageLocksOnCreateMMRests(Transaction& tx, Score* score, Measure* first, Measure* last)
{
    // NOTE: this must be done during layout as the mmRests get created.

    for (const RangeLock* lock : score->pageLocks()->locksContainedInRange(first, last)) {
        // These locks are inside the range of the mmRest so remove them
        undoRemovePageLock(tx, score, lock);
    }

    const RangeLock* lockOnFirst = score->pageLocks()->lockContaining(first);
    const RangeLock* lockOnLast = score->pageLocks()->lockContaining(last);

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
            undoRemovePageLock(tx, score, lockOnFirst);
            undoAddPageLock(tx, score, new RangeLock(startMB, endMB));
        }
    }

    if (!lockOnLast || lockOnLast == lockOnFirst) {
        return;
    }

    MeasureBase* startMB = lockOnLast->startMB();
    MeasureBase* endMB = lockOnLast->endMB();
    assert(startMB->isAfter(first) && endMB->isAfter(last));

    undoRemovePageLock(tx, score, lockOnLast);
    startMB = last->nextMM();
    undoAddPageLock(tx, score, new RangeLock(startMB, endMB));
}
