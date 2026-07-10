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

#include "global/async/channel.h"

#include "automationtypes.h"

namespace mu::engraving {
class IAutomation
{
public:
    virtual ~IAutomation() = default;

    virtual const AutomationCurveMap& curves() const = 0;
    virtual const AutomationCurve& curve(const AutomationCurveKey& key) const = 0;
    virtual const AutomationPoint* point(const AutomationCurveKey& key, utick_t tick) const = 0;

    virtual bool isEmpty() const = 0;

    //! NOTE: full replacement; any existing key absent from the argument is removed; notifies as a full reset
    virtual void setCurves(const AutomationCurveMap& curves) = 0;

    //! NOTE: replaces only the given curves, keeping all others; keys absent from the argument are untouched
    virtual void replaceCurves(const AutomationCurveMap& curves) = 0;

    //! NOTE: creates, updates, and/or moves points in one batch
    virtual void editPoints(const AutomationCurveKey& key, const AutomationPointEdits& edits) = 0;

    //! NOTE: removes the points at the given ticks from the given curve, if present
    virtual void removePoints(const AutomationCurveKey& key, const std::set<utick_t>& ticks) = 0;

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
