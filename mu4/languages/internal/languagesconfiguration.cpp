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

#include "log.h"
#include "settings.h"
#include "languagestypes.h"
#include "languageserrors.h"

using namespace mu;
using namespace mu::framework;
using namespace mu::languages;

static std::string module_name("languages");
static const Settings::Key LANGUAGES_JSON(module_name, "languages/languagesJson");
static const Settings::Key LANGUAGE("ui", "ui/application/language");

void LanguagesConfiguration::init()
{
    settings()->addItem(LANGUAGE, Val("system"));
    settings()->valueChanged(LANGUAGES_JSON).onReceive(nullptr, [this](const Val& val) {
        LanguagesHash languagesHash = parseLanguagesConfig(io::pathToQString(val.toString()).toLocal8Bit());
        m_languagesHashChanged.send(languagesHash);
    });
}

QString LanguagesConfiguration::currentLanguageCode() const
{
    return io::pathToQString(settings()->value(LANGUAGE).toString());
}

Ret LanguagesConfiguration::setCurrentLanguageCode(const QString& languageCode) const
{
    Val value(languageCode.toStdString());
    settings()->setValue(LANGUAGE, value);
    return make_ret(Err::NoError);
}

QUrl LanguagesConfiguration::languagesUpdateUrl() const
{
    return QUrl("http://extensions.musescore.org/4.0/languages/details.json");
}

QUrl LanguagesConfiguration::languagesFileServerUrl() const
{
    return QUrl("http://extensions.musescore.org/4.0/languages/");
}

ValCh<LanguagesHash> LanguagesConfiguration::languages() const
{
    ValCh<LanguagesHash> result;
    result.val = parseLanguagesConfig(io::pathToQString(settings()->value(LANGUAGES_JSON).toString()).toLocal8Bit());
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
        language.fileName = lngMap.value("fileName").toString();
        language.fileSize = lngMap.value("fileSize").toDouble();
        language.status = static_cast<LanguageStatus::Status>(lngMap.value("status").toInt());

        result.insert(language.code, language);
    }

    return result;
}

QString LanguagesConfiguration::languagesSharePath() const
{
    return io::pathToQString(globalConfiguration()->sharePath() + "/locale");
}

QString LanguagesConfiguration::languagesDataPath() const
{
    return io::pathToQString(globalConfiguration()->dataPath() + "/locale");
}
