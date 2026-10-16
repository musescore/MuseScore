/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "instrumentnamestranslator.h"

#include <algorithm>
#include <cstring>
#include <optional>
#include <utility>

#include <QFileInfo>
#include <QLocale>
#include <QTranslator>
#include <QVariant>

#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/instrument.h"

#include "translation.h"

using namespace mu::project;
using namespace mu::engraving;
using namespace muse;
using namespace muse::languages;

//! The language of the strings in instruments.xml and in the built-in templates. It needs no translation files.
static const QString SOURCE_LANGUAGE_CODE = QStringLiteral("en_US");

//! Translation files of the instrument names, and of the rest of the program (which contain the word "in")
static const QString INSTRUMENTS_RESOURCE_NAME = QStringLiteral("instruments");
static const QString INTERFACE_RESOURCE_NAME = QStringLiteral("musescore");

//! The context of the instrument names, see translateInstrumentName() in instrtemplate.cpp
static const char* INSTRUMENTS_CONTEXT = "engraving/instruments";

InstrumentNamesTranslator::InstrumentNamesTranslator() = default;
InstrumentNamesTranslator::~InstrumentNamesTranslator() = default;

QList<Language> InstrumentNamesTranslator::availableLanguages() const
{
    // The translation files do not change while the program runs, so they only need to be looked for once
    static std::optional<QList<Language> > cache;
    if (cache) {
        return *cache;
    }

    QList<Language> result;

    for (const Language& language : languagesService()->languages()) {
        if (language.code == SOURCE_LANGUAGE_CODE || !translationFile(INSTRUMENTS_RESOURCE_NAME, language.code).empty()) {
            result << language;
        }
    }

    std::sort(result.begin(), result.end(), [](const Language& l, const Language& r) {
        return QString::localeAwareCompare(l.name, r.name) < 0;
    });

    if (!result.empty()) {
        cache = result;
    }

    return result;
}

QString InstrumentNamesTranslator::interfaceLanguageCode() const
{
    return languagesService()->currentLanguage().code;
}

QList<Language> InstrumentNamesTranslator::orderLanguages(const QList<Language>& languages, const QString& interfaceLanguageCode,
                                                          const QStringList& recentLanguageCodes)
{
    QList<Language> result;

    // Recently used languages first, in the order they were used
    for (const QString& code : recentLanguageCodes) {
        if (code == interfaceLanguageCode) {
            continue;
        }

        for (const Language& language : languages) {
            if (language.code == code) {
                result << language;
                break;
            }
        }
    }

    // Then the others, in their original order. The language of the interface is not listed:
    // choosing it would be the same as "Same as interface".
    for (const Language& language : languages) {
        if (language.code != interfaceLanguageCode && !recentLanguageCodes.contains(language.code)) {
            result << language;
        }
    }

    return result;
}

QStringList InstrumentNamesTranslator::addRecentLanguage(const QStringList& recentLanguageCodes, const QString& languageCode)
{
    if (languageCode.isEmpty()) {
        return recentLanguageCodes;
    }

    QStringList result = recentLanguageCodes;
    result.removeAll(languageCode);
    result.prepend(languageCode);

    while (result.size() > MAX_RECENT_LANGUAGES) {
        result.removeLast();
    }

    return result;
}

QString InstrumentNamesTranslator::displayName(const Language& language)
{
    // The names of the languages are in the languages themselves, e.g. "Deutsch".
    // Add the English name, which is the only other one available, e.g. "Deutsch — German".
    const QLocale locale(language.code);
    if (locale.language() == QLocale::C || locale.language() == QLocale::AnyLanguage) {
        return language.name;
    }

    QString englishName = QLocale::languageToString(locale.language());
    if (language.code.contains(u'_') && locale.territory() != QLocale::AnyTerritory) {
        englishName += QStringLiteral(" (") + QLocale::territoryToString(locale.territory()) + u')';
    }

    if (englishName.compare(language.name, Qt::CaseInsensitive) == 0) {
        return language.name;
    }

    return language.name + QStringLiteral(" — ") + englishName;
}

static QVariant choice(const QString& code, const QString& name)
{
    QVariantMap obj;
    obj["code"] = code;
    obj["name"] = name;
    return obj;
}

QVariantList InstrumentNamesTranslator::languageChoices(const QStringList& recentLanguageCodes, bool withInterfaceLanguage) const
{
    QVariantList result;
    result << choice(QString(), muse::qtrc("project/newscore", "Same as interface"));

    const QString leftOut = withInterfaceLanguage ? QString() : interfaceLanguageCode();
    for (const Language& language : orderLanguages(availableLanguages(), leftOut, recentLanguageCodes)) {
        result << choice(language.code, displayName(language));
    }

    return result;
}

void InstrumentNamesTranslator::setTranslators(Translators instrumentsTranslators, Translators interfaceTranslators)
{
    m_instrumentsTranslators = std::move(instrumentsTranslators);
    m_interfaceTranslators = std::move(interfaceTranslators);
    m_loaded = true;
}

bool InstrumentNamesTranslator::load(const QString& languageCode)
{
    m_loaded = false;
    m_instrumentsTranslators.clear();
    m_interfaceTranslators.clear();

    const Language language = languagesService()->language(languageCode);
    if (language.code.isEmpty()) {
        return false;
    }

    // Like for the interface language, the language itself is searched first, then its fallback languages
    QStringList codes { language.code };
    codes << language.fallbackLanguages;

    m_instrumentsTranslators = loadTranslators(INSTRUMENTS_RESOURCE_NAME, codes);
    m_interfaceTranslators = loadTranslators(INTERFACE_RESOURCE_NAME, codes);

    // The names are in English if the instrument names are not translated into the language; that is also
    // the case for the language of the interface, which is then still used for the other strings
    m_loaded = !m_instrumentsTranslators.empty() || language.code == SOURCE_LANGUAGE_CODE
               || language.code == interfaceLanguageCode();
    return m_loaded;
}

io::path_t InstrumentNamesTranslator::translationFile(const QString& resourceName, const QString& languageCode) const
{
    const io::path_t builtinPath = languagesConfiguration()->builtinLanguageFilePath(resourceName, languageCode);
    const io::path_t userPath = languagesConfiguration()->userLanguageFilePath(resourceName, languageCode);

    const QFileInfo builtinFile(builtinPath.toQString());
    const QFileInfo userFile(userPath.toQString());

    // A downloaded update is only used if it is newer than the installed file,
    // like for the interface language (see LanguagesService::doLoadLanguage())
    if (userFile.exists() && (!builtinFile.exists() || userFile.lastModified() > builtinFile.lastModified())) {
        return userPath;
    }

    if (builtinFile.exists()) {
        return builtinPath;
    }

    return io::path_t();
}

InstrumentNamesTranslator::Translators InstrumentNamesTranslator::loadTranslators(const QString& resourceName,
                                                                                  const QStringList& languageCodes) const
{
    Translators result;

    for (const QString& code : languageCodes) {
        const io::path_t file = translationFile(resourceName, code);
        if (file.empty()) {
            continue;
        }

        auto translator = std::make_unique<QTranslator>();
        if (translator->load(file.toQString())) {
            result.push_back(std::move(translator));
        }
    }

    return result;
}

String InstrumentNamesTranslator::translate(const Translators& translators, const char* context, const String& source,
                                            const String& disambiguation)
{
    if (source.empty()) {
        return source;
    }

    const QByteArray sourceUtf8 = source.toQString().toUtf8();
    const QByteArray disambiguationUtf8 = disambiguation.toQString().toUtf8();

    for (const std::unique_ptr<QTranslator>& translator : translators) {
        const QString translation = translator->translate(context, sourceUtf8.constData(),
                                                          disambiguationUtf8.isEmpty() ? nullptr : disambiguationUtf8.constData());
        if (!translation.isEmpty()) {
            return String::fromQString(translation);
        }
    }

    // Not translated: keep the source string, which is in English
    return source;
}

String InstrumentNamesTranslator::translate(const char* context, const String& source, const String& disambiguation) const
{
    if (!m_loaded) {
        return source;
    }

    const bool isInstrumentName = std::strcmp(context, INSTRUMENTS_CONTEXT) == 0;
    return translate(isInstrumentName ? m_instrumentsTranslators : m_interfaceTranslators, context, source, disambiguation);
}


void InstrumentNamesTranslator::applyTranslatedTraitName(const InstrumentTemplate& templ, Trait& trait) const
{
    // The markers (default, hidden) are taken from the untranslated name
    applyTraitName(trait, templ.nameSources.traitName, translateInstrumentName(templ.id, u"traitName", templ.nameSources.traitName));
}

String InstrumentNamesTranslator::translateInstrumentName(const String& instrumentId, const String& nameType,
                                                          const String& source) const
{
    return translate(m_instrumentsTranslators, INSTRUMENTS_CONTEXT, source, instrumentId + u' ' + nameType);
}

void InstrumentNamesTranslator::translate(InstrumentTemplate& templ) const
{
    if (!m_loaded) {
        return;
    }

    // The same names as InstrumentTemplate::read() gives in the language of the interface
    const InstrumentTemplate::NameSources& sources = templ.nameSources;

    if (!sources.longName.empty()) {
        templ.instrumentName.setLongName(translateInstrumentName(templ.id, u"longName", sources.longName));
    }

    if (!sources.shortName.empty()) {
        templ.instrumentName.setShortName(translateInstrumentName(templ.id, u"shortName", sources.shortName));
    }

    if (!sources.traitName.empty()) {
        applyTranslatedTraitName(templ, templ.trait);
    }

    if (!sources.trackName.empty()) {
        templ.trackName = translateInstrumentName(templ.id, u"trackName", sources.trackName);
    }
}

String InstrumentNamesTranslator::exampleName(InstrumentTemplate templ) const
{
    translate(templ);

    String result = templ.instrumentName.longName();
    if (templ.trait.type == TraitType::Transposition && !templ.trait.isHiddenOnScore && !templ.trait.name.empty()) {
        // As the staff label is drawn: with the word "in" of the interface language
        result += u' ' + muse::mtrc("notation", "in") + u' ' + templ.trait.name;
    }

    return result;
}
