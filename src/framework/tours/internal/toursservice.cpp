/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include "toursservice.h"

#include "io/file.h"
#include "serialization/json.h"

#include "log.h"

using namespace muse::tours;

void ToursService::init()
{
    initTours();
}

void ToursService::onEvent(const String& eventCode)
{
    if (!muse::contains(m_eventsMap, eventCode)) {
        return;
    }

    const Tour& tour = m_eventsMap[eventCode];

    String lastTourId = toursConfiguration()->lastShownTourIdForEvent(eventCode);
    if (lastTourId == tour.id) {
        return;
    }

    toursProvider()->showTour(tour);

    toursConfiguration()->setLastShownTourIdForEvent(eventCode, tour.id);
}

void ToursService::initTours()
{
    io::path_t filePath = toursConfiguration()->toursFilePath();
    std::unordered_map<String, Tour> result;

    ByteArray data;
    Ret ret = io::File::readFile(filePath, data);
    if (!ret) {
        LOGE() << "failed read file: " << filePath << ", err: " << ret.toString();
        return;
    }

    std::string err;
    JsonObject obj = JsonDocument::fromJson(data, &err).rootObject();
    if (!err.empty()) {
        LOGE() << "failed parse file: " << filePath << ", err: " << err;
        return;
    }

    for (const std::string& eventCode : obj.keys()) {
        Tour tour;

        JsonObject tourObj = obj.value(eventCode).toObject();
        tour.id = tourObj.value("id").toString();

        JsonArray steps = tourObj.value("steps").toArray();
        if (steps.empty()) {
            continue;
        }

        for (size_t itemIdx = 0; itemIdx < steps.size(); ++itemIdx) {
            JsonObject itemObj = steps.at(itemIdx).toObject();
            if (itemObj.empty()) {
                continue;
            }

            TourStep step;
            step.title = itemObj.value("title").toString();
            step.description = itemObj.value("description").toString();
            step.videoExplanationUrl = itemObj.value("video_explanation_url").toString();
            step.controlUri = Uri(itemObj.value("control_uri").toString());

            tour.steps.emplace_back(step);
        }

        if (!tour.steps.empty()) {
            result.insert({ String::fromStdString(eventCode), tour });
        }
    }

    m_eventsMap = result;
}
