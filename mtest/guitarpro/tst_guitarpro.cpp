//=============================================================================
//  MuseScore
//  Music Composition & Notation
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
    void gpTestIrrTuplet() { gpReadTest("testIrrTuplet", "gp"); }
    void gpxTestIrrTuplet() { gpReadTest("testIrrTuplet", "gpx"); }
    void gp4TestIrrTuplet() { gpReadTest("testIrrTuplet", "gp4"); }
    void gpSforzato() { gpReadTest("sforzato", "gp"); }
    void gpxSforzato() { gpReadTest("sforzato", "gpx"); }
    void gp4Sforzato() { gpReadTest("sforzato", "gp4"); }
    void gpHeavyAccent() { gpReadTest("heavy-accent", "gp"); }
    void gpxHeavyAccent() { gpReadTest("heavy-accent", "gpx"); }
    void gp5HeavyAccent() { gpReadTest("heavy-accent", "gp5"); }
    void gpTremolos() { gpReadTest("tremolos", "gp"); }
    void gpxTremolos() { gpReadTest("tremolos", "gpx"); }
    void gp5Tremolos() { gpReadTest("tremolos", "gp5"); }
    void gpTrill() { gpReadTest("trill", "gp"); }
    void gpxTrill() { gpReadTest("trill", "gpx"); }
    void gp4Trill() { gpReadTest("trill", "gp4"); }
    void gpDynamic() { gpReadTest("dynamic", "gp"); }
    void gpxDynamic() { gpReadTest("dynamic", "gpx"); }
    void gp5Dynamic() { gpReadTest("dynamic", "gp5"); }
//      void gpGraceNote()      { gpReadTest("grace", "gp"); }
//      void gpxGraceNote()     { gpReadTest("grace", "gpx"); }
    void gp5GraceNote() { gpReadTest("grace", "gp5"); }
//      void gpVolta()          { gpReadTest("volta", "gp"); }
//      void gpxVolta()         { gpReadTest("volta", "gpx"); }
    void gp5Volta() { gpReadTest("volta", "gp5"); }
    void gp4Volta() { gpReadTest("volta", "gp4"); }
    void gp3Volta() { gpReadTest("volta", "gp3"); }
    void gpcopyright() { gpReadTest("copyright", "gp"); }
    void gpxcopyright() { gpReadTest("copyright", "gpx"); }
    void gp5copyright() { gpReadTest("copyright", "gp5"); }
    void gp4copyright() { gpReadTest("copyright", "gp4"); }
    void gp3copyright() { gpReadTest("copyright", "gp3"); }
    void gpTempo() { gpReadTest("tempo", "gp"); }
    void gpxTempo() { gpReadTest("tempo", "gpx"); }
    void gp5Tempo() { gpReadTest("tempo", "gp5"); }
    void gp4Tempo() { gpReadTest("tempo", "gp4"); }
    void gp3Tempo() { gpReadTest("tempo", "gp3"); }
    void gpBasicBend() { gpReadTest("basic-bend", "gp"); }
    void gpxBasicBend() { gpReadTest("basic-bend", "gpx"); }
    void gp5BasicBend() { gpReadTest("basic-bend", "gp5"); }
//      void gpBend()           { gpReadTest("bend", "gp"); }
//      void gpxBend()          { gpReadTest("bend", "gpx"); }
//      void gp5Bend()          { gpReadTest("bend", "gp5"); }
//      void gp4Bend()          { gpReadTest("bend", "gp4"); }
//      void gp3Bend()          { gpReadTest("bend", "gp3"); }
    void gpKeysig() { gpReadTest("keysig", "gp"); }
    void gpxKeysig() { gpReadTest("keysig", "gpx"); }
    void gp5Keysig() { gpReadTest("keysig", "gp5"); }
    void gp4Keysig() { gpReadTest("keysig", "gp4"); }
    void gpDottedTuplets() { gpReadTest("dotted-tuplets", "gp"); }
    void gpxDottedTuplets() { gpReadTest("dotted-tuplets", "gpx"); }
    void gp5DottedTuplets() { gpReadTest("dotted-tuplets", "gp5"); }
//      void gpTupletSlur()     { gpReadTest("tuplet-with-slur", "gp"); }
//      void gpxTupletSlur()    { gpReadTest("tuplet-with-slur", "gpx"); }
//      void gp4TupletSlur()    { gpReadTest("tuplet-with-slur", "gp4"); }
//      void gpBeamsStemsLL()   { gpReadTest("beams-stems-ledger-lines", "gp"); }
//      void gpxBeamsStemsLL()  { gpReadTest("beams-stems-ledger-lines", "gpx"); }
//      void gp5BeamsStemsLL()  { gpReadTest("beams-stems-ledger-lines", "gp5"); }
//      void gpFretDiagram_2Instr()  { gpReadTest("fret-diagram_2instruments", "gp"); }
//      void gpxFretDiagram_2Instr() { gpReadTest("fret-diagram_2instruments", "gpx"); }
//      void gpFretDiagram()    { gpReadTest("fret-diagram", "gp"); }
//      void gpxFretDiagram()   { gpReadTest("fret-diagram", "gpx"); }
//      void gp5FretDiagram()   { gpReadTest("fret-diagram", "gp5"); }
//      void gp4FretDiagram()   { gpReadTest("fret-diagram", "gp4"); }
    void gpFadeIn() { gpReadTest("fade-in", "gp"); }
    void gpxFadeIn() { gpReadTest("fade-in", "gpx"); }
    void gp5FadeIn() { gpReadTest("fade-in", "gp5"); }
    void gp4FadeIn() { gpReadTest("fade-in", "gp4"); }
//      void gpSlurNoteMask()   { gpReadTest("slur-notes-effect-mask", "gp"); }
//      void gpxSlurNoteMask()  { gpReadTest("slur-notes-effect-mask", "gpx"); }
    void gp5SlurNoteMask() { gpReadTest("slur-notes-effect-mask", "gp5"); }
    void gpCentered() { gpReadTest("rest-centered", "gp"); }
    void gpxCentered() { gpReadTest("rest-centered", "gpx"); }
    void gp5Centered() { gpReadTest("rest-centered", "gp5"); }
    void gp4Centered() { gpReadTest("rest-centered", "gp4"); }
//      void gpSlideInAbove()   { gpReadTest("slide-in-above", "gp"); }
//      void gpxSlideInAbove()  { gpReadTest("slide-in-above", "gpx"); }
//      void gp5SlideInAbove()  { gpReadTest("slide-in-above", "gp5"); }
    void gp4SlideInAbove() { gpReadTest("slide-in-above", "gp4"); }
//      void gpSlideInBelow()   { gpReadTest("slide-in-below", "gp"); }
//      void gpxSlideInBelow()  { gpReadTest("slide-in-below", "gpx"); }
//      void gp5SlideInBelow()  { gpReadTest("slide-in-below", "gp5"); }
//      void gp4SlideInBelow()  { gpReadTest("slide-in-below", "gp4"); }
//      void gpSlideOutUp()     { gpReadTest("slide-out-up", "gp"); }
//      void gpxSlideOutUp()    { gpReadTest("slide-out-up", "gpx"); }
//      void gp5SlideOutUp()    { gpReadTest("slide-out-up", "gp5"); }
//      void gp4SlideOutUp()    { gpReadTest("slide-out-up", "gp4"); }
//      void gpSlideOutDown()   { gpReadTest("slide-out-down", "gp"); }
//      void gpxSlideOutDown()  { gpReadTest("slide-out-down", "gpx"); }
//      void gp5SlideOutDown()  { gpReadTest("slide-out-down", "gp5"); }
//      void gp4SlideOutDown()  { gpReadTest("slide-out-down", "gp4"); }
//      void gpLegatoSlide()    { gpReadTest("legato-slide", "gp"); }
//      void gpxLegatoSlide()   { gpReadTest("legato-slide", "gpx"); }
//      void gp5LegatoSlide()   { gpReadTest("legato-slide", "gp5"); }
//      void gp4LegatoSlide()   { gpReadTest("legato-slide", "gp4"); }
//      void gpShiftSlide()     { gpReadTest("shift-slide", "gp"); }
//      void gpxShiftSlide()    { gpReadTest("shift-slide", "gpx"); }
//      void gp5ShiftSlide()    { gpReadTest("shift-slide", "gp5"); }
    void gp4ShiftSlide() { gpReadTest("shift-slide", "gp4"); }
    void gpDoubleBar() { gpReadTest("double-bar", "gp"); }
    void gpxDoubleBar() { gpReadTest("double-bar", "gpx"); }
    void gpCrecDim() { gpReadTest("crescendo-diminuendo", "gp"); }
    void gpxCrecDim() { gpReadTest("crescendo-diminuendo", "gpx"); }
    void gpDeadNote() { gpReadTest("dead-note", "gp"); }
    void gpxDeadNote() { gpReadTest("dead-note", "gpx"); }
    void gpWah() { gpReadTest("wah", "gp"); }
    void gpxWah() { gpReadTest("wah", "gpx"); }
    void gpAccent() { gpReadTest("accent", "gp"); }
    void gpxAccent() { gpReadTest("accent", "gpx"); }
    void gpArpeggio() { gpReadTest("arpeggio", "gp"); }
    void gpxArpeggio() { gpReadTest("arpeggio", "gpx"); }
    void gpTurn() { gpReadTest("turn", "gp"); }
    void gpxTurn() { gpReadTest("turn", "gpx"); }
    void gpMordents() { gpReadTest("mordents", "gp"); }
    void gpxMordents() { gpReadTest("mordents", "gpx"); }
    void gpPickUpDown() { gpReadTest("pick-up-down", "gp"); }
    void gpxPickUpDown() { gpReadTest("pick-up-down", "gpx"); }
    void gp5PickUpDown() { gpReadTest("pick-up-down", "gp5"); }
    void gp4PickUpDown() { gpReadTest("pick-up-down", "gp4"); }
    void gpFingering() { gpReadTest("fingering", "gp"); }
    void gpxFingering() { gpReadTest("fingering", "gpx"); }
    void gp5Fingering() { gpReadTest("fingering", "gp5"); }
    void gp4Fingering() { gpReadTest("fingering", "gp4"); }
    void gpBrush() { gpReadTest("brush", "gp"); }
    void gpxBrush() { gpReadTest("brush", "gpx"); }
    void gp5Brush() { gpReadTest("brush", "gp5"); }
    void gp4Brush() { gpReadTest("brush", "gp4"); }
    void gpRepeats() { gpReadTest("repeats", "gp"); }
    void gpxRepeats() { gpReadTest("repeats", "gpx"); }
//      void gpGraceBefore()    { gpReadTest("grace-before-beat", "gp"); }
//      void gpxGraceBefore()   { gpReadTest("grace-before-beat", "gpx"); }
//      void gpGraceOn()        { gpReadTest("grace-on-beat", "gp"); }
//      void gpxGraceOn()       { gpReadTest("grace-on-beat", "gpx"); }
    void gpPalmMute() { gpReadTest("palm-mute", "gp"); }
    void gpxPalmMute() { gpReadTest("palm-mute", "gpx"); }
    void gp5PalmMute() { gpReadTest("palm-mute", "gp5"); }
    void gp4PalmMute() { gpReadTest("palm-mute", "gp4"); }
    void gpLetRing() { gpReadTest("let-ring", "gp"); }
    void gpxLetRing() { gpReadTest("let-ring", "gpx"); }
    void gp5LetRing() { gpReadTest("let-ring", "gp5"); }
    void gp4LetRing() { gpReadTest("let-ring", "gp4"); }
    void gpTapSlapPop() { gpReadTest("tap-slap-pop", "gp"); }
    void gpxTapSlapPop() { gpReadTest("tap-slap-pop", "gpx"); }
    void gp5TapSlapPop() { gpReadTest("tap-slap-pop", "gp5"); }
//      void gpBarre()          { gpReadTest("barre", "gp"); }
//      void gpxBarre()         { gpReadTest("barre", "gpx"); }
//      void gpTimer()          { gpReadTest("timer", "gp"); }
//      void gpxTimer()         { gpReadTest("timer", "gpx"); }
    void gpText() { gpReadTest("text", "gp"); }
    void gpxText() { gpReadTest("text", "gpx"); }
    void gpArtHarmonic() { gpReadTest("artificial-harmonic", "gp"); }
    void gpxArtHarmonic() { gpReadTest("artificial-harmonic", "gpx"); }
//      void gpGhost()          { gpReadTest("ghost-note", "gp"); }
//      void gpxGhost()         { gpReadTest("ghost-note", "gpx"); }
    void gp3GhostNote() { gpReadTest("ghost_note", "gp3"); }
//      void gpRasg()           { gpReadTest("rasg", "gp"); }
//      void gpxRasg()          { gpReadTest("rasg", "gpx"); }
//      void gpPercussion()     { gpReadTest("all-percussion", "gp"); }
//      void gpxPercussion()    { gpReadTest("all-percussion", "gpx"); }
//      void gp5Percussion()    { gpReadTest("all-percussion", "gp5"); }
//      void gpFermata()        { gpReadTest("fermata", "gp"); }
//      void gpxFermata()       { gpReadTest("fermata", "gpx"); }
//      void gpDirections()     { gpReadTest("directions", "gp"); }
////ws: no idea why this does not work      void gpxDirections()   { gpReadTest("directions", "gpx"); }
    void gpSlur() { gpReadTest("slur", "gp"); }
    void gpxSlur() { gpReadTest("slur", "gpx"); }
    void gp4Slur() { gpReadTest("slur", "gp4"); }
    void gpSlurHS() { gpReadTest("slur_hammer_slur", "gp"); }
    void gpxSlurHS() { gpReadTest("slur_hammer_slur", "gpx"); }
    void gpSlur3M() { gpReadTest("slur_over_3_measures", "gp"); }
    void gpxSlur3M() { gpReadTest("slur_over_3_measures", "gpx"); }
    void gpSlurSH() { gpReadTest("slur_slur_hammer", "gp"); }
    void gpxSlurSH() { gpReadTest("slur_slur_hammer", "gpx"); }
    void gpSlurV() { gpReadTest("slur_voices", "gp"); }
    void gpxSlurV() { gpReadTest("slur_voices", "gpx"); }
    void gpVibrato() { gpReadTest("vibrato", "gp"); }
    void gpxVibrato() { gpReadTest("vibrato", "gpx"); }
    void gp5Vibrato() { gpReadTest("vibrato", "gp5"); }
    void gpVolumeSwell() { gpReadTest("volume-swell", "gp"); }
    void gpxVolumeSwell() { gpReadTest("volume-swell", "gpx"); }
//      void gpTremoloBar()     { gpReadTest("tremolo-bar", "gp"); }
//      void gpxTremoloBar()    { gpReadTest("tremolo-bar", "gpx"); }
    void gpFreeTime() { gpReadTest("free-time", "gp"); }
    void gpxFreeTime() { gpReadTest("free-time", "gpx"); }
    void gpRepeatBar() { gpReadTest("repeated-bars", "gp"); }
    void gpxRepeatBar() { gpReadTest("repeated-bars", "gpx"); }
    void gpDottedGliss() { gpReadTest("dotted-gliss", "gp"); }
    void gpxDottedGliss() { gpReadTest("dotted-gliss", "gpx"); }
    void gp3DottedGliss() { gpReadTest("dotted-gliss", "gp3"); }
    void gpHighPitch() { gpReadTest("high-pitch", "gp"); }
    void gpxHighPitch() { gpReadTest("high-pitch", "gpx"); }
    void gp3HighPitch() { gpReadTest("high-pitch", "gp3"); }
    void gpMultiVoices() { gpReadTest("multivoices", "gp"); }
    void gpxMultiVoices() { gpReadTest("multivoices", "gpx"); }
    void gpOttava1() { gpReadTest("ottava1", "gp"); }
    void gpxOttava1() { gpReadTest("ottava1", "gpx"); }
    void gpOttava2() { gpReadTest("ottava2", "gp"); }
    void gpxOttava2() { gpReadTest("ottava2", "gpx"); }
    void gpOttava3() { gpReadTest("ottava3", "gp"); }
    void gpxOttava3() { gpReadTest("ottava3", "gpx"); }
    void gpOttava4() { gpReadTest("ottava4", "gp"); }
    void gpxOttava4() { gpReadTest("ottava4", "gpx"); }
    void gpOttava5() { gpReadTest("ottava5", "gp"); }
    void gpxOttava5() { gpReadTest("ottava5", "gpx"); }
//      void gpChornamesKeyboard()  { gpReadTest("chordnames_keyboard", "gp"); }
//      void gpxChornamesKeyboard() { gpReadTest("chordnames_keyboard", "gpx"); }
    void gpClefs() { gpReadTest("clefs", "gp"); }
    void gpxClefs() { gpReadTest("clefs", "gpx"); }
    void gpxTuplets() { gpReadTest("tuplets", "gpx"); }
    void gpxTuplets2() { gpReadTest("tuplets2", "gpx"); }
    void gp3CapoFret() { gpReadTest("capo-fret", "gp3"); }
    void gp4CapoFret() { gpReadTest("capo-fret", "gp4"); }
    void gp5CapoFret() { gpReadTest("capo-fret", "gp5"); }
    void gpxUncompletedMeasure() { gpReadTest("UncompletedMeasure", "gpx"); }
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
    QSKIP("At the moment `preferences` are not used in GuitarPro import");
    preferences.setPreference(PREF_IMPORT_GUITARPRO_CHARSET, "");
    MasterScore* score = readScore(DIR + file + "." + ext);
    QVERIFY(score);

    QVERIFY(saveCompareScore(score, QString("%1.%2.mscx").arg(file).arg(ext),
                             DIR + QString("%1.%2-ref.mscx").arg(file).arg(ext)));
    delete score;
}

QTEST_MAIN(TestGuitarPro)
#include "tst_guitarpro.moc"
