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

#define DIR QString("libmscore/compat300/")

using namespace Ms;

//---------------------------------------------------------
//   TestCompat300
//---------------------------------------------------------

class TestCompat300 : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void compat300(const QString&);
      void accidentals()      { compat300("accidentals");      }
      void ambitus()          { compat300("ambitus");          }
      void articulations()    { compat300("articulations");    }
      void breath()           { compat300("breath");           }
      void clefs()            { compat300("clefs");            }
      void drumset()          { compat300("drumset");          }
      void markers()          { compat300("markers");          }
      void noteheads()        { compat300("noteheads");        }
      void textstyles()       { compat300("textstyles");       }
      void tuplets()          { compat300("tuplets");          }
      void hairpin()          { compat300("hairpin");          }
      void brlines()          { compat300("barlines");         }
      void grace()            { compat300("grace");            }
      void irregular()        { compat300("irregular");        }
      void parts()            { compat300("parts");            }
      void linkNotMaster()    { compat300("link-not-master");  }
//failed: different beam      void moonlight()        { compat300("moonlight");        }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestCompat300::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   compat300
//---------------------------------------------------------

void TestCompat300::compat300(const QString& file)
      {
      QString src(DIR + file + ".mscx");
      QString reference(DIR + file + "-ref.mscx");
      QString test(file + "-test.mscx");

      MasterScore* score = readScore(src);
      QVERIFY(score);
      score->doLayout();
      QFileInfo fiTest(test);
      QVERIFY(score->Score::saveFile(fiTest));

      QVERIFY(compareFiles(test, reference));
      }

QTEST_MAIN(TestCompat300)
#include "tst_compat300.moc"

