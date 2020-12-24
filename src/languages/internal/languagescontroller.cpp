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
#include "languagescontroller.h"

#include <QDir>
#include <QTranslator>
#include <QApplication>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QBuffer>
#include <QCryptographicHash>
#include <QtConcurrent>
#include "log.h"
#include "languageserrors.h"

using namespace mu;
using namespace mu::languages;
using namespace mu::framework;

static const QString DEFAULT_LANGUAGE("system");
static const QString ANALYSING_STATUS = qtrc("languages", "Analysing...");
static const QString DOWNLOADING_STATUS = qtrc("languages", "Downloading...");

static const QStringList languageFileTypes = {
    "mscore",
    "instruments",
    "tours"
};

void LanguagesController::init()
{
    fileSystem()->makePath(configuration()->languagesSharePath());
    fileSystem()->makePath(configuration()->languagesDataPath());

    QString code = configuration()->currentLanguageCode();
    loadLanguage(code);

    QtConcurrent::run(this, &LanguagesController::th_refreshLanguages);
}

ValCh<LanguagesHash> LanguagesController::languages() const
{
    ValCh<LanguagesHash> languagesHash = configuration()->languages();
    languagesHash.val = correctLanguagesStates(languagesHash.val).val;

    return languagesHash;
}

RetCh<LanguageProgress> LanguagesController::install(const QString& languageCode)
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
            closeOperation(languageCode, languageProgressStatus);
            return;
        }

        LanguagesHash languageHash = this->languages().val;

        languageHash[languageCode].status = LanguageStatus::Status::Installed;

        Ret updateConfigRet = configuration()->setLanguages(languageHash);
        if (!updateConfigRet) {
            LOGW() << updateConfigRet.toString();
            closeOperation(languageCode, languageProgressStatus);
            return;
        }

        m_languageChanged.send(languageHash[languageCode]);
        closeOperation(languageCode, languageProgressStatus);
    }, Asyncable::AsyncMode::AsyncSetRepeat);
    QtConcurrent::run(this, &LanguagesController::th_install, languageCode, languageProgressStatus, languageFinishChannel);

    return result;
}

RetCh<LanguageProgress> LanguagesController::update(const QString& languageCode)
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
            closeOperation(languageCode, languageProgressStatus);
            return;
        }

        LanguagesHash languageHash = this->languages().val;

        languageHash[languageCode].status = LanguageStatus::Status::Installed;

        Ret updateConfigRet = configuration()->setLanguages(languageHash);
        if (!updateConfigRet) {
            LOGW() << updateConfigRet.toString();
            closeOperation(languageCode, languageProgressStatus);
            return;
        }

        m_languageChanged.send(languageHash[languageCode]);
        closeOperation(languageCode, languageProgressStatus);
    }, Asyncable::AsyncMode::AsyncSetRepeat);

    QtConcurrent::run(this, &LanguagesController::th_update, languageCode, languageProgressStatus, languageFinishChannel);

    return result;
}

Ret LanguagesController::uninstall(const QString& languageCode)
{
    LanguagesHash languagesHash = languages().val;

    if (!languagesHash.contains(languageCode)) {
        return make_ret(Err::ErrorLanguageNotFound);
    }

    Ret remove = removeLanguage(languageCode);
    if (!remove) {
        return remove;
    }

    if (languagesHash[languageCode].isCurrent) {
        resetLanguageByDefault();
    }

    languagesHash[languageCode].status = LanguageStatus::Status::NoInstalled;
    Ret ret = configuration()->setLanguages(languagesHash);
    if (!ret) {
        return ret;
    }

    m_languageChanged.send(languagesHash[languageCode]);

    return make_ret(Err::NoError);
}

RetVal<Language> LanguagesController::currentLanguage() const
{
    RetVal<Language> result;

    QString languageCode = configuration()->currentLanguageCode();

    if (languageCode == DEFAULT_LANGUAGE) {
        result.ret = make_ret(Err::NoError);
        result.val.code = DEFAULT_LANGUAGE;
        return result;
    }

    LanguagesHash languageHash = this->languages().val;
    if (!languageHash.contains(languageCode)) {
        result.ret = make_ret(Err::ErrorLanguageNotFound);
        return result;
    }

    result.ret = make_ret(Err::NoError);
    result.val = languageHash[languageCode];
    return result;
}

Ret LanguagesController::setCurrentLanguage(const QString& languageCode)
{
    if (languageCode == DEFAULT_LANGUAGE) {
        resetLanguageByDefault();
        return make_ret(Err::NoError);
    }

    LanguagesHash languageHash = this->languages().val;
    if (!languageHash.contains(languageCode)) {
        return make_ret(Err::ErrorLanguageNotFound);
    }

    for (QTranslator* t: m_translatorList) {
        qApp->removeTranslator(t);
        delete t;
    }
    m_translatorList.clear();

    Ret load = loadLanguage(languageCode);
    if (!load) {
        return load;
    }

    QString previousLanguage = configuration()->currentLanguageCode();

    Ret save = configuration()->setCurrentLanguageCode(languageCode);
    if (!save) {
        return save;
    }

    languageHash[previousLanguage].isCurrent = false;
    m_languageChanged.send(languageHash[previousLanguage]);

    languageHash[languageCode].isCurrent = true;
    m_languageChanged.send(languageHash[languageCode]);

    return save;
}

RetCh<Language> LanguagesController::languageChanged()
{
    RetCh<Language> result;
    result.ret = make_ret(Err::NoError);
    result.ch = m_languageChanged;
    return result;
}

RetVal<LanguagesHash> LanguagesController::parseLanguagesConfig(const QByteArray& json) const
{
    RetVal<LanguagesHash> result;

    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
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
        language.status = LanguageStatus::Status::Undefined;

        result.val.insert(key, language);
    }

    return result;
}

LanguageFiles LanguagesController::parseLanguageFiles(const QJsonObject& languageObject) const
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

bool LanguagesController::isLanguageExists(const QString& languageCode) const
{
    io::paths files = configuration()->languageFilePaths(languageCode);
    return !files.empty();
}

bool LanguagesController::checkLanguageFilesHash(const QString& languageCode, const LanguageFiles& languageFiles) const
{
    io::paths filePaths = configuration()->languageFilePaths(languageCode);
    int filesSize = static_cast<int>(filePaths.size());
    if (filesSize != languageFiles.size()) {
        return false;
    }

    QHash<QString, QString> filesHash;
    for (const LanguageFile& file: languageFiles) {
        filesHash.insert(file.name, file.hash);
    }

    QCryptographicHash localHash(QCryptographicHash::Sha1);
    for (const io::path& filePath: filePaths) {
        QString fileName = io::filename(filePath).toQString();
        if (!filesHash.contains(fileName)) {
            continue;
        }

        RetVal<QByteArray> fileBytes = fileSystem()->readFile(filePath);
        if (!fileBytes.ret) {
            LOGW() << fileBytes.ret.toString();
            return false;
        }

        localHash.reset();
        localHash.addData(fileBytes.val);
        QString fileHash = QString(localHash.result().toHex());
        QString actualHash = filesHash[fileName];
        if (actualHash != fileHash) {
            return false;
        }
    }

    return true;
}

RetVal<LanguagesHash> LanguagesController::correctLanguagesStates(LanguagesHash& languages) const
{
    RetVal<LanguagesHash> result;
    bool isNeedUpdate = false;

    QString currentLanguage = configuration()->currentLanguageCode();

    for (Language& language: languages) {
        LanguageStatus::Status status = languageStatus(language);
        if (status != language.status) {
            language.status = status;
            isNeedUpdate = true;
        }

        language.isCurrent = (language.code == currentLanguage);
    }

    if (isNeedUpdate) {
        Ret update = configuration()->setLanguages(languages);
        if (!update) {
            result.ret = update;
            return result;
        }
    }

    result.ret = make_ret(Err::NoError);
    result.val = languages;
    return result;
}

LanguageStatus::Status LanguagesController::languageStatus(const Language& language) const
{
    if (!isLanguageExists(language.code)) {
        return LanguageStatus::Status::NoInstalled;
    }

    if (!checkLanguageFilesHash(language.code, language.files)) {
        return LanguageStatus::Status::NeedUpdate;
    }

    return LanguageStatus::Status::Installed;
}

RetVal<QString> LanguagesController::downloadLanguage(const QString& languageCode, async::Channel<LanguageProgress>* progressChannel) const
{
    RetVal<QString> result;

    QString languageArchivePath = configuration()->languageArchivePath(languageCode).toQString();

    QBuffer buff;
    INetworkManagerPtr networkManagerPtr = networkManagerCreator()->makeNetworkManager();

    async::Channel<Progress> downloadChannel = networkManagerPtr->progressChannel();
    downloadChannel.onReceive(new async::Asyncable(), [&progressChannel](const Progress& progress) {
        progressChannel->send(LanguageProgress(DOWNLOADING_STATUS, progress.current, progress.total));
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

Ret LanguagesController::removeLanguage(const QString& languageCode) const
{
    io::paths files = configuration()->languageFilePaths(languageCode);

    for (const io::path& filePath: files) {
        Ret ret = fileSystem()->remove(filePath);
        if (!ret) {
            LOGE() << "Error remove file" << filePath << ret.toString();
            return make_ret(Err::ErrorRemoveLanguageDirectory);
        }
    }

    return make_ret(Err::NoError);
}

Ret LanguagesController::loadLanguage(const QString& languageCode)
{
    io::paths files = configuration()->languageFilePaths(languageCode);

    for (const io::path& filePath: files) {
        QTranslator* translator = new QTranslator;
        bool ok = translator->load(filePath.toQString());
        if (ok) {
            qApp->installTranslator(translator);
            m_translatorList.append(translator);
        } else {
            LOGE() << "Error load translate" << filePath;
            delete translator;
        }
    }

    QLocale locale(languageCode);
    QLocale::setDefault(locale);
    qApp->setLayoutDirection(locale.textDirection());

    return make_ret(Err::NoError);
}

void LanguagesController::resetLanguageByDefault()
{
    Ret load = loadLanguage(DEFAULT_LANGUAGE);
    if (!load) {
        return;
    }

    configuration()->setCurrentLanguageCode(DEFAULT_LANGUAGE);
}

void LanguagesController::th_refreshLanguages()
{
    QBuffer buff;
    INetworkManagerPtr networkManagerPtr = networkManagerCreator()->makeNetworkManager();

    Ret getLanguagessInfo = networkManagerPtr->get(configuration()->languagesUpdateUrl().toString(), &buff);

    if (!getLanguagessInfo) {
        LOGE() << getLanguagessInfo.toString();
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

            savedLanguage.status = languageStatus(savedLanguage);
        } else {
            language.status = LanguageStatus::Status::NoInstalled;
            resultLanguages.insert(language.code, language);
        }
    }

    configuration()->setLanguages(resultLanguages);
}

void LanguagesController::th_install(const QString& languageCode, async::Channel<LanguageProgress>* progressChannel,
                                     async::Channel<Ret>* finishChannel)
{
    progressChannel->send(LanguageProgress(ANALYSING_STATUS, true));

    RetVal<QString> download = downloadLanguage(languageCode, progressChannel);
    if (!download.ret) {
        finishChannel->send(download.ret);
        return;
    }

    progressChannel->send(LanguageProgress(ANALYSING_STATUS, true));

    QString languageArchivePath = download.val;

    Ret unpack = languageUnpacker()->unpack(languageCode, languageArchivePath, configuration()->languagesSharePath().toQString());
    if (!unpack) {
        LOGE() << unpack.toString();
        fileSystem()->remove(languageArchivePath);

        finishChannel->send(unpack);
        return;
    }

    fileSystem()->remove(languageArchivePath);

    finishChannel->send(make_ret(Err::NoError));
}

void LanguagesController::th_update(const QString& languageCode, async::Channel<LanguageProgress>* progressChannel,
                                    async::Channel<Ret>* finishChannel)
{
    progressChannel->send(LanguageProgress(ANALYSING_STATUS, true));

    RetVal<QString> download = downloadLanguage(languageCode, progressChannel);
    if (!download.ret) {
        finishChannel->send(download.ret);
    }

    progressChannel->send(LanguageProgress(ANALYSING_STATUS, true));

    QString languageArchivePath = download.val;

    Ret remove = removeLanguage(languageCode);
    if (!remove) {
        finishChannel->send(remove);
    }

    Ret unpack = languageUnpacker()->unpack(languageCode, languageArchivePath, configuration()->languagesSharePath().toQString());
    if (!unpack) {
        LOGE() << "Error unpack" << unpack.toString();
        finishChannel->send(unpack);
    }

    fileSystem()->remove(languageArchivePath);

    finishChannel->send(make_ret(Err::NoError));
}

void LanguagesController::closeOperation(const QString& languageCode, async::Channel<LanguageProgress>* progressChannel)
{
    progressChannel->close();
    m_operationsHash.remove(languageCode);
}
