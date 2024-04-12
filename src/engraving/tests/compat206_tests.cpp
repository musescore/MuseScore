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

static const String COMPAT206_DATA_DIR("compat206_data/");

class Engraving_Compat206Tests : public ::testing::Test
{
public:
    void compat(const char* filename);
};

void Engraving_Compat206Tests::compat(const char* filename)
{
    String _filename = String::fromUtf8(filename);
    QString readFile(COMPAT206_DATA_DIR + _filename + u".mscx");
    QString writeFile(_filename + "-test.mscx");
    QString reference(COMPAT206_DATA_DIR + _filename + u"-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    ASSERT_TRUE(score);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile, reference));

    delete score;
}

TEST_F(Engraving_Compat206Tests, accidentals) {
    compat("accidentals");
}
TEST_F(Engraving_Compat206Tests, ambitus) {
    compat("ambitus");
}
TEST_F(Engraving_Compat206Tests, articulations) {
    compat("articulations");
}
TEST_F(Engraving_Compat206Tests, articulationsDouble) {
    compat("articulations-double");
}
TEST_F(Engraving_Compat206Tests, breath) {
    compat("breath");
}
TEST_F(Engraving_Compat206Tests, barlines) {
    compat("barlines");
}
TEST_F(Engraving_Compat206Tests, clefs) {
    compat("clefs");
}
TEST_F(Engraving_Compat206Tests, drumset) {
    compat("drumset");
}
TEST_F(Engraving_Compat206Tests, fermata) {
    compat("fermata");
}
TEST_F(Engraving_Compat206Tests, frame_utf8) {
    compat("frame_text2");
}
TEST_F(Engraving_Compat206Tests, hairpin) {
    compat("hairpin");
}
TEST_F(Engraving_Compat206Tests, instrumentNameAlign) {
    compat("instrumentNameAlign");
}
TEST_F(Engraving_Compat206Tests, lidEmptyText) {
    compat("lidemptytext");
}
TEST_F(Engraving_Compat206Tests, markers) {
    compat("markers");
}
TEST_F(Engraving_Compat206Tests, noteheads) {
    compat("noteheads");
}
TEST_F(Engraving_Compat206Tests, textstyles) {
    compat("textstyles");
}
TEST_F(Engraving_Compat206Tests, tuplets) {
    compat("tuplets");
}
TEST_F(Engraving_Compat206Tests, userStylesParts) {
    compat("userstylesparts");
}
