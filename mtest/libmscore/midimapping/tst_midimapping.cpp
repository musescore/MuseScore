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
#include "mtest/testutils.h"
#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/staff.h"

#define DIR QString("libmscore/midimapping/")

using namespace Ms;

//---------------------------------------------------------
//   TestMidiMapping
//---------------------------------------------------------

class TestMidiMapping : public QObject, public MTest
      {
      Q_OBJECT
        
      void testReadWrite(const char* f1);
      void testReadChangeWrite(const char* f1, const char* ref, int page);
      void testReadChangeWrite2(const char* f1, const char* ref);
      void testReadChangeWrite3(const char* f1, const char* ref, int p1, int p2);
      void testCreateWrite(const char* f1, const char* ref);
      
   private slots:
      void initTestCase();
      void midiMapping1() { testReadWrite("test1withDrums.mscx"); }    // No channels => no channels
      void midiMapping2() { testReadWrite("test1withoutDrums.mscx"); } // No channels => no channels
      void midiMapping3() { testReadWrite("test2.mscx"); }             // Mapping => mapping
      // with Instrument Change elements
      void midiMapping4() { testReadWrite("test3withMapping.mscx"); }  // Mapping => mapping
      void midiMapping5() { testReadWrite("test3withoutMapping.mscx"); }  // No channels => No channels
      // Delete first part
      void midiMapping6() { testReadChangeWrite("test1withDrums", "test6-ref.mscx", 0); }    // No channels => Mapping
      // Delete part #13
      void midiMapping7() { testReadChangeWrite2("test3withMapping", "test7-ref.mscx"); }    // Mapping => No channels
      // Swap two parts
      void midiMapping8() { testReadChangeWrite3("test2", "test8-ref.mscx", 0, 1); }          // Mapping => No channels
      void midiMapping9() { testReadChangeWrite3("test1withDrums", "test9-ref.mscx", 1, 3); } // No channels => Mapping

      
      /*void midiMapping2() { testReadWrite("midiMapping02.mscx", "midiMapping02-ref.mscx");} // Channels => channels
      void midiMapping3() { testReadChangeWrite("midiMapping03.mscx", "midiMapping03-ref.mscx");} // No channels => channels
      void midiMapping4() { testReadChangeWrite("midiMapping04.mscx", "midiMapping04-ref.mscx");} // Channels => no channels
      // Instr changes
      void midiMapping5() { testReadChangeWrite("midiMapping05.mscx", "midiMapping05-ref.mscx");} // Channels => no channels
      void midiMapping6() { testCreateWrite("midiMapping06-ref.mscx");} //  => no channels
      void midiMapping7() { testCreateWrite("midiMapping07-ref.mscx");} //  => channels
      */
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestMidiMapping::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   testReadWrite
//---------------------------------------------------------

void TestMidiMapping::testReadWrite(const char* f1)
    {
    Score* score = readScore(DIR + f1);
    score->doLayout();
    QVERIFY(score);
    qDebug()<<"score staves size:"<<score->staves().size();

    score->rebuildMidiMapping();
    qDebug()<<"2 score staves size:"<<score->staves().size();

// test # 1, 3, 
    QVERIFY(saveCompareScore(score, f1, DIR + QString(f1)));
    delete score;
    }
    
//---------------------------------------------------------
//   testReadChangeWrite
//---------------------------------------------------------

void TestMidiMapping::testReadChangeWrite(const char* f1, const char* ref, int page)
    {
    Score* score = readScore(DIR + f1 +QString(".mscx"));
    score->doLayout();
    QVERIFY(score);
    score->rebuildMidiMapping();
    score->cmdRemovePart(score->parts()[page]);
    score->rebuildMidiMapping();
    QVERIFY(saveCompareScore(score, f1+QString("_changed.mscx"), DIR + ref));
    delete score;
    }

void TestMidiMapping::testReadChangeWrite2(const char* f1, const char* ref)
    {
    Score* score = readScore(DIR + f1 +QString(".mscx"));
    score->doLayout();
    QVERIFY(score);
    score->rebuildMidiMapping();
    MeasureBase* mb = score->measures()->last();
    while (mb && mb->type() != Element::Type::MEASURE)
          mb = mb->prev();
    score->deleteItem(static_cast<Measure*>(mb));
    score->rebuildMidiMapping();
    QVERIFY(saveCompareScore(score, f1+QString("_changed.mscx"), DIR + ref));
    delete score;
    }

void TestMidiMapping::testReadChangeWrite3(const char* f1, const char* ref, int p1, int p2)
    {
    Score* score = readScore(DIR + f1+QString(".mscx"));
    score->doLayout();
    QVERIFY(score);
    score->rebuildMidiMapping();

    QList<int> dl;

    for(Staff* staff : score->staves()) {
        int idx = score->staves().indexOf(staff);
        if ((idx == p1 || idx == p2) &&
                    ((idx != 0 && staff->part() == score->staves()[idx-1]->part())
                     || (idx != score->nstaves() && staff->part() == score->staves()[idx+1]->part())))
            qDebug()<<"You're probably trying to swap a part with several staves. This can lead to wrong results!";

          if (idx != -1)
                dl.push_back(idx);
          }
    dl.swap(p1, p2);

    score->sortStaves(dl);
    score->rebuildMidiMapping();
    QVERIFY(saveCompareScore(score, f1+QString("_changed3.mscx"), DIR + ref));
    delete score;
    }
    
//---------------------------------------------------------
//   testCreateWrite
//---------------------------------------------------------

void TestMidiMapping::testCreateWrite(const char* f1, const char* ref)
    {
    Score* score = new Score();
    score->doLayout();
    QVERIFY(score);
    score->rebuildMidiMapping();
    
    // Add some elements, for test #6
    QVERIFY(saveCompareScore(score, f1, DIR + ref));
    delete score;
    }

QTEST_MAIN(TestMidiMapping)
#include "tst_midimapping.moc"
