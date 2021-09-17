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

#include "testing/qtestsuite.h"
#include "testbase.h"
#include "libmscore/masterscore.h"
#include "libmscore/segment.h"
#include "libmscore/dynamic.h"
#include "libmscore/stafftext.h"
#include "libmscore/chordrest.h"
#include "libmscore/textedit.h"
#include "libmscore/tie.h"

using namespace Ms;

static const QString TEXTBASE_DATA_DIR("textbase_data/");

//---------------------------------------------------------
//   TestElement
//---------------------------------------------------------

class TestTextBase : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase() { initMTest(); }
    void createDynamic();
    void dynamicAddTextBefore();
    void dynamicAddTextAfter();
    void dynamicAddTextNoItalic();
    void getFontStyleProperty();
    void undoChangeFontStyleProperty();
    void musicalSymbolsNotBold();
    void musicalSymbolsNotItalic();

private:
    Dynamic* addDynamic();
    StaffText* addStaffText();
};

void TestTextBase::createDynamic()
{
    addDynamic();
    QVERIFY(saveCompareScore(score, "createDynamic.mscx", TEXTBASE_DATA_DIR + "createDynamic-ref.mscx"));
}

void TestTextBase::dynamicAddTextBefore()
{
    Dynamic* dynamic = addDynamic();
    dynamic->startEdit(ed);
    score->undo(new InsertText(dynamic->cursor(), QString("poco ")), &ed);
    dynamic->endEdit(ed);
    QVERIFY(saveCompareScore(score, "dynamicAddTextBefore.mscx", TEXTBASE_DATA_DIR + "dynamicAddTextBefore-ref.mscx"));
}

void TestTextBase::dynamicAddTextAfter()
{
    Dynamic* dynamic = addDynamic();
    dynamic->startEdit(ed);
    dynamic->cursor()->moveCursorToEnd();
    score->undo(new InsertText(dynamic->cursor(), QString(" ma non troppo")), &ed);
    dynamic->endEdit(ed);
    QVERIFY(saveCompareScore(score, "dynamicAddTextAfter.mscx", TEXTBASE_DATA_DIR + "dynamicAddTextAfter-ref.mscx"));
}

void TestTextBase::dynamicAddTextNoItalic()
{
    Dynamic* dynamic = addDynamic();
    dynamic->startEdit(ed);
    dynamic->setProperty(Ms::Pid::FONT_STYLE, QVariant::fromValue(0));
    score->undo(new InsertText(dynamic->cursor(), QString("moderately ")), &ed);
    dynamic->endEdit(ed);
    QVERIFY(saveCompareScore(score, "dynamicAddTextNoItalic.mscx", TEXTBASE_DATA_DIR + "dynamicAddTextNoItalic-ref.mscx"));
}

void TestTextBase::getFontStyleProperty()
{
    StaffText* staffText = addStaffText();
    staffText->startEdit(ed);
    score->undo(new InsertText(staffText->cursor(), QString("normal ")), &ed);
    staffText->setProperty(Ms::Pid::FONT_STYLE, QVariant::fromValue(static_cast<int>(FontStyle::Bold)));
    score->undo(new InsertText(staffText->cursor(), QString("bold")), &ed);
    staffText->cursor()->moveCursorToStart();
    QCOMPARE(staffText->getProperty(Ms::Pid::FONT_STYLE), QVariant::fromValue(0));
    staffText->cursor()->movePosition(TextCursor::MoveOperation::NextWord, TextCursor::MoveMode::KeepAnchor);
    QCOMPARE(staffText->getProperty(Ms::Pid::FONT_STYLE), QVariant::fromValue(0));
    staffText->cursor()->movePosition(TextCursor::MoveOperation::End, TextCursor::MoveMode::KeepAnchor);
    QCOMPARE(staffText->getProperty(Ms::Pid::FONT_STYLE), QVariant::fromValue(0));
    staffText->cursor()->movePosition(TextCursor::MoveOperation::WordLeft, TextCursor::MoveMode::MoveAnchor);
    QCOMPARE(staffText->getProperty(Ms::Pid::FONT_STYLE), QVariant::fromValue(static_cast<int>(FontStyle::Bold)));
    staffText->cursor()->movePosition(TextCursor::MoveOperation::NextWord, TextCursor::MoveMode::KeepAnchor);
    QCOMPARE(staffText->getProperty(Ms::Pid::FONT_STYLE), QVariant::fromValue(static_cast<int>(FontStyle::Bold)));
    staffText->endEdit(ed);
    QCOMPARE(staffText->getProperty(Ms::Pid::FONT_STYLE), QVariant::fromValue(0));
}

void TestTextBase::undoChangeFontStyleProperty()
{
    StaffText* staffText = addStaffText();
    staffText->setXmlText("normal <b>bold</b> <u>underline</u> <i>italic</i>");
    staffText->layout();
    score->startCmd();
    staffText->undoChangeProperty(Ms::Pid::FONT_STYLE, QVariant::fromValue(0), PropertyFlags::UNSTYLED);
    score->endCmd();
    QCOMPARE(staffText->xmlText(), "normal <b>bold</b> <u>underline</u> <i>italic</i>");
    score->startCmd();
    staffText->undoChangeProperty(Ms::Pid::FONT_STYLE, QVariant::fromValue(static_cast<int>(FontStyle::Bold)), PropertyFlags::UNSTYLED);
    score->endCmd();
    QCOMPARE(staffText->xmlText(), "<b>normal bold <u>underline</u> <i>italic</i></b>");
    score->undoStack()->undo(&ed);
    QCOMPARE(staffText->xmlText(), "normal <b>bold</b> <u>underline</u> <i>italic</i>");
    score->undoStack()->redo(&ed);
    QCOMPARE(staffText->xmlText(), "<b>normal bold <u>underline</u> <i>italic</i></b>");
    score->startCmd();
    staffText->undoChangeProperty(Ms::Pid::FONT_STYLE, QVariant::fromValue(
                                      static_cast<int>(FontStyle::Italic + FontStyle::Bold)), PropertyFlags::UNSTYLED);
    score->endCmd();
    QCOMPARE(staffText->xmlText(), "<b><i>normal bold <u>underline</u> italic</i></b>");
    score->startCmd();
    staffText->undoChangeProperty(Ms::Pid::FONT_STYLE,
                                  QVariant::fromValue(
                                      static_cast<int>(FontStyle::Italic + FontStyle::Bold + FontStyle::Underline)),
                                  PropertyFlags::UNSTYLED);
    score->endCmd();
    QCOMPARE(staffText->xmlText(), "<b><i><u>normal bold underline italic</u></i></b>");
    score->startCmd();
    staffText->undoChangeProperty(Ms::Pid::FONT_STYLE, QVariant::fromValue(0), PropertyFlags::UNSTYLED);
    score->endCmd();
    QCOMPARE(staffText->xmlText(), "normal bold underline italic");
}

void TestTextBase::musicalSymbolsNotBold()
{
    StaffText* staffText = addStaffText();
    staffText->setXmlText("<b>Allegro <sym>metNoteQuarterUp</sym> = 120</b>");
    staffText->layout();
    auto fragmentList = staffText->fragmentList();
    QVERIFY(fragmentList[0].font(staffText).bold());
    QVERIFY(!fragmentList[1].font(staffText).bold());
}

void TestTextBase::musicalSymbolsNotItalic()
{
    Dynamic* dynamic = addDynamic();
    dynamic->setXmlText("molto <sym>dynamicForte</sym>");
    dynamic->layout();
    auto fragmentList = dynamic->fragmentList();
    QVERIFY(fragmentList[0].font(dynamic).italic());
    QVERIFY(!fragmentList[1].font(dynamic).italic());
}

Dynamic* TestTextBase::addDynamic()
{
    score = readScore("test.mscx");
    Dynamic* dynamic = new Dynamic(score->dummy()->segment());
    dynamic->setXmlText("<sym>dynamicForte</sym>");
    ChordRest* chordRest = score->firstSegment(SegmentType::ChordRest)->nextChordRest(0);
    ed.dropElement = dynamic;
    chordRest->drop(ed);
    return dynamic;
}

StaffText* TestTextBase::addStaffText()
{
    score = readScore("test.mscx");
    StaffText* staffText = new StaffText(score->dummy()->segment());
    ChordRest* chordRest = score->firstSegment(SegmentType::ChordRest)->nextChordRest(0);
    ed.dropElement = staffText;
    chordRest->drop(ed);
    return staffText;
}

QTEST_MAIN(TestTextBase)

#include "tst_textbase.moc"
