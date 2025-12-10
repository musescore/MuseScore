/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "automationtypes.h"

#include "global/types/bytearray.h"

namespace mu::engraving {
class IAutomation
{
public:
    virtual ~IAutomation() = default;

    virtual const AutomationCurve& curve(const AutomationCurveKey& key) const = 0;

    virtual void addPoint(const AutomationCurveKey& key, int utick, const AutomationPoint& p) = 0;
    virtual void removePoint(const AutomationCurveKey& key, int utick) = 0;
    virtual void movePoint(const AutomationCurveKey& key, int srcUtick, int dstUtick) = 0;

    virtual void setPointInValue(const AutomationCurveKey& key, int utick, double value) = 0;
    virtual void setPointOutValue(const AutomationCurveKey& key, int utick, double value) = 0;

    //! NOTE: moves all points with keys >= utickFrom by diff ticks
    virtual void moveTicks(int utickFrom, int diff) = 0;

    //! NOTE: removes points in [from, to], moves later points back to close the gap
    virtual void removeTicks(int utickFrom, int utickTo) = 0;

    virtual void read(const muse::ByteArray& json) = 0;
    virtual muse::ByteArray toJson() const = 0;
};
}
