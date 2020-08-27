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
#include <QCoreApplication>
#include <QtConcurrent>

#include "log.h"
#include "languageserrors.h"

using namespace mu;
using namespace mu::languages;
using namespace mu::framework;

static const QString DEFAULT_LANGUAGE("system");
static const QString ANALYSING_STATUS = qtrc("languages", "Analysing...");
static const QString DOWNLOADING_STATUS = qtrc("languages", "Downloading...");

void LanguagesController::init()
{
    fileSystem()->makePath(configuration()->languagesSharePath());
    fileSystem()->makePath(configuration()->languagesDataPath());

    QString code = configuration()->currentLanguageCode();
    loadLanguage(code);

    refreshLanguages();
}

Ret LanguagesController::refreshLanguages()
{
    QBuffer buff;
    INetworkManagerPtr networkManagerPtr = networkManagerCreator()->makeNetworkManager();

    Ret getLanguagessInfo = networkManagerPtr->get(configuration()->languagesUpdateUrl().toString(), &buff);

    if (!getLanguagessInfo) {
        LOGE() << "Error get languages" << getLanguagessInfo.code() << getLanguagessInfo.text();
        return getLanguagessInfo;
    }

    QByteArray json = buff.data();
    RetVal<LanguagesHash> actualLanguages = parseLanguagesConfig(json);

    if (!actualLanguages.ret) {
        return actualLanguages.ret;
    }

    LanguagesHash savedLanguages = configuration()->languages().val;

    LanguagesHash resultLanguages = savedLanguages;

    for (Language& language : actualLanguages.val) {
        if (resultLanguages.contains(language.code)) {
            Language& savedLanguage = resultLanguages[language.code];

            if (!isLanguageExists(language.code)) {
                savedLanguage.status = LanguageStatus::Status::NoInstalled;
                continue;
            }

            savedLanguage.status = LanguageStatus::Status::Installed;
        } else {
            language.status = LanguageStatus::Status::NoInstalled;
            resultLanguages.insert(language.code, language);
        }
    }

    return configuration()->setLanguages(resultLanguages);
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
    result.ret = make_ret(Err::NoError);
    result.ch = m_languageProgressStatus;

    m_languageFinishCh.onReceive(this, [this, languageCode](const Ret& ret) {
        if (!ret) {
            return;
        }

        LanguagesHash languageHash = this->languages().val;

        languageHash[languageCode].status = LanguageStatus::Status::Installed;

        Ret updateConfigRet = configuration()->setLanguages(languageHash);
        if (!updateConfigRet) {
            LOGW() << updateConfigRet.code() << updateConfigRet.text();
            m_languageProgressStatus.close();
            return;
        }

        m_languageChanged.send(languageHash[languageCode]);

        m_languageProgressStatus.close();
    });

    QtConcurrent::run(this, &LanguagesController::th_install, languageCode, m_languageProgressStatus, m_languageFinishCh);

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
        language.fileName = value.value("file_name").toString();
        language.fileSize = value.value("file_size").toDouble();
        language.status = LanguageStatus::Status::Undefined;

        result.val.insert(key, language);
    }

    return result;
}

bool LanguagesController::isLanguageExists(const QString& languageCode) const
{
    io::paths files = configuration()->languageFilePaths(languageCode);
    return !files.empty();
}

RetVal<LanguagesHash> LanguagesController::correctLanguagesStates(LanguagesHash& languages) const
{
    RetVal<LanguagesHash> result;
    bool isNeedUpdate = false;

    QString currentLanguage = configuration()->currentLanguageCode();

    for (Language& language: languages) {
        if (language.status == LanguageStatus::Status::Installed && !isLanguageExists(language.code)) {
            language.status = LanguageStatus::Status::NoInstalled;
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

RetVal<QString> LanguagesController::downloadLanguage(const QString& languageCode, async::Channel<LanguageProgress>& progressChannel) const
{
    RetVal<QString> result;

    QString languageArchivePath = configuration()->languageArchivePath(languageCode).toQString();

    QBuffer buff;
    INetworkManagerPtr networkManagerPtr = networkManagerCreator()->makeNetworkManager();

    async::Channel<Progress> downloadChannel = networkManagerPtr->downloadProgressChannel();
    downloadChannel.onReceive(new deto::async::Asyncable(), [&progressChannel](const Progress& progress) {
        progressChannel.send(LanguageProgress(DOWNLOADING_STATUS, progress.current, progress.total));
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
            LOGE() << "Error remove file" << filePath << ret.code() << ret.text();
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

void LanguagesController::th_install(const QString& languageCode, async::Channel<LanguageProgress> progressChannel,
                                     async::Channel<Ret> finishChannel)
{
    progressChannel.send(LanguageProgress(ANALYSING_STATUS, true));

    RetVal<QString> download = downloadLanguage(languageCode, progressChannel);
    if (!download.ret) {
        finishChannel.send(download.ret);
        return;
    }

    progressChannel.send(LanguageProgress(ANALYSING_STATUS, true));

    QString languageArchivePath = download.val;

    Ret unpack = languageUnpacker()->unpack(languageCode, languageArchivePath, configuration()->languagesSharePath().toQString());
    if (!unpack) {
        LOGE() << "Error unpack" << unpack.code();
        finishChannel.send(unpack);
        return;
    }

    QFile languageArchive(languageArchivePath);
    languageArchive.remove();

    finishChannel.send(make_ret(Err::NoError));
}
