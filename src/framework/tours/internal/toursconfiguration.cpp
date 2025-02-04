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
#include "toursconfiguration.h"

#include "settings.h"

#include "global/configreader.h"

#include "log.h"

using namespace muse;
using namespace muse::tours;
using namespace muse::async;

static const Settings::Key UI_LAST_SHOWN_TOURS_KEY("tours", "tours/lastShownTours");

String ToursConfiguration::lastShownTourIdForEvent(const String& eventCode) const
{
    StringList allLastShownTours = lastShownTours();
    for (const String& tourChainId : allLastShownTours) {
        if (tourChainId.startsWith(eventCode)) {
            String chainId = tourChainId;
            return chainId.remove(eventCode + u"/");
        }
    }

    return String();
}

void ToursConfiguration::setLastShownTourIdForEvent(const String& eventCode, const String& tourId)
{
    StringList allLastShownTours = lastShownTours();
    bool changed = false;

    String newTourId = eventCode + u"/" + tourId;

    for (size_t i = 0; i < allLastShownTours.size(); ++i) {
        const String& lastTourId = allLastShownTours[i];
        if (lastTourId.startsWith(eventCode)) {
            allLastShownTours[i] = newTourId;
            changed = true;
        }
    }

    if (!changed) {
        allLastShownTours.push_back(newTourId);
    }

    settings()->setSharedValue(UI_LAST_SHOWN_TOURS_KEY, Val(allLastShownTours.join(u",").toStdString()));
}

io::path_t ToursConfiguration::toursFilePath() const
{
    return ":/resources/tours.json";
}

StringList ToursConfiguration::lastShownTours() const
{
    return String::fromStdString(settings()->value(UI_LAST_SHOWN_TOURS_KEY).toString()).split(u",");
}
