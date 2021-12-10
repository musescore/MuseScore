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

#include "libmscore/factory.h"
#include "libmscore/masterscore.h"
#include "libmscore/undo.h"
#include "libmscore/measure.h"
#include "libmscore/breath.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

static const QString BREATH_DATA_DIR("breath_data/");

using namespace mu::engraving;
using namespace Ms;

class BreathTests : public ::testing::Test
{
};

TEST_F(BreathTests, breath)
{
    QString readFile(BREATH_DATA_DIR + "breath.mscx");
    QString writeFile1("breath01-test.mscx");
    QString reference1(BREATH_DATA_DIR + "breath01-ref.mscx");
    QString writeFile2("breath02-test.mscx");
    QString reference2(BREATH_DATA_DIR + "breath02-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    EXPECT_TRUE(score);

    score->doLayout();

    // do
    score->startCmd();
    score->cmdSelectAll();
    for (EngravingItem* e : score->selection().elements()) {
        EditData dd(0);
        Breath* b = Factory::createBreath(score->dummy()->segment());
        b->setSymId(SymId::breathMarkComma);
        dd.dropElement = b;
        if (e->acceptDrop(dd)) {
            e->drop(dd);
        }
    }
    score->endCmd();
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    score->undoStack()->undo(0);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}
