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
    ScoreElementScanner::ElementInfo makeInfo(const String& name, const String& notes = u"") const
    {
        ScoreElementScanner::ElementInfo info;
        info.name = name;
        info.notes = notes;

        return info;
    }
};

TEST_F(Converter_ScoreElementsTests, ScanElements)
{
    // [GIVEN] Score to be scanned
    Score* score = ScoreRW::readScore(CONVERTER_DATA_DIR + "score_elements.mscx");
    ASSERT_TRUE(score);

    // [GIVEN] Scanner options
    ScoreElementScanner::Options options;
    options.avoidDuplicates = true;
    options.acceptedTypes = {
        // 1st measure
        ElementType::KEYSIG,
        ElementType::TIMESIG,
        ElementType::ARPEGGIO,
        ElementType::CHORD,
        ElementType::TREMOLO_SINGLECHORD,

        // 2nd measure
        ElementType::ORNAMENT,

        // 3rd measure
        ElementType::TRILL,

        // 4th measure
        ElementType::GRADUAL_TEMPO_CHANGE,
        ElementType::HAIRPIN,

        // 5th measure
        ElementType::PLAYTECH_ANNOTATION,
    };

    // [WHEN] Scan the score
    ScoreElementScanner::InstrumentElementMap result = ScoreElementScanner::scanElements(score, options);

    // [THEN] The map matches the expected one
    ScoreElementScanner::ElementMap expectedMap;
    // 1st measure
    expectedMap[ElementType::KEYSIG] = { makeInfo(u"C major / A minor") };
    expectedMap[ElementType::TIMESIG] = { makeInfo(u"4/4 time") };
    expectedMap[ElementType::ARPEGGIO] = { makeInfo(u"Up arpeggio", u"C5 E5 G5 B5") };
    expectedMap[ElementType::CHORD] = { makeInfo(u"", u"C5 E5 G5 B5") };
    expectedMap[ElementType::TREMOLO_SINGLECHORD] = { makeInfo(u"32nd through stem", u"F4 A4 C5") };

    // 2nd measure
    expectedMap[ElementType::ORNAMENT] = { makeInfo(u"Turn", u"A4 E5") }; // skip duplicates

    // 3rd measure
    expectedMap[ElementType::TRILL] = { makeInfo(u"Trill line") };

    // 4th measure
    expectedMap[ElementType::GRADUAL_TEMPO_CHANGE] = { makeInfo(u"accel.") };
    expectedMap[ElementType::HAIRPIN] = { makeInfo(u"Crescendo hairpin") };

    // 5th measure
    expectedMap[ElementType::PLAYTECH_ANNOTATION] = { makeInfo(u"Pizzicato") };

    ASSERT_EQ(result.size(), 1);
    const mu::engraving::InstrumentTrackId expectedTrackId { muse::ID(1), u"piano" };
    EXPECT_EQ(result.begin()->first, expectedTrackId);

    const ScoreElementScanner::ElementMap& actualMap = result.begin()->second;
    EXPECT_EQ(actualMap.size(), expectedMap.size());

    for (const auto& pair : actualMap) {
        auto it = expectedMap.find(pair.first);
        ASSERT_TRUE(it != expectedMap.end());

        const ScoreElementScanner::ElementInfoList& expectedInfoList = it->second;
        ASSERT_EQ(pair.second.size(), expectedInfoList.size());

        for (size_t i = 0; i < pair.second.size(); ++i) {
            const ScoreElementScanner::ElementInfo& actualInfo = pair.second.at(i);
            const ScoreElementScanner::ElementInfo& expectedInfo = expectedInfoList.at(i);

            EXPECT_EQ(actualInfo.name, expectedInfo.name);
            EXPECT_EQ(actualInfo.notes, expectedInfo.notes);
        }
    }

    delete score;
}
