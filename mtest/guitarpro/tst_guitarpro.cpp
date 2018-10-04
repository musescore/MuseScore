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
      void gpGhostNote()     { gpReadTest("ghost_note", "gp3"); }
      void gpGraceNote()     { gpReadTest("grace", "gp5"); }
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
//      void gp5Bend()         { gpReadTest("bend", "gp5"); }
//      void gp4Bend()         { gpReadTest("bend", "gp4"); }
//      void gp3Bend()         { gpReadTest("bend", "gp3"); }
      void gp5Keysig()       { gpReadTest("keysig", "gp5"); }
      void gp4Keysig()       { gpReadTest("keysig", "gp4"); }
      void gpDottedTuplets() { gpReadTest("dotted-tuplets", "gp5"); }
//      void gpTupletSlur()    { gpReadTest("tuplet-with-slur", "gp4"); }
//      void gpBeamsStemsLL()  { gpReadTest("beams-stems-ledger-lines", "gp5"); }
//      void gpxFretDiagram_2Instr() { gpReadTest("fret-diagram_2instruments", "gpx"); }
//      void gpxFretDiagram()  { gpReadTest("fret-diagram", "gpx"); }
//      void gp5FretDiagram()  { gpReadTest("fret-diagram", "gp5"); }
//      void gp4FretDiagram()  { gpReadTest("fret-diagram", "gp4"); }
      void gp5FadeIn()       { gpReadTest("fade-in", "gp5"); }
      void gp4FadeIn()       { gpReadTest("fade-in", "gp4"); }
      void gpSlurNoteMask()  { gpReadTest("slur-notes-effect-mask", "gp5"); }
      void gpxCentered()     { gpReadTest("rest-centered", "gpx"); }
      void gp5Centered()     { gpReadTest("rest-centered", "gp5"); }
      void gp4Centered()     { gpReadTest("rest-centered", "gp4"); }
      void gp4SlideInAbove() { gpReadTest("slide-in-above", "gp4"); }
//      void gp5SlideInAbove() { gpReadTest("slide-in-above", "gp5"); }
//      void gpxSlideInAbove() { gpReadTest("slide-in-above", "gpx"); }
//      void gp4SlideInBelow() { gpReadTest("slide-in-below", "gp4"); }
//      void gp5SlideInBelow() { gpReadTest("slide-in-below", "gp5"); }
//      void gpxSlideInBelow() { gpReadTest("slide-in-below", "gpx"); }
//      void gp4SlideOutUp()   { gpReadTest("slide-out-up", "gp4"); }
//      void gp5SlideOutUp()   { gpReadTest("slide-out-up", "gp5"); }
//      void gpxSlideOutUp()   { gpReadTest("slide-out-up", "gpx"); }
//      void gp4SlideOutDown() { gpReadTest("slide-out-down", "gp4"); }
//      void gp5SlideOutDown() { gpReadTest("slide-out-down", "gp5"); }
//      void gpxSlideOutDown() { gpReadTest("slide-out-down", "gpx"); }
//      void gp4LegatoSlide()  { gpReadTest("legato-slide", "gp4"); }
//      void gp5LegatoSlide()  { gpReadTest("legato-slide", "gp5"); }
//      void gpxLegatoSlide()  { gpReadTest("legato-slide", "gpx"); }
      void gp4ShiftSlide()   { gpReadTest("shift-slide", "gp4"); }
//      void gp5ShiftSlide()   { gpReadTest("shift-slide", "gp5"); }
//      void gpxShiftSlide()   { gpReadTest("shift-slide", "gpx"); }
      void gpDoubleBar()     { gpReadTest("double-bar", "gpx"); }
      void gpxTrill()        { gpReadTest("trill", "gpx"); }
      void gpxCrecDim()      { gpReadTest("crescendo-diminuendo", "gpx"); }
      void gpxTremolos()     { gpReadTest("tremolos", "gpx"); }
      void gpxDeadNote()     { gpReadTest("dead-note", "gpx"); }
      void gpxWah()          { gpReadTest("wah", "gpx"); }
      void gpxSforzato()     { gpReadTest("accent", "gpx"); }
      void gpxArpeggio()     { gpReadTest("arpeggio", "gpx"); }
      void gpxTurn()         { gpReadTest("turn", "gpx"); }
      void gpxMordents()     { gpReadTest("mordents", "gpx"); }
      void gpxPickUpDown()   { gpReadTest("pick-up-down", "gpx"); }
      void gp5PickUpDown()   { gpReadTest("pick-up-down", "gp5"); }
      void gp4PickUpDown()   { gpReadTest("pick-up-down", "gp4"); }
      void gpxFingering()    { gpReadTest("fingering", "gpx"); }
      void gp5Fingering()    { gpReadTest("fingering", "gp5"); }
      void gp4Fingering()    { gpReadTest("fingering", "gp4"); }
      void gpxBrush()        { gpReadTest("brush", "gpx"); }
      void gp5Brush()        { gpReadTest("brush", "gp5"); }
      void gp4Brush()        { gpReadTest("brush", "gp4"); }
      void gpxRepeats()      { gpReadTest("repeats", "gpx"); }
//      void gpxVolta()        { gpReadTest("volta", "gpx"); }
//      //void gpxGraceBefore()  { gpReadTest("grace-before-beat", "gpx"); }
//      //void gpxGraceOn()      { gpReadTest("grace-on-beat", "gpx"); }
      void gpxPalmMute()     { gpReadTest("palm-mute", "gpx"); }
      void gp5PalmMute()     { gpReadTest("palm-mute", "gp5"); }
      void gp4PalmMute()     { gpReadTest("palm-mute", "gp4"); }
      void gpxLetRing()      { gpReadTest("let-ring", "gpx"); }
      void gp5LetRing()      { gpReadTest("let-ring", "gp5"); }
      void gp4LetRing()      { gpReadTest("let-ring", "gp4"); }
      void gpxTapSlapPop()   { gpReadTest("tap-slap-pop", "gpx"); }
      void gp5TapSlapPop()   { gpReadTest("tap-slap-pop", "gp5"); }
//      void gpxBarre()        { gpReadTest("barre", "gpx"); }
//      void gpxTimer()        { gpReadTest("timer", "gpx"); }
      void gpxText()         { gpReadTest("text", "gpx"); }
      void gpxArtHarmonic()  { gpReadTest("artificial-harmonic", "gpx"); }
//      void gpxGhost()        { gpReadTest("ghost-note", "gpx"); }
//      void gpxRasg()         { gpReadTest("rasg", "gpx"); }
//      void gp5Percussion()   { gpReadTest("all-percussion", "gp5"); }
//      void gpxFermata()      { gpReadTest("fermata", "gpx"); }
////ws: no idea why this does not work      void gpxDirections()   { gpReadTest("directions", "gpx"); }
      void gpxSlur()         { gpReadTest("slur", "gpx"); }
      void gpxSlurHS()       { gpReadTest("slur_hammer_slur", "gpx"); }
      void gpxSlur3M()       { gpReadTest("slur_over_3_measures", "gpx"); }
      void gpxSlurSH()       { gpReadTest("slur_slur_hammer", "gpx"); }
      void gpxSlurV()        { gpReadTest("slur_voices", "gpx"); }
      void gp5Vibrato()      { gpReadTest("vibrato", "gp5"); }
      void gpxVibrato()      { gpReadTest("vibrato", "gpx"); }
      void gpxVolumeSwell()  { gpReadTest("volume-swell", "gpx"); }
////      void gpxTremoloBar()   { gpReadTest("tremolo-bar", "gpx"); }
      void gpxCopyright()    { gpReadTest("copyright", "gpx"); }
      void gpxFreeTime()     { gpReadTest("free-time", "gpx"); }
      void gpxRepeatBar()    { gpReadTest("repeated-bars", "gpx"); }
      void gp3DottedGliss()  { gpReadTest("dotted-gliss", "gp3"); }
      void highPitch()       { gpReadTest("high-pitch", "gp3"); }
      void gpxMultiVoices()  { gpReadTest("multivoices", "gpx"); }
      void gpxOttava1()      { gpReadTest("ottava1", "gpx"); }
      void gpxOttava2()      { gpReadTest("ottava2", "gpx"); }
      void gpxOttava3()      { gpReadTest("ottava3", "gpx"); }
      void gpxOttava4()      { gpReadTest("ottava4", "gpx"); }
      void gpxOttava5()      { gpReadTest("ottava5", "gpx"); }
//      void gpxChornamesKeyboard() { gpReadTest("chordnames_keyboard", "gpx"); }
      void gpxClefs() { gpReadTest("clefs", "gpx"); }
      void gpxTuplets()  { gpReadTest("tuplets", "gpx"); }
      void gpxTuplets2() { gpReadTest("tuplets2", "gpx"); }
      void gp3CapoFret() { gpReadTest("capo-fret", "gp3"); }
      void gp4CapoFret() { gpReadTest("capo-fret", "gp4"); }
      void gp5CapoFret() { gpReadTest("capo-fret", "gp5"); }
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
//   import file, write to a MuseScore file and verify against reference
//---------------------------------------------------------

void TestGuitarPro::gpReadTest(const char* file, const char* ext)
      {
      preferences.setPreference(PREF_IMPORT_GUITARPRO_CHARSET, "");
      MasterScore* score = readScore(DIR + file + "." + ext);
      QVERIFY(score);

      QVERIFY(saveCompareScore(score, QString("%1.%2.mscx").arg(file).arg(ext),
                               DIR + QString("%1.%2-ref.mscx").arg(file).arg(ext)));
      delete score;
      }

QTEST_MAIN(TestGuitarPro)
#include "tst_guitarpro.moc"
