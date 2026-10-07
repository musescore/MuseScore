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

#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantList>

#include "global/types/string.h"
#include "global/io/path.h"
#include "modularity/ioc.h"

#include "languages/ilanguagesconfiguration.h"
#include "languages/ilanguagesservice.h"
#include "notation/iinstrumentsrepository.h"

class QTranslator;

namespace mu::engraving {
class InstrumentTemplate;
struct Trait;
}

namespace mu::project {
//! Gives the names of instruments (long name, short name, transposition and track name) in a given language,
//! as if it were the language of the interface, so that instruments can be added to a score in the language
//! of the score. Names of instruments that are already in a score are never changed.
//!
//! It uses the translation files that are shipped with (or downloaded for) every language, and that are
//! normally only used for the language of the interface.
class InstrumentNamesTranslator
{
    muse::GlobalInject<muse::languages::ILanguagesConfiguration> languagesConfiguration;
    muse::GlobalInject<muse::languages::ILanguagesService> languagesService;
    muse::GlobalInject<notation::IInstrumentsRepository> instrumentsRepository;

public:
    using Translators = std::vector<std::unique_ptr<QTranslator> >;

    //! Creates a translator without translations: load() or setTranslators() must be called before it translates
    InstrumentNamesTranslator();
    ~InstrumentNamesTranslator();

    //! Languages that instrument names are available in on this machine, sorted by name
    QList<muse::languages::Language> availableLanguages() const;

    //! The code of the language of the interface
    QString interfaceLanguageCode() const;

    //! The choices of the dropdown of languages, as a list of { code, name }: "Same as interface" (with an empty
    //! code), then the recently used languages, then the others. The language of the interface is left out,
    //! unless \a withInterfaceLanguage (e.g. for an existing score, whose language may be that one).
    QVariantList languageChoices(const QStringList& recentLanguageCodes, bool withInterfaceLanguage = false) const;

    //! Loads the translations for the language. Returns false if they are not available.
    bool load(const QString& languageCode);

    //! Uses the given translations instead of loading them, e.g. for tests
    void setTranslators(Translators instrumentsTranslators, Translators interfaceTranslators);

    //! Gives the names of the template in the language, as InstrumentTemplate::read() gives them
    //! in the language of the interface
    void translate(engraving::InstrumentTemplate& templ) const;

    //! An example of the names of an instrument as it would be added, e.g. "Clarinette in Si♭"
    muse::String exampleName(engraving::InstrumentTemplate templ) const;

    //! Translates any string of the program, e.g. of the palettes. Returns \a source if there is no translation.
    muse::String translate(const char* context, const muse::String& source, const muse::String& disambiguation) const;

    //! How many recently used languages are remembered
    static constexpr int MAX_RECENT_LANGUAGES = 4;

    //! Puts the recently used languages first, and leaves out the language of the interface
    static QList<muse::languages::Language> orderLanguages(const QList<muse::languages::Language>& languages,
                                                           const QString& interfaceLanguageCode, const QStringList& recentLanguageCodes);

    //! Adds a language to the front of the recently used languages
    static QStringList addRecentLanguage(const QStringList& recentLanguageCodes, const QString& languageCode);

    //! The name of the language to show, e.g. "Deutsch — German"
    static QString displayName(const muse::languages::Language& language);

private:
    //! The translation file of the resource (e.g. "instruments") for the language, or an empty path if there is none.
    //! A downloaded update is used if it is newer than the installed file, like for the language of the interface.
    muse::io::path_t translationFile(const QString& resourceName, const QString& languageCode) const;

    //! Loads the translation files of the resource for the languages, in that order (e.g. a language and its fallbacks)
    Translators loadTranslators(const QString& resourceName, const QStringList& languageCodes) const;

    //! The first translation of \a source by the translators, or \a source itself if none of them translates it
    static muse::String translate(const Translators& translators, const char* context, const muse::String& source,
                                  const muse::String& disambiguation);

    //! The translation of a name of an instrument (e.g. its "longName"), as InstrumentTemplate::read() translates it
    muse::String translateInstrumentName(const muse::String& instrumentId, const muse::String& nameType, const muse::String& source) const;

    //! Sets the name of \a trait from its translation, and its markers (default, hidden) from the untranslated name
    void applyTranslatedTraitName(const engraving::InstrumentTemplate& templ, engraving::Trait& trait) const;

    bool m_loaded = false;
    Translators m_instrumentsTranslators;
    Translators m_interfaceTranslators;
};
}
