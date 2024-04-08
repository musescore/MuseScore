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

#include "appupdateservice.h"

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

using namespace muse;
using namespace muse::update;
using namespace muse::network;

muse::RetVal<ReleaseInfo> AppUpdateService::checkForUpdate()
{
    RetVal<ReleaseInfo> result;
    result.ret = make_ret(Err::NoUpdate);

    clear();

    QBuffer buff;
    m_networkManager = networkManagerCreator()->makeNetworkManager();
    Ret getUpdateInfo = m_networkManager->get(QString::fromStdString(configuration()->checkForAppUpdateUrl()), &buff,
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

    Version current(application()->fullVersion());
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
    releaseInfo.previousReleasesNotes = previousReleasesNotes(update);

    result.ret = muse::make_ok();
    result.val = std::move(releaseInfo);

    m_lastCheckResult = result.val;

    return result;
}

muse::RetVal<muse::io::path_t> AppUpdateService::downloadRelease()
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
            muse::qtrc("update", "Downloading MuseScore %1").arg(QString::fromStdString(m_lastCheckResult.version)).toStdString());
    });

    Ret ret = m_networkManager->get(fileUrl, &buff);
    if (!ret) {
        result.ret = ret;
        return result;
    }

    io::path_t installerPath = configuration()->updateDataPath() + "/" + m_lastCheckResult.fileName;
    fileSystem()->makePath(muse::io::absoluteDirpath(installerPath));

    ret = fileSystem()->writeFile(installerPath, ByteArray::fromQByteArrayNoCopy(buff.data()));
    if (ret) {
        result.ret = ret;
        result.val = installerPath;
    }

    return result;
}

void AppUpdateService::cancelUpdate()
{
    if (m_networkManager) {
        m_networkManager->abort();
    }
}

Progress AppUpdateService::updateProgress()
{
    return m_updateProgress;
}

RetVal<ReleaseInfo> AppUpdateService::parseRelease(const QByteArray& json) const
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

    result.ret = muse::make_ok();

    result.val.fileName = assetObj.value("name").toString().toStdString();
    result.val.fileUrl = assetObj.value("browser_download_url").toString().toStdString();

    QString tagName = release.value("tag_name").toString();
    QString version = tagName.replace("v", "");
    result.val.version = version.toStdString();

    result.val.notes = release.value("bodyMarkdown").toString().toStdString();

    return result;
}

std::string AppUpdateService::platformFileSuffix() const
{
    switch (systemInfo()->productType()) {
    case ISystemInfo::ProductType::Windows: return "msi";
    case ISystemInfo::ProductType::MacOS: return "dmg";
    case ISystemInfo::ProductType::Linux: return "appimage";
    case ISystemInfo::ProductType::Unknown: break;
    }

    return "";
}

ISystemInfo::CpuArchitecture AppUpdateService::assetArch(const QString& asset) const
{
    if (asset.contains("aarch64")) {
        return ISystemInfo::CpuArchitecture::Arm64;
    } else if (asset.contains("arm")) {
        return ISystemInfo::CpuArchitecture::Arm;
    }

    return ISystemInfo::CpuArchitecture::x86_64;
}

QJsonObject AppUpdateService::resolveReleaseAsset(const QJsonObject& release) const
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

PrevReleasesNotesList AppUpdateService::previousReleasesNotes(const Version& updateVersion) const
{
    PrevReleasesNotesList result;

    QBuffer buff;
    Ret getPreviousReleaseNotes = m_networkManager->get(QString::fromStdString(configuration()->previousAppReleasesNotesUrl()), &buff,
                                                        configuration()->updateHeaders());
    if (!getPreviousReleaseNotes) {
        LOGE() << "failed to get previous release notes: " << getPreviousReleaseNotes.toString();
        return result;
    }

    QByteArray json = buff.data();

    PrevReleasesNotesList previousReleasesNotes = parsePreviousReleasesNotes(json);
    if (previousReleasesNotes.empty()) {
        return result;
    }

    Version currentVersion = Version(application()->fullVersion());

    for (const PrevReleaseNotes& releaseNotes : previousReleasesNotes) {
        Version previousVersion = Version(releaseNotes.version);
        if (updateVersion == previousVersion) {
            continue;
        }

        if (currentVersion < previousVersion) {
            String notesStr = String::fromStdString(releaseNotes.notes);
            if (notesStr.startsWith(u"###")) {
                //! Release notes may be in the format of: ### MuseScore x.y.z is now available!\r\n...notes...\r\n
                //! We need to remove the title of the release notes to get the actual notes.
                static const std::regex titleRegex(R"(^###.*?\r\n)");
                std::string notesWithoutTitle = notesStr.remove(titleRegex).toStdString();
                result.emplace_back(releaseNotes.version, notesWithoutTitle);
            } else {
                result.emplace_back(releaseNotes.version, releaseNotes.notes);
            }
        }
    }

    std::sort(result.begin(), result.end(), [](const PrevReleaseNotes& a,
                                               const PrevReleaseNotes& b) {
        return Version(a.version) < Version(b.version);
    });

    return result;
}

PrevReleasesNotesList AppUpdateService::parsePreviousReleasesNotes(const QByteArray& json) const
{
    PrevReleasesNotesList result;

    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
        LOGE() << "failed parse, err: " << err.errorString();
        return result;
    }

    QJsonObject jsonObject = jsonDoc.object();

    if (jsonObject.empty()) {
        LOGE() << "failed parse, no jsonObject";
        return result;
    }

    QJsonArray releases = jsonObject.value("releases").toArray();
    for (const QJsonValue& release : releases) {
        QJsonObject releaseObj = release.toObject();
        std::string version = releaseObj.value("version").toString().toStdString();

        result.push_back({ version, releaseObj.value("notes").toString().toStdString() });
    }

    return result;
}

void AppUpdateService::clear()
{
    m_lastCheckResult = ReleaseInfo();

#if !defined(Q_OS_LINUX)
    fileSystem()->remove(configuration()->updateDataPath());
#endif
}
