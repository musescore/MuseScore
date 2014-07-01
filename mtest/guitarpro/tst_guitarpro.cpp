//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "libmscore/score.h"
#include "mscore/preferences.h"
#include "libmscore/excerpt.h"

#define DIR QString("guitarpro/")

using namespace Ms;

//---------------------------------------------------------
//   TestGuitarPro
//---------------------------------------------------------

class TestGuitarPro : public QObject, public MTest
      {
      Q_OBJECT

      void gpReadTest(const char* file,  const char* ext);

private slots:
      void initTestCase();
      void gpTestIrrTuplet() { gpReadTest("testIrrTuplet", "gp4"); }
      void gpSlur()          { gpReadTest("slur", "gp4"); }
      void gpSforzato()      { gpReadTest("sforzato", "gp4"); }
      void gpHeavyAccent()   { gpReadTest("heavy-accent", "gp5"); }
      void gpTremolos()      { gpReadTest("tremolos", "gp5"); }
      void gpTrill()         { gpReadTest("trill", "gp4"); }
      void gpDynamic()       { gpReadTest("dynamic", "gp5"); }
      void gpArpeggio()      { gpReadTest("arpeggio_up_down", "gp4"); }
      void gpGhostNote()     { gpReadTest("ghost_note", "gp3"); }
      void gpGraceNote()     { /*gpReadTest("grace", "gp5");*/ } // doesn't work
      void gp5Volta()        { gpReadTest("volta", "gp5"); }
      void gp4Volta()        { gpReadTest("volta", "gp4"); }
      void gp3Volta()        { gpReadTest("volta", "gp3"); }
      void gp5copyright()    { gpReadTest("copyright", "gp5"); }
      void gp4copyright()    { gpReadTest("copyright", "gp4"); }
      void gp3copyright()    { gpReadTest("copyright", "gp3"); }
      void gp5Tempo()        { gpReadTest("tempo", "gp5"); }
      void gp4Tempo()        { gpReadTest("tempo", "gp4"); }
      void gp3Tempo()        { gpReadTest("tempo", "gp3"); }
      void gp5BasicBend()    { gpReadTest("basic-bend", "gp5"); }
      void gp5Bend()         { gpReadTest("bend", "gp5"); }
      void gp4Bend()         { gpReadTest("bend", "gp4"); }
      void gp3Bend()         { gpReadTest("bend", "gp3"); }
      void gp5Keysig()       { gpReadTest("keysig", "gp5"); }
      void gp4Keysig()       { gpReadTest("keysig", "gp4"); }
      void gpDottedTuplets() { gpReadTest("dotted-tuplets", "gp5"); }
      void gpTupletSlur()    { gpReadTest("tuplet-with-slur", "gp4"); }
      void gpBeamsStemsLL()  { gpReadTest("beams-stems-ledger-lines", "gp5"); }
      void gpxFretDiagram()  { gpReadTest("fret-diagram", "gpx"); }
      void gp5FretDiagram()  { gpReadTest("fret-diagram", "gp5"); }
      void gp4FretDiagram()  { gpReadTest("fret-diagram", "gp4"); }
      void gp5FadeIn()       { gpReadTest("fade-in", "gp5"); }
      void gp4FadeIn()       { gpReadTest("fade-in", "gp4"); }
      void gpSlurNoteMask()  { gpReadTest("slur-notes-effect-mask", "gp5"); }
      void gpxCentered()     { gpReadTest("rest-centered", "gpx"); }
      void gp5Centered()     { gpReadTest("rest-centered", "gp5"); }
      void gp4Centered()     { gpReadTest("rest-centered", "gp4"); }
      void gp4SlideInAbove() { gpReadTest("slide-in-above", "gp4"); }
      void gp5SlideInAbove() { gpReadTest("slide-in-above", "gp5"); }
      void gpxSlideInAbove() { gpReadTest("slide-in-above", "gpx"); }
      void gp4SlideInBelow() { gpReadTest("slide-in-below", "gp4"); }
      void gp5SlideInBelow() { gpReadTest("slide-in-below", "gp5"); }
      void gpxSlideInBelow() { gpReadTest("slide-in-below", "gpx"); }
      void gp4SlideOutUp()   { gpReadTest("slide-out-up", "gp4"); }
      void gp5SlideOutUp()   { gpReadTest("slide-out-up", "gp5"); }
      void gpxSlideOutUp()   { gpReadTest("slide-out-up", "gpx"); }
      void gp4SlideOutDown() { gpReadTest("slide-out-down", "gp4"); }
      void gp5SlideOutDown() { gpReadTest("slide-out-down", "gp5"); }
      void gpxSlideOutDown() { gpReadTest("slide-out-down", "gpx"); }
      void gp4LegatoSlide()  { gpReadTest("legato-slide", "gp4"); }
      void gp5LegatoSlide()  { gpReadTest("legato-slide", "gp5"); }
      void gpxLegatoSlide()  { gpReadTest("legato-slide", "gpx"); }
      void gp4ShiftSlide()   { gpReadTest("shift-slide", "gp4"); }
      void gp5ShiftSlide()   { gpReadTest("shift-slide", "gp5"); }
      void gpxShiftSlide()   { gpReadTest("shift-slide", "gpx"); }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestGuitarPro::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   gpReadTest
//   read a Capella file, write to a MuseScore file and verify against reference
//---------------------------------------------------------

void TestGuitarPro::gpReadTest(const char* file, const char* ext)
      {
      Score* score = readScore(DIR + file + "." + ext);
      QVERIFY(score);

      score->doLayout();
      QVERIFY(saveCompareScore(score, QString("%1.%2.mscx").arg(file).arg(ext),
                               DIR + QString("%1.%2-ref.mscx").arg(file).arg(ext)));
      delete score;
      }

QTEST_MAIN(TestGuitarPro)
#include "tst_guitarpro.moc"
