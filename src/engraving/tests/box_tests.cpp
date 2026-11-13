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
#include "engraving/dom/figuredbass.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/system.h"
#include "engraving/dom/text.h"
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
    String writeFile1(u"undoRemoveVBox1-test.mscx");
    String reference1(BOX_DATA_DIR + u"undoRemoveVBox1-ref.mscx");
    String writeFile2(u"undoRemoveVBox2-test.mscx");
    String reference2(BOX_DATA_DIR + u"undoRemoveVBox2-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    ASSERT_TRUE(score);

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

//---------------------------------------------------------
//   editVBoxKeepsNextSystemLayout
///   Typing into a frame's text relays out the frame and the system after it; the system after that is
///   kept and restored. Restoring it must not add its ties to its skyline again, nor give its volta
///   another segment, and must lay out its figured bass the same as before, even though that system is not
///   among the laid out systems.
//---------------------------------------------------------
TEST_F(Engraving_BoxTests, editVBoxKeepsNextSystemLayout)
{
    MasterScore* score = ScoreRW::readScore(BOX_DATA_DIR + u"editVBoxNextSystem.mscx");
    ASSERT_TRUE(score);
    ASSERT_EQ(score->systems().size(), 4);

    Text* title = toText(toVBox(score->first())->el().front());
    const System* keptSystem = score->systems()[2];
    const Segment* figuredBassSegment = keptSystem->firstMeasure()->first(SegmentType::ChordRest);
    ASSERT_EQ(figuredBassSegment->annotations().size(), 1);
    const FiguredBass* figuredBass = toFiguredBass(figuredBassSegment->annotations().front());

    auto skylineSize = [keptSystem]() {
        const Skyline& skyline = keptSystem->staff(0)->skyline();
        return skyline.north().elements().size() + skyline.south().elements().size();
    };
    auto voltaSegmentCount = [score]() {
        size_t count = 0;
        for (const auto& pair : score->spanner()) {
            if (pair.second->isVolta()) {
                count += pair.second->spannerSegments().size();
            }
        }
        return count;
    };

    const size_t initialSkylineSize = skylineSize();
    const size_t initialVoltaSegmentCount = voltaSegmentCount();
    EXPECT_EQ(initialVoltaSegmentCount, 1);
    const std::vector<double> initialFiguredBassLines = figuredBass->ldata()->lineLengths;
    ASSERT_EQ(initialFiguredBassLines.size(), 1);
    EXPECT_GT(initialFiguredBassLines.front(), 0);

    EditData ed;
    title->startEdit(ed);
    title->cursor()->moveCursorToEnd();
    for (int i = 0; i < 3; ++i) {
        score->startCmd(TranslatableString::untranslatable("Engraving box tests"));
        ed.s = u"x";
        title->edit(ed);
        score->endCmd();
    }
    title->endEdit(ed);

    ASSERT_EQ(score->systems()[2], keptSystem);
    EXPECT_EQ(skylineSize(), initialSkylineSize);
    EXPECT_EQ(voltaSegmentCount(), initialVoltaSegmentCount);
    EXPECT_EQ(figuredBass->ldata()->lineLengths, initialFiguredBassLines);

    delete score;
}
