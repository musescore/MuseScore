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

static const Settings::Key LANGUAGE_KEY("languages", "language");

static const QString LANGUAGES_SERVER_URL("http://extensions.musescore.org/4.0/languages/");
static const io::path_t LANGUAGES_STATE_FILE("/languages.json");

static const std::string LANGUAGES_RESOURCE_NAME("LANGUAGES");

static const QString DEF_MUSESCORE_QM("musescore_%1.qm");
static const QString DEF_INSTRUMENTS_QM("instruments_%1.qm");

void LanguagesConfiguration::init()
{
    settings()->setDefaultValue(LANGUAGE_KEY, Val(SYSTEM_LANGUAGE_CODE.toStdString()));
    settings()->valueChanged(LANGUAGE_KEY).onReceive(nullptr, [this](const Val& val) {
        m_currentLanguageCodeChanged.send(val.toQString());
    });

    fileSystem()->makePath(languagesUserAppDataPath());
}

ValCh<QString> LanguagesConfiguration::currentLanguageCode() const
{
    ValCh<QString> result;
    result.ch = m_currentLanguageCodeChanged;
    result.val = settings()->value(LANGUAGE_KEY).toQString();

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
    io::path_t fileName = languageFileName(languageCode);
    return QUrl(LANGUAGES_SERVER_URL + fileName.toQString());
}

RetVal<ByteArray> LanguagesConfiguration::readDefaultLanguages() const
{
    return fileSystem()->readFile(globalConfiguration()->appDataPath() + "/locale/languages.json");
}

RetVal<ByteArray> LanguagesConfiguration::readLanguagesState() const
{
    mi::ReadResourceLockGuard lock_guard(multiInstancesProvider(), LANGUAGES_RESOURCE_NAME);
    return fileSystem()->readFile(languagesUserAppDataPath() + LANGUAGES_STATE_FILE);
}

Ret LanguagesConfiguration::writeLanguagesState(const QByteArray& data)
{
    mi::WriteResourceLockGuard lock_guard(multiInstancesProvider(), LANGUAGES_RESOURCE_NAME);
    return fileSystem()->writeFile(languagesUserAppDataPath() + LANGUAGES_STATE_FILE, ByteArray::fromQByteArrayNoCopy(data));
}

LanguagesHash LanguagesConfiguration::parseDefaultLanguages(const ByteArray& json) const
{
    TRACEFUNC;
    LanguagesHash result;

    QJsonParseError err;
    QJsonDocument jsodDoc = QJsonDocument::fromJson(json.toQByteArrayNoCopy(), &err);
    if (err.error != QJsonParseError::NoError) {
        LOGE() << "failed parse, err: " << err.errorString();
        return LanguagesHash();
    }

    const QJsonObject languages = jsodDoc.object();
    for (auto it = languages.begin(); it != languages.end(); ++it) {
        Language lang;
        lang.code = it.key();
        lang.name = it.value().toString();

        lang.files << LanguageFile(DEF_MUSESCORE_QM.arg(lang.code), QString());
        lang.files << LanguageFile(DEF_INSTRUMENTS_QM.arg(lang.code), QString());

        result.insert(lang.code, lang);
    }

    return result;
}

ValCh<LanguagesHash> LanguagesConfiguration::languages() const
{
    TRACEFUNC;

    LanguagesHash langs;
    {
        RetVal<ByteArray> rv = readLanguagesState();
        if (rv.ret) {
            langs = parseLanguagesState(rv.val);
        } else {
            LOGW() << "failed read server languages, err: " << rv.ret.toString();
        }
    }

    //! NOTE If there is a downloaded a list of languages from the server, then we use it, if not, then we use the default
    if (langs.empty()) {
        RetVal<ByteArray> rv = readDefaultLanguages();
        if (rv.ret) {
            langs = parseDefaultLanguages(rv.val);
        } else {
            LOGE() << "failed read default languages, err: " << rv.ret.toString();
        }
    }

    ValCh<LanguagesHash> result;
    result.val = langs;
    result.ch = m_languagesHashChanged;

    return result;
}

Ret LanguagesConfiguration::setLanguages(const LanguagesHash& languages)
{
    TRACEFUNC;
    QJsonObject obj;
    for (const Language& language : languages) {
        obj[language.code] = language.toJson();
    }

    QByteArray data = QJsonDocument(obj).toJson();

    Ret ret = writeLanguagesState(data);
    if (ret) {
        m_languagesHashChanged.send(languages);
    }

    return ret;
}

LanguagesHash LanguagesConfiguration::parseLanguagesState(const ByteArray& json) const
{
    TRACEFUNC;
    LanguagesHash result;

    QJsonParseError err;
    QJsonDocument jsodDoc = QJsonDocument::fromJson(json.toQByteArrayNoCopy(), &err);
    if (err.error != QJsonParseError::NoError) {
        LOGE() << "failed parse, err: " << err.errorString();
        return LanguagesHash();
    }

    const QJsonObject obj = jsodDoc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        Language language;
        language.code = it.key();

        QJsonObject langObj = it.value().toObject();

        language.name = langObj.value("name").toString();
        language.archiveFileName = langObj.value("fileName").toString();
        language.status = static_cast<LanguageStatus::Status>(langObj.value("status").toInt());

        const QJsonArray files = langObj.value("files").toArray();
        for (const QJsonValue& fileVal : files) {
            QJsonObject fileObj = fileVal.toObject();
            LanguageFile file;
            file.name = fileObj["name"].toString();
            file.hash = fileObj["hash"].toString();

            language.files << file;
        }

        result.insert(language.code, language);
    }

    return result;
}

io::path_t LanguagesConfiguration::languagesAppDataPath() const
{
    return globalConfiguration()->appDataPath() + "/locale";
}

io::path_t LanguagesConfiguration::languagesUserAppDataPath() const
{
    return globalConfiguration()->userAppDataPath() + "/locale";
}

io::paths_t LanguagesConfiguration::languageFilePaths(const QString& languageCode) const
{
    TRACEFUNC;

    auto scan = [this](const io::path_t& path, const QString& code) {
        std::string filter = QString("*%1.qm").arg(code).toStdString();
        RetVal<io::paths_t> files = fileSystem()->scanFiles(path, { filter });
        if (!files.ret) {
            LOGW() << files.ret.toString();
            return io::paths_t();
        }
        return files.val;
    };

    const io::paths_t defPaths = scan(languagesAppDataPath(), languageCode);
    io::paths_t userPaths = scan(languagesUserAppDataPath(), languageCode);

    //! NOTE Add def paths to user paths, if not presents
    for (const io::path_t& p : defPaths) {
        if (std::find(userPaths.begin(), userPaths.end(), p) == userPaths.end()) {
            userPaths.push_back(p);
        }
    }
    return userPaths;
}

io::path_t LanguagesConfiguration::languageArchivePath(const QString& languageCode) const
{
    TRACEFUNC;
    io::path_t fileName = languageFileName(languageCode);
    return languagesUserAppDataPath() + "/" + fileName;
}

io::path_t LanguagesConfiguration::languageFileName(const QString& languageCode) const
{
    TRACEFUNC;
    ValCh<LanguagesHash> _languages = languages();
    return _languages.val.value(languageCode).archiveFileName;
}
