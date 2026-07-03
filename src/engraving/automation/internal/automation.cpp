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

#include "automation.h"

#include "global/containers.h"
#include "global/realfn.h"
#include "global/log.h"

using namespace mu::engraving;

static void diffPoints(const AutomationCurveKey& key, const AutomationCurve& oldCurve, const AutomationCurve& newCurve,
                       AutomationChanges& changes)
{
    if (oldCurve == newCurve) {
        return;
    }

    auto oldIt = oldCurve.begin();
    auto newIt = newCurve.begin();

    while (oldIt != oldCurve.end() || newIt != newCurve.end()) {
        if (newIt == newCurve.end() || (oldIt != oldCurve.end() && oldIt->first < newIt->first)) {
            changes.extend(key, oldIt->first, oldIt->first); // point removed
            ++oldIt;
        } else if (oldIt == oldCurve.end() || newIt->first < oldIt->first) {
            changes.extend(key, newIt->first, newIt->first); // point added
            ++newIt;
        } else {
            if (oldIt->second != newIt->second) {
                changes.extend(key, oldIt->first, oldIt->first); // point changed
            }
            ++oldIt;
            ++newIt;
        }
    }
}

const AutomationCurveMap& Automation::curves() const
{
    return m_curveMap;
}

const AutomationCurve& Automation::curve(const AutomationCurveKey& key) const
{
    auto curveIt = m_curveMap.find(key);
    if (curveIt == m_curveMap.end()) {
        static const AutomationCurve dummy;
        return dummy;
    }

    return curveIt->second;
}

const AutomationPoint* Automation::activePoint(const AutomationCurveKey& key, utick_t tick) const
{
    const AutomationCurve& keyCurve = curve(key);
    const auto it = muse::findLessOrEqual(keyCurve, tick);
    return it != keyCurve.cend() ? &it->second : nullptr;
}

bool Automation::isEmpty() const
{
    return m_curveMap.empty();
}

void Automation::clear()
{
    if (m_curveMap.empty()) {
        return;
    }

    m_curveMap.clear();
    m_pendingChanges.isFullReset = true;
    notifyChanged();
}

void Automation::replaceCurves(AutomationCurveMap&& curves)
{
    static const AutomationCurve EMPTY_CURVE;

    for (auto& [key, newCurve] : curves) {
        const auto oldIt = m_curveMap.find(key);
        const AutomationCurve& oldCurve = oldIt != m_curveMap.end() ? oldIt->second : EMPTY_CURVE;
        diffPoints(key, oldCurve, newCurve, m_pendingChanges);

        if (newCurve.empty()) {
            if (oldIt != m_curveMap.end()) {
                m_curveMap.erase(oldIt);
            }
        } else if (oldIt != m_curveMap.end()) {
            oldIt->second = std::move(newCurve);
        } else {
            m_curveMap.emplace(key, std::move(newCurve));
        }
    }

    notifyChanged();
}

void Automation::addPoint(const AutomationCurveKey& key, utick_t tick, const AutomationPoint& p)
{
    auto curveIt = m_curveMap.find(key);
    if (curveIt == m_curveMap.end()) {
        curveIt = m_curveMap.try_emplace(key).first;
    }

    curveIt->second.insert_or_assign(tick, p);
    m_pendingChanges.extend(key, tick, tick);
    notifyChanged();
}

void Automation::removePoint(const AutomationCurveKey& key, utick_t tick)
{
    auto curveIt = m_curveMap.find(key);
    IF_ASSERT_FAILED(curveIt != m_curveMap.end()) {
        return;
    }

    AutomationCurve& curve = curveIt->second;
    bool ok = muse::remove(curve, tick);
    IF_ASSERT_FAILED(ok) {
        return;
    }

    if (curve.empty()) {
        m_curveMap.erase(curveIt);
    }

    m_pendingChanges.extend(key, tick, tick);
    notifyChanged();
}

void Automation::movePoint(const AutomationCurveKey& key, utick_t srcTick, utick_t dstTick)
{
    if (srcTick == dstTick) {
        return;
    }

    auto curveIt = m_curveMap.find(key);
    IF_ASSERT_FAILED(curveIt != m_curveMap.end()) {
        return;
    }

    AutomationCurve& curve = curveIt->second;
    auto node = curve.extract(srcTick);
    IF_ASSERT_FAILED(!node.empty()) {
        return;
    }

    node.key() = dstTick;
    curve.insert(std::move(node));

    m_pendingChanges.extend(key, std::min(srcTick, dstTick), std::max(srcTick, dstTick));
    notifyChanged();
}

void Automation::setPointInValue(const AutomationCurveKey& key, utick_t tick, double value)
{
    auto curveIt = m_curveMap.find(key);
    IF_ASSERT_FAILED(curveIt != m_curveMap.end()) {
        return;
    }

    AutomationCurve& curve = curveIt->second;
    auto pointIt = curve.find(tick);
    IF_ASSERT_FAILED(pointIt != curve.end()) {
        return;
    }

    if (muse::RealIsEqual(pointIt->second.inValue, value)) {
        return;
    }

    pointIt->second.inValue = value;
    m_pendingChanges.extend(key, tick, tick);
    notifyChanged();
}

void Automation::setPointOutValue(const AutomationCurveKey& key, utick_t tick, double value)
{
    auto curveIt = m_curveMap.find(key);
    IF_ASSERT_FAILED(curveIt != m_curveMap.end()) {
        return;
    }

    AutomationCurve& curve = curveIt->second;
    auto pointIt = curve.find(tick);
    IF_ASSERT_FAILED(pointIt != curve.end()) {
        return;
    }

    if (muse::RealIsEqual(pointIt->second.outValue, value)) {
        return;
    }

    pointIt->second.outValue = value;
    m_pendingChanges.extend(key, tick, tick);
    notifyChanged();
}

void Automation::removePoints(const PointRemoveAccepted& accepted)
{
    for (auto& [key, curve] : m_curveMap) {
        for (auto it = curve.begin(); it != curve.end();) {
            if (accepted(key, it->first, it->second)) {
                m_pendingChanges.extend(key, it->first, it->first);
                it = curve.erase(it);
            } else {
                ++it;
            }
        }
    }

    for (auto it = m_curveMap.begin(); it != m_curveMap.end();) {
        it = it->second.empty() ? m_curveMap.erase(it) : std::next(it);
    }

    notifyChanged();
}

void Automation::moveTicks(utick_t tickFrom, utick_t diff)
{
    for (auto& [key, curve] : m_curveMap) {
        const auto startIt = curve.lower_bound(tickFrom);
        if (startIt == curve.end()) {
            continue;
        }

        m_pendingChanges.extend(key, startIt->first, std::numeric_limits<utick_t>::max());
        std::vector<std::pair<utick_t, AutomationPoint> > toMove;
        for (auto it = startIt; it != curve.end(); ++it) {
            toMove.emplace_back(it->first + diff, it->second);
        }

        curve.erase(startIt, curve.end());
        for (auto& pair : toMove) {
            curve.insert(curve.end(), std::move(pair));
        }
    }

    notifyChanged();
}

void Automation::removeTicks(utick_t tickFrom, utick_t tickTo)
{
    IF_ASSERT_FAILED(tickFrom <= tickTo) {
        return;
    }

    const utick_t diff = tickFrom - tickTo;

    for (auto& [key, curve] : m_curveMap) {
        const auto eraseFromIt = curve.lower_bound(tickFrom);
        if (eraseFromIt == curve.end()) {
            continue;
        }

        m_pendingChanges.extend(key, eraseFromIt->first, std::numeric_limits<utick_t>::max());
        curve.erase(eraseFromIt, curve.upper_bound(tickTo));

        const auto startIt = curve.lower_bound(tickTo);
        std::vector<std::pair<utick_t, AutomationPoint> > toMove;
        for (auto it = startIt; it != curve.end(); ++it) {
            toMove.emplace_back(it->first + diff, it->second);
        }

        curve.erase(startIt, curve.end());
        for (auto& pair : toMove) {
            curve.insert(curve.end(), std::move(pair));
        }
    }

    for (auto it = m_curveMap.begin(); it != m_curveMap.end();) {
        it = it->second.empty() ? m_curveMap.erase(it) : std::next(it);
    }

    notifyChanged();
}

muse::async::Channel<AutomationChanges> Automation::changed() const
{
    return m_changesChannel;
}

void Automation::beginTransaction()
{
    IF_ASSERT_FAILED(!m_transactionStarted) {
        return;
    }

    m_snapshot = m_curveMap;
    m_transactionStarted = true;
}

void Automation::commitTransaction()
{
    IF_ASSERT_FAILED(m_transactionStarted) {
        return;
    }

    m_transactionStarted = false;
    m_snapshot.clear();

    if (m_notifyPending) {
        m_notifyPending = false;
        m_changesChannel.send(m_pendingChanges);
        m_pendingChanges.clear();
    }
}

void Automation::rollbackTransaction()
{
    IF_ASSERT_FAILED(m_transactionStarted) {
        return;
    }

    m_curveMap = std::move(m_snapshot);
    m_transactionStarted = false;
    m_notifyPending = false;
    m_pendingChanges.clear();
}

void Automation::notifyChanged()
{
    if (m_pendingChanges.isEmpty()) {
        return;
    }

    if (m_transactionStarted) {
        m_notifyPending = true;
    } else {
        m_changesChannel.send(m_pendingChanges);
        m_pendingChanges.clear();
    }
}
