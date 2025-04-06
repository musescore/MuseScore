/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include <QJsonObject>
#include <QJsonParseError>
#include <QQmlEngine>
#include <QTranslator>

#include "languageserrors.h"

#include "multiwindows/resourcelockguard.h"

#include "global/io/buffer.h"
#include "global/serialization/zipreader.h"
#include "global/translation.h"
#include "global/log.h"

using namespace Qt::Literals::StringLiterals;

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
        m_restartRequiredToApplyLanguage = newLanguageCode != activeLanguageCode;
        m_restartRequiredToApplyLanguageChanged.send(m_restartRequiredToApplyLanguage);
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
        mi::ReadResourceLockGuard lock_guard(multiwindowsProvider(), LANGUAGES_RESOURCE_NAME);
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
    m_translators.reserve(lang.files.size());

    auto installTranslatorsForLanguage = [this](const Language& l) {
        for (const io::path_t& file : l.files) {
            QTranslator* translator = new QTranslator();
            bool ok = translator->load(file.toQString());
            if (ok) {
                QCoreApplication::instance()->installTranslator(translator);
                m_translators.push_back(translator);
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
    languageCode.replace(u'-', u'_');

    auto tryCode = [this](const QString& code) -> QString {
        if (m_languagesHash.contains(code)) {
            return code;
        }

        static const std::map<QString, QString> SPECIAL_CASES {
            { u"ca_valencia"_s, u"ca@valencia"_s },
            { u"ca_ES_valencia"_s, u"ca@valencia"_s },
            { u"en_AU"_s, u"en_GB"_s },
            { u"en_NZ"_s, u"en_GB"_s },
            { u"en"_s, u"en_US"_s },
            { u"hi"_s, u"hi_IN"_s },
            { u"mn"_s, u"mn_MN"_s },
            { u"zh"_s, u"zh_CN"_s }
        };

        auto it = SPECIAL_CASES.find(code);
        if (it != SPECIAL_CASES.cend()) {
            return it->second;
        }

        // Not found
        return QString();
    };

    if (languageCode.isEmpty() || languageCode == SYSTEM_LANGUAGE_CODE) {
        const QStringList systemLanguages = QLocale::system().uiLanguages();
        LOGI() << "System languages: " << systemLanguages;

        for (QString code : systemLanguages) {
            code.replace(u'-', u'_');

            // Prefer Swedish (Modern) over Swedish (Traditional)
            // when using system language.
            if (code == u"sv_SE"_s) {
                code = u"sv"_s;
            }

            code = tryCode(code);
            if (!code.isEmpty()) {
                return code;
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
    if (m_languageUpdateInProgress) {
        LOGW() << "Language update already in progress";
        return {};
    }

    m_languageUpdateInProgress = true;

    QString effectiveLanguageCode = this->effectiveLanguageCode(languageCode);
    IF_ASSERT_FAILED(!effectiveLanguageCode.isEmpty()) {
        return {};
    }

    Language& lang = m_languagesHash[effectiveLanguageCode];
    if (!lang.isLoaded()) {
        loadLanguage(lang);
    }

    if (!m_networkManager) {
        m_networkManager = networkManagerCreator()->makeNetworkManager();
    }

    Progress progress;
    progress.start();
    progress.progress(0, 0, muse::trc("global", "Checking for updates…"));

    downloadServerLanguagesInfo(
        effectiveLanguageCode,
        [this, languageCode, progress]( const RetVal<QJsonObject>& res) mutable {
        if (!res.ret) {
            LOGE() << "Failed to download server languages info: " << res.ret.toString();
            progress.finish(res.ret);
            return;
        }

        QJsonObject serverLanguagesInfo = res.val;
        QStringList languagesToUpdate = this->languagesToUpdate(languageCode, serverLanguagesInfo);

        if (languagesToUpdate.isEmpty()) {
            progress.finish(make_ret(Err::AlreadyUpToDate));
            return;
        }

        doUpdateLanguages(languagesToUpdate, progress, [this, progress](const Ret& ret) mutable {
            m_languageUpdateInProgress = false;
            progress.finish(ret);

            if (ret) {
                m_restartRequiredToApplyLanguage = true;
                m_restartRequiredToApplyLanguageChanged.send(true);
            }
        });
    });

    return progress;
}

bool LanguagesService::restartRequiredToApplyLanguage() const
{
    return m_restartRequiredToApplyLanguage;
}

async::Channel<bool> LanguagesService::restartRequiredToApplyLanguageChanged() const
{
    return m_restartRequiredToApplyLanguageChanged;
}

void LanguagesService::downloadServerLanguagesInfo(const QString& languageCode, std::function<void(const RetVal<QJsonObject>&)> finished)
{
    auto buff = std::make_shared<QBuffer>();
    RetVal<Progress> progress = m_networkManager->get(configuration()->languagesUpdateUrl().toString(), buff);
    if (!progress.ret) {
        finished(RetVal<QJsonObject>::make_ret(progress.ret));
        return;
    }

    progress.val.finished().onReceive(this, [languageCode, buff, finished](const ProgressResult& res) {
        if (!res.ret) {
            finished(RetVal<QJsonObject>::make_ret(res.ret));
            return;
        }

        QJsonParseError err;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(buff->data(), &err);
        if (err.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
            finished(RetVal<QJsonObject>::make_ret(make_ret(Err::ErrorInvalidServerLanguagesInfo)));
            return;
        }

        finished(RetVal<QJsonObject>::make_ok(jsonDoc.object()));
    });
}

QStringList LanguagesService::languagesToUpdate(const QString& mainLanguageCode, const QJsonObject& serverLanguagesInfo) const
{
    QStringList languagesToUpdate;

    auto languageNeedsUpdate = [this, &serverLanguagesInfo](const QString& code) {
        const Language& lang = m_languagesHash[code];
        const QJsonObject languageObject = serverLanguagesInfo.value(code).toObject();

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
        languagesToUpdate.append(mainLanguageCode);
    }

    for (const QString& fallbackCode : m_languagesHash[mainLanguageCode].fallbackLanguages) {
        if (languageNeedsUpdate(fallbackCode)) {
            languagesToUpdate.append(fallbackCode);
        }
    }

    return languagesToUpdate;
}

void LanguagesService::doUpdateLanguages(const QStringList& languageCodes, Progress overallProgress,
                                         std::function<void(const Ret&)> overallFinished)
{
    assert(!languageCodes.empty());

    // Can't create recursion with self-capturing lambda, because we can't
    // capture any local variable by reference (they will have been destroyed by
    // the time the lambda is executed). So we pass the lambda as a parameter
    // instead. The type of this parameter cannot be written directly, as it
    // would be a recursive type.
    auto updateLanguage
        = [this, languageCodes, overallProgress, overallFinished](int languageIndex, auto updateNextLanguage) mutable -> void {
        auto progressCallback
            = [languageCodes, overallProgress, languageIndex](int64_t current, int64_t total, const std::string& msg) mutable {
            const int64_t overallTotal = languageCodes.size() * 10000;
            const int64_t overallCurrent = languageIndex * 10000 + current * 10000 / total;
            overallProgress.progress(overallCurrent, overallTotal, msg);
        };

        auto finished
            = [languageCodes, overallFinished, languageIndex, updateNextLanguage](const Ret& ret) mutable {
            if (ret) {
                if (++languageIndex < languageCodes.size()) {
                    updateNextLanguage(languageIndex, updateNextLanguage);
                } else {
                    overallFinished(make_ok());
                }
            } else {
                overallFinished(ret);
            }
        };

        doUpdateLanguage(languageCodes[languageIndex], progressCallback, finished);
    };

    updateLanguage(0, updateLanguage);
}

void LanguagesService::doUpdateLanguage(const QString& languageCode,
                                        std::function<void(int64_t current, int64_t total, const std::string&)> progressCallback,
                                        std::function<void(const Ret&)> finished)
{
    auto qbuff = std::make_shared<QBuffer>();
    const QUrl url = configuration()->languageFileServerUrl(languageCode);
    RetVal<Progress> downloadProgress = m_networkManager->get(url, qbuff);
    if (!downloadProgress.ret) {
        finished(make_ret(Err::ErrorDownloadLanguage));
        return;
    }

    const std::string downloadingMsg = muse::qtrc("global", "Downloading %1…").arg(languageCode).toStdString();
    progressCallback(0, 1, downloadingMsg);

    downloadProgress.val.progressChanged().onReceive(this,
                                                     [progressCallback, downloadingMsg](int64_t current, int64_t total, const std::string&) {
        progressCallback(current, total,
                         downloadingMsg);
    });

    downloadProgress.val.finished().onReceive(this, [this, progressCallback, finished, languageCode, qbuff](const ProgressResult& res) {
        if (!res.ret) {
            finished(make_ret(Err::ErrorDownloadLanguage));
            return;
        }

        progressCallback(1, 1, muse::qtrc("global", "Unpacking %1…").arg(languageCode).toStdString());

        Ret ret = unpackAndWriteLanguage(qbuff->data());
        finished(ret);
    });
}

Ret LanguagesService::unpackAndWriteLanguage(const QByteArray& zipData)
{
    TRACEFUNC;

    ByteArray ba = ByteArray::fromQByteArrayNoCopy(zipData);
    io::Buffer buff(&ba);
    ZipReader zipReader(&buff);

    {
        mi::WriteResourceLockGuard lock_guard(multiwindowsProvider(), LANGUAGES_RESOURCE_NAME);

        for (const auto& info : zipReader.fileInfoList()) {
            io::path_t userFilePath = configuration()->languagesUserAppDataPath().appendingComponent(info.filePath);
            Ret ret = fileSystem()->writeFile(userFilePath, zipReader.fileData(info.filePath.toStdString()));
            if (!ret) {
                return make_ret(Err::ErrorWriteLanguage);
            }
        }
    }

    return make_ok();
}

RetVal<QString> LanguagesService::fileHash(const io::path_t& path) const
{
    RetVal<ByteArray> fileBytes;
    {
        mi::ReadResourceLockGuard lock_guard(multiwindowsProvider(), LANGUAGES_RESOURCE_NAME);
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
