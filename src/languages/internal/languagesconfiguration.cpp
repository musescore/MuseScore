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

#include "multiinstances/resourcelockguard.h"

using namespace mu;
using namespace mu::framework;
using namespace mu::languages;

static const QString SYSTEM_LANGUAGE_CODE("system");
static const Settings::Key LANGUAGE_KEY("languages", "language");

static const QString LANGUAGES_SERVER_URL("http://extensions.musescore.org/4.0/languages/");
static const io::path LANGUAGES_STATE_FILE("/languages.json");

static const std::string LANGUAGES_RESOURCE_NAME("LANGUAGES");

static QString correctLanguageCode(const QString& languageCode)
{
    QString result = languageCode;
    if (result == SYSTEM_LANGUAGE_CODE) {
        result = QLocale::system().name();
    }
    return result;
}

void LanguagesConfiguration::init()
{
    settings()->setDefaultValue(LANGUAGE_KEY, Val(SYSTEM_LANGUAGE_CODE.toStdString()));
    settings()->valueChanged(LANGUAGE_KEY).onReceive(nullptr, [this](const Val& val) {
        m_currentLanguageCodeChanged.send(correctLanguageCode(val.toQString()));
    });

    fileSystem()->makePath(languagesUserAppDataPath());
}

ValCh<QString> LanguagesConfiguration::currentLanguageCode() const
{
    ValCh<QString> result;
    result.ch = m_currentLanguageCodeChanged;
    result.val = correctLanguageCode(settings()->value(LANGUAGE_KEY).toQString());

    return result;
}

void LanguagesConfiguration::setCurrentLanguageCode(const QString& languageCode) const
{
    Val value(languageCode.toStdString());
    settings()->setSharedValue(LANGUAGE_KEY, value);
}

QUrl LanguagesConfiguration::languagesUpdateUrl() const
{
    return QUrl(LANGUAGES_SERVER_URL + "details.json");
}

QUrl LanguagesConfiguration::languageFileServerUrl(const QString& languageCode) const
{
    TRACEFUNC;
    io::path fileName = languageFileName(languageCode);
    return QUrl(LANGUAGES_SERVER_URL + fileName.toQString());
}

RetVal<QByteArray> LanguagesConfiguration::readLanguagesState() const
{
    mi::ReadResourceLockGuard lock_guard(multiInstancesProvider(), LANGUAGES_RESOURCE_NAME);
    return fileSystem()->readFile(languagesUserAppDataPath() + LANGUAGES_STATE_FILE);
}

Ret LanguagesConfiguration::writeLanguagesState(const QByteArray& data)
{
    mi::WriteResourceLockGuard lock_guard(multiInstancesProvider(), LANGUAGES_RESOURCE_NAME);
    return fileSystem()->writeToFile(languagesUserAppDataPath() + LANGUAGES_STATE_FILE, data);
}

ValCh<LanguagesHash> LanguagesConfiguration::languages() const
{
    TRACEFUNC;
    RetVal<QByteArray> rv = readLanguagesState();
    if (!rv.ret) {
        LOGE() << rv.ret.toString();
        return ValCh<LanguagesHash>();
    }

    ValCh<LanguagesHash> result;
    result.val = parseLanguagesConfig(rv.val);
    result.ch = m_languagesHashChanged;

    return result;
}

Ret LanguagesConfiguration::setLanguages(const LanguagesHash& languages)
{
    TRACEFUNC;
    QJsonArray jsonArray;
    for (const Language& language : languages) {
        QJsonObject obj;
        obj[language.code] = language.toJson();

        jsonArray << obj;
    }

    QByteArray data = QJsonDocument(jsonArray).toJson();

    Ret ret = writeLanguagesState(data);
    if (ret) {
        m_languagesHashChanged.send(languages);
    }

    return ret;
}

LanguagesHash LanguagesConfiguration::parseLanguagesConfig(const QByteArray& json) const
{
    TRACEFUNC;
    LanguagesHash result;

    QJsonParseError err;
    QJsonDocument jsodDoc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !jsodDoc.isArray()) {
        LOGE() << "failed parse, err: " << err.errorString();
        return LanguagesHash();
    }

    const QVariantList languages = jsodDoc.array().toVariantList();
    for (const QVariant& languagesObj : languages) {
        QMap<QString, QVariant> value = languagesObj.toMap();
        QVariantMap lngMap = value.first().toMap();

        Language language;
        language.code = value.keys().first();
        language.name = lngMap.value("name").toString();
        language.archiveFileName = lngMap.value("fileName").toString();
        language.status = static_cast<LanguageStatus::Status>(lngMap.value("status").toInt());

        const QVariantList files = lngMap.value("files").toList();
        for (const QVariant& fileVar : files) {
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

io::path LanguagesConfiguration::languagesAppDataPath() const
{
    return globalConfiguration()->appDataPath() + "/locale";
}

io::path LanguagesConfiguration::languagesUserAppDataPath() const
{
    return globalConfiguration()->userAppDataPath() + "/locale";
}

io::paths LanguagesConfiguration::languageFilePaths(const QString& languageCode) const
{
    TRACEFUNC;

    auto scan = [this](const io::path& path, const QString& code) {
        QStringList filters = { QString("*%1.qm").arg(code) };
        RetVal<io::paths> files = fileSystem()->scanFiles(path, filters);
        if (!files.ret) {
            LOGW() << files.ret.toString();
            return io::paths();
        }
        return files.val;
    };

    const io::paths defPaths = scan(languagesAppDataPath(), languageCode);
    io::paths userPaths = scan(languagesUserAppDataPath(), languageCode);

    //! NOTE Add def paths to user paths, if not presents
    for (const io::path& p : defPaths) {
        if (std::find(userPaths.begin(), userPaths.end(), p) == userPaths.end()) {
            userPaths.push_back(p);
        }
    }
    return userPaths;
}

io::path LanguagesConfiguration::languageArchivePath(const QString& languageCode) const
{
    TRACEFUNC;
    io::path fileName = languageFileName(languageCode);
    return languagesUserAppDataPath() + "/" + fileName;
}

io::path LanguagesConfiguration::languageFileName(const QString& languageCode) const
{
    TRACEFUNC;
    ValCh<LanguagesHash> _languages = languages();
    return _languages.val.value(languageCode).archiveFileName;
}
