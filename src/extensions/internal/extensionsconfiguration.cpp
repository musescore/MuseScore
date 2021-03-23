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
#include <QJsonArray>

#include "log.h"
#include "settings.h"
#include "extensionserrors.h"

using namespace mu;
using namespace mu::framework;
using namespace mu::extensions;

static const std::string module_name("extensions");

static const Settings::Key EXTENSIONS_JSON(module_name, "extensions/extensionsJson");
static const Settings::Key CHECK_FOR_UPDATE(module_name, "extensions/checkForUpdate");
static const Settings::Key USER_EXTENSIONS_PATH(module_name, "application/paths/myExtensions");

static const io::path EXTENSIONS_DIR("Extensions");
static const io::path WORKSPACES_DIR("workspaces");
static const io::path INSTRUMENTS_DIR("instruments");
static const io::path TEMPLATES_DIR("templates");

static const QString WORKSPACE_FILTER("*.workspace");
static const QString MSCZ_FILTER("*.mscz");
static const QString MSCX_FILTER(".mscx");
static const QString XML_FILTER("*.xml");

void ExtensionsConfiguration::init()
{
    settings()->setDefaultValue(CHECK_FOR_UPDATE, Val(true));

    settings()->setDefaultValue(USER_EXTENSIONS_PATH, Val(globalConfiguration()->sharePath().toStdString() + EXTENSIONS_DIR.toStdString()));
    settings()->valueChanged(USER_EXTENSIONS_PATH).onReceive(nullptr, [this](const Val& val) {
        m_extensionsPathChanged.send(val.toString());
    });

    settings()->valueChanged(EXTENSIONS_JSON).onReceive(nullptr, [this](const Val& val) {
        ExtensionsHash extensionHash = parseExtensionConfig(val.toQString().toLocal8Bit());
        m_extensionHashChanged.send(extensionHash);
    });

    fileSystem()->makePath(extensionsPath().val);
    fileSystem()->makePath(extensionsDataPath());
}

QUrl ExtensionsConfiguration::extensionsUpdateUrl() const
{
    return QUrl("http://extensions.musescore.org/4.0/extensions/details.json");
}

QUrl ExtensionsConfiguration::extensionFileServerUrl(const QString& extensionCode) const
{
    io::path fileName = extensionFileName(extensionCode);
    return QUrl("http://extensions.musescore.org/4.0/extensions/" + fileName.toQString());
}

bool ExtensionsConfiguration::needCheckForUpdate() const
{
    return settings()->value(CHECK_FOR_UPDATE).toBool();
}

void ExtensionsConfiguration::setNeedCheckForUpdate(bool needCheck)
{
    settings()->setValue(CHECK_FOR_UPDATE, Val(needCheck));
}

ValCh<ExtensionsHash> ExtensionsConfiguration::extensions() const
{
    ValCh<ExtensionsHash> result;
    result.val = parseExtensionConfig(settings()->value(EXTENSIONS_JSON).toQString().toLocal8Bit());
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

io::path ExtensionsConfiguration::extensionPath(const QString& extensionCode) const
{
    return extensionsPath().val + "/" + extensionCode;
}

io::path ExtensionsConfiguration::extensionWorkspacesPath(const QString& extensionCode) const
{
    return extensionsPath().val + "/" + extensionCode + "/" + WORKSPACES_DIR;
}

io::path ExtensionsConfiguration::extensionArchivePath(const QString& extensionCode) const
{
    io::path fileName = extensionFileName(extensionCode);
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

io::path ExtensionsConfiguration::extensionFileName(const QString& extensionCode) const
{
    ValCh<ExtensionsHash> _extensions = extensions();
    return _extensions.val.value(extensionCode).fileName;
}

io::paths ExtensionsConfiguration::fileList(const io::path& directory, const QStringList& filters) const
{
    RetVal<io::paths> files = fileSystem()->scanFiles(directory, filters);
    if (!files.ret) {
        LOGW() << files.ret.code() << files.ret.text();
    }

    return files.val;
}

io::path ExtensionsConfiguration::extensionInstrumentsPath(const QString& extensionCode) const
{
    return extensionsPath().val + "/" + extensionCode + "/" + INSTRUMENTS_DIR;
}

io::paths ExtensionsConfiguration::extensionWorkspaceFiles(const QString& extensionCode) const
{
    io::path _extensionWorkspacesPath = extensionWorkspacesPath(extensionCode);
    return fileList(_extensionWorkspacesPath, { WORKSPACE_FILTER });
}

io::paths ExtensionsConfiguration::workspacesPaths() const
{
    io::paths paths;

    ExtensionsHash extensions = this->extensions().val;

    for (const Extension& extension : extensions.values()) {
        io::path _extensionWorkspacesPath = extensionWorkspacesPath(extension.code);
        io::paths files = fileList(_extensionWorkspacesPath, { WORKSPACE_FILTER });

        if (!files.empty()) {
            paths.push_back(_extensionWorkspacesPath);
        }
    }

    return paths;
}

io::paths ExtensionsConfiguration::extensionInstrumentFiles(const QString& extensionCode) const
{
    io::path _extensionInstrumentsPath = extensionInstrumentsPath(extensionCode);
    return fileList(_extensionInstrumentsPath, { XML_FILTER });
}

io::paths ExtensionsConfiguration::instrumentsPaths() const
{
    io::paths paths;

    ExtensionsHash extensions = this->extensions().val;

    for (const Extension& extension: extensions.values()) {
        io::path _extensionInstrumentsPath = extensionInstrumentsPath(extension.code);
        io::paths files = fileList(_extensionInstrumentsPath, { XML_FILTER });

        if (!files.empty()) {
            paths.push_back(_extensionInstrumentsPath);
        }
    }

    return paths;
}

io::paths ExtensionsConfiguration::templatesPaths() const
{
    io::paths paths;

    ExtensionsHash extensions = this->extensions().val;

    for (const Extension& extension: extensions.values()) {
        io::path _extensionTemplatesPath = extensionTemplatesPath(extension.code);
        io::paths files = fileList(_extensionTemplatesPath, { MSCZ_FILTER, MSCX_FILTER });

        if (!files.empty()) {
            paths.push_back(_extensionTemplatesPath);
        }
    }

    return paths;
}

ValCh<io::path> ExtensionsConfiguration::extensionsPath() const
{
    ValCh<io::path> result;
    result.ch = m_extensionsPathChanged;
    result.val = settings()->value(USER_EXTENSIONS_PATH).toString();

    return result;
}

void ExtensionsConfiguration::setExtensionsPath(const io::path& path)
{
    settings()->setValue(USER_EXTENSIONS_PATH, Val(path.toStdString()));
}

io::path ExtensionsConfiguration::extensionTemplatesPath(const QString& extensionCode) const
{
    return extensionsPath().val + "/" + extensionCode + "/" + TEMPLATES_DIR;
}

io::path ExtensionsConfiguration::extensionsDataPath() const
{
    return globalConfiguration()->dataPath() + EXTENSIONS_DIR;
}
