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

    languageCode.ch.onReceive(this, [this](const QString&) {
        //! NOTE To change the language at the moment, a restart is required
        m_needRestartToApplyLanguageChange = true;
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
        Language lang;
        lang.code = it.key();
        lang.name = it.value().toString();

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

    for (QTranslator* t : m_translators) {
        qApp->removeTranslator(t);
        delete t;
    }
    m_translators.clear();

    for (const io::path_t& file : lang.files) {
        QTranslator* translator = new QTranslator();
        bool ok = translator->load(file.toQString());
        if (ok) {
            qApp->installTranslator(translator);
            m_translators << translator;
        } else {
            LOGE() << "Error loading translator " << file.toQString();
            delete translator;
        }
    }

    QLocale locale(lang.code);
    QLocale::setDefault(locale);
    qApp->setLayoutDirection(locale.textDirection());

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

    assert([&lang]() {
        for (const QString& resourceName : LANGUAGE_RESOURCE_NAMES) {
            if (!lang.files.contains(resourceName)) {
                LOGE() << "Could not find resource " << resourceName << " for language " << lang.code;
                return false;
            }
        }

        return true;
    }());

    return muse::make_ok();
}

Progress LanguagesService::update(const QString& languageCode)
{
    QString effectiveLanguageCode = this->effectiveLanguageCode(languageCode);
    IF_ASSERT_FAILED(!effectiveLanguageCode.isEmpty()) {
        return {};
    }

    if (m_updateOperationsHash.contains(effectiveLanguageCode)) {
        return m_updateOperationsHash[effectiveLanguageCode];
    }

    Language& lang = m_languagesHash[effectiveLanguageCode];

    if (!lang.isLoaded()) {
        loadLanguage(lang);
    }

    Progress progress;

    m_updateOperationsHash.insert(effectiveLanguageCode, progress);

    progress.finished.onReceive(this, [this, effectiveLanguageCode](const ProgressResult& res) {
        if (!res.ret && res.ret.code() != static_cast<int>(Err::AlreadyUpToDate)) {
            LOGE() << res.ret.toString();
        }

        m_updateOperationsHash.remove(effectiveLanguageCode);
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

void LanguagesService::th_update(const QString& languageCode, Progress progress)
{
    progress.started.notify();

    progress.progressChanged.send(0, 0, muse::trc("languages", "Checking for updates…"));

    if (!canUpdate(languageCode)) {
        progress.finished.send(make_ret(Err::AlreadyUpToDate));
        return;
    }

    Ret ret = downloadLanguage(languageCode, progress);
    if (!ret) {
        progress.finished.send(ret);
        return;
    }

    m_needRestartToApplyLanguageChange = true;
    m_needRestartToApplyLanguageChangeChanged.send(m_needRestartToApplyLanguageChange);

    progress.finished.send(make_ret(Err::NoError));
}

bool LanguagesService::canUpdate(const QString& languageCode)
{
    QBuffer buff;
    INetworkManagerPtr networkManagerPtr = networkManagerCreator()->makeNetworkManager();

    Ret ret = networkManagerPtr->get(configuration()->languagesUpdateUrl().toString(), &buff);
    if (!ret) {
        LOGE() << ret.toString();
        return false;
    }

    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(buff.data(), &err);
    if (err.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
        return false;
    }

    QJsonObject languageObject = jsonDoc.object().value(languageCode).toObject();

    Language& language = m_languagesHash[languageCode];

    for (const QString& resource : LANGUAGE_RESOURCE_NAMES) {
        RetVal<QString> hash = fileHash(language.files[resource]);
        QString latestHash = languageObject.value(resource).toObject().value("hash").toString();

        if (!hash.ret || hash.val != latestHash) {
            return true;
        }
    }

    return false;
}

Ret LanguagesService::downloadLanguage(const QString& languageCode, Progress progress) const
{
    std::string downloadingStatusTitle = muse::trc("languages", "Downloading…");
    progress.progressChanged.send(0, 0, downloadingStatusTitle);

    QBuffer qbuff;
    INetworkManagerPtr networkManagerPtr = networkManagerCreator()->makeNetworkManager();

    networkManagerPtr->progress().progressChanged.onReceive(
        this, [&progress, &downloadingStatusTitle](int64_t current, int64_t total, const std::string&) {
        progress.progressChanged.send(current, total, downloadingStatusTitle);
    });

    Ret ret = networkManagerPtr->get(configuration()->languageFileServerUrl(languageCode), &qbuff);
    if (!ret) {
        LOGE() << "Error while downloading: " << ret.toString();
        return make_ret(Err::ErrorDownloadLanguage);
    }

    progress.progressChanged.send(0, 0, muse::trc("languages", "Unpacking…"));

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
