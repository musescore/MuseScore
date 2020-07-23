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

#include "log.h"
#include "mscore/downloadUtils.h"
#include "extensionserrors.h"

using namespace mu;
using namespace mu::extensions;

void ExtensionsController::init()
{
    fsOperation()->makePath(configuration()->extensionsSharePath());
    fsOperation()->makePath(configuration()->extensionsDataPath());
}

Ret ExtensionsController::refreshExtensions()
{
    Ms::DownloadUtils js;
    js.setTarget(configuration()->extensionsUpdateUrl().toString());
    js.download();

    QByteArray json = js.returnData();
    RetVal<ExtensionsHash> actualExtensions = parseExtensionConfig(json);

    if (!actualExtensions.ret) {
        return actualExtensions.ret;
    }

    ExtensionsHash savedExtensions = configuration()->extensions().val;

    ExtensionsHash resultExtensions = savedExtensions;

    for (Extension& extension : actualExtensions.val) {
        if (!resultExtensions.contains(extension.code)) {
            extension.status = ExtensionStatus::Status::NoInstalled;
            resultExtensions.insert(extension.code, extension);
            continue;
        }

        Extension& savedExtension = resultExtensions[extension.code];

        if (!isExtensionExists(extension.code)) {
            savedExtension.status = ExtensionStatus::Status::NoInstalled;
            continue;
        }

        if (savedExtension.version < extension.version) {
            savedExtension.status = ExtensionStatus::Status::NeedUpdate;
        }

        savedExtension.status = ExtensionStatus::Status::Installed;
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

Ret ExtensionsController::install(const QString& extensionCode)
{
    RetVal<QString> download = downloadExtension(extensionCode);
    if (!download.ret) {
        return download.ret;
    }

    QString extensionArchivePath = download.val;

    Ret unpack = extensionUnpacker()->unpack(extensionArchivePath, configuration()->extensionsSharePath());
    if (!unpack) {
        LOGE() << "Error unpack" << unpack.code();
        return unpack;
    }

    fsOperation()->remove(extensionArchivePath);

    ExtensionsHash extensionHash = this->extensions().val;

    extensionHash[extensionCode].status = ExtensionStatus::Status::Installed;
    extensionHash[extensionCode].types = extensionTypes(extensionCode);

    Ret ret = configuration()->setExtensions(extensionHash);
    if (!ret) {
        return ret;
    }

    m_extensionChanged.send(extensionHash[extensionCode]);

    return make_ret(Err::NoError);
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

    extensionHash[extensionCode].status = ExtensionStatus::Status::NoInstalled;
    Ret ret = configuration()->setExtensions(extensionHash);
    if (!ret) {
        return ret;
    }

    m_extensionChanged.send(extensionHash[extensionCode]);

    return make_ret(Err::NoError);
}

Ret ExtensionsController::update(const QString& extensionCode)
{
    RetVal<QString> download = downloadExtension(extensionCode);
    if (!download.ret) {
        return download.ret;
    }

    QString extensionArchivePath = download.val;

    Ret remove = removeExtension(extensionCode);
    if (!remove) {
        return remove;
    }

    Ret unpack = extensionUnpacker()->unpack(extensionArchivePath, configuration()->extensionsSharePath());
    if (!unpack) {
        LOGE() << "Error unpack" << unpack.code();
        return unpack;
    }

    fsOperation()->remove(extensionArchivePath);

    ExtensionsHash extensionHash = extensions().val;

    extensionHash[extensionCode].status = ExtensionStatus::Status::Installed;
    extensionHash[extensionCode].types = extensionTypes(extensionCode);

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
    return fsOperation()->exists(configuration()->extensionsSharePath() + "/" + extensionCode);
}

RetVal<ExtensionsHash> ExtensionsController::correctExtensionsStates(ExtensionsHash& extensions) const
{
    RetVal<ExtensionsHash> result;
    bool isNeedUpdate = false;

    for (Extension& extension: extensions) {
        if (extension.status == ExtensionStatus::Status::Installed && !isExtensionExists(extension.code)) {
            extension.status = ExtensionStatus::Status::NoInstalled;
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

RetVal<QString> ExtensionsController::downloadExtension(const QString& extensionCode) const
{
    RetVal<QString> result;

    ValCh<ExtensionsHash> extensions = configuration()->extensions();
    QString fileName = extensions.val.value(extensionCode).fileName;

    QString extensionArchivePath = configuration()->extensionsDataPath() + "/" + fileName;

    Ms::DownloadUtils js;
    js.setTarget(configuration()->extensionsFileServerUrl().toString() + fileName);
    js.setLocalFile(extensionArchivePath);
    js.download(true);

    if (!js.saveFile()) {
        LOGE() << "Error save file";
        result.ret = make_ret(Err::ErrorLoadingExtension);
        return result;
    }

    result.ret = make_ret(Err::NoError);
    result.val = extensionArchivePath;
    return result;
}

Ret ExtensionsController::removeExtension(const QString& extensionCode) const
{
    QString extensionPath = configuration()->extensionsSharePath() + "/" + extensionCode;
    Ret ret = fsOperation()->remove(extensionPath);
    if (!ret) {
        return make_ret(Err::ErrorRemoveExtensionDirectory);
    }

    return make_ret(Err::NoError);
}

Extension::ExtensionTypes ExtensionsController::extensionTypes(const QString& extensionCode) const
{
    Extension::ExtensionTypes result;
    QString workspacesPath(configuration()->extensionsSharePath() + "/" + extensionCode + "/workspaces");
    RetVal<QStringList> files = fsOperation()->directoryFileList(workspacesPath, { QString("*.workspace") }, QDir::Files);
    if (files.ret && !files.val.empty()) {
        result.setFlag(Extension::Workspaces);
    }

    return result;
}
