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

#define DIR QString("libmscore/noregress300old/")

using namespace Ms;

//---------------------------------------------------------
//   TestNoRegress300old
//---------------------------------------------------------

class TestNoRegress300old : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void noregress(const QString&);
      void accidentals()      { noregress("accidentals");      }
      void ambitus()          { noregress("ambitus");          }
      void articulations()    { noregress("articulations");    }
      void breath()           { noregress("breath");           }
      void clefs()            { noregress("clefs");            }
      void drumset()          { noregress("drumset");          }
      void markers()          { noregress("markers");          }
      void noteheads()        { noregress("noteheads");        }
      void textstyles()       { noregress("textstyles");       }
      void tuplets()          { noregress("tuplets");          }
      void hairpin()          { noregress("hairpin");          }
      void brlines()          { noregress("barlines");         }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestNoRegress300old::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   noregress
//   Converts the old-formatted file to the new-formatted
//   one and back again, then compares the result with the
//   original file. Such a test should track:
//   1) compatibility with the old 3.00 format flavor;
//   2) equality of both format flavors (i.e. no
//      information is lost during the conversion).
//---------------------------------------------------------

void TestNoRegress300old::noregress(const QString& file)
      {
      QString reference(DIR + file + "-ref.mscx");
      QString writeFileNewFormat(file + "-test-new.mscx");
      QString writeFileOldFormat(file + "-test-old.mscx");

      MasterScore* score = readScore(reference);
      QVERIFY(score);
      score->doLayout();
      QFileInfo fiNewFmt(writeFileNewFormat);
      QVERIFY(score->Score::saveFile(fiNewFmt, /* oldFormat */ false));

      MasterScore* scoreNewFmt = readCreatedScore(writeFileNewFormat);
      QVERIFY(scoreNewFmt);
      scoreNewFmt->doLayout();
      QFileInfo fiOldFmt(writeFileOldFormat);
      QVERIFY(scoreNewFmt->Score::saveFile(fiOldFmt, /* oldFormat */ true));

      QVERIFY(compareFiles(writeFileOldFormat, reference));
      }

QTEST_MAIN(TestNoRegress300old)
#include "tst_noregress300old.moc"

