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

#include "log.h"
#include "mscore/downloadUtils.h"
#include "languageserrors.h"

using namespace mu;
using namespace mu::languages;

static const QString DEFAULT_LANGUAGE("system");

void LanguagesController::init()
{
    QString code = configuration()->currentLanguageCode();
    LOGD() << "==========" << code;
    loadLanguage(code);
}

Ret LanguagesController::refreshLanguages()
{
    Ms::DownloadUtils* js = new Ms::DownloadUtils();
    js->setTarget(configuration()->languagesUpdateUrl().toString());
    js->download();

    QByteArray json = js->returnData();
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

    Ret ret = configuration()->setLanguages(resultLanguages);
    return ret;
}

ValCh<LanguagesHash> LanguagesController::languages()
{
    ValCh<LanguagesHash> languagesHash = configuration()->languages();
    languagesHash.val = correctLanguagesStates(languagesHash.val).val;

    return languagesHash;
}

Ret LanguagesController::install(const QString& languageCode)
{
    RetVal<QString> download = downloadLanguage(languageCode);
    if (!download.ret) {
        return download.ret;
    }

    QString languageArchivePath = download.val;

    QDir languagesShareDir(configuration()->languagesSharePath());
    if (!languagesShareDir.exists()) {
        languagesShareDir.mkpath(languagesShareDir.absolutePath());
    }

    Ret unpack = languageUnpacker()->unpack(languageCode, languageArchivePath, languagesShareDir.absolutePath());
    if (!unpack) {
        LOGE() << "Error unpack" << unpack.code();
        return unpack;
    }

    QFile languageArchive(languageArchivePath);
    languageArchive.remove();

    LanguagesHash languageHash = this->languages().val;

    languageHash[languageCode].status = LanguageStatus::Status::Installed;

    Ret ret = configuration()->setLanguages(languageHash);
    if (!ret) {
        return ret;
    }

    m_languageChanged.send(languageHash[languageCode]);

    return make_ret(Err::NoError);
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

Ret LanguagesController::setLanguage(const QString& languageCode)
{
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
    QJsonDocument jsodDoc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !jsodDoc.isObject()) {
        result.ret = make_ret(Err::ErrorParseConfig);
        return result;
    }

    result.ret = make_ret(Err::NoError);
    QStringList languages = jsodDoc.object().keys();
    for (const QString& key : languages) {
        if (!jsodDoc.object().value(key).isObject()) {
            continue;
        }

        QJsonObject value = jsodDoc.object().value(key).toObject();

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
    QDir languagesDir(configuration()->languagesSharePath());
    QStringList files = languagesDir.entryList({ QString("*%1.qm").arg(languageCode) }, QDir::Files);
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

RetVal<QString> LanguagesController::downloadLanguage(const QString& languageCode) const
{
    RetVal<QString> result;

    ValCh<LanguagesHash> languages = configuration()->languages();
    QString fileName = languages.val.value(languageCode).fileName;

    QDir languagesDir(configuration()->languagesDataPath());
    if (!languagesDir.exists()) {
        languagesDir.mkpath(languagesDir.absolutePath());
    }

    QString languageArchivePath = languagesDir.absolutePath() + "/" + fileName;

    Ms::DownloadUtils* js = new Ms::DownloadUtils();
    js->setTarget(configuration()->languagesFileServerUrl().toString() + fileName);
    js->setLocalFile(languageArchivePath);
    js->download(true);

    if (!js->saveFile()) {
        LOGE() << "Error save file";
        result.ret = make_ret(Err::ErrorDownloadLanguage);
        return result;
    }

    result.ret = make_ret(Err::NoError);
    result.val = languageArchivePath;
    return result;
}

Ret LanguagesController::removeLanguage(const QString& languageCode) const
{
    QDir languageDir(configuration()->languagesSharePath());
    QStringList files = languageDir.entryList({ QString("*%1.qm").arg(languageCode) }, QDir::Files);
    for (const QString& fileName: files) {
        QString filePath(languageDir.absolutePath() + "/" + fileName);
        QFile file(filePath);
        if (!file.remove()) {
            LOGE() << "Error remove file" << filePath << file.errorString();
            return make_ret(Err::ErrorRemoveLanguageDirectory);
        }
    }

    return make_ret(Err::NoError);
}

Ret LanguagesController::loadLanguage(const QString &languageCode)
{
    QDir languageDir(configuration()->languagesSharePath());
    QStringList files = languageDir.entryList({ QString("*%1.qm").arg(languageCode) }, QDir::Files);

    for (const QString& fileName: files) {
        QFileInfo file(fileName);
        QString filePath(languageDir.absolutePath() + "/" + file.baseName());

        QTranslator* translator = new QTranslator;
        bool ok = translator->load(filePath);
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
