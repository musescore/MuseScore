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

const QString INSTALLED_WEEK_BEGINNING_KEY("Installed-Week-Beginning");
const QString PREVIOUS_REQUEST_DAY_KEY("Previous-Request-Day");

static QDate calculateWeekBeginForDate(const QDate& date)
{
    // 1 (Monday) + 6 mod 7 = 0
    // 2 (Tuesday) + 6 mod 7 = 1
    // etc...
    int diffToWeekBegin = (date.dayOfWeek() + 6) % 7;
    return QDate::currentDate().addDays(-diffToWeekBegin);
}

void AppUpdateService::init()
{
    io::path_t historyPath = configuration()->updateRequestHistoryJsonPath();
    if (fileSystem()->exists(historyPath)) {
        return;
    }

    // If the request history file doesn't exist, perform a first time setup using today's date...
    UpdateRequestHistory updateRequestHistory;
    updateRequestHistory.installedWeekBeginning = calculateWeekBeginForDate(QDate::currentDate());
    updateRequestHistory.previousRequestDay = QDate::currentDate();

    Ret ret = writeUpdateRequestHistory(historyPath, updateRequestHistory);
    if (!ret) {
        LOGE() << ret.toString();
    }
}

muse::RetVal<ReleaseInfo> AppUpdateService::checkForUpdate()
{
    RetVal<ReleaseInfo> result;
    result.ret = make_ret(Err::NoUpdate);

    clear();

    QBuffer buff;
    m_networkManager = networkManagerCreator()->makeNetworkManager();

    // Read request history file...
    io::path_t historyPath = configuration()->updateRequestHistoryJsonPath();
    RetVal<UpdateRequestHistory> rv = readUpdateRequestHistory(historyPath);
    if (!rv.ret) {
        LOGE() << rv.ret.toString();
    }
    UpdateRequestHistory updateRequestHistory = rv.val;

    RequestHeaders headers = configuration()->updateHeaders();

    if (updateRequestHistory.isValid()) {
        // Prepare history headers...
        std::string iwbString = updateRequestHistory.installedWeekBeginning.toString(Qt::ISODate).toStdString();
        QByteArray iwbKey = QByteArray::fromStdString(INSTALLED_WEEK_BEGINNING_KEY.toStdString());
        headers.rawHeaders[iwbKey] = QByteArray::fromStdString(iwbString);

        std::string prdString = updateRequestHistory.previousRequestDay.toString(Qt::ISODate).toStdString();
        QByteArray prdKey = QByteArray::fromStdString(PREVIOUS_REQUEST_DAY_KEY.toStdString());
        headers.rawHeaders[prdKey] = QByteArray::fromStdString(prdString);
    }

    Ret getUpdateInfoRet = m_networkManager->get(QString::fromStdString(configuration()->checkForAppUpdateUrl()), &buff, headers);
    if (!getUpdateInfoRet) {
        result.ret = getUpdateInfoRet;
        return result;
    }

    // Successfully performed request, update history file...
    updateRequestHistory.previousRequestDay = QDate::currentDate();
    Ret ret = writeUpdateRequestHistory(historyPath, updateRequestHistory);
    if (!ret) {
        LOGE() << ret.toString();
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

    m_lastCheckResult = result;

    return result;
}

muse::RetVal<ReleaseInfo> AppUpdateService::lastCheckResult() const
{
    return m_lastCheckResult;
}

RetVal<AppUpdateService::UpdateRequestHistory> AppUpdateService::readUpdateRequestHistory(const io::path_t& path) const
{
    RetVal<ByteArray> rv = fileSystem()->readFile(path);
    if (!rv.ret) {
        return RetVal<UpdateRequestHistory>(rv.ret);
    }

    QJsonDocument historyDoc = QJsonDocument::fromJson(rv.val.toQByteArrayNoCopy());
    QJsonObject historyObject = historyDoc.object();

    UpdateRequestHistory updateRequestHistory;

    QString installedWeekBeginning = historyObject.value(INSTALLED_WEEK_BEGINNING_KEY).toString();
    updateRequestHistory.installedWeekBeginning = QDate::fromString(installedWeekBeginning, Qt::ISODate);

    QString previousRequestDay = historyObject.value(PREVIOUS_REQUEST_DAY_KEY).toString();
    updateRequestHistory.previousRequestDay = QDate::fromString(previousRequestDay, Qt::ISODate);

    return RetVal<UpdateRequestHistory>::make_ok(updateRequestHistory);
}

Ret AppUpdateService::writeUpdateRequestHistory(const io::path_t& path, const UpdateRequestHistory& updateRequestHistory)
{
    if (!updateRequestHistory.isValid()) {
        return make_ret(Ret::Code::UnknownError);
    }

    QJsonObject historyObject;

    const QDate& iwb = updateRequestHistory.installedWeekBeginning;
    const QDate& prd = updateRequestHistory.previousRequestDay;

    historyObject[INSTALLED_WEEK_BEGINNING_KEY] = iwb.toString(Qt::ISODate);
    historyObject[PREVIOUS_REQUEST_DAY_KEY] = prd.toString(Qt::ISODate);

    QByteArray byteArray = QJsonDocument(historyObject).toJson();

    return fileSystem()->writeFile(path, ByteArray::fromQByteArrayNoCopy(byteArray));
}

muse::RetVal<muse::io::path_t> AppUpdateService::downloadRelease()
{
    RetVal<io::path_t> result;
    ReleaseInfo info = m_lastCheckResult.val;

    QBuffer buff;
    QUrl fileUrl = QUrl::fromUserInput(QString::fromStdString(info.fileUrl));

    m_updateProgress.start();

    m_networkManager = networkManagerCreator()->makeNetworkManager();
    m_networkManager->progress().progressChanged().onReceive(this, [this, info](int64_t current, int64_t total, const std::string&) {
        m_updateProgress.progress(
            current, total,

            //: Means that the download is currently in progress.
            //: %1 will be replaced by the version number of the version that is being downloaded.
            muse::qtrc("update", "Downloading MuseScore Studio %1").arg(QString::fromStdString(info.version)).toStdString());
    });

    Ret ret = m_networkManager->get(fileUrl, &buff);
    if (!ret) {
        result.ret = ret;
        return result;
    }

    io::path_t installerPath = configuration()->updateDataPath() + "/" + info.fileName;
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
    Ret getPreviousReleaseNotes = m_networkManager->get(QString::fromStdString(configuration()->previousAppReleasesNotesUrl()), &buff);
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
    m_lastCheckResult = RetVal<ReleaseInfo>();

#if !defined(Q_OS_LINUX)
    fileSystem()->remove(configuration()->updateDataPath());
#endif
}
