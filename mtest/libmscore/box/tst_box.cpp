//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "libmscore/score.h"
#include "libmscore/system.h"
#include "libmscore/undo.h"
#include "libmscore/box.h"

#define DIR QString("libmscore/box/")

using namespace Ms;

//---------------------------------------------------------
//   TestBox
//---------------------------------------------------------

class TestBox : public QObject, public MTest
{
    Q_OBJECT

private slots:
    void initTestCase();
    void undoRemoveVBox();
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestBox::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   undoRemoveVBox
///   read a file with a vbox. Delete it, and undo. Check that the VBox still exists.
//---------------------------------------------------------

void TestBox::undoRemoveVBox()
{
    QString readFile(DIR + "undoRemoveVBox.mscx");
    QString writeFile1("undoRemoveVBox1-test.mscx");
    QString reference1(DIR + "undoRemoveVBox1-ref.mscx");
    QString writeFile2("undoRemoveVBox2-test.mscx");
    QString reference2(DIR + "undoRemoveVBox2-ref.mscx");

    MasterScore* score = readScore(readFile);
    score->doLayout();

    System* s = score->systems()[0];
    VBox* box = toVBox(s->measure(0));

    score->startCmd();
    score->select(box);
    score->cmdDeleteSelection();
    score->endCmd();
    QVERIFY(saveCompareScore(score, writeFile1, reference1));

    // undo
    score->undoStack()->undo(nullptr);
    QVERIFY(saveCompareScore(score, writeFile2, reference2));

    delete score;
}

QTEST_MAIN(TestBox)
#include "tst_box.moc"
