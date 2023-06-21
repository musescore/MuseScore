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

#include "updateservice.h"

#include <QBuffer>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonDocument>

#include "../updateerrors.h"
#include "types/version.h"
#include "muversion.h"

#include "translation.h"
#include "log.h"

using namespace mu::update;
using namespace mu::network;
using namespace mu::framework;

static std::string platformFileSuffix()
{
#if defined(Q_OS_WIN)
    return "msi";
#elif defined(Q_OS_MACOS)
    return "dmg";
#elif defined(Q_OS_LINUX)
    return "appimage";
#else
    return "";
#endif
}

mu::RetVal<ReleaseInfo> UpdateService::checkForUpdate()
{
    RetVal<ReleaseInfo> result;
    result.ret = make_ret(Err::NoUpdate);

    clear();

    QBuffer buff;
    m_networkManager = networkManagerCreator()->makeNetworkManager();
    Ret getUpdateInfo = m_networkManager->get(QString::fromStdString(configuration()->checkForUpdateUrl()), &buff,
                                              configuration()->checkForUpdateHeaders());

    if (!getUpdateInfo) {
        result.ret = make_ret(Err::NetworkError);
        return result;
    }

    QByteArray json = buff.data();
    LOGD() << "json: " << json;

    RetVal<ReleaseInfo> releaseInfo = parseRelease(json);
    if (!releaseInfo.ret) {
        return result;
    }

    if (!releaseInfo.val.isValid()) {
        return result;
    }

    Version current(MUVersion::fullVersion());
    Version update(String::fromStdString(releaseInfo.val.version));
    if (update <= current) {
        return result;
    }

    result.ret = make_ok();
    result.val = releaseInfo.val;

    m_lastCheckResult = result.val;
    return result;
}

mu::RetVal<mu::io::path_t> UpdateService::downloadRelease()
{
    RetVal<io::path_t> result;

    QBuffer buff;
    QUrl fileUrl = QUrl::fromUserInput(QString::fromStdString(m_lastCheckResult.fileUrl));

    m_updateProgress.started.notify();

    m_networkManager = networkManagerCreator()->makeNetworkManager();
    m_networkManager->progress().progressChanged.onReceive(this, [this](int64_t current, int64_t total, const std::string&) {
        m_updateProgress.progressChanged.send(current, total, trc("update", "Downloading MuseScore") + " " + m_lastCheckResult.version);
    });

    Ret ret = m_networkManager->get(fileUrl, &buff);
    if (!ret) {
        result.ret = ret;
        return result;
    }

    io::path_t installerPath = configuration()->updateDataPath() + "/" + m_lastCheckResult.fileName;
    fileSystem()->makePath(io::absoluteDirpath(installerPath));

    ret = fileSystem()->writeFile(installerPath, ByteArray::fromQByteArrayNoCopy(buff.data()));
    if (ret) {
        result.ret = ret;
        result.val = installerPath;
    }

    return result;
}

void UpdateService::cancelUpdate()
{
    if (m_networkManager) {
        m_networkManager->abort();
    }
}

mu::framework::Progress UpdateService::updateProgress()
{
    return m_updateProgress;
}

mu::RetVal<ReleaseInfo> UpdateService::parseRelease(const QByteArray& json) const
{
    RetVal<ReleaseInfo> result;

    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
        LOGE() << "failed parse, err: " << err.errorString();
        result.ret = make_ret(Ret::Code::InternalError);
        return result;
    }

    result.ret = make_ok();

    QJsonObject release = jsonDoc.object();

    QString tagName = release.value("tag_name").toString();
    QString version = tagName.replace("v", "");
    result.val.version = version.toStdString();

    result.val.notes = QString("### MuseScore %1\n\n%2")
                       .arg(version, release.value("bodyMarkdown").toString())
                       .toStdString();

    std::string fileSuffix = platformFileSuffix();

    QJsonArray assets = release.value("assets").toArray();
    for (const QJsonValue asset : assets) {
        QJsonObject assetObj = asset.toObject();

        QString name = assetObj.value("name").toString();
        if (io::suffix(name) != fileSuffix) {
            continue;
        }

        result.val.fileName = name.toStdString();
        result.val.fileUrl = assetObj.value("browser_download_url").toString().toStdString();
    }

    return result;
}

void UpdateService::clear()
{
    m_lastCheckResult = ReleaseInfo();

#if !defined(Q_OS_LINUX)
    fileSystem()->remove(configuration()->updateDataPath());
#endif
}
