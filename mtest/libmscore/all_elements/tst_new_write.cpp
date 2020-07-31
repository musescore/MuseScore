//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "libmscore/score.h"

#define DIR QString("libmscore/all_elements/")

using namespace Ms;

//---------------------------------------------------------
//   TestNewWrite
//---------------------------------------------------------

class TestNewWrite : public QObject, public MTest
{
    Q_OBJECT

    void tstTree(QString file);
    void traverseTree(ScoreElement* element);

    QString elementToText(ScoreElement* element);

private slots:
    void initTestCase();
    // void tstTreeMoonlight() { tstTree("moonlight.mscx"); }
    void tstTreeElements() { tstTree("layout_elements.mscx"); }
    // void tstTreeTablature() { tstTree("layout_elements_tab.mscx"); }
    // void tstTreeGoldberg()  { tstTree("goldberg.mscx");            }
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestNewWrite::initTestCase()
{
    initMTest();
}

//---------------------------------------------------------
//   tstTree
//---------------------------------------------------------

static void saveScoreNew(Score* s, QString name);

void TestNewWrite::tstTree(QString file)
{
    MasterScore* score = readScore(DIR + file);
    saveScoreNew(score, file + ".new");
    QVERIFY(compareFiles(file + ".new", DIR + file));
}

static void saveScoreNew(Score* s, QString name)
{
    QFile fp(name);
    fp.open(QIODevice::WriteOnly);
    XmlWriter xml(s, &fp);
    s->treeWrite(xml);
    fp.close();
}

QTEST_MAIN(TestNewWrite)
#include "tst_new_write.moc"
