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
#include "cloudconfiguration.h"

#include "config.h"
#include "settings.h"

#include <QRandomGenerator>

using namespace mu::cloud;
using namespace mu::network;
using namespace mu::framework;

static const std::string module_name("cloud");
static const Settings::Key CLIENT_ID_KEY(module_name, "cloud/clientId");

static QByteArray generateClientId()
{
    QByteArray qtGeneratedId(QSysInfo::machineUniqueId());
    if (!qtGeneratedId.isEmpty()) {
        return qtGeneratedId;
    }

    QRandomGenerator* generator = QRandomGenerator::global();

    long long randId = generator->generate();
    constexpr size_t randBytes = sizeof(decltype(generator->generate()));

    for (size_t bytes = randBytes; bytes < sizeof(randId); bytes += randBytes) {
        randId <<= 8 * randBytes;
        randId += generator->generate();
    }

    return QString::number(randId, 16).toLatin1();
}

static QString userAgent()
{
    static const QStringList systemInfo {
        QSysInfo::kernelType(),
        QSysInfo::kernelVersion(),
        QSysInfo::productType(),
        QSysInfo::productVersion(),
        QSysInfo::currentCpuArchitecture()
    };

    return QString("MS_EDITOR/%1.%2 (%3)")
           .arg(VERSION)
           .arg(BUILD_NUMBER)
           .arg(systemInfo.join(' ')).toLatin1();
}

void CloudConfiguration::init()
{
    if (settings()->value(CLIENT_ID_KEY).isNull()) {
        settings()->setSharedValue(CLIENT_ID_KEY, Val(generateClientId().toStdString()));
    }
}

RequestHeaders CloudConfiguration::headers() const
{
    RequestHeaders headers;
    headers.rawHeaders["Accept"] = "application/json";
    headers.rawHeaders["X-MS-CLIENT-ID"] = QByteArray::fromStdString(settings()->value(CLIENT_ID_KEY).toString());
    headers.knownHeaders[QNetworkRequest::UserAgentHeader] = userAgent();

    return headers;
}

QByteArray CloudConfiguration::uploadingLicense() const
{
    return "all-rights-reserved";
}

QUrl CloudConfiguration::cloudUrl() const
{
    return QUrl("https://musescore.com");
}

QUrl CloudConfiguration::authorizationUrl() const
{
    return QUrl("https://musescore.com/oauth/authorize");
}

QUrl CloudConfiguration::signUpUrl() const
{
    return QUrl("https://musescore.com/oauth/authorize-new");
}

QUrl CloudConfiguration::signInSuccessUrl() const
{
    return QUrl("https://musescore.com/desktop-signin-success");
}

QUrl CloudConfiguration::scoreManagerUrl() const
{
    return QUrl("https://musescore.com/my-scores");
}

QUrl CloudConfiguration::accessTokenUrl() const
{
    return apiRootUrl() + "/oauth/token";
}

QUrl CloudConfiguration::refreshApiUrl() const
{
    return apiRootUrl() + "/oauth/refresh";
}

QUrl CloudConfiguration::userInfoApiUrl() const
{
    return apiRootUrl() + "/me";
}

QUrl CloudConfiguration::logoutApiUrl() const
{
    return apiRootUrl() + "/oauth/logout";
}

QUrl CloudConfiguration::scoreInfoApiUrl() const
{
    return apiRootUrl() + "/score/info";
}

QUrl CloudConfiguration::uploadScoreApiUrl() const
{
    return apiRootUrl() + "/score/upload";
}

QUrl CloudConfiguration::uploadAudioApiUrl() const
{
    return apiRootUrl() + "/score/audio";
}

mu::io::path_t CloudConfiguration::tokensFilePath() const
{
    return globalConfiguration()->userAppDataPath() + "/cred.dat";
}

QString CloudConfiguration::apiRootUrl() const
{
    return "https://desktop.musescore.com/editor/v1";
}
