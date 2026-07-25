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

#include "measure.h"
#include "measurebase.h"
#include "rangelock.h"

#include "log.h"

namespace mu::engraving {
bool RangeLock::contains(const MeasureBase* mb) const
{
    return m_startMB->isBeforeOrEqual(mb) && mb->isBeforeOrEqual(m_endMB);
}

void RangeLocks::add(const RangeLock* lock)
{
    m_rangeLocks.emplace(lock->startMB(), lock);

#ifndef NDEBUG
    sanityCheck();
#endif
}

void RangeLocks::remove(const RangeLock* lock)
{
    m_rangeLocks.erase(lock->startMB());

#ifndef NDEBUG
    sanityCheck();
#endif
}

const RangeLock* RangeLocks::lockStartingAt(const MeasureBase* mb) const
{
    auto iter = m_rangeLocks.find(mb);
    return iter != m_rangeLocks.end() ? iter->second : nullptr;
}

const RangeLock* RangeLocks::lockContaining(const MeasureBase* mb) const
{
    if (m_rangeLocks.empty()) {
        return nullptr;
    }

    auto iter = m_rangeLocks.lower_bound(mb);
    if (iter != m_rangeLocks.begin()
        && (iter == m_rangeLocks.end() || mb->isBefore(iter->second->startMB()))) {
        --iter;
    }

    const RangeLock* lock = iter->second;

    return lock->contains(mb) ? lock : nullptr;
}

std::vector<const RangeLock*> RangeLocks::locksContainedInRange(const MeasureBase* start, const MeasureBase* end) const
{
    std::vector<const RangeLock*> result;

    for (auto& pair : m_rangeLocks) {
        const RangeLock* lock = pair.second;
        if (start->isBeforeOrEqual(lock->startMB()) && lock->endMB()->isBeforeOrEqual(end)) {
            result.push_back(lock);
        }
        if (lock->startMB()->isAfter(end)) {
            break;
        }
    }

    return result;
}

std::vector<const RangeLock*> RangeLocks::allLocks() const
{
    return muse::values(m_rangeLocks);
}

#ifndef NDEBUG
void RangeLocks::sanityCheck()
{
    for (auto iter = m_rangeLocks.begin(); iter != m_rangeLocks.end();) {
        auto curIter = iter;
        auto nextIter = ++iter;
        if (nextIter == m_rangeLocks.end()) {
            break;
        }

        const MeasureBase* curMB = curIter->first;
        const RangeLock* curLock = curIter->second;

        DO_ASSERT(curLock->startMB() == curMB);

        const MeasureBase* nextMB = nextIter->first;
        const RangeLock* nextLock = nextIter->second;

        DO_ASSERT(nextLock->startMB() == nextMB);
        DO_ASSERT(curMB->isBefore(nextMB));
        DO_ASSERT(curLock->endMB()->isBefore(nextLock->startMB()));
    }
}

void RangeLocks::dump() const
{
    for (auto& pair : m_rangeLocks) {
        const RangeLock* sl = pair.second;
        const Measure* startMeasure = sl->startMB()->isMeasure() ? toMeasure(sl->startMB()) : sl->startMB()->prevMeasure();
        const Measure* endMeasure = sl->endMB()->isMeasure() ? toMeasure(sl->endMB()) : sl->endMB()->prevMeasure();
        LOGD() << "RangeLock --- Start measure: " << (startMeasure ? startMeasure->measureNumber() : -1)
               << ", End Measure: " << (endMeasure ? endMeasure->measureNumber() : -1);
    }
}

#endif
} // namespace mu::engraving
