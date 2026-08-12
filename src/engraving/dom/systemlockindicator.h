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

#pragma once

#include "indicatoricon.h"

namespace mu::engraving {
class RangeLock;

class SystemLockIndicator : public IndicatorIcon
{
    OBJECT_ALLOCATOR(engraving, SystemLockIndicator)
    DECLARE_CLASSOF(ElementType::SYSTEM_LOCK_INDICATOR)

public:
    SystemLockIndicator(System* parent, const RangeLock* lock);

    void setSelected(bool v) override;

    const RangeLock* systemLock() const { return m_systemLock; }

    char16_t iconCode() const override { return 0xF487; }

    String formatBarsAndBeats() const override;

private:
    const RangeLock* m_systemLock = nullptr;
};
} // namespace mu::engraving
