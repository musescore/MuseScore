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
#include "extensionsservice.h"

#include <QBuffer>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QtConcurrent>
#include "log.h"
#include "translation.h"
#include "extensionserrors.h"

using namespace mu;
using namespace mu::extensions;
using namespace mu::framework;
using namespace mu::network;

static const QString ANALYSING_STATUS = qtrc("extensions", "Analysing...");
static const QString DOWNLOADING_STATUS = qtrc("extensions", "Downloading...");

void ExtensionsService::init()
{
    if (configuration()->needCheckForUpdate()) {
        QtConcurrent::run(this, &ExtensionsService::th_refreshExtensions);
    }
}

ValCh<ExtensionsHash> ExtensionsService::extensions() const
{
    ValCh<ExtensionsHash> extensionHash = configuration()->extensions();
    extensionHash.val = correctExtensionsStates(extensionHash.val).val;

    return extensionHash;
}

RetCh<ExtensionProgress> ExtensionsService::install(const QString& extensionCode)
{
    RetCh<ExtensionProgress> result;

    if (m_operationsHash.contains(extensionCode)) {
        if (m_operationsHash[extensionCode].type != OperationType::Install) {
            result.ret = make_ret(Err::ErrorAnotherOperationStarted);
            return result;
        }

        result.ret = make_ret(Err::NoError);
        result.ch = *m_operationsHash[extensionCode].progressChannel;
        return result;
    }

    async::Channel<ExtensionProgress>* extensionProgressStatus = new async::Channel<ExtensionProgress>();
    result.ch = *extensionProgressStatus;
    result.ret = make_ret(Err::NoError);
    m_operationsHash.insert(extensionCode, Operation(OperationType::Install, extensionProgressStatus));

    async::Channel<Ret>* extensionFinishChannel = new async::Channel<Ret>();
    extensionFinishChannel->onReceive(this, [this, extensionCode, extensionProgressStatus](const Ret& ret) {
        if (!ret) {
            closeOperation(extensionCode, extensionProgressStatus);
            return;
        }

        ExtensionsHash extensionHash = this->extensions().val;

        extensionHash[extensionCode].status = ExtensionStatus::Installed;
        extensionHash[extensionCode].types = extensionTypes(extensionCode);

        Ret updateConfigRet = configuration()->setExtensions(extensionHash);
        if (!updateConfigRet) {
            LOGE() << "Error when set extensions" << updateConfigRet.toString();
            closeOperation(extensionCode, extensionProgressStatus);
            return;
        }

        m_extensionChanged.send(extensionHash[extensionCode]);
        closeOperation(extensionCode, extensionProgressStatus);
    }, Asyncable::AsyncMode::AsyncSetRepeat);

    QtConcurrent::run(this, &ExtensionsService::th_install, extensionCode, extensionProgressStatus, extensionFinishChannel);

    return result;
}

RetCh<ExtensionProgress> ExtensionsService::update(const QString& extensionCode)
{
    RetCh<ExtensionProgress> result;

    if (m_operationsHash.contains(extensionCode)) {
        if (m_operationsHash[extensionCode].type != OperationType::Update) {
            result.ret = make_ret(Err::ErrorAnotherOperationStarted);
            return result;
        }

        result.ret = make_ret(Err::NoError);
        result.ch = *m_operationsHash[extensionCode].progressChannel;
        return result;
    }

    async::Channel<ExtensionProgress>* extensionProgressStatus = new async::Channel<ExtensionProgress>();
    result.ch = *extensionProgressStatus;
    result.ret = make_ret(Err::NoError);
    m_operationsHash.insert(extensionCode, Operation(OperationType::Update, extensionProgressStatus));

    async::Channel<Ret>* extensionFinishChannel = new async::Channel<Ret>();
    extensionFinishChannel->onReceive(this, [this, extensionCode, extensionProgressStatus](const Ret& ret) {
        if (!ret) {
            closeOperation(extensionCode, extensionProgressStatus);
            return;
        }

        ExtensionsHash extensionHash = extensions().val;

        extensionHash[extensionCode].status = ExtensionStatus::Installed;
        extensionHash[extensionCode].types = extensionTypes(extensionCode);

        Ret updateConfigRet = configuration()->setExtensions(extensionHash);
        if (!updateConfigRet) {
            LOGE() << "Error when set extensions" << updateConfigRet.toString();
            closeOperation(extensionCode, extensionProgressStatus);
            return;
        }

        m_extensionChanged.send(extensionHash[extensionCode]);
        closeOperation(extensionCode, extensionProgressStatus);
    }, Asyncable::AsyncMode::AsyncSetRepeat);

    QtConcurrent::run(this, &ExtensionsService::th_update, extensionCode, extensionProgressStatus, extensionFinishChannel);

    return result;
}

Ret ExtensionsService::uninstall(const QString& extensionCode)
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

RetCh<Extension> ExtensionsService::extensionChanged() const
{
    RetCh<Extension> result;
    result.ret = make_ret(Err::NoError);
    result.ch = m_extensionChanged;
    return result;
}

RetVal<ExtensionsHash> ExtensionsService::parseExtensionConfig(const QByteArray& json) const
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

bool ExtensionsService::isExtensionExists(const QString& extensionCode) const
{
    return fileSystem()->exists(configuration()->extensionPath(extensionCode));
}

RetVal<ExtensionsHash> ExtensionsService::correctExtensionsStates(ExtensionsHash& extensions) const
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

RetVal<QString> ExtensionsService::downloadExtension(const QString& extensionCode,
                                                     async::Channel<ExtensionProgress>* progressChannel) const
{
    RetVal<QString> result;

    QString extensionArchivePath = configuration()->extensionArchivePath(extensionCode).toQString();

    QBuffer buff;
    INetworkManagerPtr networkManagerPtr = networkManagerCreator()->makeNetworkManager();

    async::Channel<Progress> downloadChannel = networkManagerPtr->progressChannel();
    downloadChannel.onReceive(new deto::async::Asyncable(), [&progressChannel](const Progress& progress) {
        progressChannel->send(ExtensionProgress(DOWNLOADING_STATUS, progress.current,
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

Ret ExtensionsService::removeExtension(const QString& extensionCode) const
{
    io::path extensionPath = configuration()->extensionPath(extensionCode);
    Ret ret = fileSystem()->remove(extensionPath);
    if (!ret) {
        return make_ret(Err::ErrorRemoveExtensionDirectory);
    }

    return make_ret(Err::NoError);
}

Extension::ExtensionTypes ExtensionsService::extensionTypes(const QString& extensionCode) const
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

void ExtensionsService::th_refreshExtensions()
{
    QBuffer buff;
    INetworkManagerPtr networkManagerPtr = networkManagerCreator()->makeNetworkManager();

    Ret getExtensionsInfo = networkManagerPtr->get(configuration()->extensionsUpdateUrl(), &buff);

    if (!getExtensionsInfo) {
        LOGE() << "Error get extensions" << getExtensionsInfo.toString();
        return;
    }

    QByteArray json = buff.data();
    RetVal<ExtensionsHash> actualExtensions = parseExtensionConfig(json);

    if (!actualExtensions.ret) {
        LOGE() << actualExtensions.ret.toString();
        return;
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

    configuration()->setExtensions(resultExtensions);
}

void ExtensionsService::th_install(const QString& extensionCode,
                                   async::Channel<ExtensionProgress>* progressChannel,
                                   async::Channel<Ret>* finishChannel)
{
    progressChannel->send(ExtensionProgress(ANALYSING_STATUS, true));

    RetVal<QString> download = downloadExtension(extensionCode, progressChannel);
    if (!download.ret) {
        finishChannel->send(download.ret);
        return;
    }

    progressChannel->send(ExtensionProgress(ANALYSING_STATUS, true));

    QString extensionArchivePath = download.val;

    Ret unpack = extensionUnpacker()->unpack(extensionArchivePath, configuration()->extensionsPath().val.toQString());
    if (!unpack) {
        LOGE() << "Error unpack" << unpack.toString();
        fileSystem()->remove(extensionArchivePath);

        finishChannel->send(unpack);
        return;
    }

    fileSystem()->remove(extensionArchivePath);

    finishChannel->send(make_ret(Err::NoError));
}

void ExtensionsService::th_update(const QString& extensionCode, async::Channel<ExtensionProgress>* progressChannel,
                                  async::Channel<Ret>* finishChannel)
{
    progressChannel->send(ExtensionProgress(ANALYSING_STATUS, true));

    RetVal<QString> download = downloadExtension(extensionCode, progressChannel);
    if (!download.ret) {
        finishChannel->send(download.ret);
    }

    progressChannel->send(ExtensionProgress(ANALYSING_STATUS, true));

    QString extensionArchivePath = download.val;

    Ret remove = removeExtension(extensionCode);
    if (!remove) {
        finishChannel->send(remove);
    }

    Ret unpack = extensionUnpacker()->unpack(extensionArchivePath, configuration()->extensionsPath().val.toQString());
    if (!unpack) {
        LOGE() << "Error unpack" << unpack.toString();
        finishChannel->send(unpack);
    }

    fileSystem()->remove(extensionArchivePath);

    finishChannel->send(make_ret(Err::NoError));
}

void ExtensionsService::closeOperation(const QString& extensionCode, async::Channel<ExtensionProgress>* progressChannel)
{
    progressChannel->close();
    m_operationsHash.remove(extensionCode);
}
