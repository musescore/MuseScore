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

#include "musesamplerupdateservice.h"

#include <QBuffer>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonDocument>

#include "../updateerrors.h"
#include "types/version.h"

#include "translation.h"
#include "log.h"

using namespace muse::update;
using namespace muse::network;

muse::RetVal<ReleaseInfo> MuseSamplerUpdateService::checkForUpdate()
{
    RetVal<ReleaseInfo> result;
    result.ret = make_ret(Err::NoUpdate);

    clear();

    QBuffer buff;
    m_networkManager = networkManagerCreator()->makeNetworkManager();
    Ret getUpdateInfo = m_networkManager->get(QString::fromStdString(configuration()->checkForMuseSamplerUpdateUrl()), &buff,
                                              configuration()->updateHeaders());

    if (!getUpdateInfo) {
        result.ret = make_ret(Err::NetworkError);
        return result;
    }

    QByteArray json = buff.data();

    RetVal<ReleaseInfo> releaseInfoRetVal = parseRelease(json);
    if (!releaseInfoRetVal.ret) {
        return result;
    }

    if (!releaseInfoRetVal.val.isValid()) {
        return result;
    }

    Version current(museSamplerInfo()->version());
    Version update(releaseInfoRetVal.val.version);

    bool allowUpdateOnPreRelease = configuration()->allowUpdateOnPreRelease();
    bool isPreRelease = update.preRelease();

    if (!allowUpdateOnPreRelease && isPreRelease) {
        return result;
    }

    if (update <= current) {
        return result;
    }

    ReleaseInfo releaseInfo = releaseInfoRetVal.val;

    result.ret = make_ok();
    result.val = std::move(releaseInfo);

    m_lastCheckResult = result.val;

    return result;
}

muse::RetVal<ReleaseInfo> MuseSamplerUpdateService::lastCheckResult()
{
    if (m_lastCheckResult.isValid()) {
        return RetVal<ReleaseInfo>::make_ok(m_lastCheckResult);
    }

    return RetVal<ReleaseInfo>();
}

muse::Progress MuseSamplerUpdateService::updateProgress()
{
    return m_updateProgress;
}

void MuseSamplerUpdateService::openMuseHub()
{
    auto openMuseHubWebsite = [this]() {
        static const std::string MUSEHUB_URL = "https://www.musehub.com/";
        interactive()->openUrl(MUSEHUB_URL);
    };

#ifdef Q_OS_WIN
    static const std::string MUSEHUB_APPV1_IDENTIFIER = "muse-hub://";
    interactive()->openApp(MUSEHUB_APPV1_IDENTIFIER).onReject(this, [=](int, const std::string&) {
        openMuseHubWebsite();
    });
    return;
#elif defined(Q_OS_MAC)
    static const std::string MUSEHUB_UNIVERSAL_URL = "muse://launch?from=musescore";
    interactive()->openApp(MUSEHUB_UNIVERSAL_URL).onReject(this, [=](int, const std::string&) {
        static const std::string MUSEHUB_APP_IDENTIFIER = "com.muse.hub";
        interactive()->openApp(MUSEHUB_APP_IDENTIFIER).onReject(this, [=](int, const std::string&) {
            openMuseHubWebsite();
        });
    });
    return;
#else
    openMuseHubWebsite();
#endif
}

muse::RetVal<ReleaseInfo> MuseSamplerUpdateService::parseRelease(const QByteArray& json) const
{
    RetVal<ReleaseInfo> result;

    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
        LOGE() << "failed parse, err: " << err.errorString();
        result.ret = make_ret(Ret::Code::InternalError);
        return result;
    }

    QJsonObject releaseObj = jsonDoc.object();

    if (releaseObj.empty()) {
        LOGE() << "failed parse, no jsonObject";
        return result;
    }

    QString languageCode = languagesConfiguration()->currentLanguageCode().val;
    QString locale(QLocale(languageCode).bcp47Name());

    QJsonObject contentObj = releaseObj.value("content").toObject();

    if (!contentObj.contains(locale)) {
        static const QString DEFAULT_LOCALE = "en";
        locale = DEFAULT_LOCALE;

        if (!contentObj.contains(locale)) {
            LOGE() << "failed parse, no release content";
            result.ret = make_ret(Ret::Code::InternalError);
            return result;
        }
    }

    QJsonObject contentLocaleObj = contentObj.value(locale).toObject();

    result.ret = make_ok();

    result.val.version = releaseObj.value("version").toString().toStdString();

    result.val.notes = contentLocaleObj.value("notes").toString().toStdString();

    ValList featuresList;
    QJsonArray features = contentLocaleObj.value("features").toArray();
    for (const QJsonValue& feature : features) {
        featuresList.push_back(Val(feature.toString().toStdString()));
    }

    result.val.additionInfo.insert({ "features", Val(featuresList) });

    return result;
}

void MuseSamplerUpdateService::clear()
{
    m_lastCheckResult = ReleaseInfo();
}
