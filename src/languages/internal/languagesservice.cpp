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

#include <QDir>
#include <QTranslator>
#include <QApplication>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QBuffer>
#include <QCryptographicHash>
#include <QtConcurrent>
#include <QQmlEngine>

#include "languageserrors.h"
#include "log.h"

using namespace mu;
using namespace mu::languages;
using namespace mu::framework;
using namespace mu::network;

static const QStringList languageFileTypes = {
    "musescore",
    "instruments"
};

namespace mu::languages {
static QString downloadingStatusTitle()
{
    return qtrc("languages", "Downloading…");
}
}

void LanguagesService::init()
{
    ValCh<QString> languageCode = configuration()->currentLanguageCode();
    setCurrentLanguage(languageCode.val);

    //! NOTE To change the language at the moment, a restart is required

    m_inited = true;
}

void LanguagesService::refreshLanguages()
{
    QtConcurrent::run(this, &LanguagesService::th_refreshLanguages);
}

ValCh<LanguagesHash> LanguagesService::languages() const
{
    ValCh<LanguagesHash> languagesHash = configuration()->languages();

    return languagesHash;
}

ValCh<Language> LanguagesService::currentLanguage() const
{
    ValCh<Language> result;

    ValCh<QString> languageCode = configuration()->currentLanguageCode();
    result.ch = m_currentLanguageChanged;

    LanguagesHash languageHash = languages().val;
    if (!languageHash.contains(languageCode.val)) {
        return result;
    }

    result.val = languageHash[languageCode.val];
    return result;
}

LanguageStatus::Status LanguagesService::languageStatus(const QString& languageCode) const
{
    TRACEFUNC;
    if (!isLanguageExists(languageCode)) {
        return LanguageStatus::Status::NoInstalled;
    }

    Language language = languages().val[languageCode];
    if (!checkLanguageFilesHash(languageCode, language.files)) {
        return LanguageStatus::Status::NeedUpdate;
    }

    return LanguageStatus::Status::Installed;
}

RetCh<LanguageProgress> LanguagesService::install(const QString& languageCode)
{
    RetCh<LanguageProgress> result;

    if (m_operationsHash.contains(languageCode)) {
        if (m_operationsHash[languageCode].type != OperationType::Install) {
            result.ret = make_ret(Err::ErrorAnotherOperationStarted);
            return result;
        }

        result.ret = make_ret(Err::NoError);
        result.ch = *m_operationsHash[languageCode].progressChannel;
        return result;
    }

    async::Channel<LanguageProgress>* languageProgressStatus = new async::Channel<LanguageProgress>();
    result.ch = *languageProgressStatus;
    result.ret = make_ret(Err::NoError);
    m_operationsHash.insert(languageCode, Operation(OperationType::Install, languageProgressStatus));

    async::Channel<Ret>* languageFinishChannel = new async::Channel<Ret>();
    languageFinishChannel->onReceive(this, [this, languageCode, languageProgressStatus](const Ret& ret) {
        if (!ret) {
            LOGE() << ret.toString();
            return;
        }

        closeOperation(languageCode, languageProgressStatus);
    }, Asyncable::AsyncMode::AsyncSetRepeat);
    QtConcurrent::run(this, &LanguagesService::th_install, languageCode, languageProgressStatus, languageFinishChannel);

    return result;
}

RetCh<LanguageProgress> LanguagesService::update(const QString& languageCode)
{
    RetCh<LanguageProgress> result;

    if (m_operationsHash.contains(languageCode)) {
        if (m_operationsHash[languageCode].type != OperationType::Install) {
            result.ret = make_ret(Err::ErrorAnotherOperationStarted);
            return result;
        }

        result.ret = make_ret(Err::NoError);
        result.ch = *m_operationsHash[languageCode].progressChannel;
        return result;
    }

    async::Channel<LanguageProgress>* languageProgressStatus = new async::Channel<LanguageProgress>();
    result.ch = *languageProgressStatus;
    result.ret = make_ret(Err::NoError);
    m_operationsHash.insert(languageCode, Operation(OperationType::Install, languageProgressStatus));

    async::Channel<Ret>* languageFinishChannel = new async::Channel<Ret>();
    languageFinishChannel->onReceive(this, [this, languageCode, languageProgressStatus](const Ret& ret) {
        if (!ret) {
            LOGE() << ret.toString();
            return;
        }

        closeOperation(languageCode, languageProgressStatus);
    }, Asyncable::AsyncMode::AsyncSetRepeat);

    QtConcurrent::run(this, &LanguagesService::th_update, languageCode, languageProgressStatus, languageFinishChannel);

    return result;
}

Ret LanguagesService::uninstall(const QString& languageCode)
{
    LanguagesHash languagesHash = languages().val;

    if (!languagesHash.contains(languageCode)) {
        return make_ret(Err::ErrorLanguageNotFound);
    }

    Ret remove = removeLanguage(languageCode);
    if (!remove) {
        return remove;
    }

    QString currentLanguageCode = currentLanguage().val.code;
    if (languageCode == currentLanguageCode) {
        resetLanguageToSystemLanguage();
    }

    return make_ret(Err::NoError);
}

void LanguagesService::setCurrentLanguage(const QString& languageCode)
{
    if (languageCode == SYSTEM_LANGUAGE_CODE) {
        resetLanguageToSystemLanguage();
        return;
    }

    if (languageCode == PLACEHOLDER_LANGUAGE_CODE) {
        Ret load = loadLanguage(languageCode);
        if (!load) {
            LOGE() << load.toString();
        }
        return; // no hash for this language
    }

    LanguagesHash languageHash = languages().val;
    if (!languageHash.contains(languageCode)) {
        LOGE() << "Unknown language: " << languageCode;
        return;
    }

    Ret load = loadLanguage(languageCode);
    if (!load) {
        LOGE() << load.toString();
        return;
    }

    Language& language = languageHash[languageCode];
    m_currentLanguageChanged.send(language);
}

RetVal<LanguagesHash> LanguagesService::parseLanguagesConfig(const QByteArray& json) const
{
    RetVal<LanguagesHash> result;

    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
        LOGE() << "failed parse, err: " << err.errorString();
        result.ret = make_ret(Err::ErrorParseConfig);
        return result;
    }

    result.ret = make_ret(Err::NoError);
    QStringList languages = jsonDoc.object().keys();
    for (const QString& key : languages) {
        if (!jsonDoc.object().value(key).isObject()) {
            continue;
        }

        QJsonObject value = jsonDoc.object().value(key).toObject();

        Language language;
        language.code = key;
        language.name = value.value("name").toString();
        language.archiveFileName = value.value("file_name").toString();
        language.files = parseLanguageFiles(value);

        result.val.insert(key, language);
    }

    return result;
}

LanguageFiles LanguagesService::parseLanguageFiles(const QJsonObject& languageObject) const
{
    LanguageFiles files;

    for (const QString& fileType: languageFileTypes) {
        QJsonObject instrumentsFile = languageObject.value(fileType).toObject();
        if (!instrumentsFile.isEmpty()) {
            files << LanguageFile(instrumentsFile.value("file_name").toString(),
                                  instrumentsFile.value("hash").toString());
        }
    }

    return files;
}

bool LanguagesService::isLanguageExists(const QString& languageCode) const
{
    io::paths_t files = configuration()->languageFilePaths(languageCode);
    return !files.empty();
}

bool LanguagesService::checkLanguageFilesHash(const QString& languageCode, const LanguageFiles& languageFiles) const
{
    io::paths_t filePaths = configuration()->languageFilePaths(languageCode);
    int filesSize = static_cast<int>(filePaths.size());
    if (filesSize != languageFiles.size()) {
        return false;
    }

    QHash<QString, QString> filesHash;
    for (const LanguageFile& file: languageFiles) {
        filesHash.insert(file.name, file.hash);
    }

    QCryptographicHash localHash(QCryptographicHash::Sha1);
    for (const io::path_t& filePath: filePaths) {
        QString fileName = io::filename(filePath).toQString();
        if (!filesHash.contains(fileName)) {
            continue;
        }

        RetVal<ByteArray> fileBytes = fileSystem()->readFile(filePath);
        if (!fileBytes.ret) {
            LOGW() << fileBytes.ret.toString();
            return false;
        }

        localHash.reset();
        localHash.addData(fileBytes.val.toQByteArrayNoCopy());
        QString fileHash = QString(localHash.result().toHex());
        QString actualHash = filesHash[fileName];
        if (actualHash != fileHash) {
            return false;
        }
    }

    return true;
}

Language LanguagesService::language(const QString& languageCode) const
{
    return languages().val[languageCode];
}

RetVal<QString> LanguagesService::downloadLanguage(const QString& languageCode, async::Channel<LanguageProgress>* progressChannel) const
{
    RetVal<QString> result;

    QString languageArchivePath = configuration()->languageArchivePath(languageCode).toQString();

    QBuffer buff;
    INetworkManagerPtr networkManagerPtr = networkManagerCreator()->makeNetworkManager();

    networkManagerPtr->progress().progressChanged.onReceive(new async::Asyncable(),
                                                            [&progressChannel](int64_t current, int64_t total, const std::string&) {
        progressChannel->send(LanguageProgress(downloadingStatusTitle(), current, total));
    });

    Ret getLanguage = networkManagerPtr->get(configuration()->languageFileServerUrl(languageCode), &buff);
    if (!getLanguage) {
        LOGE() << "Error save file";
        result.ret = make_ret(Err::ErrorDownloadLanguage);
        return result;
    }

    QFile file(languageArchivePath);
    file.open(QIODevice::WriteOnly);
    file.write(buff.data());
    file.close();

    result.ret = make_ret(Err::NoError);
    result.val = languageArchivePath;
    return result;
}

Ret LanguagesService::removeLanguage(const QString& languageCode) const
{
    io::paths_t files = configuration()->languageFilePaths(languageCode);

    for (const io::path_t& filePath: files) {
        Ret ret = fileSystem()->remove(filePath);
        if (!ret) {
            LOGE() << "Error remove file " << filePath << ret.toString();
            return make_ret(Err::ErrorRemoveLanguageDirectory);
        }
    }

    return make_ret(Err::NoError);
}

Ret LanguagesService::loadLanguage(const QString& languageCode)
{
    io::paths_t files = configuration()->languageFilePaths(languageCode);
    if (files.empty()) {
        QString shortLanguageCode = languageCode.left(languageCode.lastIndexOf("_"));
        files = configuration()->languageFilePaths(shortLanguageCode);
    }

    if (files.empty()) {
        return make_ret(Err::UnknownError);
    }

    for (QTranslator* t : m_translatorList) {
        qApp->removeTranslator(t);
        delete t;
    }
    m_translatorList.clear();

    for (const io::path_t& filePath : files) {
        QTranslator* translator = new QTranslator();
        bool ok = translator->load(filePath.toQString());
        if (ok) {
            qApp->installTranslator(translator);
            m_translatorList.append(translator);
        } else {
            LOGE() << "Error load translate " << filePath;
            delete translator;
        }
    }

    QLocale locale(languageCode);
    QLocale::setDefault(locale);
    qApp->setLayoutDirection(locale.textDirection());

    if (m_inited) {
        if (QQmlEngine* engine = uiEngine()->qmlEngine()) {
            engine->retranslate();
        }
    }

    return make_ret(Err::NoError);
}

void LanguagesService::resetLanguageToSystemLanguage()
{
    Ret load = loadLanguage(QLocale::system().name());
    if (!load) {
        LOGE() << load.toString();
        return;
    }
}

void LanguagesService::th_refreshLanguages()
{
    QBuffer buff;
    INetworkManagerPtr networkManagerPtr = networkManagerCreator()->makeNetworkManager();

    Ret getLanguagesInfo = networkManagerPtr->get(configuration()->languagesUpdateUrl().toString(), &buff);

    if (!getLanguagesInfo) {
        LOGE() << getLanguagesInfo.toString();
        return;
    }

    QByteArray json = buff.data();
    RetVal<LanguagesHash> actualLanguages = parseLanguagesConfig(json);

    if (!actualLanguages.ret) {
        LOGE() << actualLanguages.ret.toString();
        return;
    }

    LanguagesHash savedLanguages = configuration()->languages().val;

    LanguagesHash resultLanguages = savedLanguages;

    for (Language& language : actualLanguages.val) {
        if (resultLanguages.contains(language.code)) {
            Language& savedLanguage = resultLanguages[language.code];
            savedLanguage = language;
        } else {
            resultLanguages.insert(language.code, language);
        }
    }

    configuration()->setLanguages(resultLanguages);
}

void LanguagesService::th_install(const QString& languageCode, async::Channel<LanguageProgress>* progressChannel,
                                  async::Channel<Ret>* finishChannel)
{
    progressChannel->send(LanguageProgress(downloadingStatusTitle(), true));

    RetVal<QString> download = downloadLanguage(languageCode, progressChannel);
    if (!download.ret) {
        finishChannel->send(download.ret);
        return;
    }

    progressChannel->send(LanguageProgress(downloadingStatusTitle(), true));

    QString languageArchivePath = download.val;

    Ret unpack = languageUnpacker()->unpack(languageCode, languageArchivePath, configuration()->languagesUserAppDataPath());
    if (!unpack) {
        LOGE() << unpack.toString();
        fileSystem()->remove(languageArchivePath);

        finishChannel->send(unpack);
        return;
    }

    fileSystem()->remove(languageArchivePath);

    finishChannel->send(make_ret(Err::NoError));
}

void LanguagesService::th_update(const QString& languageCode, async::Channel<LanguageProgress>* progressChannel,
                                 async::Channel<Ret>* finishChannel)
{
    progressChannel->send(LanguageProgress(downloadingStatusTitle(), true));

    RetVal<QString> download = downloadLanguage(languageCode, progressChannel);
    if (!download.ret) {
        finishChannel->send(download.ret);
    }

    progressChannel->send(LanguageProgress(downloadingStatusTitle(), true));

    QString languageArchivePath = download.val;

    Ret remove = removeLanguage(languageCode);
    if (!remove) {
        finishChannel->send(remove);
    }

    Ret unpack = languageUnpacker()->unpack(languageCode, languageArchivePath, configuration()->languagesUserAppDataPath());
    if (!unpack) {
        LOGE() << "Error unpack " << unpack.toString();
        finishChannel->send(unpack);
    }

    fileSystem()->remove(languageArchivePath);

    finishChannel->send(make_ret(Err::NoError));
}

void LanguagesService::closeOperation(const QString& languageCode, async::Channel<LanguageProgress>* progressChannel)
{
    progressChannel->close();
    m_operationsHash.remove(languageCode);
}
