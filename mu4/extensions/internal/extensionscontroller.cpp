//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "extensionscontroller.h"

#include <QBuffer>
#include <QtConcurrent>

#include "log.h"
#include "translation.h"
#include "extensionserrors.h"

using namespace mu;
using namespace mu::extensions;
using namespace mu::framework;

static const QString ANALYSING_STATUS = qtrc("extensions", "Analysing...");
static const QString DOWNLOADING_STATUS = qtrc("extensions", "Downloading...");

void ExtensionsController::init()
{
    fileSystem()->makePath(configuration()->extensionsSharePath());
    fileSystem()->makePath(configuration()->extensionsDataPath());

    refreshExtensions();
}

Ret ExtensionsController::refreshExtensions()
{
    QBuffer buff;
    INetworkManagerPtr networkManagerPtr = networkManagerCreator()->makeNetworkManager();

    Ret getExtensionsInfo = networkManagerPtr->get(configuration()->extensionsUpdateUrl(), &buff);

    if (!getExtensionsInfo) {
        LOGE() << "Error get extensions" << getExtensionsInfo.toString();
        return getExtensionsInfo;
    }

    QByteArray json = buff.data();
    RetVal<ExtensionsHash> actualExtensions = parseExtensionConfig(json);

    if (!actualExtensions.ret) {
        return actualExtensions.ret;
    }

    ExtensionsHash savedExtensions = configuration()->extensions().val;

    ExtensionsHash resultExtensions = savedExtensions;

    for (Extension& extension : actualExtensions.val) {
        if (!resultExtensions.contains(extension.code)) {
            extension.status = ExtensionStatus::NoInstalled;
            resultExtensions.insert(extension.code, extension);
            continue;
        }

        Extension& savedExtension = resultExtensions[extension.code];

        if (!isExtensionExists(extension.code)) {
            savedExtension.status = ExtensionStatus::NoInstalled;
            continue;
        }

        if (savedExtension.version < extension.version) {
            savedExtension.status = ExtensionStatus::NeedUpdate;
        }

        savedExtension.status = ExtensionStatus::Installed;
    }

    Ret ret = configuration()->setExtensions(resultExtensions);
    return ret;
}

ValCh<ExtensionsHash> ExtensionsController::extensions() const
{
    ValCh<ExtensionsHash> extensionHash = configuration()->extensions();
    extensionHash.val = correctExtensionsStates(extensionHash.val).val;

    return extensionHash;
}

RetCh<ExtensionProgress> ExtensionsController::install(const QString& extensionCode)
{
    RetCh<ExtensionProgress> result;
    result.ret = make_ret(Err::NoError);
    result.ch = m_extensionProgressStatus;

    m_extensionFinishChannel.onReceive(this, [this, extensionCode](const Ret& ret) {
        if (!ret) {
            return;
        }

        ExtensionsHash extensionHash = this->extensions().val;

        extensionHash[extensionCode].status = ExtensionStatus::Installed;
        extensionHash[extensionCode].types = extensionTypes(extensionCode);

        Ret updateConfigRet = configuration()->setExtensions(extensionHash);
        if (!updateConfigRet) {
            LOGE() << "Error when set extensions" << updateConfigRet.toString();
            m_extensionProgressStatus.close();
            return;
        }

        m_extensionChanged.send(extensionHash[extensionCode]);

        m_extensionProgressStatus.close();
    });

    QtConcurrent::run(this, &ExtensionsController::th_install, extensionCode, m_extensionProgressStatus, m_extensionFinishChannel);

    return result;
}

RetCh<ExtensionProgress> ExtensionsController::update(const QString& extensionCode)
{
    RetCh<ExtensionProgress> result;
    result.ret = make_ret(Err::NoError);
    result.ch = m_extensionProgressStatus;

    m_extensionFinishChannel.onReceive(this, [this, extensionCode](const Ret& ret) {
        if (!ret) {
            return;
        }

        ExtensionsHash extensionHash = extensions().val;

        extensionHash[extensionCode].status = ExtensionStatus::Installed;
        extensionHash[extensionCode].types = extensionTypes(extensionCode);

        Ret updateConfigRet = configuration()->setExtensions(extensionHash);
        if (!updateConfigRet) {
            LOGE() << "Error when set extensions" << updateConfigRet.toString();
            m_extensionProgressStatus.close();
            return;
        }

        m_extensionChanged.send(extensionHash[extensionCode]);

        m_extensionProgressStatus.close();
    });

    QtConcurrent::run(this, &ExtensionsController::th_install, extensionCode, m_extensionProgressStatus, m_extensionFinishChannel);

    return result;
}

Ret ExtensionsController::uninstall(const QString& extensionCode)
{
    ExtensionsHash extensionHash = extensions().val;

    if (!extensionHash.contains(extensionCode)) {
        return make_ret(Err::ErrorExtensionNotFound);
    }

    Ret remove = removeExtension(extensionCode);
    if (!remove) {
        return remove;
    }

    extensionHash[extensionCode].status = ExtensionStatus::NoInstalled;
    Ret ret = configuration()->setExtensions(extensionHash);
    if (!ret) {
        return ret;
    }

    m_extensionChanged.send(extensionHash[extensionCode]);

    return make_ret(Err::NoError);
}

RetCh<Extension> ExtensionsController::extensionChanged() const
{
    RetCh<Extension> result;
    result.ret = make_ret(Err::NoError);
    result.ch = m_extensionChanged;
    return result;
}

RetVal<ExtensionsHash> ExtensionsController::parseExtensionConfig(const QByteArray& json) const
{
    RetVal<ExtensionsHash> result;

    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
        result.ret = make_ret(Err::ErrorParseConfig);
        return result;
    }

    result.ret = make_ret(Err::NoError);
    QStringList extensions = jsonDoc.object().keys();
    for (const QString& key : extensions) {
        if (!jsonDoc.object().value(key).isObject()) {
            continue;
        }

        QJsonObject value = jsonDoc.object().value(key).toObject();

        Extension extension;
        extension.code = key;
        extension.name = value.value("name").toString();
        extension.description = value.value("description").toString();
        extension.fileName = value.value("file_name").toString();
        extension.fileSize = value.value("file_size").toDouble();
        extension.version = QVersionNumber::fromString(value.value("version").toString());
        extension.types = {};

        result.val.insert(key, extension);
    }

    return result;
}

bool ExtensionsController::isExtensionExists(const QString& extensionCode) const
{
    return fileSystem()->exists(configuration()->extensionPath(extensionCode));
}

RetVal<ExtensionsHash> ExtensionsController::correctExtensionsStates(ExtensionsHash& extensions) const
{
    RetVal<ExtensionsHash> result;
    bool isNeedUpdate = false;

    for (Extension& extension: extensions) {
        if (extension.status == ExtensionStatus::Installed && !isExtensionExists(extension.code)) {
            extension.status = ExtensionStatus::NoInstalled;
            isNeedUpdate = true;
        }

        extension.types = extensionTypes(extension.code);
    }

    if (isNeedUpdate) {
        Ret ret = configuration()->setExtensions(extensions);
        if (!ret) {
            result.ret = ret;
            return result;
        }
    }

    result.ret = make_ret(Err::NoError);
    result.val = extensions;
    return result;
}

RetVal<QString> ExtensionsController::downloadExtension(const QString& extensionCode,
                                                        async::Channel<ExtensionProgress>& progressChannel) const
{
    RetVal<QString> result;

    QString extensionArchivePath = configuration()->extensionArchivePath(extensionCode).toQString();

    QBuffer buff;
    INetworkManagerPtr networkManagerPtr = networkManagerCreator()->makeNetworkManager();

    async::Channel<Progress> downloadChannel = networkManagerPtr->downloadProgressChannel();
    downloadChannel.onReceive(new deto::async::Asyncable(), [&progressChannel](const Progress& progress) {
        progressChannel.send(ExtensionProgress(DOWNLOADING_STATUS, progress.current,
                                               progress.total));
    });

    Ret getExtension = networkManagerPtr->get(configuration()->extensionFileServerUrl(extensionCode), &buff);

    if (!getExtension) {
        LOGE() << "Error save file" << getExtension.toString();
        result.ret = make_ret(Err::ErrorLoadingExtension);
        return result;
    }

    QFile file(extensionArchivePath);
    file.open(QIODevice::WriteOnly);
    file.write(buff.data());
    file.close();

    result.ret = make_ret(Err::NoError);
    result.val = extensionArchivePath;
    return result;
}

Ret ExtensionsController::removeExtension(const QString& extensionCode) const
{
    io::path extensionPath = configuration()->extensionPath(extensionCode);
    Ret ret = fileSystem()->remove(extensionPath);
    if (!ret) {
        return make_ret(Err::ErrorRemoveExtensionDirectory);
    }

    return make_ret(Err::NoError);
}

Extension::ExtensionTypes ExtensionsController::extensionTypes(const QString& extensionCode) const
{
    Extension::ExtensionTypes result;

    io::paths workspaceFiles = configuration()->extensionWorkspaceFiles(extensionCode);
    if (!workspaceFiles.empty()) {
        result.setFlag(Extension::Workspaces);
    }

    io::paths instrumentFiles = configuration()->extensionInstrumentFiles(extensionCode);
    if (!instrumentFiles.empty()) {
        result.setFlag(Extension::Instruments);
    }

    return result;
}

void ExtensionsController::th_install(const QString& extensionCode,
                                      async::Channel<ExtensionProgress> progressChannel,
                                      async::Channel<Ret> finishChannel)
{
    progressChannel.send(ExtensionProgress(ANALYSING_STATUS, true));

    RetVal<QString> download = downloadExtension(extensionCode, progressChannel);
    if (!download.ret) {
        finishChannel.send(download.ret);
        return;
    }

    progressChannel.send(ExtensionProgress(ANALYSING_STATUS, true));

    QString extensionArchivePath = download.val;

    Ret unpack = extensionUnpacker()->unpack(extensionArchivePath, configuration()->extensionsSharePath().toQString());
    if (!unpack) {
        LOGE() << "Error unpack" << unpack.toString();
        finishChannel.send(unpack);
        return;
    }

    fileSystem()->remove(extensionArchivePath);

    finishChannel.send(make_ret(Err::NoError));
}

void ExtensionsController::th_update(const QString& extensionCode, async::Channel<ExtensionProgress> progressChannel,
                                     async::Channel<Ret> finishChannel)
{
    progressChannel.send(ExtensionProgress(ANALYSING_STATUS, true));

    RetVal<QString> download = downloadExtension(extensionCode, progressChannel);
    if (!download.ret) {
        finishChannel.send(download.ret);
    }

    progressChannel.send(ExtensionProgress(ANALYSING_STATUS, true));

    QString extensionArchivePath = download.val;

    Ret remove = removeExtension(extensionCode);
    if (!remove) {
        finishChannel.send(remove);
    }

    Ret unpack = extensionUnpacker()->unpack(extensionArchivePath, configuration()->extensionsSharePath().toQString());
    if (!unpack) {
        LOGE() << "Error unpack" << unpack.toString();
        finishChannel.send(unpack);
    }

    fileSystem()->remove(extensionArchivePath);

    finishChannel.send(make_ret(Err::NoError));
}
