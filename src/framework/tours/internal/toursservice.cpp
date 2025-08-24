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

using namespace muse::tours;

void ToursService::registerTour(const String& eventCode, const Tour& tour)
{
    if (muse::contains(m_eventsMap, eventCode)) {
        return;
    }

    m_eventsMap.insert({ eventCode, tour });
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
