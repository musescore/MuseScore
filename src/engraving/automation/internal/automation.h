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

#include "engraving/automation/iautomation.h"

namespace mu::engraving {
class Automation : public IAutomation
{
public:
    const AutomationCurveMap& curves() const override;
    const AutomationCurve& curve(const AutomationCurveKey& key) const override;
    const AutomationPoint* activePoint(const AutomationCurveKey& key, utick_t tick) const override;

    bool isEmpty() const override;

    void clear() override;

    void replaceCurves(AutomationCurveMap&& curves) override;

    void addPoint(const AutomationCurveKey& key, utick_t tick, const AutomationPoint& p) override;
    void removePoint(const AutomationCurveKey& key, utick_t tick) override;
    void movePoint(const AutomationCurveKey& key, utick_t srcTick, utick_t dstTick) override;

    void removePoints(const PointRemoveAccepted& accepted) override;

    void setPointInValue(const AutomationCurveKey& key, utick_t tick, double value) override;
    void setPointOutValue(const AutomationCurveKey& key, utick_t tick, double value) override;

    void moveTicks(utick_t tickFrom, utick_t diff) override;
    void removeTicks(utick_t tickFrom, utick_t tickTo) override;

    muse::async::Channel<AutomationChanges> changed() const override;

    void beginTransaction() override;
    void commitTransaction() override;
    void rollbackTransaction() override;

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
