/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "engraving/dom/box.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/system.h"
#include "engraving/editing/transaction/undostack.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu::engraving;

static const String BOX_DATA_DIR(u"box_data/");

class Engraving_BoxTests : public ::testing::Test
{
};

//---------------------------------------------------------
//   undoRemoveVBox
///   read a file with a vbox. Delete it, and undo. Check that the VBox still exists.
//---------------------------------------------------------
TEST_F(Engraving_BoxTests, undoRemoveVBox)
{
    String readFile(BOX_DATA_DIR + u"undoRemoveVBox.mscx");
    String beforeFile(u"undoRemoveVBox-before.mscx");
    String writeFile1(u"undoRemoveVBox1-test.mscx");
    String reference1(BOX_DATA_DIR + u"undoRemoveVBox1-ref.mscx");
    String writeFile2(u"undoRemoveVBox2-test.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    ASSERT_TRUE(score);

    score->doLayout();

    EXPECT_TRUE(ScoreRW::saveScore(score, beforeFile));

    System* s = score->systems()[0];
    VBox* box = toVBox(s->measure(0));

    score->startCmd(TranslatableString::untranslatable("Engraving box tests"));
    score->select(box);
    score->cmdDeleteSelection();
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    score->undoStack()->undo(nullptr);
    score->doLayout();

    EXPECT_TRUE(ScoreComp::saveCompareWithSavedScore(score, writeFile2, beforeFile));

    delete score;
}
