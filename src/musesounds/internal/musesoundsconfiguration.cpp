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
#include "log.h"

using namespace mu::musesounds;
using namespace muse;
using namespace muse::network;

static const std::string module_name("musesounds");
static const Settings::Key CHECK_FOR_MUSESOUNDS_UPDATE_KEY(module_name, "musesounds/checkForUpdate");
static const Settings::Key GET_SOUNDS_TEST_MODE_KEY(module_name, "musesounds/getSoundsTestMode");
static const Settings::Key LAST_MUSESOUNDS_SHOWN_VERSION_KEY(module_name, "application/lastShownMuseSoundsReleaseVersion");

static const Settings::Key MUSESOUNDS_CHECK_FOR_UPDATE_TEST_MODE(module_name, "musesounds/checkForUpdateTestMode");
static const Settings::Key MUSESAMPLER_CHECK_FOR_UPDATE_TEST_MODE(module_name, "museSampler/checkForUpdateTestMode");
static const Settings::Key MUSESAMPLER_UPDATE_AVAILABLE(module_name, "museSampler/updateAvailable");

static const muse::String OPEN_SOUND_URL("https://www.musehub.com/muse-sounds/");

static QString osCode()
{
#if defined(Q_OS_WIN)
    return "win";
#elif defined(Q_OS_MAC)
    return "mac";
#elif defined(Q_OS_LINUX)
    return "linux";
#else
    UNREACHABLE;
    return QString();
#endif
}

void MuseSoundsConfiguration::init()
{
    settings()->setDefaultValue(CHECK_FOR_MUSESOUNDS_UPDATE_KEY, Val(true));
    settings()->setCanBeManuallyEdited(CHECK_FOR_MUSESOUNDS_UPDATE_KEY, true);
    settings()->setDescription(CHECK_FOR_MUSESOUNDS_UPDATE_KEY, muse::trc("musesounds", "Show occasional MuseHub promotions"));

    settings()->setDefaultValue(GET_SOUNDS_TEST_MODE_KEY, Val(false));
    settings()->setDefaultValue(MUSESAMPLER_CHECK_FOR_UPDATE_TEST_MODE, Val(false));

    settings()->setDefaultValue(MUSESAMPLER_UPDATE_AVAILABLE, Val(false));
    settings()->setDefaultValue(MUSESOUNDS_CHECK_FOR_UPDATE_TEST_MODE, Val(false));
}

bool MuseSoundsConfiguration::needCheckForMuseSoundsUpdate() const
{
    return settings()->value(CHECK_FOR_MUSESOUNDS_UPDATE_KEY).toBool();
}

RequestHeaders MuseSoundsConfiguration::headers() const
{
    RequestHeaders headers;
    headers.rawHeaders["Content-Type"] = "application/json";
    return headers;
}

UriQuery MuseSoundsConfiguration::soundsUri() const
{
    return !getSoundsTestMode()
           ? UriQuery("https://cosmos-customer-webservice.azurewebsites.net/graphql")
           : UriQuery("https://cosmos-customer-webservice-dev.azurewebsites.net/graphql");
}

UriQuery MuseSoundsConfiguration::soundPageUri(const muse::String& soundCode) const
{
    String utm = String("?utm_source=mss-app&utm_medium=mh-cta&utm_campaign=mss-app-home-mh-%1").arg(soundCode);
    return UriQuery(OPEN_SOUND_URL + soundCode + utm);
}

QUrl MuseSoundsConfiguration::checkForMuseSoundsUpdateUrl() const
{
    return !getSoundsTestMode()
           ? QUrl("https://updates.musescore.org/feed/musesounds.latest.xml")
           : QUrl("https://updates.musescore.org/feed/musesounds.latest.test.xml");
}

QUrl MuseSoundsConfiguration::checkForMuseSamplerUpdateUrl() const
{
    return QUrl("https://cosmos-customer-webservice.azurewebsites.net/graphql");
}

QString MuseSoundsConfiguration::getMuseSamplerVersionQuery() const
{
    return QString("{\"query\": \"{ product(id: \\\"8428d876-f808-4524-918b-63ae5ca3c70e\\\", productType: application) { "
                   "... on ProductApplication { assets(os: %1) { version } } } }\"}").arg(osCode());
}

std::string MuseSoundsConfiguration::lastShownMuseSoundsReleaseVersion() const
{
    return settings()->value(LAST_MUSESOUNDS_SHOWN_VERSION_KEY).toString();
}

void MuseSoundsConfiguration::setLastShownMuseSoundsReleaseVersion(const std::string& version)
{
    settings()->setSharedValue(LAST_MUSESOUNDS_SHOWN_VERSION_KEY, Val(version));
}

bool MuseSoundsConfiguration::museSoundsCheckForUpdateTestMode() const
{
    return settings()->value(MUSESAMPLER_CHECK_FOR_UPDATE_TEST_MODE).toBool();
}

bool MuseSoundsConfiguration::museSamplerCheckForUpdateTestMode() const
{
    return settings()->value(MUSESAMPLER_CHECK_FOR_UPDATE_TEST_MODE).toBool();
}

bool MuseSoundsConfiguration::museSamplerUpdateAvailable() const
{
    return settings()->value(MUSESAMPLER_UPDATE_AVAILABLE).toBool();
}

void MuseSoundsConfiguration::setMuseSamplerUpdateAvailable(bool value)
{
    settings()->setSharedValue(MUSESAMPLER_UPDATE_AVAILABLE, Val(value));
}

bool MuseSoundsConfiguration::getSoundsTestMode() const
{
    return settings()->value(GET_SOUNDS_TEST_MODE_KEY).toBool();
}
