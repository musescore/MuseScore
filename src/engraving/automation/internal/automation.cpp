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

#include "automation.h"

#include "global/serialization/json.h"
#include "global/containers.h"
#include "global/log.h"

using namespace mu::engraving;

static const std::unordered_map<AutomationType, muse::String> AUTOMATION_TYPE_TO_STRING {
    { AutomationType::Dynamics, u"Dynamics" },
};

static const std::unordered_map<AutomationPoint::InterpolationType, muse::String> INTERPOLATION_TYPE_TO_STRING {
    { AutomationPoint::InterpolationType::Linear, u"Linear" },
    { AutomationPoint::InterpolationType::Exponential, u"Exponential" },
};

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

void Automation::clear()
{
    m_curveMap.clear();
}

bool Automation::isEmpty() const
{
    return m_curveMap.empty();
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

void Automation::read(const muse::ByteArray& json)
{
    TRACEFUNC;

    if (json.empty()) {
        return;
    }

    std::string err;
    const muse::JsonDocument doc = muse::JsonDocument::fromJson(json, &err);
    if (!err.empty() || !doc.isArray()) {
        LOGE() << "Failed to parse automation json: " << err;
        return;
    }

    m_curveMap.clear();

    const muse::JsonArray rootArray = doc.rootArray();
    for (size_t i = 0; i < rootArray.size(); ++i) {
        const muse::JsonObject curveObj = rootArray.at(i).toObject();
        AutomationCurveKey key;
        key.type = muse::key(AUTOMATION_TYPE_TO_STRING, curveObj.value("type").toString(), AutomationType::Unknown);
        key.staffId = muse::ID(curveObj.value("staffId").toString().toStdString());

        if (curveObj.contains("voiceId")) {
            key.voiceIdx = static_cast<size_t>(curveObj.value("voiceId").toInt());
        }

        IF_ASSERT_FAILED(key.isValid()) {
            continue;
        }

        const muse::JsonArray pointArray = curveObj.value("points").toArray();
        AutomationCurve curve;

        for (size_t j = 0; j < pointArray.size(); ++j) {
            const muse::JsonObject pointObj = pointArray.at(j).toObject();

            AutomationPoint point;
            point.inValue = pointObj.value("inValue").toDouble();
            point.outValue = pointObj.value("outValue").toDouble();
            point.interpolation = muse::key(INTERPOLATION_TYPE_TO_STRING, pointObj.value("interpolation").toString(),
                                            AutomationPoint::InterpolationType::Linear);

            const utick_t tick = pointObj.value("tick").toInt();
            curve.insert_or_assign(tick, point);
        }

        if (!curve.empty()) {
            m_curveMap.insert_or_assign(key, std::move(curve));
        }
    }
}

muse::ByteArray Automation::toJson() const
{
    TRACEFUNC;

    muse::JsonArray rootArray;
    for (const auto& [key, curve] : m_curveMap) {
        muse::JsonObject curveObj;
        curveObj["type"] = muse::value(AUTOMATION_TYPE_TO_STRING, key.type);
        curveObj["staffId"] = key.staffId.toStdString();

        if (key.voiceIdx.has_value()) {
            curveObj["voiceId"] = static_cast<int>(key.voiceIdx.value());
        }

        muse::JsonArray pointArray;
        for (const auto& [utick, point] : curve) {
            muse::JsonObject pointObj;
            pointObj["tick"] = utick;
            pointObj["inValue"] = point.inValue;
            pointObj["outValue"] = point.outValue;
            pointObj["interpolation"] = muse::value(INTERPOLATION_TYPE_TO_STRING, point.interpolation);
            pointArray << pointObj;
        }

        curveObj["points"] = pointArray;
        rootArray << curveObj;
    }

    return muse::JsonDocument(rootArray).toJson();
}
