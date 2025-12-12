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

#include "automation.h"

#include "engraving/dom/score.h"
#include "engraving/dom/repeatlist.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/staff.h"
#include "engraving/types/typesconv.h"

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

//! NOTE: Linear values
static const std::unordered_map<DynamicType, double> ORDINARY_DYNAMIC_VALUES {
    { DynamicType::N,      0.000 },
    { DynamicType::PPPPPP, 0.071 },
    { DynamicType::PPPPP,  0.143 },
    { DynamicType::PPPP,   0.214 },
    { DynamicType::PPP,    0.286 },
    { DynamicType::PP,     0.357 },
    { DynamicType::P,      0.429 },
    { DynamicType::MP,     0.500 },
    { DynamicType::MF,     0.571 },
    { DynamicType::F,      0.643 },
    { DynamicType::FF,     0.714 },
    { DynamicType::FFF,    0.786 },
    { DynamicType::FFFF,   0.857 },
    { DynamicType::FFFFF,  0.929 },
    { DynamicType::FFFFFF, 1.000 },
};

static const std::unordered_map<DynamicType, double> SINGLE_NOTE_DYNAMIC_VALUES {
    { DynamicType::SF, ORDINARY_DYNAMIC_VALUES.at(DynamicType::F) },
    { DynamicType::SFZ, ORDINARY_DYNAMIC_VALUES.at(DynamicType::F) },
    { DynamicType::SFF, ORDINARY_DYNAMIC_VALUES.at(DynamicType::FF) },
    { DynamicType::SFFZ, ORDINARY_DYNAMIC_VALUES.at(DynamicType::FF) },
    { DynamicType::SFFF, ORDINARY_DYNAMIC_VALUES.at(DynamicType::FFF) },
    { DynamicType::SFFFZ, ORDINARY_DYNAMIC_VALUES.at(DynamicType::FFF) },
    { DynamicType::RFZ, ORDINARY_DYNAMIC_VALUES.at(DynamicType::F) },
    { DynamicType::RF, ORDINARY_DYNAMIC_VALUES.at(DynamicType::F) },
};

static const std::unordered_map<DynamicType, std::pair<double, double> > COMPOUND_DYNAMIC_VALUES {
    { DynamicType::FP, { ORDINARY_DYNAMIC_VALUES.at(DynamicType::F), ORDINARY_DYNAMIC_VALUES.at(DynamicType::P) } },
    { DynamicType::PF, { ORDINARY_DYNAMIC_VALUES.at(DynamicType::P), ORDINARY_DYNAMIC_VALUES.at(DynamicType::F) } },
    { DynamicType::SFP, { ORDINARY_DYNAMIC_VALUES.at(DynamicType::F), ORDINARY_DYNAMIC_VALUES.at(DynamicType::P) } },
    { DynamicType::SFPP, { ORDINARY_DYNAMIC_VALUES.at(DynamicType::F), ORDINARY_DYNAMIC_VALUES.at(DynamicType::PP) } },
};

void Automation::init(Score* score)
{
    for (const RepeatSegment* repeatSegment : score->repeatList()) {
        const int tickOffset = repeatSegment->utick - repeatSegment->tick;

        for (const Measure* measure : repeatSegment->measureList()) {
            for (const Segment* segment = measure->first(); segment; segment = segment->next()) {
                handleSegmentAnnotations(segment, tickOffset);
            }
        }
    }
}

void Automation::handleSegmentAnnotations(const Segment* segment, int tickOffset)
{
    for (const EngravingItem* annotation : segment->annotations()) {
        if (!annotation) {
            continue;
        }

        if (annotation->isDynamic()) {
            handleDynamic(toDynamic(annotation), segment, tickOffset);
        }
    }
}

void Automation::handleDynamic(const Dynamic* dynamic, const Segment* segment, int tickOffset)
{
    AutomationCurveKey key;
    key.type = AutomationType::Dynamics;
    key.staffId = dynamic->staff() ? dynamic->staff()->id() : muse::ID();

    IF_ASSERT_FAILED(key.isValid()) {
        return;
    }

    if (!muse::contains(m_curveMap, key)) {
        m_curveMap[key] = AutomationCurve();
    }

    const int dynamicTick = dynamic->tick().ticks() + tickOffset;
    AutomationPoint prevPoint = activePoint(key, dynamicTick);

    if (muse::contains(ORDINARY_DYNAMIC_VALUES, dynamic->dynamicType())) {
        AutomationPoint point;
        point.inValue = prevPoint.outValue;
        point.outValue = muse::value(ORDINARY_DYNAMIC_VALUES, dynamic->dynamicType());
        point.interpolation = AutomationPoint::InterpolationType::Linear;
        addPoint(key, dynamicTick, point);
        return;
    }

    if (muse::contains(SINGLE_NOTE_DYNAMIC_VALUES, dynamic->dynamicType())) {
        AutomationPoint point;
        point.inValue = prevPoint.outValue;
        point.outValue = muse::value(SINGLE_NOTE_DYNAMIC_VALUES, dynamic->dynamicType());
        point.interpolation = AutomationPoint::InterpolationType::Linear;
        addPoint(key, dynamicTick, point);

        if (segment->next()) {
            prevPoint.inValue = point.outValue;
            addPoint(key, segment->next()->tick().ticks() + tickOffset, prevPoint);
        }

        return;
    }

    if (muse::contains(COMPOUND_DYNAMIC_VALUES, dynamic->dynamicType())) {
        const std::pair<double, double>& values = COMPOUND_DYNAMIC_VALUES.at(dynamic->dynamicType());
        const int startTick = dynamic->tick().ticks() + tickOffset;
        const int endTick = startTick + dynamic->velocityChangeLength().ticks();

        AutomationPoint startPoint;
        startPoint.inValue = prevPoint.outValue;
        startPoint.outValue = values.first;
        startPoint.interpolation = AutomationPoint::InterpolationType::Exponential;
        addPoint(key, startTick, startPoint);

        AutomationPoint endPoint;
        endPoint.inValue = startPoint.outValue;
        endPoint.outValue = values.second;
        endPoint.interpolation = AutomationPoint::InterpolationType::Linear;
        addPoint(key, endTick, endPoint);

        return;
    }
}

const AutomationPoint& Automation::activePoint(const AutomationCurveKey& key, int utick) const
{
    const AutomationCurve& curve = this->curve(key);
    auto it = muse::findLessOrEqual(curve, utick);
    if (it == curve.cend()) {
        static const AutomationPoint MIDPOINT { 0.5, 0.5, AutomationPoint::InterpolationType::Linear };
        return MIDPOINT;
    }

    return it->second;
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

void Automation::addPoint(const AutomationCurveKey& key, int utick, const AutomationPoint& p)
{
    auto curveIt = m_curveMap.find(key);
    IF_ASSERT_FAILED(curveIt != m_curveMap.end()) {
        return;
    }

    AutomationCurve& curve = curveIt->second;
    curve.insert_or_assign(utick, p);
}

void Automation::removePoint(const AutomationCurveKey& key, int utick)
{
    auto curveIt = m_curveMap.find(key);
    IF_ASSERT_FAILED(curveIt != m_curveMap.end()) {
        return;
    }

    AutomationCurve& curve = curveIt->second;
    bool ok = muse::remove(curve, utick);
    DO_ASSERT(ok);
}

void Automation::movePoint(const AutomationCurveKey& key, int srcUtick, int dstUtick)
{
    auto curveIt = m_curveMap.find(key);
    IF_ASSERT_FAILED(curveIt != m_curveMap.end()) {
        return;
    }

    AutomationCurve& curve = curveIt->second;
    auto node = curve.extract(srcUtick);
    IF_ASSERT_FAILED(!node.empty()) {
        return;
    }

    node.key() = dstUtick;
    curve.insert(std::move(node));
}

void Automation::setPointInValue(const AutomationCurveKey& key, int utick, double value)
{
    auto curveIt = m_curveMap.find(key);
    IF_ASSERT_FAILED(curveIt != m_curveMap.end()) {
        return;
    }

    AutomationCurve& curve = curveIt->second;
    auto pointIt = curve.find(utick);
    IF_ASSERT_FAILED(pointIt != curve.end()) {
        return;
    }

    AutomationPoint& p = pointIt->second;
    p.inValue = value;
}

void Automation::setPointOutValue(const AutomationCurveKey& key, int utick, double value)
{
    auto curveIt = m_curveMap.find(key);
    IF_ASSERT_FAILED(curveIt != m_curveMap.end()) {
        return;
    }

    AutomationCurve& curve = curveIt->second;
    auto pointIt = curve.find(utick);
    IF_ASSERT_FAILED(pointIt != curve.end()) {
        return;
    }

    AutomationPoint& p = pointIt->second;
    p.outValue = value;
}

void Automation::moveTicks(int utickFrom, int diff)
{
    for (auto& [_, curve] : m_curveMap) {
        // Step 1: find the first point >= utickFrom
        auto startIt = curve.lower_bound(utickFrom);

        std::vector<std::pair<int, AutomationPoint> > toMove;
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

void Automation::removeTicks(int utickFrom, int utickTo)
{
    IF_ASSERT_FAILED(utickFrom <= utickTo) {
        return;
    }

    const int diff = -(utickTo - utickFrom + 1);

    for (auto& [_, curve] : m_curveMap) {
        auto eraseFrom = curve.lower_bound(utickFrom);
        auto eraseTo = curve.upper_bound(utickTo);

        curve.erase(eraseFrom, eraseTo);
        moveTicks(utickTo, diff);
    }
}

void Automation::read(const muse::ByteArray& json)
{
    TRACEFUNC;

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
        key.staffId = static_cast<uint64_t>(curveObj.value("staffId").toInt());

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

            const int tick = pointObj.value("tick").toInt();
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
