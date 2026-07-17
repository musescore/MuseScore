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

#include "automationdata.h"

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

const AutomationCurveMap& AutomationData::curves() const
{
    return m_curveMap;
}

const AutomationCurve& AutomationData::curve(const AutomationCurveKey& key) const
{
    auto curveIt = m_curveMap.find(key);
    if (curveIt == m_curveMap.end()) {
        static const AutomationCurve dummy;
        return dummy;
    }

    return curveIt->second;
}

const AutomationPoint* AutomationData::point(const AutomationCurveKey& key, utick_t tick) const
{
    const AutomationCurve& keyCurve = curve(key);
    const auto it = keyCurve.find(tick);
    return it != keyCurve.cend() ? &it->second : nullptr;
}

bool AutomationData::isEmpty() const
{
    return m_curveMap.empty();
}

void AutomationData::setCurves(const AutomationCurveMap& curves)
{
    if (curves == m_curveMap) {
        return;
    }

    m_curveMap = curves;

    m_pendingChanges.isFullReset = true;
    notifyChanged();
}

void AutomationData::replaceCurves(const AutomationCurveMap& curves)
{
    static const AutomationCurve EMPTY_CURVE;

    for (const auto& [key, newCurve] : curves) {
        const auto oldIt = m_curveMap.find(key);
        const AutomationCurve& oldCurve = oldIt != m_curveMap.end() ? oldIt->second : EMPTY_CURVE;
        diffPoints(key, oldCurve, newCurve, m_pendingChanges);

        if (newCurve.empty()) {
            if (oldIt != m_curveMap.end()) {
                m_curveMap.erase(oldIt);
            }
        } else if (oldIt != m_curveMap.end()) {
            oldIt->second = newCurve;
        } else {
            m_curveMap.emplace(key, newCurve);
        }
    }

    notifyChanged();
}

void AutomationData::editPoints(const AutomationCurveKey& key, const AutomationPointEdits& edits)
{
    if (edits.empty()) {
        return;
    }

    AutomationCurve& curve = m_curveMap.try_emplace(key).first->second;

    for (const auto& edit : edits) {
        utick_t rangeFrom = edit.tick;
        utick_t rangeTo = edit.tick;
        bool moved = false;

        if (edit.moveFrom && *edit.moveFrom != edit.tick) {
            curve.erase(*edit.moveFrom);
            rangeFrom = std::min(rangeFrom, *edit.moveFrom);
            rangeTo = std::max(rangeTo, *edit.moveFrom);
            moved = true;
        }

        auto pointIt = curve.lower_bound(edit.tick);
        const bool exists = pointIt != curve.end() && pointIt->first == edit.tick;

        if (exists && pointIt->second == edit.point) {
            if (moved) {
                m_pendingChanges.extend(key, rangeFrom, rangeTo);
            }
            continue;
        }

        if (exists) {
            pointIt->second = edit.point;
        } else {
            curve.emplace_hint(pointIt, edit.tick, edit.point);
        }

        m_pendingChanges.extend(key, rangeFrom, rangeTo);
    }

    notifyChanged();
}

void AutomationData::removePoints(const AutomationCurveKey& key, const std::set<utick_t>& ticks)
{
    if (ticks.empty()) {
        return;
    }

    const auto curveIt = m_curveMap.find(key);
    if (curveIt == m_curveMap.end()) {
        return;
    }

    AutomationCurve& curve = curveIt->second;

    for (utick_t tick : ticks) {
        const auto pointIt = curve.find(tick);
        if (pointIt == curve.end()) {
            continue;
        }

        curve.erase(pointIt);
        m_pendingChanges.extend(key, tick, tick);
    }

    if (curve.empty()) {
        m_curveMap.erase(curveIt);
    }

    notifyChanged();
}

void AutomationData::moveTicks(utick_t tickFrom, utick_t diff)
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

void AutomationData::removeTicks(utick_t tickFrom, utick_t tickTo)
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

muse::async::Channel<AutomationChanges> AutomationData::changed() const
{
    return m_changesChannel;
}

void AutomationData::beginTransaction()
{
    IF_ASSERT_FAILED(!m_transactionStarted) {
        return;
    }

    m_snapshot = m_curveMap;
    m_transactionStarted = true;
}

void AutomationData::commitTransaction()
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

void AutomationData::rollbackTransaction()
{
    IF_ASSERT_FAILED(m_transactionStarted) {
        return;
    }

    m_curveMap = m_snapshot;
    m_snapshot.clear();
    m_transactionStarted = false;
    m_notifyPending = false;
    m_pendingChanges.clear();
}

void AutomationData::notifyChanged()
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
