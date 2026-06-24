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
#include "global/log.h"

using namespace mu::engraving;

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

const AutomationPoint& Automation::activePoint(const AutomationCurveKey& key, utick_t tick) const
{
    const AutomationCurve& curve = this->curve(key);
    auto it = muse::findLessOrEqual(curve, tick);
    if (it == curve.cend()) {
        static const AutomationPoint MIDPOINT { 0.5, 0.5, AutomationPoint::InterpolationType::Linear, std::nullopt };
        return MIDPOINT;
    }

    return it->second;
}

bool Automation::isEmpty() const
{
    return m_curveMap.empty();
}

void Automation::clear()
{
    m_curveMap.clear();
}

void Automation::addPoint(const AutomationCurveKey& key, utick_t tick, const AutomationPoint& p)
{
    auto curveIt = m_curveMap.find(key);
    if (curveIt == m_curveMap.end()) {
        curveIt = m_curveMap.try_emplace(key).first;
    }

    AutomationCurve& curve = curveIt->second;
    curve.insert({ tick, p });
}

void Automation::removePoint(const AutomationCurveKey& key, utick_t tick)
{
    auto curveIt = m_curveMap.find(key);
    IF_ASSERT_FAILED(curveIt != m_curveMap.end()) {
        return;
    }

    AutomationCurve& curve = curveIt->second;
    bool ok = muse::remove(curve, tick);
    DO_ASSERT(ok);

    if (curve.empty()) {
        m_curveMap.erase(curveIt);
    }
}

void Automation::movePoint(const AutomationCurveKey& key, utick_t srcTick, utick_t dstTick)
{
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

    AutomationPoint& p = pointIt->second;
    p.inValue = value;
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

    AutomationPoint& p = pointIt->second;
    p.outValue = value;
}

void Automation::removePoints(const PointRemoveAccepted& accepted)
{
    for (auto& [key, curve] : m_curveMap) {
        for (auto it = curve.begin(); it != curve.end();) {
            it = accepted(key, it->first, it->second) ? curve.erase(it) : std::next(it);
        }
    }

    for (auto it = m_curveMap.begin(); it != m_curveMap.end();) {
        it = it->second.empty() ? m_curveMap.erase(it) : std::next(it);
    }
}

void Automation::moveTicks(utick_t tickFrom, utick_t diff)
{
    for (auto& [_, curve] : m_curveMap) {
        // Step 1: find the first point >= tickFrom
        auto startIt = curve.lower_bound(tickFrom);

        std::vector<std::pair<utick_t, AutomationPoint> > toMove;
        toMove.reserve(std::distance(startIt, curve.end()));

        // Step 2: copy affected points
        for (auto it = startIt; it != curve.end(); ++it) {
            toMove.emplace_back(it->first + diff, it->second);
        }

        // Step 3: erase the original range
        curve.erase(startIt, curve.end());

        // Step 4: reinsert with updated keys
        for (auto& pair : toMove) {
            curve.insert(std::move(pair));
        }
    }
}

void Automation::removeTicks(utick_t tickFrom, utick_t tickTo)
{
    IF_ASSERT_FAILED(tickFrom <= tickTo) {
        return;
    }

    for (auto& [_, curve] : m_curveMap) {
        curve.erase(curve.lower_bound(tickFrom), curve.upper_bound(tickTo));
    }

    for (auto it = m_curveMap.begin(); it != m_curveMap.end();) {
        it = it->second.empty() ? m_curveMap.erase(it) : std::next(it);
    }

    moveTicks(tickTo, tickFrom - tickTo);
}
