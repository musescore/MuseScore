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

#include "engraving/automation/automationdata.h"

using namespace mu::engraving;

static const std::unordered_map<AutomationType, muse::String> AUTOMATION_TYPE_TO_STRING {
    { AutomationType::Dynamics, u"Dynamics" },
    { AutomationType::Volume, u"Volume" },
    { AutomationType::Pan, u"Pan" },
};

static AutomationCurveKey readKey(const muse::JsonObject& obj)
{
    const AutomationType type = muse::key(AUTOMATION_TYPE_TO_STRING, obj.value("type").toString(), AutomationType::Unknown);

    if (obj.contains("partId")) {
        InstrumentTrackId trackId;
        trackId.partId = muse::ID(obj.value("partId").toString().toStdString());
        trackId.instrumentId = obj.value("instrumentId").toString();
        return AutomationCurveKey::instrument(type, trackId);
    }

    if (obj.contains("staffId")) {
        const muse::ID staffId(obj.value("staffId").toString().toStdString());

        std::optional<size_t> voiceIdx;
        if (obj.contains("voiceId")) {
            voiceIdx = static_cast<size_t>(obj.value("voiceId").toInt());
        }

        return AutomationCurveKey::staff(type, staffId, voiceIdx);
    }

    return AutomationCurveKey::global(type);
}

static AutomationPoint readPoint(const muse::JsonObject& obj)
{
    AutomationPoint point;
    point.value.outValue = obj.value("outValue").toDouble();

    const muse::String inValueKind = obj.contains("inValueKind") ? obj.value("inValueKind").toString() : muse::String();
    if (inValueKind == u"FromPrevious") {
        point.value.inValue = AutomationPoint::ArrivalFromPrevious {};
    } else {
        AutomationPoint::Bend bend;
        if (obj.contains("bend")) {
            const muse::JsonObject bendObj = obj.value("bend").toObject();
            bend.t = bendObj.value("t").toDouble();
            bend.value = bendObj.value("value").toDouble();
        }

        const muse::real_t value = muse::real_t(obj.value("inValue").toDouble());
        point.value.inValue = AutomationPoint::ExplicitArrival { value, bend };
    }

    if (obj.contains("itemId")) {
        point.itemId = EID::fromStdString(obj.value("itemId").toString().toStdString());
    }
    if (obj.contains("generated")) {
        point.generated = obj.value("generated").toBool();
    }

    return point;
}

static void writeKey(const AutomationCurveKey& key, muse::JsonObject& obj)
{
    obj["type"] = muse::value(AUTOMATION_TYPE_TO_STRING, key.type);

    if (const std::optional<InstrumentTrackId> trackId = key.trackId()) {
        obj["partId"] = trackId->partId.toStdString();
        obj["instrumentId"] = trackId->instrumentId;
        return;
    }

    if (const std::optional<muse::ID> staffId = key.staffId()) {
        obj["staffId"] = staffId->toStdString();

        if (const std::optional<size_t> voiceIdx = key.voiceIdx()) {
            obj["voiceId"] = static_cast<int>(*voiceIdx);
        }
    }
}

static void writePoint(const AutomationPoint& point, muse::JsonObject& obj)
{
    if (std::holds_alternative<AutomationPoint::ArrivalFromPrevious>(point.value.inValue)) {
        obj["inValueKind"] = muse::String(u"FromPrevious");
    } else {
        const AutomationPoint::ExplicitArrival& explicitArrival = std::get<AutomationPoint::ExplicitArrival>(point.value.inValue);
        obj["inValue"] = explicitArrival.value;
        if (!explicitArrival.bend.isNone()) {
            muse::JsonObject bendObj;
            bendObj["t"] = explicitArrival.bend.t.raw();
            bendObj["value"] = explicitArrival.bend.value.raw();
            obj["bend"] = bendObj;
        }
    }

    obj["outValue"] = point.value.outValue;

    if (point.itemId.has_value()) {
        obj["itemId"] = point.itemId->toStdString();
    }
    if (point.generated) {
        obj["generated"] = point.generated;
    }
}

void AutomationRW::read(AutomationData& data, const muse::ByteArray& json)
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

    AutomationCurveMap curves;

    const muse::JsonArray rootArray = doc.rootArray();
    for (size_t i = 0; i < rootArray.size(); ++i) {
        const muse::JsonObject curveObj = rootArray.at(i).toObject();
        const AutomationCurveKey key = readKey(curveObj);
        IF_ASSERT_FAILED(key.isValid()) {
            continue;
        }

        const muse::JsonArray pointArray = curveObj.value("points").toArray();
        if (pointArray.empty()) {
            continue;
        }

        AutomationCurve& curve = curves[key];
        for (size_t j = 0; j < pointArray.size(); ++j) {
            const muse::JsonObject pointObj = pointArray.at(j).toObject();
            const AutomationPoint point = readPoint(pointObj);
            const utick_t tick = pointObj.value("tick").toInt();
            curve.insert_or_assign(tick, point);
        }
    }

    data.setCurves(curves);
}

muse::ByteArray AutomationRW::write(const AutomationData& data, bool writeGenerated)
{
    TRACEFUNC;

    muse::JsonArray rootArray;
    for (const auto& [key, curve] : data.curves()) {
        if (curve.empty()) {
            continue;
        }

        muse::JsonObject curveObj;
        writeKey(key, curveObj);

        muse::JsonArray pointArray;
        for (const auto& [tick, point] : curve) {
            if (!writeGenerated && point.generated) {
                continue;
            }

            muse::JsonObject pointObj;
            pointObj["tick"] = tick;
            writePoint(point, pointObj);
            pointArray << pointObj;
        }

        if (!pointArray.empty()) {
            curveObj["points"] = pointArray;
            rootArray << curveObj;
        }
    }

    return muse::JsonDocument(rootArray).toJson();
}
