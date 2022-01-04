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

#include "libmscore/masterscore.h"
#include "libmscore/system.h"
#include "libmscore/undo.h"
#include "libmscore/box.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

static const QString BOX_DATA_DIR("box_data/");

using namespace mu::engraving;
using namespace Ms;

class BoxTests : public ::testing::Test
{
};

//---------------------------------------------------------
//   undoRemoveVBox
///   read a file with a vbox. Delete it, and undo. Check that the VBox still exists.
//---------------------------------------------------------
TEST_F(BoxTests, undoRemoveVBox)
{
    QString readFile(BOX_DATA_DIR + "undoRemoveVBox.mscx");
    QString writeFile1("undoRemoveVBox1-test.mscx");
    QString reference1(BOX_DATA_DIR + "undoRemoveVBox1-ref.mscx");
    QString writeFile2("undoRemoveVBox2-test.mscx");
    QString reference2(BOX_DATA_DIR + "undoRemoveVBox2-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    EXPECT_TRUE(score);

    score->doLayout();

    System* s = score->systems()[0];
    VBox* box = toVBox(s->measure(0));

    score->startCmd();
    score->select(box);
    score->cmdDeleteSelection();
    score->endCmd();

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    score->undoStack()->undo(nullptr);

    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}
