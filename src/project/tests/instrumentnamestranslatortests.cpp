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

#include <gtest/gtest.h>

#include <map>
#include <memory>
#include <utility>

#include <QString>
#include <QStringList>
#include <QTranslator>

#include "project/internal/instrumentnamestranslator.h"

#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/instrument.h"

using namespace Qt::StringLiterals;
using namespace mu::project;
using namespace muse;
using namespace muse::languages;

namespace {
Language makeLanguage(const QString& code, const QString& name)
{
    Language language;
    language.code = code;
    language.name = name;
    return language;
}

//! A translator with fixed translations, keyed by source text and disambiguation
class FakeTranslator : public QTranslator
{
public:
    using Translations = std::map<std::pair<QString, QString>, QString>;

    explicit FakeTranslator(Translations translations)
        : m_translations(std::move(translations)) {}

    bool isEmpty() const override { return m_translations.empty(); }

    QString translate(const char*, const char* sourceText, const char* disambiguation, int) const override
    {
        const auto it = m_translations.find({ QString::fromUtf8(sourceText),
                                              disambiguation ? QString::fromUtf8(disambiguation) : QString() });
        return it == m_translations.end() ? QString() : it->second;
    }

private:
    Translations m_translations;
};

InstrumentNamesTranslator::Translators makeTranslators(FakeTranslator::Translations translations)
{
    InstrumentNamesTranslator::Translators result;
    result.push_back(std::make_unique<FakeTranslator>(std::move(translations)));
    return result;
}

//! The template of the B♭ clarinet, as read from instruments.xml (in English)
mu::engraving::InstrumentTemplate makeClarinetTemplate()
{
    mu::engraving::InstrumentTemplate templ;
    templ.id = u"bb-clarinet";
    templ.nameSources.longName = u"Clarinet";
    templ.nameSources.shortName = u"Cl.";
    templ.nameSources.traitName = u"*B♭";
    templ.nameSources.trackName = u"Clarinet";
    templ.instrumentName.setLongName(u"Clarinet");
    templ.instrumentName.setShortName(u"Cl.");
    templ.trait.type = mu::engraving::TraitType::Transposition;
    mu::engraving::applyTraitName(templ.trait, templ.nameSources.traitName);
    return templ;
}

//! French translations, like the real ones: the translation of the key has lost its marker ('*')
void setFrenchTranslations(InstrumentNamesTranslator& translator)
{
    translator.setTranslators(makeTranslators({
        { { u"Clarinet"_s, u"bb-clarinet longName"_s }, u"Clarinette"_s },
        { { u"Cl."_s, u"bb-clarinet shortName"_s }, u"Cl."_s },
        { { u"*B♭"_s, u"bb-clarinet traitName"_s }, u"Si♭"_s },
        { { u"Clarinet"_s, u"bb-clarinet trackName"_s }, u"Clarinette"_s },
    }), {});
}

QStringList codes(const QList<Language>& languages)
{
    QStringList result;
    for (const Language& language : languages) {
        result << language.code;
    }
    return result;
}
}

class Project_InstrumentNamesTranslatorTests : public ::testing::Test
{
};

TEST_F(Project_InstrumentNamesTranslatorTests, OrderLanguages)
{
    const QList<Language> languages = {
        makeLanguage(u"de"_s, u"Deutsch"_s),
        makeLanguage(u"en_US"_s, u"English (US)"_s),
        makeLanguage(u"fr"_s, u"Français"_s),
        makeLanguage(u"it"_s, u"Italiano"_s),
    };

    // No recent languages: the interface language is left out, the others keep their order
    EXPECT_EQ(codes(InstrumentNamesTranslator::orderLanguages(languages, u"en_US"_s, {})),
              QStringList({ u"de"_s, u"fr"_s, u"it"_s }));

    // Recent languages come first, in their order; unknown ones and the interface language are skipped
    EXPECT_EQ(codes(InstrumentNamesTranslator::orderLanguages(languages, u"en_US"_s, { u"it"_s, u"xx"_s, u"en_US"_s, u"de"_s })),
              QStringList({ u"it"_s, u"de"_s, u"fr"_s }));

    // Nothing is left out if there is no language to leave out
    EXPECT_EQ(codes(InstrumentNamesTranslator::orderLanguages(languages, QString(), {})),
              QStringList({ u"de"_s, u"en_US"_s, u"fr"_s, u"it"_s }));
}

TEST_F(Project_InstrumentNamesTranslatorTests, AddRecentLanguage)
{
    QStringList recent;
    recent = InstrumentNamesTranslator::addRecentLanguage(recent, u"de"_s);
    recent = InstrumentNamesTranslator::addRecentLanguage(recent, u"fr"_s);
    EXPECT_EQ(recent, QStringList({ u"fr"_s, u"de"_s }));

    // Choosing a language again moves it to the front
    recent = InstrumentNamesTranslator::addRecentLanguage(recent, u"de"_s);
    EXPECT_EQ(recent, QStringList({ u"de"_s, u"fr"_s }));

    // "Same as interface" is not remembered
    EXPECT_EQ(InstrumentNamesTranslator::addRecentLanguage(recent, QString()), recent);

    // At most MAX_RECENT_LANGUAGES are kept
    for (const QString& code : { u"it"_s, u"es"_s, u"nl"_s, u"pt"_s }) {
        recent = InstrumentNamesTranslator::addRecentLanguage(recent, code);
    }
    EXPECT_EQ(recent.size(), InstrumentNamesTranslator::MAX_RECENT_LANGUAGES);
    EXPECT_EQ(recent.first(), u"pt"_s);
}

TEST_F(Project_InstrumentNamesTranslatorTests, DisplayName)
{
    EXPECT_EQ(InstrumentNamesTranslator::displayName(makeLanguage(u"de"_s, u"Deutsch"_s)), u"Deutsch — German"_s);

    // No English name is added if it is the same
    EXPECT_EQ(InstrumentNamesTranslator::displayName(makeLanguage(u"en"_s, u"English"_s)), u"English"_s);
}

TEST_F(Project_InstrumentNamesTranslatorTests, TranslateTemplate)
{
    InstrumentNamesTranslator translator;
    setFrenchTranslations(translator);

    mu::engraving::InstrumentTemplate templ = makeClarinetTemplate();
    translator.translate(templ);

    EXPECT_EQ(templ.instrumentName.longName(), String(u"Clarinette"));
    EXPECT_EQ(templ.instrumentName.shortName(), String(u"Cl."));
    EXPECT_EQ(templ.trackName, String(u"Clarinette"));
    EXPECT_EQ(templ.trait.name, String(u"Si♭"));

    // The marker of the default key is taken from the untranslated name
    EXPECT_TRUE(templ.trait.isDefault);
}

TEST_F(Project_InstrumentNamesTranslatorTests, TranslateTemplate_NotLoaded)
{
    // Without translations, the names stay as they are
    InstrumentNamesTranslator translator;

    mu::engraving::InstrumentTemplate templ = makeClarinetTemplate();
    translator.translate(templ);

    EXPECT_EQ(templ.instrumentName.longName(), String(u"Clarinet"));
    EXPECT_EQ(templ.trait.name, String(u"B♭"));
}

TEST_F(Project_InstrumentNamesTranslatorTests, ExampleName)
{
    InstrumentNamesTranslator translator;
    setFrenchTranslations(translator);

    // The word "in" is MuseScore's own, in the language of the interface (as the staff label is drawn)
    EXPECT_EQ(translator.exampleName(makeClarinetTemplate()), String(u"Clarinette in Si♭"));
}
