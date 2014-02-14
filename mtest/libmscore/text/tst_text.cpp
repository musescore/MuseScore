//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>

#include "libmscore/text.h"
#include "libmscore/score.h"
#include "mtest/testutils.h"

using namespace Ms;

//---------------------------------------------------------
//   TestNote
//---------------------------------------------------------

class TestText : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void testText();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestText::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
///   testText
///   read/write test of note
//---------------------------------------------------------

void TestText::testText()
      {
      Text* text = new Text(score);
      text->setTextStyle(score->textStyle(TEXT_STYLE_DYNAMICS));

      text->setEditMode("true");
      text->layout();

      text->moveCursorToEnd();
      text->insertText("a");
      text->endEdit();
      QCOMPARE(text->text(), QString("a"));

      text->setEditMode("true");
      text->moveCursorToEnd();
      text->insertText("b");
      text->endEdit();
      QCOMPARE(text->text(), QString("ab"));
      }

QTEST_MAIN(TestText)

#include "tst_text.moc"

