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

#include "musesoundscheckupdateservice.h"

#include <QBuffer>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonDocument>

#include "musesoundserrors.h"

#include "log.h"

static const muse::Uri MUSEHUB_APP_URI("musehub://?from=musescore");
static const muse::Uri MUSEHUB_APP_V1_URI("muse-hub://?from=musescore");

using namespace mu::musesounds;
using namespace muse;
using namespace muse::update;
using namespace muse::network;

muse::Ret MuseSoundsCheckUpdateService::needCheckForUpdate() const
{
    if (!configuration()->needCheckForMuseSoundsUpdate()) {
        return false;
    }

#ifdef Q_OS_WIN
    return true;
#elif defined(Q_OS_MAC)
    //! NOTE: If there is installed MuseHub, but we can't open it, then we shouldn't check update
    static const std::string MUSEHUB_APP_IDENTIFIER = "com.muse.hub";
    bool isMuseHubExists = interactive()->isAppExists(MUSEHUB_APP_IDENTIFIER);
    if (isMuseHubExists) {
        bool canOpenMuseHubByUniversalUrl = interactive()->canOpenApp(MUSEHUB_APP_URI);
        return canOpenMuseHubByUniversalUrl;
    }

    return true;
#else
    return false;
#endif
}

muse::RetVal<ReleaseInfo> MuseSoundsCheckUpdateService::checkForUpdate()
{
    RetVal<ReleaseInfo> result;
    result.ret = make_ret(Err::NoUpdate);

    m_lastCheckResult = result;

    clear();

    QBuffer buff;
    QUrl url = configuration()->checkForMuseSoundsUpdateUrl();
    m_networkManager = networkManagerCreator()->makeNetworkManager();

    Ret getUpdateInfo = m_networkManager->get(url, &buff);

    if (!getUpdateInfo) {
        LOGE() << getUpdateInfo.toString();
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

    ReleaseInfo releaseInfo = releaseInfoRetVal.val;

    result.ret = make_ok();
    result.val = std::move(releaseInfo);

    m_lastCheckResult = result;

    return result;
}

muse::RetVal<ReleaseInfo> MuseSoundsCheckUpdateService::lastCheckResult()
{
    return m_lastCheckResult;
}

muse::Progress MuseSoundsCheckUpdateService::updateProgress()
{
    return m_updateProgress;
}

muse::RetVal<ReleaseInfo> MuseSoundsCheckUpdateService::parseRelease(const QByteArray& json) const
{
    RetVal<ReleaseInfo> result;

    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
        LOGE() << "parse error: " << err.errorString();
        result.ret = make_ret(Err::NoUpdate);
        return result;
    }

    /*
    {
        "version": String,
        "image_url": String,  // it can be base64 data, like "data:image/png;base64,iVBORw0KGgoA......"
        "content": {
            "locale_code": {
                "notes": String,
                "features": [String]
                "action_title": String // title of action button
            }
        },

        // open app or web page url, try in order,
        // like this ["musehub://?from=musescore", "muse-hub://?from=musescore", "https://www.musehub.com"]
        "actions": {
            "windows": [String],
            "macos": [String],
            "linux": [String]
        }
    }
    */

    QJsonObject releaseObj = jsonDoc.object();

    if (releaseObj.empty()) {
        result.ret = make_ret(Err::NoUpdate);
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
    result.val.imageUrl = releaseObj.value("image_url").toString().toStdString();

    result.val.notes = contentLocaleObj.value("notes").toString().toStdString();
    ValList featuresList;
    QJsonArray features = contentLocaleObj.value("features").toArray();
    for (const QJsonValue& feature : features) {
        featuresList.push_back(Val(feature.toString().toStdString()));
    }
    result.val.additionInfo.insert({ "features", Val(featuresList) });

    result.val.actionTitle = contentLocaleObj.value("action_title").toString().toStdString();
    result.val.cancelTitle = contentLocaleObj.value("cancel_title").toString().toStdString();

    QJsonObject actionsObj = releaseObj.value("actions").toObject();

#ifdef Q_OS_WIN
    QJsonArray actionsArr = actionsObj.value("windows").toArray();
    for (const QJsonValue& a : actionsArr) {
        result.val.actions.push_back(Val(a.toString().toStdString()));
    }

    // def
    if (result.val.actions.empty()) {
        result.val.actions.push_back(Val(MUSEHUB_APP_URI.toString()));
        result.val.actions.push_back(Val(MUSEHUB_APP_V1_URI.toString()));
        result.val.actions.push_back(Val(globalConfiguration()->museHubWebUrl()));
    }

#elif defined(Q_OS_MAC)
    QJsonArray actionsArr = actionsObj.value("macos").toArray();
    for (const QJsonValue& a : actionsArr) {
        result.val.actions.push_back(Val(a.toString().toStdString()));
    }

    // def
    if (result.val.actions.empty()) {
        result.val.actions.push_back(Val(MUSEHUB_APP_URI.toString()));
        result.val.actions.push_back(Val(globalConfiguration()->museHubWebUrl()));
    }
#else
    QJsonArray actionsArr = actionsObj.value("linux").toArray();
    for (const QJsonValue& a : actionsArr) {
        result.val.actions.push_back(Val(a.toString().toStdString()));
    }

    // def
    if (result.val.actions.empty()) {
        result.val.actions.push_back(Val(globalConfiguration()->museHubWebUrl()));
    }
#endif

    return result;
}

void MuseSoundsCheckUpdateService::clear()
{
    m_lastCheckResult = RetVal<ReleaseInfo>();
}
