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

#include "engraving/dom/pitchspelling.h"
#include "notationscene/qml/MuseScore/NotationScene/elementpopups/stringtuningsutils.h"

using namespace mu::engraving;
using namespace mu::notation;

namespace {
struct FormatCase {
    NoteSpellingType spelling;
    int pitch;
    bool useFlats;
    QString expected;

    FormatCase(NoteSpellingType spelling, int pitch, bool useFlats, QString expected)
        : spelling(spelling), pitch(pitch), useFlats(useFlats), expected(std::move(expected)) {}
};

struct ParseCase {
    NoteSpellingType spelling;
    muse::String input;
    int expectedPitch;
    bool expectedUseFlat;

    ParseCase(NoteSpellingType spelling, muse::String input, int expectedPitch, bool expectedUseFlat)
        : spelling(spelling), input(std::move(input)), expectedPitch(expectedPitch), expectedUseFlat(expectedUseFlat) {}
};
}

TEST(StringTuningsUtilsTests, PitchToStringUsesLocalizedSpellingMatrix)
{
    const std::vector<FormatCase> cases {
        { NoteSpellingType::STANDARD, 0, false, QStringLiteral("C-1") },
        { NoteSpellingType::STANDARD, 50, false, QStringLiteral("D3") },
        { NoteSpellingType::GERMAN, 50, false, QStringLiteral("D3") },
        { NoteSpellingType::GERMAN_PURE, 50, false, QStringLiteral("D3") },
        { NoteSpellingType::SOLFEGGIO, 50, false, QStringLiteral("Re3") },
        { NoteSpellingType::FRENCH, 50, false, QStringLiteral("Ré3") },
        { NoteSpellingType::STANDARD, 58, true, QStringLiteral("B♭3") },
        { NoteSpellingType::GERMAN, 58, true, QStringLiteral("B3") },
        { NoteSpellingType::GERMAN_PURE, 58, true, QStringLiteral("B3") },
        { NoteSpellingType::GERMAN_PURE, 54, false, QStringLiteral("Fis3") },
        { NoteSpellingType::FRENCH, 127, false, QStringLiteral("Sol9") },
    };

    for (const FormatCase& testCase : cases) {
        EXPECT_EQ(stringTuningPitchToString(testCase.pitch, testCase.useFlats, testCase.spelling), testCase.expected);
    }

    EXPECT_TRUE(stringTuningPitchToString(INVALID_PITCH, false, NoteSpellingType::STANDARD).isEmpty());
}

TEST(StringTuningsUtilsTests, InputToPitchParsesLocalizedPopupInputs)
{
    const std::vector<ParseCase> cases {
        { NoteSpellingType::STANDARD, muse::String::fromUtf8("C-1"), 0, false },
        { NoteSpellingType::STANDARD, muse::String::fromUtf8("E♭3"), 51, true },
        { NoteSpellingType::STANDARD, muse::String::fromUtf8("E ♭ 3"), 51, true },
        { NoteSpellingType::SOLFEGGIO, muse::String::fromUtf8("Sol3"), 55, false },
        { NoteSpellingType::FRENCH, muse::String::fromUtf8("Ré3"), 50, false },
        { NoteSpellingType::FRENCH, muse::String::fromUtf8("Re\u03013"), 50, false },
        { NoteSpellingType::GERMAN, muse::String::fromUtf8("B3"), 58, true },
        { NoteSpellingType::GERMAN_PURE, muse::String::fromUtf8("B3"), 58, true },
        { NoteSpellingType::GERMAN_PURE, muse::String::fromUtf8("Fis3"), 54, false },
        { NoteSpellingType::FRENCH, muse::String::fromUtf8("Sol9"), 127, false },
    };

    for (const ParseCase& testCase : cases) {
        bool useFlat = false;
        EXPECT_EQ(stringTuningInputToPitch(testCase.input, testCase.spelling, &useFlat), testCase.expectedPitch);
        EXPECT_EQ(useFlat, testCase.expectedUseFlat);
    }
}

TEST(StringTuningsUtilsTests, InputToPitchRejectsInvalidPopupInputs)
{
    EXPECT_EQ(stringTuningInputToPitch(muse::String::fromUtf8("Ré"), NoteSpellingType::FRENCH), mu::engraving::INVALID_PITCH);
    EXPECT_EQ(stringTuningInputToPitch(muse::String::fromUtf8("B#3"), NoteSpellingType::GERMAN), mu::engraving::INVALID_PITCH);
    EXPECT_EQ(stringTuningInputToPitch(muse::String::fromUtf8("C-2"), NoteSpellingType::STANDARD), mu::engraving::INVALID_PITCH);
    EXPECT_EQ(stringTuningInputToPitch(muse::String::fromUtf8("C10"), NoteSpellingType::STANDARD), mu::engraving::INVALID_PITCH);
    EXPECT_EQ(stringTuningInputToPitch(muse::String::fromUtf8(""), NoteSpellingType::STANDARD), mu::engraving::INVALID_PITCH);
}
