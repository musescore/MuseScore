/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
};

TEST_F(Engraving_ImplodeExplodeTests, undoExplode)
{
    String readFile(IMPLODEEXP_DATA_DIR + "undoExplode.mscx");
    String writeFile1("undoExplode01-test.mscx");
    String reference1(IMPLODEEXP_DATA_DIR + "undoExplode01-ref.mscx");
    String writeFile2("undoExplode02-test.mscx");
    String reference2(IMPLODEEXP_DATA_DIR + "undoExplode02-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    EXPECT_TRUE(score);
    score->doLayout();

    // select all
    score->startCmd();
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd();
    score->cmdExplode();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}

TEST_F(Engraving_ImplodeExplodeTests, undoImplode)
{
    String readFile(IMPLODEEXP_DATA_DIR + "undoImplode.mscx");
    String writeFile1("undoImplode01-test.mscx");
    String reference1(IMPLODEEXP_DATA_DIR + "undoImplode01-ref.mscx");
    String writeFile2("undoImplode02-test.mscx");
    String reference2(IMPLODEEXP_DATA_DIR + "undoImplode02-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    EXPECT_TRUE(score);
    score->doLayout();

    // select all
    score->startCmd();
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd();
    score->cmdImplode();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}

TEST_F(Engraving_ImplodeExplodeTests, undoImplodeVoice)
{
    String readFile(IMPLODEEXP_DATA_DIR + "undoImplodeVoice.mscx");
    String writeFile1("undoImplodeVoice01-test.mscx");
    String reference1(IMPLODEEXP_DATA_DIR + "undoImplodeVoice01-ref.mscx");
    String writeFile2("undoImplodeVoice02-test.mscx");
    String reference2(IMPLODEEXP_DATA_DIR + "undoImplodeVoice02-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    EXPECT_TRUE(score);
    score->doLayout();

    // select all
    score->startCmd();
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd();
    score->cmdImplode();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}

TEST_F(Engraving_ImplodeExplodeTests, implode1)
{
    String readFile(IMPLODEEXP_DATA_DIR + "implode1.mscx");
    String writeFile1("implode1-test1.mscx");
    String writeFile2("implode1-test2.mscx");
    String reference(IMPLODEEXP_DATA_DIR + "implode1-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    EXPECT_TRUE(score);
    score->doLayout();

    // select all
    score->startCmd();
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd();
    score->cmdImplode();
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, readFile));

    delete score;
}
