/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#include "musesoundsconfiguration.h"

#include "settings.h"

using namespace mu::musesounds;
using namespace muse;
using namespace muse::network;

static const std::string module_name("musesounds");
static const Settings::Key GET_SOUNDS_TESTING_MODE_KEY(module_name, "musesounds/getSoundsTestingMode");

static const muse::String OPEN_SOUND_URL("https://www.musehub.com/muse-sounds/");

void MuseSoundsConfiguration::init()
{
    settings()->setDefaultValue(GET_SOUNDS_TESTING_MODE_KEY, Val(false));
}

RequestHeaders MuseSoundsConfiguration::headers() const
{
    RequestHeaders headers;
    headers.rawHeaders["Content-Type"] = "application/json";
    return headers;
}

UriQuery MuseSoundsConfiguration::soundsUri() const
{
    return !isTestingMode() ? UriQuery("https://cosmos-customer-webservice.azurewebsites.net/graphql")
           : UriQuery("https://cosmos-customer-webservice-dev.azurewebsites.net/graphql");
}

UriQuery MuseSoundsConfiguration::soundPageUri(const muse::String& soundCode) const
{
    String utm = String("?utm_source=mss-app&utm_medium=mh-cta&utm_campaign=mss-app-home-mh-%1").arg(soundCode);
    return UriQuery(OPEN_SOUND_URL + soundCode + utm);
}

bool MuseSoundsConfiguration::isTestingMode() const
{
    return settings()->value(GET_SOUNDS_TESTING_MODE_KEY).toBool();
}
