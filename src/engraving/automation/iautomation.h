/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include <functional>

#include "global/async/channel.h"

#include "automationtypes.h"

namespace mu::engraving {
class IAutomation
{
public:
    virtual ~IAutomation() = default;

    virtual const AutomationCurveMap& curves() const = 0;
    virtual const AutomationCurve& curve(const AutomationCurveKey& key) const = 0;
    virtual const AutomationPoint* activePoint(const AutomationCurveKey& key, utick_t tick) const = 0;

    virtual bool isEmpty() const = 0;

    virtual void clear() = 0;

    virtual void addPoint(const AutomationCurveKey& key, utick_t tick, const AutomationPoint& p) = 0;
    virtual void removePoint(const AutomationCurveKey& key, utick_t tick) = 0;
    virtual void movePoint(const AutomationCurveKey& key, utick_t srcTick, utick_t dstTick) = 0;

    using PointRemoveAccepted = std::function<bool(const AutomationCurveKey&, utick_t tick, const AutomationPoint&)>;
    virtual void removePoints(const PointRemoveAccepted& accepted) = 0;

    virtual void setPointInValue(const AutomationCurveKey& key, utick_t tick, double value) = 0;
    virtual void setPointOutValue(const AutomationCurveKey& key, utick_t tick, double value) = 0;

    //! NOTE: moves all points with tick >= tickFrom by diff ticks
    virtual void moveTicks(utick_t tickFrom, utick_t diff) = 0;

    //! NOTE: removes points in [tickFrom, tickTo], shifts later points back to close the gap
    virtual void removeTicks(utick_t tickFrom, utick_t tickTo) = 0;

    virtual muse::async::Channel<AutomationChanges> changed() const = 0;

    virtual void beginTransaction() = 0;
    virtual void commitTransaction() = 0;
    virtual void rollbackTransaction() = 0;
};
}
