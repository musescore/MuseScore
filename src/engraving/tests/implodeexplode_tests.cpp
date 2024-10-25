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
#include "dom/undo.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String IMPLODEEXP_DATA_DIR("implode_explode_data/");

class Engraving_ImplodeExplodeTests : public ::testing::Test
{
public:
    void testUndoExplode(String fileName);
    void testUndoImplode(String fileName);
};

void Engraving_ImplodeExplodeTests::testUndoExplode(String fileName)
{
    String readFile(IMPLODEEXP_DATA_DIR + fileName + ".mscx");
    String writeFile1(fileName + "01-test.mscx");
    String reference1(IMPLODEEXP_DATA_DIR + fileName + "01-ref.mscx");
    String writeFile2(fileName + "02-test.mscx");
    String reference2(IMPLODEEXP_DATA_DIR + fileName + "02-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    EXPECT_TRUE(score);
    score->doLayout();

    // select all
    score->startCmd(TranslatableString::untranslatable("Implode/explode select all"));
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd(TranslatableString::untranslatable("Implode/explode tests"));
    score->cmdExplode();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}

void Engraving_ImplodeExplodeTests::testUndoImplode(String filename)
{
    String readFile(IMPLODEEXP_DATA_DIR + filename + ".mscx");
    String writeFile1(filename + "01-test.mscx");
    String reference1(IMPLODEEXP_DATA_DIR + filename + "01-ref.mscx");
    String writeFile2(filename + "02-test.mscx");
    String reference2(IMPLODEEXP_DATA_DIR + filename + "02-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    EXPECT_TRUE(score);
    score->doLayout();

    // select all
    score->startCmd(TranslatableString::untranslatable("Implode/explode select all"));
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd(TranslatableString::untranslatable("Implode/explode tests"));
    score->cmdImplode();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}

TEST_F(Engraving_ImplodeExplodeTests, undoExplode)
{
    testUndoExplode(u"undoExplode");
}

TEST_F(Engraving_ImplodeExplodeTests, undoImplode)
{
    testUndoImplode(u"undoImplode");
}

TEST_F(Engraving_ImplodeExplodeTests, undoImplodeVoice)
{
    testUndoImplode(u"undoImplodeVoice");
}

TEST_F(Engraving_ImplodeExplodeTests, implodeScore)
{
    testUndoImplode(u"implodeScore");
}

TEST_F(Engraving_ImplodeExplodeTests, explodeDynamics)
{
    bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    testUndoExplode(u"explodeDynamics");

    MScore::useRead302InTestMode = useRead302;
}

TEST_F(Engraving_ImplodeExplodeTests, implodeDynamics)
{
    bool useRead302 = MScore::useRead302InTestMode;
    MScore::useRead302InTestMode = false;

    testUndoImplode(u"implodeDynamics");

    MScore::useRead302InTestMode = useRead302;
}
