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

#include "automationrw.h"

#include "global/serialization/json.h"
#include "global/containers.h"
#include "global/log.h"

#include "engraving/automation/iautomation.h"

using namespace mu::engraving;

static const std::unordered_map<AutomationType, muse::String> AUTOMATION_TYPE_TO_STRING {
    { AutomationType::Dynamics, u"Dynamics" },
};

static const std::unordered_map<AutomationPoint::InterpolationType, muse::String> INTERPOLATION_TYPE_TO_STRING {
    { AutomationPoint::InterpolationType::Linear, u"Linear" },
    { AutomationPoint::InterpolationType::Exponential, u"Exponential" },
};

void AutomationRW::read(IAutomation& automation, const muse::ByteArray& json)
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

    automation.beginTransaction();
    automation.clear();

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
        for (size_t j = 0; j < pointArray.size(); ++j) {
            const muse::JsonObject pointObj = pointArray.at(j).toObject();

            AutomationPoint point;
            point.inValue = pointObj.value("inValue").toDouble();
            point.outValue = pointObj.value("outValue").toDouble();
            point.interpolation = muse::key(INTERPOLATION_TYPE_TO_STRING, pointObj.value("interpolation").toString(),
                                            AutomationPoint::InterpolationType::Linear);

            const utick_t tick = pointObj.value("tick").toInt();
            automation.addPoint(key, tick, point);
        }
    }

    automation.commitTransaction();
}

muse::ByteArray AutomationRW::write(const IAutomation& automation, bool writeGenerated)
{
    TRACEFUNC;

    muse::JsonArray rootArray;
    for (const auto& [key, curve] : automation.curves()) {
        muse::JsonObject curveObj;
        curveObj["type"] = muse::value(AUTOMATION_TYPE_TO_STRING, key.type);
        curveObj["staffId"] = key.staffId.toStdString();

        if (key.voiceIdx.has_value()) {
            curveObj["voiceId"] = static_cast<int>(key.voiceIdx.value());
        }

        muse::JsonArray pointArray;
        for (const auto& [tick, point] : curve) {
            if (!writeGenerated && point.itemId.has_value()) {
                continue;
            }

            muse::JsonObject pointObj;
            pointObj["tick"] = tick;
            pointObj["inValue"] = point.inValue;
            pointObj["outValue"] = point.outValue;
            pointObj["interpolation"] = muse::value(INTERPOLATION_TYPE_TO_STRING, point.interpolation);
            pointArray << pointObj;
        }

        if (pointArray.empty()) {
            continue;
        }

        curveObj["points"] = pointArray;
        rootArray << curveObj;
    }

    return muse::JsonDocument(rootArray).toJson();
}
