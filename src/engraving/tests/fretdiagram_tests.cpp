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

#include "engraving/dom/factory.h"
#include "engraving/dom/fret.h"
#include "engraving/dom/harmony.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

static const String FRETDIAGRAM_DATA_DIR(u"fretdiagrams_data/");

class Engraving_FretDiagramTests : public ::testing::Test
{
protected:
    void testChordSymToFretDiagram(MasterScore* score);
};

void Engraving_FretDiagramTests::testChordSymToFretDiagram(MasterScore* score)
{
    EXPECT_TRUE(score);
    Measure* measure = score->firstMeasure();
    EXPECT_TRUE(measure);
    static const String FRET_PATTERN_REF = u"XO[4-O][2-O]OO";

    while (measure) {
        Segment* s1 = measure->first(SegmentType::ChordRest);
        EXPECT_TRUE(s1);

        Harmony* harmony = toHarmony(s1->findAnnotation(ElementType::HARMONY, 0, 0));
        EXPECT_TRUE(harmony);

        FretDiagram* diagram = Factory::createFretDiagram(score->dummy()->segment());
        EXPECT_TRUE(diagram);
        diagram->updateDiagram(harmony->harmonyName());
        String pattern = FretDiagram::patternFromDiagram(diagram);
        EXPECT_EQ(pattern, FRET_PATTERN_REF);
        measure = measure->nextMeasure();
        delete diagram;
    }
}

TEST_F(Engraving_FretDiagramTests, harmonyToFretDiagram)
{
    MasterScore* score = ScoreRW::readScore(FRETDIAGRAM_DATA_DIR + u"harmonytofrettest.mscx");

    testChordSymToFretDiagram(score);
}

TEST_F(Engraving_FretDiagramTests, harmonyToFretDiagramSolfeggio)
{
    MasterScore* score = ScoreRW::readScore(FRETDIAGRAM_DATA_DIR + u"harmonytofrettest.mscx");

    score->startCmd(TranslatableString::untranslatable("Fret diagram tests"));
    score->undoChangeStyleVal(Sid::chordSymbolSpelling, NoteSpellingType::SOLFEGGIO);
    score->endCmd();

    testChordSymToFretDiagram(score);
}
