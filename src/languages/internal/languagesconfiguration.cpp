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
#include "languagesconfiguration.h"

#include <QDir>
#include <QVariant>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

#include "log.h"
#include "settings.h"
#include "languagestypes.h"
#include "languageserrors.h"

using namespace mu;
using namespace mu::framework;
using namespace mu::languages;

static const QString SYSTEM_LANGUAGE_CODE("system");
static const std::string module_name("languages");
static const Settings::Key LANGUAGES_JSON(module_name, "languages/languagesJson");
static const Settings::Key LANGUAGE("ui", "ui/application/language");

QString correctLanguageCode(const QString& languageCode)
{
    QString result = languageCode;

    if (result == SYSTEM_LANGUAGE_CODE) {
        result = QLocale::system().name();
    }

    return result;
}

void LanguagesConfiguration::init()
{
    settings()->setDefaultValue(LANGUAGE, Val(SYSTEM_LANGUAGE_CODE.toStdString()));
    settings()->valueChanged(LANGUAGE).onReceive(nullptr, [this](const Val& val) {
        m_currentLanguageCodeChanged.send(correctLanguageCode(val.toQString()));
    });

    settings()->valueChanged(LANGUAGES_JSON).onReceive(nullptr, [this](const Val& val) {
        LanguagesHash languagesHash = parseLanguagesConfig(val.toQString().toLocal8Bit());
        m_languagesHashChanged.send(languagesHash);
    });
}

ValCh<QString> LanguagesConfiguration::currentLanguageCode() const
{
    ValCh<QString> result;
    result.ch = m_currentLanguageCodeChanged;
    result.val = correctLanguageCode(settings()->value(LANGUAGE).toQString());

    return result;
}

void LanguagesConfiguration::setCurrentLanguageCode(const QString& languageCode) const
{
    Val value(languageCode.toStdString());
    settings()->setValue(LANGUAGE, value);
}

QUrl LanguagesConfiguration::languagesUpdateUrl() const
{
    return QUrl("http://extensions.musescore.org/4.0/languages/details.json");
}

QUrl LanguagesConfiguration::languageFileServerUrl(const QString& languageCode) const
{
    io::path fileName = languageFileName(languageCode);
    return QUrl("http://extensions.musescore.org/4.0/languages/" + fileName.toQString());
}

ValCh<LanguagesHash> LanguagesConfiguration::languages() const
{
    ValCh<LanguagesHash> result;
    result.val = parseLanguagesConfig(settings()->value(LANGUAGES_JSON).toQString().toLocal8Bit());
    result.ch = m_languagesHashChanged;

    return result;
}

Ret LanguagesConfiguration::setLanguages(const LanguagesHash& languages) const
{
    QJsonArray jsonArray;
    for (const Language& language: languages) {
        QJsonObject obj;
        obj[language.code] = language.toJson();

        jsonArray << obj;
    }

    QJsonDocument jsonDoc(jsonArray);

    Val value(jsonDoc.toJson(QJsonDocument::Compact).constData());
    settings()->setValue(LANGUAGES_JSON, value);

    return make_ret(Err::NoError);
}

LanguagesHash LanguagesConfiguration::parseLanguagesConfig(const QByteArray& json) const
{
    LanguagesHash result;

    QJsonParseError err;
    QJsonDocument jsodDoc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !jsodDoc.isArray()) {
        return LanguagesHash();
    }

    QVariantList languages = jsodDoc.array().toVariantList();
    for (const QVariant& languagesObj: languages) {
        QMap<QString, QVariant> value = languagesObj.toMap();
        QVariantMap lngMap = value.first().toMap();

        Language language;
        language.code = value.keys().first();
        language.name = lngMap.value("name").toString();
        language.archiveFileName = lngMap.value("fileName").toString();
        language.status = static_cast<LanguageStatus::Status>(lngMap.value("status").toInt());

        QVariantList files = lngMap.value("files").toList();
        for (const QVariant& fileVar: files) {
            QVariantMap fileObj = fileVar.toMap();
            LanguageFile file;
            file.name = fileObj["name"].toString();
            file.hash = fileObj["hash"].toString();

            language.files << file;
        }

        result.insert(language.code, language);
    }

    return result;
}

io::path LanguagesConfiguration::languagesSharePath() const
{
    return globalConfiguration()->sharePath() + "/locale";
}

io::path LanguagesConfiguration::languagesDataPath() const
{
    return globalConfiguration()->dataPath() + "/locale";
}

io::paths LanguagesConfiguration::languageFilePaths(const QString& languageCode) const
{
    io::path languagesDirPath = languagesSharePath();
    QStringList filters = { QString("*%1.qm").arg(languageCode) };
    RetVal<io::paths> files = fileSystem()->scanFiles(languagesDirPath, filters);

    if (!files.ret) {
        LOGW() << files.ret.toString();
        return io::paths();
    }

    return files.val;
}

io::path LanguagesConfiguration::languageArchivePath(const QString& languageCode) const
{
    io::path fileName = languageFileName(languageCode);
    return languagesDataPath() + "/" + fileName;
}

io::path LanguagesConfiguration::languageFileName(const QString& languageCode) const
{
    ValCh<LanguagesHash> _languages = languages();
    return _languages.val.value(languageCode).archiveFileName;
}
