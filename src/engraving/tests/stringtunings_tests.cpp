/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/stringtunings.h"

using namespace mu::engraving;

namespace {
void setChordSymbolSpelling(MasterScore* score, NoteSpellingType spelling)
{
    score->startCmd(TranslatableString::untranslatable("String tunings tests"));
    score->undoChangeStyleVal(Sid::chordSymbolSpelling, spelling);
    score->endCmd();
}

StringTunings* createStringTunings(MasterScore* score, std::vector<instrString> strings,
                                   const std::vector<string_idx_t>& visibleStrings)
{
    StringTunings* stringTunings = Factory::createStringTunings(score->dummy()->segment());
    stringTunings->setStringData(StringData(24, strings));
    stringTunings->setVisibleStrings(visibleStrings);
    return stringTunings;
}
}

class Engraving_StringTuningsTests : public ::testing::Test
{
};

TEST_F(Engraving_StringTuningsTests, GenerateTextUsesCurrentChordSymbolSpelling)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    StringTunings* stringTunings = createStringTunings(score, { instrString(50, false) }, { 0 });

    setChordSymbolSpelling(score, NoteSpellingType::FRENCH);
    stringTunings->updateText();
    const QString frenchText = stringTunings->xmlText().toQString();
    EXPECT_TRUE(frenchText.contains(u"Ré"));
    EXPECT_FALSE(frenchText.contains(u"RÉ"));

    setChordSymbolSpelling(score, NoteSpellingType::SOLFEGGIO);
    stringTunings->updateText();
    const QString solfeggioText = stringTunings->xmlText().toQString();
    EXPECT_TRUE(solfeggioText.contains(u"Re"));
    EXPECT_FALSE(solfeggioText.contains(u"RE"));

    delete stringTunings;
    delete score;
}

TEST_F(Engraving_StringTuningsTests, AccessibleInfoUsesLocalizedPitchNamesWithOctave)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);

    instrString string(51, false);
    string.useFlat = true;

    StringTunings* stringTunings = createStringTunings(score, { string }, { 0 });

    setChordSymbolSpelling(score, NoteSpellingType::FRENCH);
    const QString accessibleInfo = stringTunings->accessibleInfo().toQString();

    EXPECT_TRUE(accessibleInfo.contains(u"Mi♭3"));
    EXPECT_FALSE(accessibleInfo.contains(u"MI♭3"));

    delete stringTunings;
    delete score;
}

TEST_F(Engraving_StringTuningsTests, AccessibleInfoSkipsInvalidStringPitches)
{
    MasterScore* score = compat::ScoreAccess::createMasterScore(nullptr);
    StringTunings* stringTunings = createStringTunings(score, { instrString(INVALID_PITCH, false), instrString(127, false) }, { 0, 1 });

    setChordSymbolSpelling(score, NoteSpellingType::FRENCH);
    const QString accessibleInfo = stringTunings->accessibleInfo().toQString();

    EXPECT_TRUE(accessibleInfo.contains(u"Sol9"));
    EXPECT_FALSE(accessibleInfo.contains(QStringLiteral("-1")));

    delete stringTunings;
    delete score;
}
