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

#include "measure.h"
#include "measurebase.h"
#include "page.h"
#include "score.h"
#include "system.h"
#include "systemlock.h"

#include "log.h"

using namespace muse::draw;

namespace mu::engraving {
bool SystemLock::contains(const MeasureBase* mb) const
{
    return m_startMB->isBeforeOrEqual(mb) && mb->isBeforeOrEqual(m_endMB);
}

void SystemLocks::add(const SystemLock* lock)
{
    m_systemLocks.emplace(lock->startMB(), lock);

#ifndef NDEBUG
    sanityCheck();
#endif
}

void SystemLocks::remove(const SystemLock* lock)
{
    m_systemLocks.erase(lock->startMB());

#ifndef NDEBUG
    sanityCheck();
#endif
}

const SystemLock* SystemLocks::lockStartingAt(const MeasureBase* mb) const
{
    auto iter = m_systemLocks.find(mb);
    return iter != m_systemLocks.end() ? iter->second : nullptr;
}

const SystemLock* SystemLocks::lockContaining(const MeasureBase* mb) const
{
    if (m_systemLocks.empty()) {
        return nullptr;
    }

    auto iter = m_systemLocks.lower_bound(mb);
    if (iter != m_systemLocks.begin()
        && (iter == m_systemLocks.end() || mb->isBefore(iter->second->startMB()))) {
        --iter;
    }

    const SystemLock* lock = iter->second;

    return lock->contains(mb) ? lock : nullptr;
}

std::vector<const SystemLock*> SystemLocks::locksContainedInRange(const MeasureBase* start, const MeasureBase* end) const
{
    std::vector<const SystemLock*> result;

    for (auto& pair : m_systemLocks) {
        const SystemLock* lock = pair.second;
        if (start->isBeforeOrEqual(lock->startMB()) && lock->endMB()->isBeforeOrEqual(end)) {
            result.push_back(lock);
        }
        if (lock->startMB()->isAfter(end)) {
            break;
        }
    }

    return result;
}

std::vector<const SystemLock*> SystemLocks::allLocks() const
{
    std::vector <const SystemLock* > locks;
    locks.reserve(m_systemLocks.size());
    for (auto& pair : m_systemLocks) {
        locks.push_back(pair.second);
    }
    return locks;
}

#ifndef NDEBUG
void SystemLocks::sanityCheck()
{
    for (auto iter = m_systemLocks.begin(); iter != m_systemLocks.end();) {
        auto curIter = iter;
        auto nextIter = ++iter;
        if (nextIter == m_systemLocks.end()) {
            break;
        }

        const MeasureBase* curMB = curIter->first;
        const SystemLock* curSysLock = curIter->second;

        DO_ASSERT(curSysLock->startMB() == curMB);

        const MeasureBase* nextMB = nextIter->first;
        const SystemLock* nextSysLock = nextIter->second;

        DO_ASSERT(nextSysLock->startMB() == nextMB);
        DO_ASSERT(curMB->isBefore(nextMB));
        DO_ASSERT(curSysLock->endMB()->isBefore(nextSysLock->startMB()));
    }
}

void SystemLocks::dump()
{
    for (auto& pair : m_systemLocks) {
        const SystemLock* sl = pair.second;
        const Measure* startMeasure = sl->startMB()->isMeasure() ? toMeasure(sl->startMB()) : sl->startMB()->prevMeasure();
        const Measure* endMeasure = sl->endMB()->isMeasure() ? toMeasure(sl->endMB()) : sl->endMB()->prevMeasure();
        LOGD() << "SystemLock --- Start measure: " << (startMeasure ? startMeasure->measureNumber() : -1)
               << ", End Measure: " << (endMeasure ? endMeasure->measureNumber() : -1);
    }
}

#endif

SystemLockIndicator::SystemLockIndicator(System* parent, const SystemLock* lock)
    : IndicatorIcon(ElementType::SYSTEM_LOCK_INDICATOR, parent, ElementFlag::SYSTEM | ElementFlag::GENERATED), m_systemLock(lock) {}

void SystemLockIndicator::setSelected(bool v)
{
    EngravingItem::setSelected(v);
    renderer()->layoutItem(this);
    system()->page()->invalidateBspTree();
}

String SystemLockIndicator::formatBarsAndBeats() const
{
    const MeasureBase* startMB = m_systemLock->startMB();
    const MeasureBase* endMB = m_systemLock->endMB();
    const Measure* startMeasure = startMB->isMeasure() ? toMeasure(startMB) : toMeasure(startMB->prevMeasure());
    const Measure* endMeasure = endMB->isMeasure() ? toMeasure(endMB) : toMeasure(endMB->prevMeasure());
    const int startMeasureNum = startMeasure ? startMeasure->measureNumber() : -1;
    const int endMeasureNum = endMeasure ? endMeasure->measureNumber() : -1;
    return muse::mtrc("engraving", "Start measure: %1; End measure: %2").arg(startMeasureNum).arg(endMeasureNum);
}
} // namespace mu::engraving
