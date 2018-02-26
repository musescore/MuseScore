//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include <QVector>
#include "mtest/testutils.h"
#include "mscore/stringutils.h"

#define DIR QString("stringutils/")

using namespace Ms;

//---------------------------------------------------------
//   TestStringUtils
//---------------------------------------------------------

class TestStringUtils : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void tst_stringutils();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestStringUtils::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   stringutils
//---------------------------------------------------------

void TestStringUtils::tst_stringutils()
      {
      QString testEnglish("Test test");
      QVERIFY(stringutils::removeDiacritics(testEnglish) == testEnglish);
      QVERIFY(stringutils::removeLigatures(testEnglish) == testEnglish);

      QString testNonLatin1("Πκολο");
      QVERIFY(stringutils::removeDiacritics(testNonLatin1) == testNonLatin1);
      QVERIFY(stringutils::removeLigatures(testNonLatin1) == testNonLatin1);

      QString testNonLatin2("超倍低音长笛");
      QVERIFY(stringutils::removeDiacritics(testNonLatin2) == testNonLatin2);
      QVERIFY(stringutils::removeLigatures(testNonLatin2) == testNonLatin2);

      QString testNonLatin3("פיקולו");
      QVERIFY(stringutils::removeDiacritics(testNonLatin3) == testNonLatin3);
      QVERIFY(stringutils::removeLigatures(testNonLatin3) == testNonLatin3);

      QString testNonLatin4("پیکولو");
      QVERIFY(stringutils::removeDiacritics(testNonLatin4) == testNonLatin4);
      QVERIFY(stringutils::removeLigatures(testNonLatin4) == testNonLatin4);

      const QVector<QChar> ligatureVector({
                                          QChar(0xA732),
                                          QChar(0xA733),
                                          QChar(0x00C6),
                                          QChar(0x00C4),
                                          QChar(0x00E6),
                                          QChar(0x00E4),
                                          QChar(0xA734),
                                          QChar(0xA735),
                                          QChar(0xA736),
                                          QChar(0xA737),
                                          QChar(0x0132),
                                          QChar(0x0133),
                                          QChar(0x1E9E),
                                          QChar(0x00DF),
                                          QChar(0x00D8),
                                          QChar(0x00F8),
                                          QChar(0x0152),
                                          QChar(0x00D6),
                                          QChar(0x0153),
                                          QChar(0x00F6),
                                          QChar(0xA74E),
                                          QChar(0xA74F),
                                          QChar(0x00DC),
                                          QChar(0x00FC),
                                          QChar(0x1D6B)});
      QString testLigatures;
      for (int i = 0; i < ligatureVector.size(); i++) {
            testLigatures.append(ligatureVector.at(i));
            }
      QVERIFY(stringutils::removeLigatures(testLigatures) == QString("AaaaAeAeaeaeAoaoAuauIJijSSssOoOeOeoeoeOoooUeueue"));

      const QVector<QChar> diacriticVector({
                                           QChar(0x00E9), // acute e
                                           QChar(0x00C9), // acute E
                                           QChar(0x00E8), // grave e
                                           QChar(0x00C8), // grave E
                                           QChar(0x00EA), // circumflex e
                                           QChar(0x00CA), // circumflex E
                                           QChar(0x00E7), // cedilla c
                                           QChar(0x00C7), // cedilla C
                                           QChar(0x00F1), // tilde n
                                           QChar(0x00D1), // tilde N
                                           QChar(0x00E5), // ring a
                                           QChar(0x00C5), // ring A
                                           QChar(0x010D), // caron c
                                           QChar(0x010C)}); // caron C
      QString testDiacritics;
      for (int i = 0; i < diacriticVector.size(); i++) {
            testDiacritics.append(diacriticVector.at(i));
            }
      QVERIFY(stringutils::removeDiacritics(testDiacritics) == QString("eEeEeEcCnNaAcC"));
      }

QTEST_MAIN(TestStringUtils)
#include "tst_stringutils.moc"
