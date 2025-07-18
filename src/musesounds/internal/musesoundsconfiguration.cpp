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
#include "translation.h"

using namespace mu::musesounds;
using namespace muse;
using namespace muse::network;

static const std::string module_name("musesounds");
static const Settings::Key CHECK_FOR_UPDATE_KEY(module_name, "musesounds/checkForUpdate");
static const Settings::Key GET_SOUNDS_TESTING_MODE_KEY(module_name, "musesounds/getSoundsTestingMode");
static const Settings::Key LAST_MUSESOUNDS_SHOWN_VERSION_KEY(module_name, "application/lastShownMuseSoundsReleaseVersion");

static const muse::String OPEN_SOUND_URL("https://www.musehub.com/muse-sounds/");

void MuseSoundsConfiguration::init()
{
    settings()->setDefaultValue(CHECK_FOR_UPDATE_KEY, Val(true));
    settings()->setCanBeManuallyEdited(CHECK_FOR_UPDATE_KEY, true);
    settings()->setDescription(CHECK_FOR_UPDATE_KEY, muse::trc("musesounds", "Show occasional MuseHub promotions"));

    settings()->setDefaultValue(GET_SOUNDS_TESTING_MODE_KEY, Val(false));
}

bool MuseSoundsConfiguration::needCheckForUpdate() const
{
    return settings()->value(CHECK_FOR_UPDATE_KEY).toBool();
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

UriQuery MuseSoundsConfiguration::checkForMuseSoundsUpdateUrl()
{
    return !isTestingMode()
           ? UriQuery("https://updates.musescore.org/feed/musesounds.latest.xml")
           : UriQuery("https://updates.musescore.org/feed/musesounds.latest.test.xml");
}

std::string MuseSoundsConfiguration::lastShownMuseSoundsReleaseVersion() const
{
    return settings()->value(LAST_MUSESOUNDS_SHOWN_VERSION_KEY).toString();
}

void MuseSoundsConfiguration::setLastShownMuseSoundsReleaseVersion(const std::string& version)
{
    settings()->setSharedValue(LAST_MUSESOUNDS_SHOWN_VERSION_KEY, Val(version));
}

bool MuseSoundsConfiguration::isTestingMode() const
{
    return settings()->value(GET_SOUNDS_TESTING_MODE_KEY).toBool();
}
