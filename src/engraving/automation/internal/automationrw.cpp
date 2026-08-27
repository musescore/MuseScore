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

#include "engraving/automation/automationdata.h"

#include "global/serialization/json.h"
#include "global/containers.h"
#include "global/log.h"

using namespace mu::engraving;

static const std::unordered_map<AutomationType, muse::String> AUTOMATION_TYPE_TO_STRING {
    { AutomationType::Dynamics, u"Dynamics" },
    { AutomationType::Tempo, u"Tempo" },
    { AutomationType::Volume, u"Volume" },
    { AutomationType::Pan, u"Pan" },
};

static constexpr const char* TYPE_KEY = "type";
static constexpr const char* PART_ID_KEY = "partId";
static constexpr const char* INSTRUMENT_ID_KEY = "instrumentId";
static constexpr const char* STAFF_ID_KEY = "staffId";
static constexpr const char* VOICE_ID_KEY = "voiceId";
static constexpr const char* OUT_VALUE_KEY = "outValue";
static constexpr const char* IN_VALUE_KEY = "inValue";
static constexpr const char* IN_VALUE_KIND_KEY = "inValueKind";
static constexpr const char* EASE_KEY = "ease";
static constexpr const char* CONTROL_POINT_T_KEY = "t";
static constexpr const char* CONTROL_POINT_VALUE_KEY = "value";
static constexpr const char* ITEM_ID_KEY = "itemId";
static constexpr const char* GENERATED_KEY = "generated";
static constexpr const char* TICK_KEY = "tick";
static constexpr const char* POINTS_KEY = "points";

static const std::string IN_VALUE_KIND_FROM_PREVIOUS = "FromPrevious";

static AutomationCurveKey readKey(const muse::JsonObject& obj)
{
    const AutomationType type = muse::key(AUTOMATION_TYPE_TO_STRING, obj.value(TYPE_KEY).toString(), AutomationType::Unknown);

    if (obj.contains(PART_ID_KEY)) {
        InstrumentTrackId trackId;
        trackId.partId = muse::ID(obj.value(PART_ID_KEY).toString().toStdString());
        trackId.instrumentId = obj.value(INSTRUMENT_ID_KEY).toString();
        return AutomationCurveKey::instrument(type, trackId);
    }

    if (obj.contains(STAFF_ID_KEY)) {
        const muse::ID staffId(obj.value(STAFF_ID_KEY).toString().toStdString());

        std::optional<size_t> voiceIdx;
        if (obj.contains(VOICE_ID_KEY)) {
            voiceIdx = static_cast<size_t>(obj.value(VOICE_ID_KEY).toInt());
        }

        return AutomationCurveKey::staff(type, staffId, voiceIdx);
    }

    return AutomationCurveKey::global(type);
}

static AutomationPoint readPoint(const muse::JsonObject& obj)
{
    AutomationPoint point;
    point.value.outValue = obj.value(OUT_VALUE_KEY).toDouble();

    const std::string inValueKind = obj.contains(IN_VALUE_KIND_KEY) ? obj.value(IN_VALUE_KIND_KEY).toStdString() : std::string();
    if (inValueKind == IN_VALUE_KIND_FROM_PREVIOUS) {
        point.value.inValue = AutomationPoint::ArrivalFromPrevious {};
    } else {
        AutomationPoint::Ease ease;
        if (obj.contains(EASE_KEY)) {
            // Array for forward-compat; only element 0 is used, since Ease only holds one point
            const muse::JsonArray easeArray = obj.value(EASE_KEY).toArray();
            if (!easeArray.empty()) {
                const muse::JsonObject controlPointObj = easeArray.at(0).toObject();
                ease.t = controlPointObj.value(CONTROL_POINT_T_KEY).toDouble();
                ease.value = controlPointObj.value(CONTROL_POINT_VALUE_KEY).toDouble();
            }
        }

        const muse::real_t value = muse::real_t(obj.value(IN_VALUE_KEY).toDouble());
        point.value.inValue = AutomationPoint::ExplicitArrival { value, ease };
    }

    if (obj.contains(ITEM_ID_KEY)) {
        point.itemId = EID::fromStdString(obj.value(ITEM_ID_KEY).toString().toStdString());
    }
    if (obj.contains(GENERATED_KEY)) {
        point.generated = obj.value(GENERATED_KEY).toBool();
    }

    return point;
}

static void writeKey(const AutomationCurveKey& key, muse::JsonObject& obj)
{
    obj[TYPE_KEY] = muse::value(AUTOMATION_TYPE_TO_STRING, key.type);

    if (const std::optional<InstrumentTrackId> trackId = key.trackId()) {
        obj[PART_ID_KEY] = trackId->partId.toStdString();
        obj[INSTRUMENT_ID_KEY] = trackId->instrumentId;
        return;
    }

    if (const std::optional<muse::ID> staffId = key.staffId()) {
        obj[STAFF_ID_KEY] = staffId->toStdString();

        if (const std::optional<size_t> voiceIdx = key.voiceIdx()) {
            obj[VOICE_ID_KEY] = static_cast<int>(*voiceIdx);
        }
    }
}

static void writePoint(const AutomationPoint& point, muse::JsonObject& obj)
{
    if (std::holds_alternative<AutomationPoint::ArrivalFromPrevious>(point.value.inValue)) {
        obj[IN_VALUE_KIND_KEY] = IN_VALUE_KIND_FROM_PREVIOUS;
    } else {
        const AutomationPoint::ExplicitArrival& explicitArrival = std::get<AutomationPoint::ExplicitArrival>(point.value.inValue);
        obj[IN_VALUE_KEY] = explicitArrival.value;
        if (!explicitArrival.ease.isNone()) {
            muse::JsonObject controlPointObj;
            controlPointObj[CONTROL_POINT_T_KEY] = explicitArrival.ease.t.raw();
            controlPointObj[CONTROL_POINT_VALUE_KEY] = explicitArrival.ease.value.raw();

            muse::JsonArray easeArray;
            easeArray << controlPointObj;
            obj[EASE_KEY] = easeArray;
        }
    }

    obj[OUT_VALUE_KEY] = point.value.outValue;

    if (point.itemId.has_value()) {
        obj[ITEM_ID_KEY] = point.itemId->toStdString();
    }
    if (point.generated) {
        obj[GENERATED_KEY] = point.generated;
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

        const muse::JsonArray pointArray = curveObj.value(POINTS_KEY).toArray();
        if (pointArray.empty()) {
            continue;
        }

        AutomationCurve& curve = curves[key];
        for (size_t j = 0; j < pointArray.size(); ++j) {
            const muse::JsonObject pointObj = pointArray.at(j).toObject();
            const AutomationPoint point = readPoint(pointObj);
            const utick_t tick = pointObj.value(TICK_KEY).toInt();
            // Points are written in ascending tick order, so end() is always the right hint
            curve.try_emplace(curve.end(), tick, point);
        }
    }

    data.setCurves(curves);
}

muse::ByteArray AutomationRW::write(const AutomationData& data, bool writeGenerated)
{
    TRACEFUNC;

    muse::JsonArray rootArray;
    for (const auto& [key, curve] : data.curves()) {
        IF_ASSERT_FAILED(key.isValid()) {
            continue;
        }

        muse::JsonArray pointArray;
        for (const auto& [tick, point] : curve) {
            if (!writeGenerated && point.generated) {
                continue;
            }

            muse::JsonObject pointObj;
            pointObj[TICK_KEY] = tick;
            writePoint(point, pointObj);
            pointArray << pointObj;
        }

        if (!pointArray.empty()) {
            muse::JsonObject curveObj;
            writeKey(key, curveObj);
            curveObj[POINTS_KEY] = pointArray;
            rootArray << curveObj;
        }
    }

    return muse::JsonDocument(rootArray).toJson();
}
