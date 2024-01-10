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

    RetVal<ReleaseInfo> releaseInfo = parseRelease(json);
    if (!releaseInfo.ret) {
        return result;
    }

    if (!releaseInfo.val.isValid()) {
        return result;
    }

    Version current(MUVersion::fullVersion());
    Version update(String::fromStdString(releaseInfo.val.version));

    bool allowUpdateOnPreRelease = configuration()->allowUpdateOnPreRelease();
    bool isPreRelease = update.preRelease();

    if (!allowUpdateOnPreRelease && isPreRelease) {
        return result;
    }

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
        m_updateProgress.progressChanged.send(
            current, total,

            //: Means that the download is currently in progress.
            //: %1 will be replaced by the version number of the version that is being downloaded.
            qtrc("update", "Downloading MuseScore %1").arg(QString::fromStdString(m_lastCheckResult.version)).toStdString());
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

    QJsonObject release = jsonDoc.object();

    QJsonObject assetObj = resolveReleaseAsset(release);
    if (assetObj.empty()) {
        LOGE() << "failed parse, no release asset";
        result.ret = make_ret(Ret::Code::InternalError);
        return result;
    }

    result.ret = make_ok();

    result.val.fileName = assetObj.value("name").toString().toStdString();
    result.val.fileUrl = assetObj.value("browser_download_url").toString().toStdString();

    QString tagName = release.value("tag_name").toString();
    QString version = tagName.replace("v", "");
    result.val.version = version.toStdString();

    result.val.notes = release.value("bodyMarkdown").toString().toStdString();

    return result;
}

std::string UpdateService::platformFileSuffix() const
{
    switch (systemInfo()->productType()) {
    case ISystemInfo::ProductType::Windows: return "msi";
    case ISystemInfo::ProductType::MacOS: return "dmg";
    case ISystemInfo::ProductType::Linux: return "appimage";
    case ISystemInfo::ProductType::Unknown: break;
    }

    return "";
}

mu::ISystemInfo::CpuArchitecture UpdateService::assetArch(const QString& asset) const
{
    if (asset.contains("aarch64")) {
        return ISystemInfo::CpuArchitecture::Arm64;
    } else if (asset.contains("arm")) {
        return ISystemInfo::CpuArchitecture::Arm;
    }

    return ISystemInfo::CpuArchitecture::x86_64;
}

QJsonObject UpdateService::resolveReleaseAsset(const QJsonObject& release) const
{
    std::string fileSuffix = platformFileSuffix();
    ISystemInfo::ProductType productType = systemInfo()->productType();
    ISystemInfo::CpuArchitecture arch = systemInfo()->cpuArchitecture();

    QJsonArray assets = release.value("assets").toArray();
    for (const QJsonValue& asset : release.value("assetsNew").toArray()) {
        assets.push_back(asset);
    }

    for (const QJsonValue asset : assets) {
        QJsonObject assetObj = asset.toObject();

        QString name = assetObj.value("name").toString();
        if (io::suffix(name) != fileSuffix) {
            continue;
        }

        if (productType == ISystemInfo::ProductType::Linux) {
            if (arch != ISystemInfo::CpuArchitecture::Unknown && arch != assetArch(name)) {
                continue;
            }
        }

        return assetObj;
    }

    return QJsonObject();
}

void UpdateService::clear()
{
    m_lastCheckResult = ReleaseInfo();

#if !defined(Q_OS_LINUX)
    fileSystem()->remove(configuration()->updateDataPath());
#endif
}
