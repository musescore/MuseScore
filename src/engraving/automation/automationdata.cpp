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

#include "log.h"

using namespace mu::engraving;

static const AutomationCurve EMPTY_CURVE;

static void diffPoints(const AutomationCurveKey& key, const AutomationCurve& oldCurve, const AutomationCurve& newCurve,
                       AutomationChanges& changes)
{
    auto oldIt = oldCurve.begin();
    auto newIt = newCurve.begin();

    bool changed = false;
    utick_t firstTick = 0;
    utick_t lastTick = 0;

    while (oldIt != oldCurve.end() && newIt != newCurve.end()) {
        utick_t tick;
        bool differs;

        if (oldIt->first < newIt->first) {
            tick = oldIt->first; // point removed
            differs = true;
            ++oldIt;
        } else if (newIt->first < oldIt->first) {
            tick = newIt->first; // point added
            differs = true;
            ++newIt;
        } else {
            tick = oldIt->first;
            differs = oldIt->second != newIt->second; // point changed
            ++oldIt;
            ++newIt;
        }

        if (differs) {
            if (!changed) {
                firstTick = tick;
            }
            lastTick = tick;
            changed = true;
        }
    }

    if (oldIt != oldCurve.end() || newIt != newCurve.end()) {
        const bool oldHasTail = oldIt != oldCurve.end();
        const utick_t tailFirstTick = oldHasTail ? oldIt->first : newIt->first;
        const utick_t tailLastTick = oldHasTail ? oldCurve.rbegin()->first : newCurve.rbegin()->first;

        if (!changed) {
            firstTick = tailFirstTick;
        }
        lastTick = tailLastTick;
        changed = true;
    }

    if (changed) {
        changes.extend(key, firstTick, lastTick);
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
        return EMPTY_CURVE;
    }

    return curveIt->second;
}

bool AutomationData::isEmpty() const
{
    return m_curveMap.empty();
}

void AutomationData::setCurves(const AutomationCurveMap& curves)
{
    TRACEFUNC;

    if (m_curveMap.empty() && !curves.empty()) {
        m_curveMap = curves;
        AutomationChanges changes;
        changes.isFullReset = true;
        notifyChanged(changes);
        return;
    }

    size_t overlapCount = 0;
    AutomationChanges changes;

    for (const auto& [key, newCurve] : curves) {
        const auto oldIt = m_curveMap.find(key);
        const bool existed = oldIt != m_curveMap.end();
        if (existed) {
            ++overlapCount;
        }

        const AutomationCurve& oldCurve = existed ? oldIt->second : EMPTY_CURVE;
        diffPoints(key, oldCurve, newCurve, changes);
    }

    for (const auto& [key, oldCurve] : m_curveMap) {
        if (!curves.contains(key)) {
            diffPoints(key, oldCurve, EMPTY_CURVE, changes);
        }
    }

    // Every key was touched by this change
    const size_t allKeysCount = curves.size() + m_curveMap.size() - overlapCount;
    if (changes.affectedKeys.size() == allKeysCount) {
        changes.isFullReset = true;
    }

    m_curveMap = curves;

    notifyChanged(changes);
}

void AutomationData::replaceCurves(const AutomationCurveMap& curves)
{
    TRACEFUNC;

    if (m_curveMap.empty() && !curves.empty()) {
        m_curveMap = curves;
        AutomationChanges changes;
        changes.isFullReset = true;
        notifyChanged(changes);
        return;
    }

    const size_t oldSize = m_curveMap.size();
    size_t overlapCount = 0;
    AutomationChanges changes;

    for (const auto& [key, newCurve] : curves) {
        const auto oldIt = m_curveMap.find(key);
        const bool existed = oldIt != m_curveMap.end();
        if (existed) {
            ++overlapCount;
        }

        const AutomationCurve& oldCurve = existed ? oldIt->second : EMPTY_CURVE;
        diffPoints(key, oldCurve, newCurve, changes);

        if (newCurve.empty()) {
            if (existed) {
                m_curveMap.erase(oldIt);
            }
        } else if (existed) {
            oldIt->second = newCurve;
        } else {
            m_curveMap.emplace(key, newCurve);
        }
    }

    // Every key was touched by this change
    const size_t allKeysCount = oldSize + curves.size() - overlapCount;
    if (allKeysCount > 0 && changes.affectedKeys.size() == allKeysCount) {
        changes.isFullReset = true;
    }

    notifyChanged(changes);
}

void AutomationData::editPoints(const AutomationCurveKey& key, const AutomationPointEdits& edits)
{
    TRACEFUNC;

    if (edits.empty()) {
        return;
    }

    const auto curveIt = m_curveMap.try_emplace(key).first;
    AutomationCurve& curve = curveIt->second;
    AutomationChanges changes;
    bool changed = false;

    for (const auto& edit : edits) {
        if (std::holds_alternative<AutomationPointEdit::ErasePoint>(edit.change)) {
            const auto it = curve.find(edit.tick);
            if (it != curve.end()) {
                curve.erase(it);
                changes.extend(edit.tick, edit.tick);
                changed = true;
            }
            continue;
        }

        utick_t rangeFrom = edit.tick;
        utick_t rangeTo = edit.tick;
        bool moved = false;

        const auto* move = std::get_if<AutomationPointEdit::MovePoint>(&edit.change);
        if (move && move->from != edit.tick) {
            curve.erase(move->from);
            rangeFrom = std::min(rangeFrom, move->from);
            rangeTo = std::max(rangeTo, move->from);
            moved = true;
        }

        const AutomationPoint& point = move ? move->point : std::get<AutomationPointEdit::SetPoint>(edit.change).point;

        auto pointIt = curve.lower_bound(edit.tick);
        const bool exists = pointIt != curve.end() && pointIt->first == edit.tick;

        if (exists && pointIt->second == point) {
            if (moved) {
                changes.extend(rangeFrom, rangeTo);
                changed = true;
            }
            continue;
        }

        if (exists) {
            pointIt->second = point;
        } else {
            curve.emplace_hint(pointIt, edit.tick, point);
        }

        changes.extend(rangeFrom, rangeTo);
        changed = true;
    }

    if (curve.empty()) {
        m_curveMap.erase(curveIt);
    }

    if (changed) {
        changes.affectedKeys.insert(key);
    }

    notifyChanged(changes);
}

muse::async::Channel<AutomationChanges> AutomationData::changed() const
{
    return m_changesChannel;
}

void AutomationData::notifyChanged(const AutomationChanges& changes)
{
    if (changes.isEmpty()) {
        return;
    }

    m_changesChannel.send(changes);
}
