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

#include "global/async/channel.h"

#include "engraving/automation/automationtypes.h"

namespace mu::engraving {
class Automation
{
public:
    const AutomationCurveMap& curves() const;
    const AutomationCurve& curve(const AutomationCurveKey& key) const;
    const AutomationPoint* point(const AutomationCurveKey& key, utick_t tick) const;

    bool isEmpty() const;

    //! NOTE: full replacement; any existing key absent from the argument is removed; notifies as a full reset
    void setCurves(const AutomationCurveMap& curves);

    //! NOTE: replaces only the given curves, keeping all others; keys absent from the argument are untouched
    void replaceCurves(const AutomationCurveMap& curves);

    //! NOTE: creates, updates, and/or moves points in one batch
    void editPoints(const AutomationCurveKey& key, const AutomationPointEdits& edits);

    //! NOTE: removes the points at the given ticks from the given curve, if present
    void removePoints(const AutomationCurveKey& key, const std::set<utick_t>& ticks);

    //! NOTE: moves all points with tick >= tickFrom by diff ticks
    void moveTicks(utick_t tickFrom, utick_t diff);

    //! NOTE: removes points in [tickFrom, tickTo], shifts later points back to close the gap
    void removeTicks(utick_t tickFrom, utick_t tickTo);

    muse::async::Channel<AutomationChanges> changed() const;

    void beginTransaction();
    void commitTransaction();
    void rollbackTransaction();

private:
    void notifyChanged();

    AutomationCurveMap m_curveMap;
    AutomationCurveMap m_snapshot;

    bool m_transactionStarted = false;
    bool m_notifyPending = false;
    AutomationChanges m_pendingChanges;
    muse::async::Channel<AutomationChanges> m_changesChannel;
};
}
