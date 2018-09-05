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

#define DIR QString("libmscore/compat206/")

using namespace Ms;

//---------------------------------------------------------
//   TestCompat206
//---------------------------------------------------------

class TestCompat206 : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void compat(const QString&);
      void accidentals()      { compat("accidentals");      }
      void ambitus()          { compat("ambitus");          }
      void articulations()    { compat("articulations");    }
      void breath()           { compat("breath");           }
      void clefs()            { compat("clefs");            }
      void drumset()          { compat("drumset");          }
      void markers()          { compat("markers");          }
      void noteheads()        { compat("noteheads");        }
//TODO::ws      void textstyles()       { compat("textstyles");       }
      void tuplets()          { compat("tuplets");          }
      void hairpin()          { compat("hairpin");          }
      void brlines()          { compat("barlines");         }
      void lidEmptyText()     { compat("lidemptytext");     }
      void intrumentNameAlign() {compat("intrumentNameAlign"); }
      void fermata()          { compat("fermata");          }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestCompat206::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   compat
//---------------------------------------------------------

void TestCompat206::compat(const QString& file)
      {
      QString readFile(DIR   + file + ".mscx");
      QString writeFile(file + "-test.mscx");
      QString reference(DIR  + file + "-ref.mscx");

      MasterScore* score = readScore(readFile);
      QVERIFY(score);
      QVERIFY(saveCompareScore(score, writeFile, reference));
      }

QTEST_MAIN(TestCompat206)
#include "tst_compat206.moc"

