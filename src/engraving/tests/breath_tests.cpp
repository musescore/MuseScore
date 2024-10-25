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

#include "dom/breath.h"
#include "dom/factory.h"
#include "dom/masterscore.h"
#include "dom/undo.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String BREATH_DATA_DIR("breath_data/");

class Engraving_BreathTests : public ::testing::Test
{
};

TEST_F(Engraving_BreathTests, breath)
{
    String readFile(BREATH_DATA_DIR + u"breath.mscx");
    String writeFile1(u"breath01-test.mscx");
    String reference1(BREATH_DATA_DIR + u"breath01-ref.mscx");
    String writeFile2(u"breath02-test.mscx");
    String reference2(BREATH_DATA_DIR + u"breath02-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    EXPECT_TRUE(score);

    score->doLayout();

    // do
    score->startCmd(TranslatableString::untranslatable("Engraving breath tests"));
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
