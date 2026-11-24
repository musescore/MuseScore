/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

#include <QLocale>

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
static const Settings::Key SHOW_LOADING_FAILED_WARNING_KEY(module_name, "musesounds/showLoadingFailedWarning");

static const Settings::Key MUSESOUNDS_CHECK_FOR_UPDATE_TEST_MODE(module_name, "musesounds/checkForUpdateTestMode");

static std::string utmParams(const std::string& campaignParams, const std::string& utmContent, const std::string& utmTerm)
{
    std::string params = campaignParams;

    if (!utmContent.empty()) {
        params += "&utm_content=" + utmContent;
    }

    if (!utmTerm.empty()) {
        params += "&utm_term=" + utmTerm;
    }

    return params;
}

static std::string museHubParams(const std::string& utmContent, const std::string& utmTerm = std::string())
{
    return utmParams("utm_medium=Referral&utm_source=mss&utm_campaign=MH_WW_REF_MSS_DESKTOP_ALL_261026_crosslink", utmContent, utmTerm);
}

static std::string museHubAppParams(const std::string& utmContent, const std::string& utmTerm = std::string())
{
    return utmParams("from=musescore&utm_medium=referral&utm_source=MSS&utm_campaign=MH_WW_REF_MSS_APP_ALL_011226_crosslink",
                     utmContent, utmTerm);
}

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
    settings()->setDefaultValue(MUSESOUNDS_CHECK_FOR_UPDATE_TEST_MODE, Val(false));
    settings()->setDefaultValue(SHOW_LOADING_FAILED_WARNING_KEY, Val(true));
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
           ? UriQuery("https://customers-api.musehub.com/graphql/v3")
           : UriQuery("https://customers-api-dev.musehub.com/graphql/v3");
}

MuseHubUrls MuseSoundsConfiguration::museHubUrls(MuseHubPage page, const std::string& utmContent,
                                                 const SoundLibraryInfo& soundLibrary) const
{
    const std::string baseUrl = globalConfiguration()->museHubWebUrl();

    switch (page) {
    case MuseHubPage::MuseSounds:
        return { "musehub://musesounds?" + museHubAppParams(utmContent),
                 baseUrl + "muse-sounds?" + museHubParams(utmContent) };
    case MuseHubPage::FreeMuseSounds:
        return { "musehub://musesounds?" + museHubAppParams(utmContent),
                 baseUrl + "free-musesounds?" + museHubParams(utmContent) };
    case MuseHubPage::MuseSoundsCore:
        return { "musehub://product/store?type=bundle&id=musesounds-core&" + museHubAppParams(utmContent),
                 baseUrl + "muse-sounds/musesounds-core?" + museHubParams(utmContent) };
    case MuseHubPage::MuseSoundsPro:
        return { "musehub://product/store?type=platformSubscription&id=musesounds-pro&" + museHubAppParams(utmContent),
                 baseUrl + "muse-sounds/musesounds-pro?" + museHubParams(utmContent) };
    case MuseHubPage::Plugins:
        return { "musehub://plugins?" + museHubAppParams(utmContent),
                 baseUrl + "plugins?" + museHubParams(utmContent) };
    case MuseHubPage::SoundLibrary: {
        IF_ASSERT_FAILED(!soundLibrary.code.empty()) {
            break;
        }

        const std::string codeStr = soundLibrary.code.toStdString();
        const std::string idStr = !soundLibrary.id.empty() ? soundLibrary.id.toStdString() : codeStr;

        return { "musehub://product/store?type=library&id=" + idStr + "&" + museHubAppParams(utmContent, codeStr),
                 baseUrl + "muse-sounds/" + codeStr + "?" + museHubParams(utmContent, codeStr) };
    }
    case MuseHubPage::MuseSamplerUpdate:
        return { "musehub://requestUpdateCheck?" + museHubAppParams(utmContent),
                 baseUrl + "?" + museHubParams(utmContent) };
    }

    return { "musehub://", baseUrl };
}

QUrl MuseSoundsConfiguration::checkForMuseSoundsUpdateUrl() const
{
    return !getSoundsTestMode()
           ? QUrl("https://updates.musescore.org/feed/musesounds.latest.xml")
           : QUrl("https://updates.musescore.org/feed/musesounds.latest.test.xml");
}

QUrl MuseSoundsConfiguration::checkForMuseSamplerUpdateUrl() const
{
    return QUrl("https://customers-api.musehub.com/graphql/v3");
}

QUrl MuseSoundsConfiguration::museSoundsTroubleshootUrl() const
{
    return QUrl(QString("https://support.musehub.com/%1/articles/15070619-musesounds-missing-in-musescore-studio")
                .arg(QLocale().bcp47Name()));
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

bool MuseSoundsConfiguration::showLoadingFailedWarning() const
{
    return settings()->value(SHOW_LOADING_FAILED_WARNING_KEY).toBool();
}

void MuseSoundsConfiguration::setShowLoadingFailedWarning(bool show)
{
    settings()->setSharedValue(SHOW_LOADING_FAILED_WARNING_KEY, Val(show));
}

bool MuseSoundsConfiguration::museSoundsCheckForUpdateTestMode() const
{
    return settings()->value(MUSESOUNDS_CHECK_FOR_UPDATE_TEST_MODE).toBool();
}

bool MuseSoundsConfiguration::getSoundsTestMode() const
{
    return settings()->value(GET_SOUNDS_TEST_MODE_KEY).toBool();
}
