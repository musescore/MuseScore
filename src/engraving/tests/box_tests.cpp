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

#include "dom/box.h"
#include "dom/masterscore.h"
#include "dom/system.h"
#include "dom/undo.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
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
    String writeFile1(u"undoRemoveVBox1-test.mscx");
    String reference1(BOX_DATA_DIR + u"undoRemoveVBox1-ref.mscx");
    String writeFile2(u"undoRemoveVBox2-test.mscx");
    String reference2(BOX_DATA_DIR + u"undoRemoveVBox2-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    EXPECT_TRUE(score);

    score->doLayout();

    System* s = score->systems()[0];
    VBox* box = toVBox(s->measure(0));

    score->startCmd(TranslatableString::untranslatable("Engraving box tests"));
    score->select(box);
    score->cmdDeleteSelection();
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    score->undoStack()->undo(nullptr);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}
