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

#include "testing/qtestsuite.h"
#include "testutils.h"
#include "dom/text.h"
#include "dom/masterscore.h"
#include "dom/sym.h"
#include "dom/xml.h"

using namespace Ms;

//---------------------------------------------------------
//   TestText
//---------------------------------------------------------

class TestText : public QObject, public MTest
{
    Q_OBJECT

    EditData editData;

private slots:
    void initTestCase();
    void testText();
    void testSpecialSymbols();
    void testPaste();
    void testTextProperties();
    void testCompatibility();
    void testDelete();
    void testReadWrite();
    void testBasicUnicodeDeletePreviousChar();
    void testSupplementaryUnicodeDeletePreviousChar();
    void testMixedTypesDeletePreviousChar();
    void testSupplementaryUnicodeInsert1();
    void testSupplementaryUnicodeInsert2();
    void testSupplementaryUnicodePaste();
    void testRightToLeftWithSupplementaryUnicode();
    void testPasteSymbolAndSupplemental();
    void testMixedSelectionDelete();
    void testChineseBasicSupplemental();
    void testDropUnicodeAfterSMUFLwhenCursorSetToSymbol();
    void testDropBasicUnicodeWhenNotInEditMode();
    void testDropSupplementaryUnicodeWhenNotInEditMode();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestText::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   testDelete
//---------------------------------------------------------

void TestText::testDelete()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::DYNAMICS);

    text->setPlainText("aaa bbb ccc\nddd eee fff\nggg hhh iii");
    text->layout();
    QCOMPARE(text->xmlText(), QString("aaa bbb ccc\nddd eee fff\nggg hhh iii"));

    text->startEdit(editData);
    text->moveCursorToStart();
    QVERIFY(text->movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, 2));
    text->deleteSelectedText();
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("ggg hhh iii"));
}

//---------------------------------------------------------
///   testText
//---------------------------------------------------------

void TestText::testText()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::DYNAMICS);

    text->startEdit(editData);
    text->layout();

    text->moveCursorToEnd();
    text->insertText("a");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("a"));

    text->startEdit(editData);
    text->moveCursorToEnd();
    text->insertText("bc");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("abc"));

    text->startEdit(editData);
    text->moveCursorToEnd();
    text->insertText("d");
    text->insertText("e");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("abcde"));

    text->startEdit(editData);
    text->moveCursorToStart();
    text->insertText("1");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("1abcde"));

    text->startEdit(editData);
    text->moveCursorToStart();
    text->insertText("0");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("01abcde"));

    text->startEdit(editData);
    text->moveCursorToStart();
    text->movePosition(QTextCursor::Right);
    text->movePosition(QTextCursor::Right);
    text->insertText("2");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("012abcde"));

    text->startEdit(editData);
    text->moveCursorToEnd();
    text->movePosition(QTextCursor::Left);
    text->movePosition(QTextCursor::Left);
    text->movePosition(QTextCursor::Left);
    text->movePosition(QTextCursor::Left);
    text->movePosition(QTextCursor::Left);
    text->insertText("3");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("0123abcde"));

    text->startEdit(editData);
    text->moveCursorToEnd();
    text->insertSym(SymId::segno);
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("0123abcde<sym>segno</sym>"));

    text->startEdit(editData);
    text->moveCursorToEnd();
    text->movePosition(QTextCursor::Left);
    text->insertText("#");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("0123abcde#<sym>segno</sym>"));
}

//---------------------------------------------------------
///   testSpecialSymbols
//---------------------------------------------------------

void TestText::testSpecialSymbols()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::DYNAMICS);

    text->startEdit(editData);
    text->layout();

    text->moveCursorToEnd();
    text->insertText("<");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("&lt;"));

    text->selectAll();
    text->deleteSelectedText();
    text->insertText("&");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("&amp;"));

    text->selectAll();
    text->deleteSelectedText();
    text->insertText(">");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("&gt;"));

    text->selectAll();
    text->deleteSelectedText();
    text->insertText("\"");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("&quot;"));

    text->selectAll();
    text->deleteSelectedText();
    text->insertText("&gt;");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("&amp;gt;"));

    text->selectAll();
    text->deleteSelectedText();
    text->insertText("&&");
    text->moveCursorToEnd();
    text->deletePreviousChar();
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("&amp;"));
    text->deletePreviousChar();
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString(""));
}

//---------------------------------------------------------
///   testPaste
//---------------------------------------------------------

void TestText::testPaste()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::DYNAMICS);

    text->startEdit(editData);
    text->layout();
    text->moveCursorToEnd();

    text->paste(0, "copy & paste");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("copy &amp; paste"));

    text->selectAll();
    text->deleteSelectedText();
    text->startEdit(editData);
    text->layout();
    text->moveCursorToEnd();
    text->paste(0, "copy &aa paste");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("copy &amp;aa paste"));

    text->selectAll();
    text->deleteSelectedText();
    text->startEdit(editData);
    text->layout();
    text->moveCursorToEnd();
    text->paste(0, "&");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("&amp;"));

    text->selectAll();
    text->deleteSelectedText();
    text->startEdit(editData);
    text->layout();
    text->moveCursorToEnd();
    text->paste(0, "&sometext");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("&amp;sometext"));
}

//---------------------------------------------------------
///   testTextProperties
//---------------------------------------------------------

void TestText::testTextProperties()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::STAFF);

    text->startEdit(editData);
    text->layout();

    text->moveCursorToEnd();
    text->insertText("ILoveMuseScore");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("ILoveMuseScore"));

    //select Love and make it bold
    text->startEdit(editData);
    text->moveCursorToStart();
    text->movePosition(QTextCursor::Right);
    text->movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 4);

    text->setFormat(FormatId::Bold, true);
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("I<b>Love</b>MuseScore"));

    //select Love and unbold it
    text->startEdit(editData);
    text->moveCursorToStart();
    text->movePosition(QTextCursor::Right);
    text->movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 4);

    text->setFormat(FormatId::Bold, false);
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("ILoveMuseScore"));

    //select Love and make it bold again
    text->startEdit(editData);
    text->moveCursorToStart();
    text->movePosition(QTextCursor::Right);
    text->movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 4);

    text->setFormat(FormatId::Bold, true);
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("I<b>Love</b>MuseScore"));

    //select veMu and make it bold
    text->startEdit(editData);
    text->moveCursorToStart();
    text->movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 3);
    text->movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 4);

    text->setFormat(FormatId::Bold, true);
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("I<b>LoveMu</b>seScore"));

    //select Mu and make it nonbold
    text->startEdit(editData);
    text->moveCursorToStart();
    text->movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 5);
    text->movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);

    text->setFormat(FormatId::Bold, false);
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("I<b>Love</b>MuseScore"));

    //make veMuse italic
    text->startEdit(editData);
    text->moveCursorToStart();
    QVERIFY(text->movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 3));
    QVERIFY(text->movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 6));

    text->setFormat(FormatId::Italic, true);
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("I<b>Lo<i>ve</i></b><i>Muse</i>Score"));
}

//---------------------------------------------------------
///   testCompatibility
//---------------------------------------------------------

void TestText::testCompatibility()
{
    Text* text = new Text(score);
    //bold
    const QString sb(
        "<html><head><meta name=\"qrichtext\" content=\"1\" /><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><style type=\"text/css\">"
        "p, li { white-space: pre-wrap; }"
        "</style></head><body style=\" font-family:'Times New Roman'; font-size:10.0006pt; font-weight:400; font-style:normal;\">"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">bold</span></p></body></html>");
    QCOMPARE(text->HtmlParser::parse(sb), QString("<font face=\"Times New Roman\"/><b>bold</b>"));

    //italic
    const QString si(
        "<html><head><meta name=\"qrichtext\" content=\"1\" /><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><style type=\"text/css\">"
        "p, li { white-space: pre-wrap; }"
        "</style></head><body style=\" font-family:'Times New Roman'; font-size:10.0006pt; font-weight:400; font-style:normal;\">"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-style:italic;\">italic</span></p></body></html>");
    QCOMPARE(text->HtmlParser::parse(si), QString("<font face=\"Times New Roman\"/><i>italic</i>"));

    //underline
    const QString su(
        "<html><head><meta name=\"qrichtext\" content=\"1\" /><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><style type=\"text/css\">"
        "p, li { white-space: pre-wrap; }"
        "</style></head><body style=\" font-family:'Times New Roman'; font-size:10.0006pt; font-weight:400; font-style:normal;\">"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" text-decoration: underline;\">underline</span></p></body></html>");
    QCOMPARE(text->HtmlParser::parse(su), QString("<font face=\"Times New Roman\"/><u>underline</u>"));

    //bold italic underline
    const QString sbiu(
        "<html><head><meta name=\"qrichtext\" content=\"1\" /><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><style type=\"text/css\">"
        "p, li { white-space: pre-wrap; }"
        "</style></head><body style=\" font-family:'Times New Roman'; font-size:10.0006pt; font-weight:400; font-style:normal;\">"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600; font-style:italic; text-decoration: underline;\">bolditalicunderline</span></p></body></html>");
    QCOMPARE(text->HtmlParser::parse(sbiu),
             QString("<font face=\"Times New Roman\"/><b><i><u>bolditalicunderline</u></i></b>"));

    const QString sbiu2(
        "<html><head><meta name=\"qrichtext\" content=\"1\" /><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><style type=\"text/css\">"
        "p, li { white-space: pre-wrap; }"
        "</style></head><body style=\" font-family:'Times New Roman'; font-size:10.0006pt; font-weight:400; font-style:normal;\">"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">bold</span><span style=\" font-style:italic;\">italic</span><span style=\" text-decoration: underline;\">underline</span></p></body></html>");
    QCOMPARE(text->HtmlParser::parse(sbiu2),
             QString("<font face=\"Times New Roman\"/><b>bold</b><i>italic</i><u>underline</u>"));

    const QString sbi(
        "<html><head><meta name=\"qrichtext\" content=\"1\" /><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><style type=\"text/css\">"
        "p, li { white-space: pre-wrap; }"
        "</style></head><body style=\" font-family:'Times New Roman'; font-size:10.0006pt; font-weight:400; font-style:normal;\">"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">bo</span><span style=\" font-weight:600; font-style:italic; text-decoration: underline;\">ldit</span>alic</p></body></html>");
    QCOMPARE(text->HtmlParser::parse(sbi),
             QString("<font face=\"Times New Roman\"/><b>bo</b><b><i><u>ldit</u></i></b>alic"));

    const QString sescape(
        "<html><head><meta name=\"qrichtext\" content=\"1\" /><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><style type=\"text/css\">"
        "p, li { white-space: pre-wrap; }"
        "</style></head><body style=\" font-family:'Times New Roman'; font-size:10.0006pt; font-weight:400; font-style:normal;\">"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">test&amp;&lt;&gt;&quot;'</p></body></html>");
    QCOMPARE(text->HtmlParser::parse(sescape), QString("<font face=\"Times New Roman\"/>test&amp;&lt;&gt;&quot;'"));
}

//---------------------------------------------------------
///   testReadWrite
//---------------------------------------------------------

void TestText::testReadWrite()
{
    auto testrw = [](Score* score, Text* t) {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        XmlWriter xml(score, &buffer);
        t->write(xml);
        buffer.close();

        XmlReader e(score, buffer.buffer());
        Text* text2 = new Text(score);
        e.readNextStartElement();
        text2->read(e);
        QCOMPARE(t->xmlText(), text2->xmlText());
    };
    Text* text = new Text(score);
    text->setXmlText("test");
    testrw(score, text);

    text = new Text(score);
    text->setXmlText("<b>Title</b><i>two</i>");
    testrw(score, text);

    text = new Text(score);
    text->setXmlText("<i>Title</i> <b>Two</b>");
    testrw(score, text);

    text = new Text(score);
    text->setXmlText("<i>Title</i>    <b>Two</b>");
    testrw(score, text);

    text = new Text(score);
    text->setXmlText("<i>Title</i>\t<b>Two</b>");
    testrw(score, text);

    text = new Text(score);
    text->setXmlText("<i>Title</i>\n<b>Two</b>");
    testrw(score, text);

    text = new Text(score);
    text->setXmlText("<i>Ti  tle</i><b>Tw  o</b>");
    testrw(score, text);
}

//---------------------------------------------------------
///   testBasicUnicodeDeletePreviousChar
///    text contains Basic Unicode symbols
//---------------------------------------------------------

void TestText::testBasicUnicodeDeletePreviousChar()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::DYNAMICS);

    text->setPlainText(QString("⟁⟂⟃⟄"));

    text->layout();
    text->startEdit(editData);
    text->moveCursorToEnd();
    text->deletePreviousChar();
    text->endEdit(editData);

    QCOMPARE(text->xmlText(), QString("⟁⟂⟃"));
}

//---------------------------------------------------------
///   testSupplementaryUnicodeDeletePreviousChar
///    text contains Supplementary Unicode symbols which store chars in pairs
//---------------------------------------------------------

void TestText::testSupplementaryUnicodeDeletePreviousChar()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::DYNAMICS);

    text->setPlainText(QString("𝄆𝄆𝄆𝄏𝄏𝄏"));

    text->layout();
    text->startEdit(editData);
    text->moveCursorToEnd();
    text->deletePreviousChar();
    text->endEdit(editData);

    QCOMPARE(text->xmlText(), QString("𝄆𝄆𝄆𝄏𝄏"));
}

//---------------------------------------------------------
///   testMixedTypesDeletePreviousChar
///    text contains unicode symbols from both Basic and Supplementary Multilingual Plane chars and SMUFL symbols
//---------------------------------------------------------

void TestText::testMixedTypesDeletePreviousChar()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::DYNAMICS);

    text->setXmlText(
        "<sym>cClefSquare</sym>𝄆<sym>repeatLeft</sym><sym>textBlackNoteLongStem</sym><sym>textBlackNoteLongStem</sym><sym>noteheadWhole</sym> ⟂<sym>repeatRight</sym> 𝄇");
    text->layout();
    text->startEdit(editData);
    text->moveCursorToEnd();
    text->deletePreviousChar();
    text->deletePreviousChar();
    text->deletePreviousChar();
    text->deletePreviousChar();
    text->deletePreviousChar();
    text->deletePreviousChar();
    text->endEdit(editData);
    QCOMPARE(text->xmlText(),
             QString(
                 "<sym>cClefSquare</sym>𝄆<sym>repeatLeft</sym><sym>textBlackNoteLongStem</sym><sym>textBlackNoteLongStem</sym>"));
}

//---------------------------------------------------------
///   testSupplementaryUnicodeInsert1
///    Insert a Supplementary Multilingual Plane unicode symbol behind another one.
//---------------------------------------------------------

void TestText::testSupplementaryUnicodeInsert1()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::DYNAMICS);
    text->setPlainText(QString("𝄏"));
    text->layout();
    text->startEdit(editData);
    text->moveCursorToStart();
    text->insertText(QString("𝄆"));
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("𝄆𝄏"));
}

//---------------------------------------------------------
///   testSupplementaryUnicodeInsert2
///    Insert a Supplementary Multilingual Plane unicode symbol behind another one.
//---------------------------------------------------------

void TestText::testSupplementaryUnicodeInsert2()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::DYNAMICS);
    text->layout();
    text->startEdit(editData);
    text->moveCursorToStart();
    text->insertText(QString("𝄏"));
    text->moveCursorToStart();
    text->insertText(QString("𝄆"));
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("𝄆𝄏"));
}

//---------------------------------------------------------
///   testSupplementaryUnicodePaste
///    Paste a Supplementary Plane unicode symbols.
//---------------------------------------------------------

void TestText::testSupplementaryUnicodePaste()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::DYNAMICS);
    text->setPlainText(QString(""));
    text->layout();

    text->startEdit(editData);
    text->moveCursorToStart();
    text->paste(0, "𝄏");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("𝄏"));

    text->startEdit(editData);
    text->moveCursorToStart();
    text->paste(0, "𝄏");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("𝄏𝄏"));

    text->startEdit(editData);
    text->moveCursorToEnd();
    text->paste(0, "𝄏");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("𝄏𝄏𝄏"));
}

//---------------------------------------------------------
///   testRightToLeftWithSupplementaryUnicode
//---------------------------------------------------------

void TestText::testRightToLeftWithSupplementaryUnicode()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::DYNAMICS);
    text->setPlainText(QString(""));
    text->layout();

    text->startEdit(editData);
    text->moveCursorToStart();
    text->insertText(QString("𝄆"));
    text->insertText(QString("م"));
    text->insertText(QString("و"));
    text->insertText(QString("س"));
    text->insertText(QString("ي"));
    text->insertText(QString("ق"));
    text->insertText(QString("ى"));
    text->insertText(QString("𝄇"));
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("𝄆موسيقى𝄇"));

    text->startEdit(editData);
    text->cursor()->setColumn(1);
    text->deletePreviousChar();
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("موسيقى𝄇"));

    text->startEdit(editData);
    text->cursor()->setColumn(5);
    text->deleteChar();
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("موسيق𝄇"));
}

//---------------------------------------------------------
///   testPasteSymbolAndSupplemental
//---------------------------------------------------------

void TestText::testPasteSymbolAndSupplemental()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::DYNAMICS);
    text->setPlainText(QString(""));
    text->layout();

    text->startEdit(editData);
    text->moveCursorToStart();
    text->paste(0, "<sym>gClef</sym>𝄎");
    text->endEdit(editData);
    QVERIFY(text->fragmentList()[0].format.type() == CharFormatType::SYMBOL);
    QVERIFY(text->fragmentList()[1].format.type() == CharFormatType::TEXT);
    QCOMPARE(text->xmlText(), QString("<sym>gClef</sym>𝄎"));

    text->startEdit(editData);
    text->moveCursorToStart();
    text->insertText(QString("𝄎"));
    text->endEdit(editData);
    QVERIFY(text->fragmentList()[0].format.type() == CharFormatType::TEXT);
    QVERIFY(text->fragmentList()[1].format.type() == CharFormatType::SYMBOL);
    QVERIFY(text->fragmentList()[2].format.type() == CharFormatType::TEXT);
    QCOMPARE(text->xmlText(), QString("𝄎<sym>gClef</sym>𝄎"));
}

//---------------------------------------------------------
///   testMixedSelectionDelete
//---------------------------------------------------------

void TestText::testMixedSelectionDelete()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::DYNAMICS);
    text->layout();

    text->startEdit(editData);
    text->moveCursorToStart();
    text->paste(0, "[A]𝄎<sym>gClef</sym> 𝄎𝄇");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("[A]𝄎<sym>gClef</sym> 𝄎𝄇"));

    text->startEdit(editData);
    text->moveCursorToStart();
    text->cursor()->setSelectColumn(4);
    text->cursor()->setColumn(7);
    text->deleteSelectedText();
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("[A]𝄎𝄇"));

    text->startEdit(editData);
    text->cursor()->setColumn(4);
    text->deletePreviousChar();
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("[A]𝄇"));

    text->startEdit(editData);
    text->moveCursorToEnd();
    text->insertSym(SymId::segno);
    text->endEdit(editData);
    text->insertText(QString("e"));
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("[A]𝄇<sym>segno</sym>e"));
}

//---------------------------------------------------------
///   testChineseBasicSupplemental
//---------------------------------------------------------

void TestText::testChineseBasicSupplemental()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::DYNAMICS);
    text->setPlainText(QString(""));
    text->layout();

    text->startEdit(editData);
    text->moveCursorToStart();
    text->insertText(QString("你"));    // this is supplemental unicode
    text->insertText(QString("好"));    // this is basic unicode
    text->insertText(QString("。"));

    text->paste(0, "我爱Musescore");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("你好。我爱Musescore"));

    text->startEdit(editData);
    text->moveCursorToStart();
    text->paste(0, "你屠槪真軔");
    text->moveCursorToEnd();
    text->paste(0, "你屠槪真軔");
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("你屠槪真軔你好。我爱Musescore你屠槪真軔"));

    text->startEdit(editData);
    text->cursor()->setSelectColumn(4);
    text->cursor()->setColumn(20);
    text->deleteSelectedText();
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("你屠槪真屠槪真軔"));   // this is only supplemental

    text->startEdit(editData);
    text->cursor()->setColumn(4);
    text->deleteChar();
    text->deletePreviousChar();
    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("你屠槪槪真軔"));   // deleted the two chars in the middle
}

//---------------------------------------------------------
///   testDropUnicodeAfterSMUFLwhenCursorSetToSymbol
///     Tests dropping unicode after SMUFL as described in https://github.com/musescore/MuseScore/pull/3020#issuecomment-281932322
///     When appending text after a symbol, TextBlock needs to always (regardless of the state of the cursor) append a new TEXT TextFragment after the SYMBOL TextFragment.
//---------------------------------------------------------

void TestText::testDropUnicodeAfterSMUFLwhenCursorSetToSymbol()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::DYNAMICS);
    text->setPlainText(QString(""));
    text->layout();
    text->startEdit(editData);

    Symbol* symbolSMUFL = new Symbol(score);   // create a new element, as Measure::drop() will eventually delete it
    symbolSMUFL->setSym(SymId::noteheadWhole);

    EditData dropSMUFL;
    dropSMUFL.dropElement = symbolSMUFL;
    text->drop(dropSMUFL);

    // the bug happened when cursor is in symbol mode
    CharFormat* cf = text->cursor()->format();
    cf->setType(CharFormatType::SYMBOL);

    EditData dropFSymbol;
    FSymbol* fsymbol = new FSymbol(score);
    fsymbol->setCode(0x0001D10E);   // unicode hex code for '𝄎'
    dropFSymbol.dropElement = fsymbol;
    text->drop(dropFSymbol);

    text->endEdit(editData);
    QCOMPARE(text->xmlText(), QString("<sym>noteheadWhole</sym>𝄎"));
}

//---------------------------------------------------------
///   testDropBasicUnicodeWhenNotInEditMode
///     Simple test dropping basic unicode, but exercising the path when edit mode not already engaged
//---------------------------------------------------------

void TestText::testDropBasicUnicodeWhenNotInEditMode()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::DYNAMICS);
    text->setPlainText(QString(""));
    text->layout();

    EditData dropFSymbol;
    FSymbol* fsymbol = new FSymbol(score);
    fsymbol->setCode(0x4D);   // Basic Unicode code for 'M'
    dropFSymbol.dropElement = fsymbol;
    text->drop(dropFSymbol);

    QCOMPARE(text->xmlText(), QString("M"));
}

//---------------------------------------------------------
///   testDropSupplementaryUnicodeWhenNotInEditMode
///     Simple test dropping supplementary unicode, but exercising the path when edit mode not already engaged
//---------------------------------------------------------

void TestText::testDropSupplementaryUnicodeWhenNotInEditMode()
{
    Text* text = new Text(score);
    text->initSubStyle(SubStyle::DYNAMICS);
    text->setPlainText(QString(""));
    text->layout();

    EditData dropFSymbol;
    FSymbol* fsymbol = new FSymbol(score);
    fsymbol->setCode(0x0001D10E);   // Supplementary Unicode code for '𝄎'
    dropFSymbol.dropElement = fsymbol;
    text->drop(dropFSymbol);

    QCOMPARE(text->xmlText(), QString("𝄎"));
}

QTEST_MAIN(TestText)

#include "tst_text.moc"
