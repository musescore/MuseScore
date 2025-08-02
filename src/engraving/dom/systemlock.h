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

#pragma once

#include <map>

#include "indicatoricon.h"
#include "measurebase.h"

namespace mu::engraving {
class SystemLock
{
public:
    SystemLock(MeasureBase* start, MeasureBase* end)
        : m_startMB(start), m_endMB(end)
    {
        assert(m_startMB->isMeasure() || m_startMB->isHBox());
        assert(m_endMB->isMeasure() || m_endMB->isHBox());
        assert(m_startMB->isBefore(m_endMB) || m_startMB == m_endMB);
    }

    MeasureBase* startMB() const { return m_startMB; }
    MeasureBase* endMB() const { return m_endMB; }

    bool contains(const MeasureBase* mb) const;

private:
    MeasureBase* m_startMB;
    MeasureBase* m_endMB;
};

class SystemLocks
{
public:
    void add(const SystemLock* lock);
    void remove(const SystemLock* lock);
    void removeLockStartingAt(const MeasureBase* mb) { m_systemLocks.erase(mb); }
    void clear() { m_systemLocks.clear(); }

    const SystemLock* lockStartingAt(const MeasureBase* mb) const;
    const SystemLock* lockContaining(const MeasureBase* mb) const;
    std::vector<const SystemLock*> locksContainedInRange(const MeasureBase* start, const MeasureBase* end) const;

    std::vector<const SystemLock*> allLocks() const;

private:
#ifndef NDEBUG
    void sanityCheck();
    void dump();
#endif

    struct Ordering
    {
        bool operator()(const MeasureBase* a, const MeasureBase* b) const
        {
            return a->isBefore(b);
        }
    };

    std::map<const MeasureBase*, const SystemLock*, Ordering> m_systemLocks;
};

class SystemLockIndicator : public IndicatorIcon
{
    OBJECT_ALLOCATOR(engraving, SystemLockIndicator)
    DECLARE_CLASSOF(ElementType::SYSTEM_LOCK_INDICATOR)

public:
    SystemLockIndicator(System* parent, const SystemLock* lock);

    void setSelected(bool v) override;

    const SystemLock* systemLock() const { return m_systemLock; }

    char16_t iconCode() const override { return 0xF487; }

    String formatBarsAndBeats() const override;

private:
    const SystemLock* m_systemLock = nullptr;
};
} // namespace mu::engraving
