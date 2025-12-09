/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#include <gmock/gmock.h>

#include "converter/internal/compat/scoreelementsscanner.h"
#include "engraving/tests/utils/scorerw.h"

using namespace mu::converter;
using namespace mu::engraving;

static const muse::String CONVERTER_DATA_DIR("data/");

class Converter_ScoreElementsTests : public ::testing::Test
{
public:
    ElementInfo makeInfo(ElementType type, const String& name = u"", const StringList& notes = {}) const
    {
        ElementInfo info;
        info.type = type;
        info.name = name;

        info.notes.reserve(notes.size());
        for (const String& notename : notes) {
            ElementInfo::Note note;
            note.name = notename;
            info.notes.push_back(note);
        }

        return info;
    }
};

TEST_F(Converter_ScoreElementsTests, ScanElements)
{
    // [GIVEN] Score to be scanned
    Score* score = ScoreRW::readScore(CONVERTER_DATA_DIR + "score_elements.mscx");
    ASSERT_TRUE(score);

    // [WHEN] Scan the score
    ElementMap result = ScoreElementScanner::scanElements(score);

    // [THEN] The list matches the expected one
    ElementInfoList expectedList;

    // 1st measure
    expectedList.emplace_back(makeInfo(ElementType::CLEF, u"Treble clef"));
    expectedList.emplace_back(makeInfo(ElementType::KEYSIG, u"C major / A minor"));
    expectedList.emplace_back(makeInfo(ElementType::TIMESIG, u"4/4 time"));
    expectedList.emplace_back(makeInfo(ElementType::ARPEGGIO, u"Up arpeggio", { u"C5", u"E5", u"G5", u"B5" }));
    expectedList.emplace_back(makeInfo(ElementType::CHORD, u"", { u"C5", u"E5", u"G5", u"B5" }));
    expectedList.emplace_back(makeInfo(ElementType::TREMOLO_SINGLECHORD, u"32nd through stem", { u"F4", u"A4", u"C5" }));
    expectedList.emplace_back(makeInfo(ElementType::REST));
    expectedList.emplace_back(makeInfo(ElementType::BAR_LINE, u"Single barline"));

    // 2nd measure
    expectedList.emplace_back(makeInfo(ElementType::ORNAMENT, u"Turn", { u"A4", u"E5" }));
    expectedList.emplace_back(makeInfo(ElementType::ORNAMENT, u"Turn", { u"A4", u"E5" }));
    expectedList.emplace_back(makeInfo(ElementType::NOTE, u"C5"));
    expectedList.emplace_back(makeInfo(ElementType::REST));
    expectedList.emplace_back(makeInfo(ElementType::BAR_LINE, u"Single barline"));

    // 3rd measure
    expectedList.emplace_back(makeInfo(ElementType::NOTE, u"A4"));
    expectedList.emplace_back(makeInfo(ElementType::TRILL, u"Trill line"));
    expectedList.emplace_back(makeInfo(ElementType::NOTE, u"C5"));
    expectedList.emplace_back(makeInfo(ElementType::NOTE, u"B4"));
    expectedList.emplace_back(makeInfo(ElementType::NOTE, u"D5"));
    expectedList.emplace_back(makeInfo(ElementType::BAR_LINE, u"Single barline"));

    // 4th measure
    expectedList.emplace_back(makeInfo(ElementType::NOTE, u"A4"));
    expectedList.emplace_back(makeInfo(ElementType::HAIRPIN, u"Crescendo hairpin"));
    expectedList.emplace_back(makeInfo(ElementType::GRADUAL_TEMPO_CHANGE, u"accel."));
    expectedList.emplace_back(makeInfo(ElementType::NOTE, u"B4"));
    expectedList.emplace_back(makeInfo(ElementType::NOTE, u"A4"));
    expectedList.emplace_back(makeInfo(ElementType::NOTE, u"B4"));
    expectedList.emplace_back(makeInfo(ElementType::BAR_LINE, u"Single barline"));

    // 5th measure
    expectedList.emplace_back(makeInfo(ElementType::NOTE, u"A4"));
    expectedList.emplace_back(makeInfo(ElementType::PLAYTECH_ANNOTATION, u"Pizzicato"));
    expectedList.emplace_back(makeInfo(ElementType::NOTE, u"B4"));
    expectedList.emplace_back(makeInfo(ElementType::NOTE, u"A4"));
    expectedList.emplace_back(makeInfo(ElementType::NOTE, u"B4"));
    expectedList.emplace_back(makeInfo(ElementType::BAR_LINE, u"Final barline"));

    ASSERT_EQ(result.size(), 1);
    const mu::engraving::InstrumentTrackId expectedTrackId { muse::ID(1), u"piano" };
    EXPECT_EQ(result.begin()->first, expectedTrackId);

    const ElementInfoList& actualList = result.begin()->second;
    EXPECT_EQ(expectedList.size(), actualList.size());

    for (size_t i = 0; i < actualList.size(); ++i) {
        const ElementInfo& actualInfo = actualList.at(i);
        const ElementInfo& expectedInfo = expectedList.at(i);

        EXPECT_EQ(actualInfo.type, expectedInfo.type);
        EXPECT_EQ(actualInfo.name, expectedInfo.name);
        EXPECT_EQ(actualInfo.data, expectedInfo.data);
        ASSERT_EQ(actualInfo.notes.size(), expectedInfo.notes.size());

        for (size_t j = 0; j < actualInfo.notes.size(); ++j) {
            const ElementInfo::Note& actualNote = actualInfo.notes.at(j);
            const ElementInfo::Note& expectedNote = expectedInfo.notes.at(j);

            EXPECT_EQ(actualNote.name, expectedNote.name);
            EXPECT_EQ(actualInfo.data, expectedInfo.data);
        }
    }

    delete score;
}
