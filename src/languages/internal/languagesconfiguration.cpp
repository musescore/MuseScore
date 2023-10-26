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

#include <QUrl>

#include "log.h"
#include "settings.h"
#include "languagestypes.h"
#include "languageserrors.h"

using namespace mu;
using namespace mu::framework;
using namespace mu::languages;

static const Settings::Key LANGUAGE_KEY("languages", "language");

static const QString LANGUAGES_SERVER_URL("http://extensions.musescore.org/4.2/languages/");

void LanguagesConfiguration::init()
{
    settings()->setDefaultValue(LANGUAGE_KEY, Val(SYSTEM_LANGUAGE_CODE.toStdString()));
    settings()->valueChanged(LANGUAGE_KEY).onReceive(nullptr, [this](const Val& val) {
        m_currentLanguageCodeChanged.send(val.toQString());
    });
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
    return QUrl(LANGUAGES_SERVER_URL + QString("locale_%1.zip").arg(languageCode));
}

io::path_t LanguagesConfiguration::languagesAppDataPath() const
{
    return globalConfiguration()->appDataPath() + "/locale";
}

io::path_t LanguagesConfiguration::languagesUserAppDataPath() const
{
    return globalConfiguration()->userAppDataPath() + "/locale";
}

io::path_t LanguagesConfiguration::builtinLanguagesJsonPath() const
{
    return languagesAppDataPath() + "/languages.json";
}

io::path_t LanguagesConfiguration::builtinLanguageFilePath(const QString& resourceName, const QString& languageCode) const
{
    return languagesAppDataPath() + QString("/%1_%2.qm").arg(resourceName, languageCode);
}

io::path_t LanguagesConfiguration::userLanguageFilePath(const QString& resourceName, const QString& languageCode) const
{
    return languagesUserAppDataPath() + QString("/%1_%2.qm").arg(resourceName, languageCode);
}
