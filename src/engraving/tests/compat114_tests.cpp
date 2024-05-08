/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "dom/masterscore.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String COMPAT114_DATA_DIR("compat114_data/");

class Engraving_Compat114Tests : public ::testing::Test
{
public:
    void compat(const char* filename);
};

void Engraving_Compat114Tests::compat(const char* filename)
{
    String _filename = String::fromUtf8(filename);
    QString readFile(COMPAT114_DATA_DIR + _filename + u".mscx");
    QString writeFile(_filename + "-test.mscx");
    QString reference(COMPAT114_DATA_DIR + _filename + u"-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    ASSERT_TRUE(score);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile, reference));

    delete score;
}

//---------------------------------------------------------
//   compat_data
//    every "xxx" test requires two *.mscx files:
//          xxx.mscx     is the mscore 1.x file
//          xxx-ref.mscx is the corresponding (correct)
//                       mscore 3.0 file
//---------------------------------------------------------

TEST_F(Engraving_Compat114Tests, accidentals) {
    compat("accidentals");
}

TEST_F(Engraving_Compat114Tests, articulations) {
    compat("articulations");
}

// TEST_F(Engraving_Compat114Tests, chord_symbol) {
//     compat("chord_symbol");
// }

TEST_F(Engraving_Compat114Tests, clef_missing_first) {
    compat("clef_missing_first");
}

TEST_F(Engraving_Compat114Tests, clefs) {
    compat("clefs");
}

TEST_F(Engraving_Compat114Tests, drumset) {
    compat("drumset");
}

TEST_F(Engraving_Compat114Tests, fingering) {
    compat("fingering");
}

TEST_F(Engraving_Compat114Tests, hairpin) {
    compat("hairpin");
}

TEST_F(Engraving_Compat114Tests, hor_frame_and_mmrest) {
    compat("hor_frame_and_mmrest");
}

TEST_F(Engraving_Compat114Tests, keysig) {
    compat("keysig");
}

TEST_F(Engraving_Compat114Tests, markers) {
    compat("markers");
}

TEST_F(Engraving_Compat114Tests, noteheads) {
    compat("noteheads");
}

TEST_F(Engraving_Compat114Tests, notes) {
    compat("notes");
}

TEST_F(Engraving_Compat114Tests, notes_useroffset) {
    compat("notes_useroffset");
}

TEST_F(Engraving_Compat114Tests, ottava) {
    compat("ottava");
}

TEST_F(Engraving_Compat114Tests, pedal) {
    compat("pedal");
}

TEST_F(Engraving_Compat114Tests, slurs) {
    compat("slurs");
}

TEST_F(Engraving_Compat114Tests, style) {
    compat("style");
}

TEST_F(Engraving_Compat114Tests, tamtam) {
    compat("tamtam");
}

// TEST_F(Engraving_Compat114Tests, text_scaling) {
//     compat("text_scaling");
// }

TEST_F(Engraving_Compat114Tests, textline) {
    compat("textline");
}

TEST_F(Engraving_Compat114Tests, textstyles) {
    compat("textstyles");
}

TEST_F(Engraving_Compat114Tests, title) {
    compat("title");
}

TEST_F(Engraving_Compat114Tests, tremolo2notes) {
    compat("tremolo2notes");
}

TEST_F(Engraving_Compat114Tests, tuplets) {
    compat("tuplets");
}

TEST_F(Engraving_Compat114Tests, tuplets_1) {
    compat("tuplets_1");
}

TEST_F(Engraving_Compat114Tests, tuplets_2) {
    compat("tuplets_2");
}
