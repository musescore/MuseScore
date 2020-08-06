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
#include "extensionsconfiguration.h"

#include <QVariant>
#include <QJsonParseError>

#include "log.h"
#include "settings.h"
#include "extensionserrors.h"

using namespace mu;
using namespace mu::framework;
using namespace mu::extensions;

static std::string module_name("extensions");
static const Settings::Key EXTENSIONS_JSON(module_name, "extensions/extensionsJson");

static const QString EXTENSIONS_DIR("/extensions");
static const QString WORKSPACES_DIR("/workspaces");
static const QString INSTRUMENTS_DIR("/instruments");

void ExtensionsConfiguration::init()
{
    settings()->valueChanged(EXTENSIONS_JSON).onReceive(nullptr, [this](const Val& val) {
        LOGD() << "EXTENSION_Json changed: " << val.toString();

        ExtensionsHash extensionHash = parseExtensionConfig(io::pathToQString(val.toString()).toLocal8Bit());
        m_extensionHashChanged.send(extensionHash);
    });
}

QUrl ExtensionsConfiguration::extensionsUpdateUrl() const
{
    return QUrl("http://extensions.musescore.org/4.0/extensions/details.json");
}

QUrl ExtensionsConfiguration::extensionFileServerUrl(const QString& extensionCode) const
{
    QString fileName = extensionFileName(extensionCode);
    return QUrl("http://extensions.musescore.org/4.0/extensions/" + fileName);
}

ValCh<ExtensionsHash> ExtensionsConfiguration::extensions() const
{
    ValCh<ExtensionsHash> result;
    result.val = parseExtensionConfig(io::pathToQString(settings()->value(EXTENSIONS_JSON).toString()).toLocal8Bit());
    result.ch = m_extensionHashChanged;

    return result;
}

Ret ExtensionsConfiguration::setExtensions(const ExtensionsHash& extensions) const
{
    QJsonArray jsonArray;
    for (const Extension& extension: extensions) {
        QJsonObject obj;
        obj[extension.code] = extension.toJson();

        jsonArray << obj;
    }

    QJsonDocument jsonDoc(jsonArray);

    Val value(jsonDoc.toJson(QJsonDocument::Compact).constData());
    settings()->setValue(EXTENSIONS_JSON, value);

    return make_ret(Err::NoError);
}

QString ExtensionsConfiguration::extensionPath(const QString& extensionCode) const
{
    return extensionsSharePath() + "/" + extensionCode;
}

QString ExtensionsConfiguration::extensionWorkspacesPath(const QString& extensionCode) const
{
    return extensionsSharePath() + "/" + extensionCode + WORKSPACES_DIR;
}

QString ExtensionsConfiguration::extensionArchivePath(const QString& extensionCode) const
{
    QString fileName = extensionFileName(extensionCode);
    return extensionsDataPath() + "/" + fileName;
}

ExtensionsHash ExtensionsConfiguration::parseExtensionConfig(const QByteArray& json) const
{
    ExtensionsHash result;

    QJsonParseError err;
    QJsonDocument jsodDoc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !jsodDoc.isArray()) {
        return ExtensionsHash();
    }

    QVariantList extensions = jsodDoc.array().toVariantList();
    for (const QVariant& extensionObj: extensions) {
        QMap<QString, QVariant> value = extensionObj.toMap();
        QVariantMap extMap = value.first().toMap();

        Extension extension;
        extension.code = value.keys().first();
        extension.name = extMap.value("name").toString();
        extension.description = extMap.value("description").toString();
        extension.fileName = extMap.value("fileName").toString();
        extension.fileSize = extMap.value("fileSize").toDouble();
        extension.version = QVersionNumber::fromString(extMap.value("version").toString());
        extension.status = static_cast<ExtensionStatus::Status>(extMap.value("status").toInt());

        result.insert(extension.code, extension);
    }

    return result;
}

QString ExtensionsConfiguration::extensionFileName(const QString& extensionCode) const
{
    ValCh<ExtensionsHash> _extensions = extensions();
    return _extensions.val.value(extensionCode).fileName;
}

QStringList ExtensionsConfiguration::workspaceFileList(const QString& directory) const
{
    RetVal<QStringList> files = fsOperation()->directoryFileList(directory, { QString("*.workspace") }, QDir::Files);
    if (!files.ret) {
        LOGW() << files.ret.code() << files.ret.text();
    }

    return files.val;
}

QStringList ExtensionsConfiguration::instrumentFileList(const QString &directory) const
{
    RetVal<QStringList> files = fsOperation()->directoryFileList(directory, { QString("*.xml") }, QDir::Files);
    if (!files.ret) {
        LOGW() << files.ret.code() << files.ret.text();
    }

    return files.val;
}

QString ExtensionsConfiguration::extensionInstrumentsPath(const QString &extensionCode) const
{
    return extensionsSharePath() + "/" + extensionCode + INSTRUMENTS_DIR;
}

QString mu::extensions::ExtensionsConfiguration::extensionsSharePath() const
{
    return io::pathToQString(globalConfiguration()->sharePath()) + EXTENSIONS_DIR;
}

QString ExtensionsConfiguration::extensionsDataPath() const
{
    return io::pathToQString(globalConfiguration()->dataPath()) + EXTENSIONS_DIR;
}

QStringList ExtensionsConfiguration::extensionWorkspaceFiles(const QString& extensionCode) const
{
    QString _extensionWorkspacesPath = extensionWorkspacesPath(extensionCode);
    return workspaceFileList(_extensionWorkspacesPath);
}

QStringList ExtensionsConfiguration::workspacesPaths() const
{
    QStringList paths;

    ExtensionsHash extensions = this->extensions().val;

    for (const Extension& extension: extensions.values()) {
        QString _extensionWorkspacesPath = extensionWorkspacesPath(extension.code);
        QStringList files = workspaceFileList(_extensionWorkspacesPath);
        if (!files.isEmpty()) {
            paths << _extensionWorkspacesPath;
        }
    }

    return paths;
}

QStringList ExtensionsConfiguration::instrumentsPaths() const
{
    QStringList paths;

    ExtensionsHash extensions = this->extensions().val;

    for (const Extension& extension: extensions.values()) {
        QString _extensionInstrumentsPath = extensionInstrumentsPath(extension.code);
        QStringList files = instrumentFileList(_extensionInstrumentsPath);
        if (!files.isEmpty()) {
            paths << _extensionInstrumentsPath;
        }
    }

    return paths;
}
