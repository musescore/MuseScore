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
#include "languagesservice.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QDir>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QQmlEngine>
#include <QTranslator>

#include "languageserrors.h"

#include "global/io/buffer.h"
#include "global/serialization/zipreader.h"
#include "global/concurrency/concurrent.h"

#include "multiinstances/resourcelockguard.h"

#include "translation.h"

#include "log.h"

using namespace muse;
using namespace muse::languages;
using namespace muse::network;

static const QStringList LANGUAGE_RESOURCE_NAMES = {
    "musescore",
    "instruments"
};

static const std::string LANGUAGES_RESOURCE_NAME("LANGUAGES");

void LanguagesService::init()
{
    TRACEFUNC;

    fileSystem()->makePath(configuration()->languagesUserAppDataPath());

    loadLanguages();

    ValCh<QString> languageCode = configuration()->currentLanguageCode();
    setCurrentLanguage(languageCode.val);

    // Remember the active language code so the "Restart required" text can go away if the language
    // is changed and later reverted in the same session. Cannot use m_currentLanguage.code
    // because m_currentLanguage holds the effective language: if the language code is "system",
    // m_currentLanguage will hold "en-us" for example (whatever the system is set to).
    QString activeLanguageCode = languageCode.val;
    languageCode.ch.onReceive(this, [this, activeLanguageCode](const QString& newLanguageCode) {
        //! NOTE To change the language at the moment, a restart is required
        m_needRestartToApplyLanguageChange = newLanguageCode != activeLanguageCode;
        m_needRestartToApplyLanguageChangeChanged.send(m_needRestartToApplyLanguageChange);
    });

    m_inited = true;
}

const LanguagesHash& LanguagesService::languages() const
{
    return m_languagesHash;
}

Language LanguagesService::language(const QString& languageCode) const
{
    QString effectiveLanguageCode = this->effectiveLanguageCode(languageCode);
    if (effectiveLanguageCode.isEmpty()) {
        return {};
    }

    if (effectiveLanguageCode == PLACEHOLDER_LANGUAGE_CODE) {
        return m_placeholderLanguage;
    }

    return m_languagesHash[effectiveLanguageCode];
}

const Language& LanguagesService::currentLanguage() const
{
    return m_currentLanguage;
}

async::Notification LanguagesService::currentLanguageChanged() const
{
    return m_currentLanguageChanged;
}

const Language& LanguagesService::placeholderLanguage() const
{
    return m_placeholderLanguage;
}

bool LanguagesService::hasPlaceholderLanguage() const
{
    for (const QString& resourceName : LANGUAGE_RESOURCE_NAMES) {
        if (!fileSystem()->exists(configuration()->builtinLanguageFilePath(resourceName, PLACEHOLDER_LANGUAGE_CODE))) {
            return false;
        }
    }

    return true;
}

void LanguagesService::loadLanguages()
{
    TRACEFUNC;

    RetVal<ByteArray> languagesJson;
    {
        mi::ReadResourceLockGuard lock_guard(multiInstancesProvider(), LANGUAGES_RESOURCE_NAME);
        languagesJson = fileSystem()->readFile(configuration()->builtinLanguagesJsonPath());
    }
    if (!languagesJson.ret) {
        LOGE() << "Failed to read languages.json: " << languagesJson.ret.toString();
        return;
    }

    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(languagesJson.val.toQByteArrayNoCopy(), &err);
    if (err.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
        LOGE() << "Failed to parse languages.json: " << err.errorString();
        return;
    }

    QJsonObject languagesObject = jsonDoc.object();
    for (auto it = languagesObject.begin(); it != languagesObject.end(); ++it) {
        QJsonObject langObject = it.value().toObject();

        Language lang;
        lang.code = it.key();
        lang.name = langObject.value("name").toString();
        lang.fallbackLanguages = langObject.value("fallbackLanguages").toVariant().toStringList();

        m_languagesHash.insert(it.key(), lang);
    }

    m_placeholderLanguage.code = PLACEHOLDER_LANGUAGE_CODE;
    m_placeholderLanguage.name = "«Placeholder translations»";
}

void LanguagesService::setCurrentLanguage(const QString& languageCode)
{
    TRACEFUNC;

    QString effectiveLanguageCode = this->effectiveLanguageCode(languageCode);
    if (effectiveLanguageCode.isEmpty()) {
        LOGE() << "Unsupported language code: " << languageCode;
        return;
    }

    if (effectiveLanguageCode == m_currentLanguage.code) {
        return;
    }

    Language& lang = languageCode == PLACEHOLDER_LANGUAGE_CODE
                     ? m_placeholderLanguage
                     : m_languagesHash[effectiveLanguageCode];

    if (!lang.isLoaded()) {
        loadLanguage(lang);
    }

    for (QTranslator* t : std::as_const(m_translators)) {
        QCoreApplication::instance()->removeTranslator(t);
        delete t;
    }
    m_translators.clear();

    auto installTranslatorsForLanguage = [this](const Language& l) {
        for (const io::path_t& file : l.files) {
            QTranslator* translator = new QTranslator();
            bool ok = translator->load(file.toQString());
            if (ok) {
                QCoreApplication::instance()->installTranslator(translator);
                m_translators << translator;
            } else {
                LOGE() << "Error loading translator " << file.toQString();
                delete translator;
            }
        }
    };

    // Install translators for fallback languages in reverse order:
    // Qt searches the most recently installed translator first,
    // and the least recently installed translator last.
    for (auto it = lang.fallbackLanguages.crbegin(); it != lang.fallbackLanguages.crend(); ++it) {
        Language& fallbackLang = m_languagesHash[*it];
        installTranslatorsForLanguage(fallbackLang);
    }

    installTranslatorsForLanguage(lang);

    QLocale locale(lang.code);
    QLocale::setDefault(locale);
    qGuiApp->setLayoutDirection(locale.textDirection());

    lang.direction = locale.textDirection();

    // Currently, no need to retranslate the UI on language change, because we require restart

    m_currentLanguage = lang;
    m_currentLanguageChanged.notify();
}

QString LanguagesService::effectiveLanguageCode(QString languageCode) const
{
    languageCode.replace('-', '_');
    // Tries decreasingly specific versions of `code`. For example:
    // "nl_NL" -> not found -> try just "nl" -> found -> returns "nl".
    auto tryCode = [this](QString code) -> QString {
        for (;;) {
            if (m_languagesHash.contains(code)) {
                return code;
            }

            static const std::map<QString, QString> SPECIAL_CASES {
                { "ca_valencia", "ca@valencia" },
                { "ca_ES_valencia", "ca@valencia" },
                { "en_AU", "en_GB" },
                { "en_NZ", "en_GB" },
                { "en", "en_US" },
                { "hi", "hi_IN" },
                { "mn", "mn_MN" },
                { "zh", "zh_CN" }
            };

            auto it = SPECIAL_CASES.find(code);
            if (it != SPECIAL_CASES.cend()) {
                return it->second;
            }

            int rightmost = code.lastIndexOf('_');
            if (rightmost <= 0) {
                break;
            }

            code.truncate(rightmost);
        }

        // Not found
        return QString();
    };

    if (languageCode.isEmpty() || languageCode == SYSTEM_LANGUAGE_CODE) {
        for (const QString& code : QLocale::system().uiLanguages()) {
            LOGI() << "System language code: " << code;
            QString effectiveCode = code;
            effectiveCode.replace('-', '_');
            // Prefer Swedish (Modern) over Swedish (Traditional)
            // when using system language.
            if (effectiveCode == "sv_SE") {
                effectiveCode = "sv";
            }
            effectiveCode = tryCode(effectiveCode);
            if (!effectiveCode.isEmpty()) {
                return effectiveCode;
            }
        }

        // Not found
        return QString();
    }

    if (languageCode == PLACEHOLDER_LANGUAGE_CODE) {
        return languageCode;
    }

    return tryCode(languageCode);
}

Ret LanguagesService::loadLanguage(Language& lang)
{
    Ret ret = doLoadLanguage(lang);
    if (!ret) {
        LOGE() << "Failed to load language " << lang.code << ": " << ret.toString();
        return ret;
    }

    for (const QString& fallbackCode : lang.fallbackLanguages) {
        Language& fallbackLang = m_languagesHash[fallbackCode];

        if (!fallbackLang.isLoaded()) {
            ret = doLoadLanguage(fallbackLang);
            if (!ret) {
                LOGE() << "Failed to load fallback language " << fallbackLang.code << ": " << ret.toString();
                return ret;
            }
        }
    }

    return muse::make_ok();
}

Ret LanguagesService::doLoadLanguage(Language& lang)
{
    io::path_t languagesAppDataPath = configuration()->languagesAppDataPath();
    io::path_t languagesUserAppDataPath = configuration()->languagesUserAppDataPath();

    QString languageSuffix = QString("_%1.qm").arg(lang.code);

    std::string filter = ("*" + languageSuffix).toStdString();

    RetVal<io::paths_t> appFilePaths = fileSystem()->scanFiles(languagesAppDataPath, { filter });
    if (!appFilePaths.ret) {
        LOGE() << "Failed to scan files for language " << lang.code << ": " << appFilePaths.ret.toString();
        return appFilePaths.ret;
    }

    for (const io::path_t& appFilePath : appFilePaths.val) {
        io::path_t filename = io::filename(appFilePath);
        io::path_t userFilePath = languagesUserAppDataPath.appendingComponent(filename);

        QFileInfo appFileInfo(appFilePath.toQString());
        QFileInfo userFileInfo(userFilePath.toQString());
        bool useUserPath = userFileInfo.exists() && userFileInfo.lastModified() > appFileInfo.lastModified();

        QString resourceName = filename.toQString().chopped(languageSuffix.size());
        lang.files[resourceName] = useUserPath ? userFilePath : appFilePath;
    }

    for (const QString& resourceName : LANGUAGE_RESOURCE_NAMES) {
        if (!lang.files.contains(resourceName)) {
            LOGE() << "Could not find resource " << resourceName << " for language " << lang.code;
            return muse::make_ret(muse::Ret::Code::InternalError);
        }
    }

    return muse::make_ok();
}

Progress LanguagesService::update(const QString& languageCode)
{
    QString effectiveLanguageCode = this->effectiveLanguageCode(languageCode);
    IF_ASSERT_FAILED(!effectiveLanguageCode.isEmpty()) {
        return {};
    }

    Language& lang = m_languagesHash[effectiveLanguageCode];

    if (!lang.isLoaded()) {
        loadLanguage(lang);
    }

    Progress progress;

    progress.finished().onReceive(this, [](const ProgressResult& res) {
        if (!res.ret && res.ret.code() != static_cast<int>(Err::AlreadyUpToDate)) {
            LOGE() << res.ret.toString();
        }
    });

    Concurrent::run(this, &LanguagesService::th_update, effectiveLanguageCode, progress);

    return progress;
}

bool LanguagesService::needRestartToApplyLanguageChange() const
{
    return m_needRestartToApplyLanguageChange;
}

async::Channel<bool> LanguagesService::needRestartToApplyLanguageChangeChanged() const
{
    return m_needRestartToApplyLanguageChangeChanged;
}

void LanguagesService::th_update(const QString& mainLanguageCode, Progress overallProgress)
{
    QMutexLocker locker(&m_updateMutex);
    overallProgress.start();

    overallProgress.progress(0, 0, muse::trc("languages", "Checking for updates…"));

    const RetVal<QJsonObject> serverLanguagesInfo = downloadServerLanguagesInfo();
    if (!serverLanguagesInfo.ret) {
        overallProgress.finish(serverLanguagesInfo.ret);
        return;
    }

    QStringList languagesNeedingUpdate;
    {
        auto languageNeedsUpdate = [this, &serverLanguagesInfo](const QString& code) {
            const Language& lang = m_languagesHash[code];
            const QJsonObject languageObject = serverLanguagesInfo.val.value(code).toObject();

            for (const QString& resource : LANGUAGE_RESOURCE_NAMES) {
                RetVal<QString> hash = fileHash(lang.files[resource]);
                QString latestHash = languageObject.value(resource).toObject().value("hash").toString();

                if (!hash.ret || hash.val != latestHash) {
                    return true;
                }
            }

            return false;
        };

        if (languageNeedsUpdate(mainLanguageCode)) {
            languagesNeedingUpdate << mainLanguageCode;
        }
        for (const QString& fallbackCode : std::as_const(m_languagesHash[mainLanguageCode].fallbackLanguages)) {
            if (languageNeedsUpdate(fallbackCode)) {
                languagesNeedingUpdate << fallbackCode;
            }
        }
    }

    if (languagesNeedingUpdate.isEmpty()) {
        overallProgress.finish(make_ret(Err::AlreadyUpToDate));
        return;
    }

    {
        const int64_t overallTotal = languagesNeedingUpdate.size() * 100;
        int64_t index = 0;

        for (const QString& code : std::as_const(languagesNeedingUpdate)) {
            const Language& lang = m_languagesHash[code];

            Progress languageProgress;
            languageProgress.progressChanged().onReceive(this, [&](int64_t current, int64_t total, const std::string& title) {
                int64_t percentage = total > 0 ? current * 100 / total : 0;
                overallProgress.progress(index * 100 + percentage, overallTotal, title);
            });

            Ret ret = downloadLanguage(lang, languageProgress);
            if (!ret) {
                overallProgress.finish(ret);
                return;
            }

            ++index;
        }
    }

    m_needRestartToApplyLanguageChange = true;
    m_needRestartToApplyLanguageChangeChanged.send(m_needRestartToApplyLanguageChange);

    overallProgress.finish(make_ret(Err::NoError));
}

RetVal<QJsonObject> LanguagesService::downloadServerLanguagesInfo() const
{
    QBuffer buff;
    INetworkManagerPtr networkManagerPtr = networkManagerCreator()->makeNetworkManager();

    Ret ret = networkManagerPtr->get(configuration()->languagesUpdateUrl().toString(), &buff);
    if (!ret) {
        LOGE() << ret.toString();
        return ret;
    }

    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(buff.data(), &err);
    if (err.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
        return make_ret(Err::ErrorInvalidServerLanguagesInfo);
    }

    return RetVal<QJsonObject>::make_ok(jsonDoc.object());
}

Ret LanguagesService::downloadLanguage(const Language& lang, Progress progress) const
{
    const std::string downloadingStatusTitle = muse::qtrc("languages", "Downloading %1…").arg(lang.name).toStdString();
    progress.progress(0, 0, downloadingStatusTitle);

    QBuffer qbuff;
    INetworkManagerPtr networkManagerPtr = networkManagerCreator()->makeNetworkManager();

    networkManagerPtr->progress().progressChanged().onReceive(
        this, [&progress, &downloadingStatusTitle](int64_t current, int64_t total, const std::string&) {
        progress.progress(current, total, downloadingStatusTitle);
    });

    Ret ret = networkManagerPtr->get(configuration()->languageFileServerUrl(lang.code), &qbuff);
    if (!ret) {
        LOGE() << "Error while downloading: " << ret.toString();
        return make_ret(Err::ErrorDownloadLanguage);
    }

    const std::string unpackingStatusTitle = muse::qtrc("languages", "Unpacking %1…").arg(lang.name).toStdString();
    progress.progress(0, 0, unpackingStatusTitle);

    ByteArray ba = ByteArray::fromQByteArrayNoCopy(qbuff.data());
    io::Buffer buff(&ba);
    ZipReader zipReader(&buff);

    {
        mi::WriteResourceLockGuard lock_guard(multiInstancesProvider(), LANGUAGES_RESOURCE_NAME);

        for (const auto& info : zipReader.fileInfoList()) {
            io::path_t userFilePath = configuration()->languagesUserAppDataPath().appendingComponent(info.filePath);
            ret = fileSystem()->writeFile(userFilePath, zipReader.fileData(info.filePath.toStdString()));
            if (!ret) {
                LOGE() << "Error while writing to disk: " << ret.toString();
                return make_ret(Err::ErrorWriteLanguage);
            }
        }
    }

    return muse::make_ok();
}

RetVal<QString> LanguagesService::fileHash(const io::path_t& path)
{
    RetVal<ByteArray> fileBytes;
    {
        mi::ReadResourceLockGuard lock_guard(multiInstancesProvider(), LANGUAGES_RESOURCE_NAME);
        fileBytes = fileSystem()->readFile(path);
    }
    if (!fileBytes.ret) {
        return fileBytes.ret;
    }

    QCryptographicHash hasher(QCryptographicHash::Sha1);
    hasher.reset();
    hasher.addData(fileBytes.val.toQByteArrayNoCopy());
    return RetVal<QString>::make_ok(QString(hasher.result().toHex()));
}
