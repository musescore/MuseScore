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

#include "pagelockindicator.h"

#include "measure.h"
#include "measurebase.h"
#include "page.h"
#include "rangelock.h"
#include "system.h"

namespace mu::engraving {
PageLockIndicator::PageLockIndicator(System* parent, const RangeLock* lock)
    : IndicatorIcon(ElementType::PAGE_LOCK_INDICATOR, parent, ElementFlag::SYSTEM | ElementFlag::GENERATED), m_pageLock(lock) {}

void PageLockIndicator::setSelected(bool v)
{
    EngravingItem::setSelected(v);
    renderer()->layoutItem(this);
    system()->page()->invalidateBspTree();
}

const Page* PageLockIndicator::page() const
{
    return system() ? system()->page() : nullptr;
}

String PageLockIndicator::formatBarsAndBeats() const
{
    const MeasureBase* startMB = m_pageLock->startMB();
    const MeasureBase* endMB = m_pageLock->endMB();
    const Measure* startMeasure = startMB->isMeasure() ? toMeasure(startMB) : toMeasure(startMB->prevMeasure());
    const Measure* endMeasure = endMB->isMeasure() ? toMeasure(endMB) : toMeasure(endMB->prevMeasure());
    const int startMeasureNum = startMeasure ? startMeasure->measureNumber() : -1;
    const int endMeasureNum = endMeasure ? endMeasure->measureNumber() : -1;
    return muse::mtrc("engraving", "Start measure: %1; End measure: %2").arg(startMeasureNum).arg(endMeasureNum);
}
} // namespace mu::engraving
