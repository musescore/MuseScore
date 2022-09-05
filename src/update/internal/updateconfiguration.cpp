/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "updateconfiguration.h"

#include "config.h"
#include "settings.h"

using namespace mu::update;
using namespace mu::framework;

static const std::string module_name("update");

static const Settings::Key CHECK_FOR_UPDATE_KEY(module_name, "application/checkForUpdate");
static const Settings::Key SKIPPED_VERSION_KEY(module_name, "application/skippedVersion");

static const std::string PRIVACY_POLICY_URL_PATH("/about/desktop-privacy-policy");
static const std::string CHECK_FOR_UPDATE_URL("https://updates.musescore.org/feed/latest.xml");

static QString userAgent()
{
    QString osName;
#if defined(Q_OS_WIN)
    osName = "Windows";
#elif defined (Q_OS_MACOS)
    osName = "Mac";
#else
    osName = "Linux";
#endif

    QString osVersion = QSysInfo::productVersion();
    QString cpuArchitecture = QSysInfo::currentCpuArchitecture();

    return QString("Musescore/%1 (%2 %3; %4)")
           .arg(VERSION, osName, osVersion, cpuArchitecture);
}

void UpdateConfiguration::init()
{
    settings()->setDefaultValue(CHECK_FOR_UPDATE_KEY, Val(isAppUpdatable()));
}

bool UpdateConfiguration::isAppUpdatable() const
{
#ifdef APP_UPDATABLE
    return true;
#else
    return false;
#endif
}

bool UpdateConfiguration::needCheckForUpdate() const
{
    return settings()->value(CHECK_FOR_UPDATE_KEY).toBool();
}

void UpdateConfiguration::setNeedCheckForUpdate(bool needCheck)
{
    settings()->setSharedValue(CHECK_FOR_UPDATE_KEY, Val(needCheck));
}

std::string UpdateConfiguration::skippedReleaseVersion() const
{
    return settings()->value(SKIPPED_VERSION_KEY).toString();
}

void UpdateConfiguration::setSkippedReleaseVersion(const std::string& version) const
{
    settings()->setSharedValue(SKIPPED_VERSION_KEY, Val(version));
}

std::string UpdateConfiguration::checkForUpdateUrl() const
{
    return CHECK_FOR_UPDATE_URL;
}

mu::network::RequestHeaders UpdateConfiguration::checkForUpdateHeaders() const
{
    network::RequestHeaders headers;
    headers.knownHeaders[QNetworkRequest::UserAgentHeader] = userAgent();

    return headers;
}

std::string UpdateConfiguration::museScoreUrl() const
{
    return globalConfiguration()->museScoreUrl();
}

std::string UpdateConfiguration::museScorePrivacyPolicyUrl() const
{
    return globalConfiguration()->museScoreUrl() + PRIVACY_POLICY_URL_PATH;
}

mu::io::path_t UpdateConfiguration::updateDataPath() const
{
    return globalConfiguration()->userAppDataPath() + "/update";
}
