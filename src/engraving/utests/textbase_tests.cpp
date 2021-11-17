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
#if (defined (_MSCVER) || defined (_MSC_VER))
#pragma warning(disable: 4459) // _t hides global declaration
#endif

#include <gtest/gtest.h>

#include "libmscore/masterscore.h"
#include "libmscore/segment.h"
#include "libmscore/dynamic.h"
#include "libmscore/stafftext.h"
#include "libmscore/chordrest.h"
#include "libmscore/textedit.h"
#include "libmscore/tie.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace Ms;
using namespace mu::engraving;

static const QString TEXTBASE_DATA_DIR("textbase_data/");

class TextBaseTests : public ::testing::Test
{
public:
    Dynamic* addDynamic(MasterScore* score);
    StaffText* addStaffText(MasterScore* score);
};

Dynamic* TextBaseTests::addDynamic(MasterScore* score)
{
    Dynamic* dynamic = new Dynamic(score->dummy()->segment());
    dynamic->setXmlText("<sym>dynamicForte</sym>");
    ChordRest* chordRest = score->firstSegment(SegmentType::ChordRest)->nextChordRest(0);
    EditData ed;
    ed.dropElement = dynamic;
    chordRest->drop(ed);
    return dynamic;
}

TEST_F(TextBaseTests, createDynamic)
{
    MasterScore* score = ScoreRW::readScore("test.mscx");
    addDynamic(score);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, "createDynamic.mscx", TEXTBASE_DATA_DIR + "createDynamic-ref.mscx"));
}

TEST_F(TextBaseTests, dynamicAddTextBefore)
{
    MasterScore* score = ScoreRW::readScore("test.mscx");
    Dynamic* dynamic = addDynamic(score);
    EditData ed;
    dynamic->startEdit(ed);
    score->undo(new InsertText(dynamic->cursor(), QString("poco ")), &ed);
    dynamic->endEdit(ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, "dynamicAddTextBefore.mscx", TEXTBASE_DATA_DIR + "dynamicAddTextBefore-ref.mscx"));
}

TEST_F(TextBaseTests, dynamicAddTextAfter)
{
    MasterScore* score = ScoreRW::readScore("test.mscx");
    Dynamic* dynamic = addDynamic(score);
    EditData ed;
    dynamic->startEdit(ed);
    dynamic->cursor()->moveCursorToEnd();
    score->undo(new InsertText(dynamic->cursor(), QString(" ma non troppo")), &ed);
    dynamic->endEdit(ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, "dynamicAddTextAfter.mscx", TEXTBASE_DATA_DIR + "dynamicAddTextAfter-ref.mscx"));
}

TEST_F(TextBaseTests, dynamicAddTextNoItalic)
{
    MasterScore* score = ScoreRW::readScore("test.mscx");
    Dynamic* dynamic = addDynamic(score);
    EditData ed;
    dynamic->startEdit(ed);
    dynamic->setProperty(Ms::Pid::FONT_STYLE, PropertyValue::fromValue(0));
    score->undo(new InsertText(dynamic->cursor(), QString("moderately ")), &ed);
    dynamic->endEdit(ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, "dynamicAddTextNoItalic.mscx", TEXTBASE_DATA_DIR + "dynamicAddTextNoItalic-ref.mscx"));
}

StaffText* TextBaseTests::addStaffText(MasterScore* score)
{
    StaffText* staffText = new StaffText(score->dummy()->segment());
    ChordRest* chordRest = score->firstSegment(SegmentType::ChordRest)->nextChordRest(0);
    EditData ed;
    ed.dropElement = staffText;
    chordRest->drop(ed);
    return staffText;
}

TEST_F(TextBaseTests, getFontStyleProperty)
{
    MasterScore* score = ScoreRW::readScore("test.mscx");
    StaffText* staffText = addStaffText(score);
    EditData ed;
    staffText->startEdit(ed);
    score->undo(new InsertText(staffText->cursor(), QString("normal ")), &ed);
    staffText->setProperty(Ms::Pid::FONT_STYLE, PropertyValue::fromValue(static_cast<int>(FontStyle::Bold)));
    score->undo(new InsertText(staffText->cursor(), QString("bold")), &ed);
    staffText->cursor()->moveCursorToStart();
    EXPECT_EQ(staffText->getProperty(Ms::Pid::FONT_STYLE), PropertyValue::fromValue(0));
    staffText->cursor()->movePosition(TextCursor::MoveOperation::NextWord, TextCursor::MoveMode::KeepAnchor);
    EXPECT_EQ(staffText->getProperty(Ms::Pid::FONT_STYLE), PropertyValue::fromValue(0));
    staffText->cursor()->movePosition(TextCursor::MoveOperation::End, TextCursor::MoveMode::KeepAnchor);
    EXPECT_EQ(staffText->getProperty(Ms::Pid::FONT_STYLE), PropertyValue::fromValue(0));
    staffText->cursor()->movePosition(TextCursor::MoveOperation::WordLeft, TextCursor::MoveMode::MoveAnchor);
    EXPECT_EQ(staffText->getProperty(Ms::Pid::FONT_STYLE), PropertyValue::fromValue(static_cast<int>(FontStyle::Bold)));
    staffText->cursor()->movePosition(TextCursor::MoveOperation::NextWord, TextCursor::MoveMode::KeepAnchor);
    EXPECT_EQ(staffText->getProperty(Ms::Pid::FONT_STYLE), PropertyValue::fromValue(static_cast<int>(FontStyle::Bold)));
    staffText->endEdit(ed);
    EXPECT_EQ(staffText->getProperty(Ms::Pid::FONT_STYLE), PropertyValue::fromValue(0));
}

TEST_F(TextBaseTests, undoChangeFontStyleProperty)
{
    MasterScore* score = ScoreRW::readScore("test.mscx");
    StaffText* staffText = addStaffText(score);
    staffText->setXmlText("normal <b>bold</b> <u>underline</u> <i>italic</i>");
    staffText->layout();
    score->startCmd();
    staffText->undoChangeProperty(Ms::Pid::FONT_STYLE, PropertyValue::fromValue(0), PropertyFlags::UNSTYLED);
    score->endCmd();
    EXPECT_EQ(staffText->xmlText(), "normal <b>bold</b> <u>underline</u> <i>italic</i>");
    score->startCmd();
    staffText->undoChangeProperty(Ms::Pid::FONT_STYLE, PropertyValue::fromValue(static_cast<int>(FontStyle::Bold)),
                                  PropertyFlags::UNSTYLED);
    score->endCmd();
    EXPECT_EQ(staffText->xmlText(), "<b>normal bold <u>underline</u> <i>italic</i></b>");
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_EQ(staffText->xmlText(), "normal <b>bold</b> <u>underline</u> <i>italic</i>");
    score->undoStack()->redo(&ed);
    EXPECT_EQ(staffText->xmlText(), "<b>normal bold <u>underline</u> <i>italic</i></b>");
    score->startCmd();
    staffText->undoChangeProperty(Ms::Pid::FONT_STYLE, PropertyValue::fromValue(
                                      static_cast<int>(FontStyle::Italic + FontStyle::Bold)), PropertyFlags::UNSTYLED);
    score->endCmd();
    EXPECT_EQ(staffText->xmlText(), "<b><i>normal bold <u>underline</u> italic</i></b>");
    score->startCmd();
    staffText->undoChangeProperty(Ms::Pid::FONT_STYLE,
                                  PropertyValue::fromValue(
                                      static_cast<int>(FontStyle::Italic + FontStyle::Bold + FontStyle::Underline)),
                                  PropertyFlags::UNSTYLED);
    score->endCmd();
    EXPECT_EQ(staffText->xmlText(), "<b><i><u>normal bold underline italic</u></i></b>");
    score->startCmd();
    staffText->undoChangeProperty(Ms::Pid::FONT_STYLE, PropertyValue::fromValue(0), PropertyFlags::UNSTYLED);
    score->endCmd();
    EXPECT_EQ(staffText->xmlText(), "normal bold underline italic");
}

TEST_F(TextBaseTests, musicalSymbolsNotBold)
{
    MasterScore* score = ScoreRW::readScore("test.mscx");
    StaffText* staffText = addStaffText(score);
    staffText->setXmlText("<b>Allegro <sym>metNoteQuarterUp</sym> = 120</b>");
    staffText->layout();
    auto fragmentList = staffText->fragmentList();
    EXPECT_TRUE(fragmentList[0].font(staffText).bold());
    EXPECT_TRUE(!fragmentList[1].font(staffText).bold());
}

TEST_F(TextBaseTests, musicalSymbolsNotItalic)
{
    MasterScore* score = ScoreRW::readScore("test.mscx");
    Dynamic* dynamic = addDynamic(score);
    dynamic->setXmlText("molto <sym>dynamicForte</sym>");
    dynamic->layout();
    auto fragmentList = dynamic->fragmentList();
    EXPECT_TRUE(fragmentList[0].font(dynamic).italic());
    EXPECT_TRUE(!fragmentList[1].font(dynamic).italic());
}
