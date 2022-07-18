/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "io/file.h"

#include "engraving/utests/utils/scorerw.h"
#include "engraving/utests/utils/scorecomp.h"

#include "libmscore/masterscore.h"
#include "libmscore/excerpt.h"

#include "modularity/ioc.h"
#include "importexport/guitarpro/iguitarproconfiguration.h"

using namespace mu;
using namespace mu::engraving;

static const String GUITARPRO_DIR(u"data/");

namespace mu::engraving {
extern Score::FileError importGTP(MasterScore*, mu::io::IODevice* io, bool createLinkedTabForce = false);
}

class GuitarPro_AllTests : public ::testing::Test
{
public:
    void gpReadTest(const char* file,  const char* ext);
};

void GuitarPro_AllTests::gpReadTest(const char* file, const char* ext)
{
    String fileName = String::fromUtf8(file) + u'.' + String::fromUtf8(ext);

    auto importFunc = [](MasterScore* score, const io::path_t& path) -> Score::FileError {
        mu::io::File file(path);
        return mu::engraving::importGTP(score, &file);
    };

    MasterScore* score = ScoreRW::readScore(GUITARPRO_DIR + fileName, false, importFunc);
    EXPECT_TRUE(score);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, fileName + u".mscx", GUITARPRO_DIR + fileName + u"-ref.mscx"));
    delete score;
}

TEST_F(GuitarPro_AllTests, gpTestIrrTuplet) {
    gpReadTest("testIrrTuplet", "gp");
}
TEST_F(GuitarPro_AllTests, gpxTestIrrTuplet) {
    gpReadTest("testIrrTuplet", "gpx");
}
TEST_F(GuitarPro_AllTests, gp4TestIrrTuplet) {
    gpReadTest("testIrrTuplet", "gp4");
}
TEST_F(GuitarPro_AllTests, gpSforzato) {
    gpReadTest("sforzato", "gp");
}
TEST_F(GuitarPro_AllTests, gpxSforzato) {
    gpReadTest("sforzato", "gpx");
}
TEST_F(GuitarPro_AllTests, gp4Sforzato) {
    gpReadTest("sforzato", "gp4");
}
TEST_F(GuitarPro_AllTests, gpHeavyAccent) {
    gpReadTest("heavy-accent", "gp");
}
TEST_F(GuitarPro_AllTests, gpxHeavyAccent) {
    gpReadTest("heavy-accent", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5HeavyAccent) {
    gpReadTest("heavy-accent", "gp5");
}
TEST_F(GuitarPro_AllTests, gpTremolos) {
    gpReadTest("tremolos", "gp");
}
TEST_F(GuitarPro_AllTests, gpxTremolos) {
    gpReadTest("tremolos", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5Tremolos) {
    gpReadTest("tremolos", "gp5");
}
TEST_F(GuitarPro_AllTests, gpTrill) {
    gpReadTest("trill", "gp");
}
TEST_F(GuitarPro_AllTests, gpxTrill) {
    gpReadTest("trill", "gpx");
}
TEST_F(GuitarPro_AllTests, gp4Trill) {
    gpReadTest("trill", "gp4");
}
TEST_F(GuitarPro_AllTests, gpDynamic) {
    gpReadTest("dynamic", "gp");
}
TEST_F(GuitarPro_AllTests, gpxDynamic) {
    gpReadTest("dynamic", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5Dynamic) {
    gpReadTest("dynamic", "gp5");
}
TEST_F(GuitarPro_AllTests, gpGraceNote) {
    gpReadTest("grace", "gp");
}
TEST_F(GuitarPro_AllTests, gpxGraceNote) {
    gpReadTest("grace", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5GraceNote) {
    gpReadTest("grace", "gp5");
}
TEST_F(GuitarPro_AllTests, gpVolta) {
    gpReadTest("volta", "gp");
}
TEST_F(GuitarPro_AllTests, gpxVolta) {
    gpReadTest("volta", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5Volta) {
    gpReadTest("volta", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4Volta) {
    gpReadTest("volta", "gp4");
}
TEST_F(GuitarPro_AllTests, gp3Volta) {
    gpReadTest("volta", "gp3");
}
TEST_F(GuitarPro_AllTests, gpcopyright) {
    gpReadTest("copyright", "gp");
}
TEST_F(GuitarPro_AllTests, gpxcopyright) {
    gpReadTest("copyright", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5copyright) {
    gpReadTest("copyright", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4copyright) {
    gpReadTest("copyright", "gp4");
}
TEST_F(GuitarPro_AllTests, gp3copyright) {
    gpReadTest("copyright", "gp3");
}
TEST_F(GuitarPro_AllTests, gpTempo) {
    gpReadTest("tempo", "gp");
}
TEST_F(GuitarPro_AllTests, gpxTempo) {
    gpReadTest("tempo", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5Tempo) {
    gpReadTest("tempo", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4Tempo) {
    gpReadTest("tempo", "gp4");
}
TEST_F(GuitarPro_AllTests, gp3Tempo) {
    gpReadTest("tempo", "gp3");
}
TEST_F(GuitarPro_AllTests, gpBasicBend) {
    gpReadTest("basic-bend", "gp");
}
TEST_F(GuitarPro_AllTests, gpxBasicBend) {
    gpReadTest("basic-bend", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5BasicBend) {
    gpReadTest("basic-bend", "gp5");
}
TEST_F(GuitarPro_AllTests, gpBend) {
    gpReadTest("bend", "gp");
}
TEST_F(GuitarPro_AllTests, gpxBend) {
    gpReadTest("bend", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5Bend) {
    gpReadTest("bend", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4Bend) {
    gpReadTest("bend", "gp4");
}
TEST_F(GuitarPro_AllTests, gp3Bend) {
    gpReadTest("bend", "gp3");
}
TEST_F(GuitarPro_AllTests, gpKeysig) {
    gpReadTest("keysig", "gp");
}
TEST_F(GuitarPro_AllTests, gpxKeysig) {
    gpReadTest("keysig", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5Keysig) {
    gpReadTest("keysig", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4Keysig) {
    gpReadTest("keysig", "gp4");
}
TEST_F(GuitarPro_AllTests, gpDottedTuplets) {
    gpReadTest("dotted-tuplets", "gp");
}
TEST_F(GuitarPro_AllTests, gpxDottedTuplets) {
    gpReadTest("dotted-tuplets", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5DottedTuplets) {
    gpReadTest("dotted-tuplets", "gp5");
}
TEST_F(GuitarPro_AllTests, gpTupletSlur) {
    gpReadTest("tuplet-with-slur", "gp");
}
TEST_F(GuitarPro_AllTests, gpxTupletSlur) {
    gpReadTest("tuplet-with-slur", "gpx");
}
TEST_F(GuitarPro_AllTests, gp4TupletSlur) {
    gpReadTest("tuplet-with-slur", "gp4");
}
TEST_F(GuitarPro_AllTests, gpBeamsStemsLL) {
    gpReadTest("beams-stems-ledger-lines", "gp");
}
TEST_F(GuitarPro_AllTests, gpxBeamsStemsLL) {
    gpReadTest("beams-stems-ledger-lines", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5BeamsStemsLL) {
    gpReadTest("beams-stems-ledger-lines", "gp5");
}
TEST_F(GuitarPro_AllTests, gpFretDiagram_2Instr) {
    gpReadTest("fret-diagram_2instruments", "gp");
}
TEST_F(GuitarPro_AllTests, gpxFretDiagram_2Instr) {
    gpReadTest("fret-diagram_2instruments", "gpx");
}
TEST_F(GuitarPro_AllTests, gpFretDiagram) {
    gpReadTest("fret-diagram", "gp");
}
TEST_F(GuitarPro_AllTests, gpxFretDiagram) {
    gpReadTest("fret-diagram", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5FretDiagram) {
    gpReadTest("fret-diagram", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4FretDiagram) {
    gpReadTest("fret-diagram", "gp4");
}
TEST_F(GuitarPro_AllTests, gpFadeIn) {
    gpReadTest("fade-in", "gp");
}
TEST_F(GuitarPro_AllTests, gpxFadeIn) {
    gpReadTest("fade-in", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5FadeIn) {
    gpReadTest("fade-in", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4FadeIn) {
    gpReadTest("fade-in", "gp4");
}
TEST_F(GuitarPro_AllTests, gpSlurNoteMask) {
    gpReadTest("slur-notes-effect-mask", "gp");
}
TEST_F(GuitarPro_AllTests, gpxSlurNoteMask) {
    gpReadTest("slur-notes-effect-mask", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5SlurNoteMask) {
    gpReadTest("slur-notes-effect-mask", "gp5");
}
TEST_F(GuitarPro_AllTests, gpCentered) {
    gpReadTest("rest-centered", "gp");
}
TEST_F(GuitarPro_AllTests, gpxCentered) {
    gpReadTest("rest-centered", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5Centered) {
    gpReadTest("rest-centered", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4Centered) {
    gpReadTest("rest-centered", "gp4");
}
TEST_F(GuitarPro_AllTests, gpSlideInAbove) {
    gpReadTest("slide-in-above", "gp");
}
TEST_F(GuitarPro_AllTests, gpxSlideInAbove) {
    gpReadTest("slide-in-above", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5SlideInAbove) {
    gpReadTest("slide-in-above", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4SlideInAbove) {
    gpReadTest("slide-in-above", "gp4");
}
TEST_F(GuitarPro_AllTests, gpSlideInBelow) {
    gpReadTest("slide-in-below", "gp");
}
TEST_F(GuitarPro_AllTests, gpxSlideInBelow) {
    gpReadTest("slide-in-below", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5SlideInBelow) {
    gpReadTest("slide-in-below", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4SlideInBelow) {
    gpReadTest("slide-in-below", "gp4");
}
TEST_F(GuitarPro_AllTests, gpSlideOutUp) {
    gpReadTest("slide-out-up", "gp");
}
TEST_F(GuitarPro_AllTests, gpxSlideOutUp) {
    gpReadTest("slide-out-up", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5SlideOutUp) {
    gpReadTest("slide-out-up", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4SlideOutUp) {
    gpReadTest("slide-out-up", "gp4");
}
TEST_F(GuitarPro_AllTests, gpSlideOutDown) {
    gpReadTest("slide-out-down", "gp");
}
TEST_F(GuitarPro_AllTests, gpxSlideOutDown) {
    gpReadTest("slide-out-down", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5SlideOutDown) {
    gpReadTest("slide-out-down", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4SlideOutDown) {
    gpReadTest("slide-out-down", "gp4");
}
TEST_F(GuitarPro_AllTests, gpLegatoSlide) {
    gpReadTest("legato-slide", "gp");
}
TEST_F(GuitarPro_AllTests, gpxLegatoSlide) {
    gpReadTest("legato-slide", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5LegatoSlide) {
    gpReadTest("legato-slide", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4LegatoSlide) {
    gpReadTest("legato-slide", "gp4");
}
TEST_F(GuitarPro_AllTests, gpShiftSlide) {
    gpReadTest("shift-slide", "gp");
}
TEST_F(GuitarPro_AllTests, gpxShiftSlide) {
    gpReadTest("shift-slide", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5ShiftSlide) {
    gpReadTest("shift-slide", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4ShiftSlide) {
    gpReadTest("shift-slide", "gp4");
}
TEST_F(GuitarPro_AllTests, gpDoubleBar) {
    gpReadTest("double-bar", "gp");
}
TEST_F(GuitarPro_AllTests, gpxDoubleBar) {
    gpReadTest("double-bar", "gpx");
}
TEST_F(GuitarPro_AllTests, gpCrecDim) {
    gpReadTest("crescendo-diminuendo", "gp");
}
TEST_F(GuitarPro_AllTests, gpxCrecDim) {
    gpReadTest("crescendo-diminuendo", "gpx");
}
TEST_F(GuitarPro_AllTests, gpDeadNote) {
    gpReadTest("dead-note", "gp");
}
TEST_F(GuitarPro_AllTests, gpxDeadNote) {
    gpReadTest("dead-note", "gpx");
}
TEST_F(GuitarPro_AllTests, gpWah) {
    gpReadTest("wah", "gp");
}
TEST_F(GuitarPro_AllTests, gpxWah) {
    gpReadTest("wah", "gpx");
}
TEST_F(GuitarPro_AllTests, gpAccent) {
    gpReadTest("accent", "gp");
}
TEST_F(GuitarPro_AllTests, gpxAccent) {
    gpReadTest("accent", "gpx");
}
TEST_F(GuitarPro_AllTests, gpArpeggio) {
    gpReadTest("arpeggio", "gp");
}
TEST_F(GuitarPro_AllTests, gpxArpeggio) {
    gpReadTest("arpeggio", "gpx");
}
TEST_F(GuitarPro_AllTests, gpTurn) {
    gpReadTest("turn", "gp");
}
TEST_F(GuitarPro_AllTests, gpxTurn) {
    gpReadTest("turn", "gpx");
}
TEST_F(GuitarPro_AllTests, gpMordents) {
    gpReadTest("mordents", "gp");
}
TEST_F(GuitarPro_AllTests, gpxMordents) {
    gpReadTest("mordents", "gpx");
}
TEST_F(GuitarPro_AllTests, gpPickUpDown) {
    gpReadTest("pick-up-down", "gp");
}
TEST_F(GuitarPro_AllTests, gpxPickUpDown) {
    gpReadTest("pick-up-down", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5PickUpDown) {
    gpReadTest("pick-up-down", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4PickUpDown) {
    gpReadTest("pick-up-down", "gp4");
}
TEST_F(GuitarPro_AllTests, gpFingering) {
    gpReadTest("fingering", "gp");
}
TEST_F(GuitarPro_AllTests, gpxFingering) {
    gpReadTest("fingering", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5Fingering) {
    gpReadTest("fingering", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4Fingering) {
    gpReadTest("fingering", "gp4");
}
TEST_F(GuitarPro_AllTests, gpBrush) {
    gpReadTest("brush", "gp");
}
TEST_F(GuitarPro_AllTests, gpxBrush) {
    gpReadTest("brush", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5Brush) {
    gpReadTest("brush", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4Brush) {
    gpReadTest("brush", "gp4");
}
TEST_F(GuitarPro_AllTests, gpRepeats) {
    gpReadTest("repeats", "gp");
}
TEST_F(GuitarPro_AllTests, gpxRepeats) {
    gpReadTest("repeats", "gpx");
}
TEST_F(GuitarPro_AllTests, gpGraceBefore) {
    gpReadTest("grace-before-beat", "gp");
}
TEST_F(GuitarPro_AllTests, gpxGraceBefore) {
    gpReadTest("grace-before-beat", "gpx");
}
TEST_F(GuitarPro_AllTests, gpGraceOn) {
    gpReadTest("grace-on-beat", "gp");
}
TEST_F(GuitarPro_AllTests, gpxGraceOn) {
    gpReadTest("grace-on-beat", "gpx");
}
TEST_F(GuitarPro_AllTests, gpPalmMute) {
    gpReadTest("palm-mute", "gp");
}
TEST_F(GuitarPro_AllTests, gpxPalmMute) {
    gpReadTest("palm-mute", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5PalmMute) {
    gpReadTest("palm-mute", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4PalmMute) {
    gpReadTest("palm-mute", "gp4");
}
TEST_F(GuitarPro_AllTests, gpLetRing) {
    gpReadTest("let-ring", "gp");
}
TEST_F(GuitarPro_AllTests, gpxLetRing) {
    gpReadTest("let-ring", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5LetRing) {
    gpReadTest("let-ring", "gp5");
}
TEST_F(GuitarPro_AllTests, gp4LetRing) {
    gpReadTest("let-ring", "gp4");
}
TEST_F(GuitarPro_AllTests, gpTapSlapPop) {
    gpReadTest("tap-slap-pop", "gp");
}
TEST_F(GuitarPro_AllTests, gpxTapSlapPop) {
    gpReadTest("tap-slap-pop", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5TapSlapPop) {
    gpReadTest("tap-slap-pop", "gp5");
}
TEST_F(GuitarPro_AllTests, gpBarre) {
    gpReadTest("barre", "gp");
}
TEST_F(GuitarPro_AllTests, gpxBarre) {
    gpReadTest("barre", "gpx");
}
TEST_F(GuitarPro_AllTests, gpTimer) {
    gpReadTest("timer", "gp");
}
TEST_F(GuitarPro_AllTests, gpxTimer) {
    gpReadTest("timer", "gpx");
}
TEST_F(GuitarPro_AllTests, gpText) {
    gpReadTest("text", "gp");
}
TEST_F(GuitarPro_AllTests, gpxText) {
    gpReadTest("text", "gpx");
}
TEST_F(GuitarPro_AllTests, gpArtHarmonic) {
    gpReadTest("artificial-harmonic", "gp");
}
TEST_F(GuitarPro_AllTests, gpxArtHarmonic) {
    gpReadTest("artificial-harmonic", "gpx");
}
TEST_F(GuitarPro_AllTests, gpGhost) {
    gpReadTest("ghost-note", "gp");
}
TEST_F(GuitarPro_AllTests, gpxGhost) {
    gpReadTest("ghost-note", "gpx");
}
TEST_F(GuitarPro_AllTests, gp3GhostNote) {
    gpReadTest("ghost_note", "gp3");
}
TEST_F(GuitarPro_AllTests, gpRasg) {
    gpReadTest("rasg", "gp");
}
TEST_F(GuitarPro_AllTests, gpxRasg) {
    gpReadTest("rasg", "gpx");
}
TEST_F(GuitarPro_AllTests, gpPercussion) {
    gpReadTest("all-percussion", "gp");
}
TEST_F(GuitarPro_AllTests, gpxPercussion) {
    gpReadTest("all-percussion", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5Percussion) {
    gpReadTest("all-percussion", "gp5");
}
TEST_F(GuitarPro_AllTests, gpFermata) {
    gpReadTest("fermata", "gp");
}
TEST_F(GuitarPro_AllTests, gpxFermata) {
    gpReadTest("fermata", "gpx");
}
TEST_F(GuitarPro_AllTests, gpDirections) {
    gpReadTest("directions", "gp");
}
TEST_F(GuitarPro_AllTests, gpxDirections) {
    gpReadTest("directions", "gpx");
}
TEST_F(GuitarPro_AllTests, gpSlur) {
    gpReadTest("slur", "gp");
}
TEST_F(GuitarPro_AllTests, gpxSlur) {
    gpReadTest("slur", "gpx");
}
TEST_F(GuitarPro_AllTests, gp4Slur) {
    gpReadTest("slur", "gp4");
}
TEST_F(GuitarPro_AllTests, gpSlurHS) {
    gpReadTest("slur_hammer_slur", "gp");
}
TEST_F(GuitarPro_AllTests, gpxSlurHS) {
    gpReadTest("slur_hammer_slur", "gpx");
}
TEST_F(GuitarPro_AllTests, gpSlur3M) {
    gpReadTest("slur_over_3_measures", "gp");
}
TEST_F(GuitarPro_AllTests, gpxSlur3M) {
    gpReadTest("slur_over_3_measures", "gpx");
}
TEST_F(GuitarPro_AllTests, gpSlurSH) {
    gpReadTest("slur_slur_hammer", "gp");
}
TEST_F(GuitarPro_AllTests, gpxSlurSH) {
    gpReadTest("slur_slur_hammer", "gpx");
}
TEST_F(GuitarPro_AllTests, gpSlurV) {
    gpReadTest("slur_voices", "gp");
}
TEST_F(GuitarPro_AllTests, gpxSlurV) {
    gpReadTest("slur_voices", "gpx");
}
TEST_F(GuitarPro_AllTests, gpVibrato) {
    gpReadTest("vibrato", "gp");
}
TEST_F(GuitarPro_AllTests, gpxVibrato) {
    gpReadTest("vibrato", "gpx");
}
TEST_F(GuitarPro_AllTests, gp5Vibrato) {
    gpReadTest("vibrato", "gp5");
}
TEST_F(GuitarPro_AllTests, gpVolumeSwell) {
    gpReadTest("volume-swell", "gp");
}
TEST_F(GuitarPro_AllTests, gpxVolumeSwell) {
    gpReadTest("volume-swell", "gpx");
}
TEST_F(GuitarPro_AllTests, gpTremoloBar) {
    gpReadTest("tremolo-bar", "gp");
}
TEST_F(GuitarPro_AllTests, gpxTremoloBar) {
    gpReadTest("tremolo-bar", "gpx");
}
TEST_F(GuitarPro_AllTests, gpFreeTime) {
    gpReadTest("free-time", "gp");
}
TEST_F(GuitarPro_AllTests, gpxFreeTime) {
    gpReadTest("free-time", "gpx");
}
TEST_F(GuitarPro_AllTests, gpRepeatBar) {
    gpReadTest("repeated-bars", "gp");
}
TEST_F(GuitarPro_AllTests, gpxRepeatBar) {
    gpReadTest("repeated-bars", "gpx");
}
TEST_F(GuitarPro_AllTests, gpDottedGliss) {
    gpReadTest("dotted-gliss", "gp");
}
TEST_F(GuitarPro_AllTests, gpxDottedGliss) {
    gpReadTest("dotted-gliss", "gpx");
}
TEST_F(GuitarPro_AllTests, gp3DottedGliss) {
    gpReadTest("dotted-gliss", "gp3");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpHighPitch) {
    gpReadTest("high-pitch", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxHighPitch) {
    gpReadTest("high-pitch", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp3HighPitch) {
    gpReadTest("high-pitch", "gp3");
}
TEST_F(GuitarPro_AllTests, gpMultiVoices) {
    gpReadTest("multivoices", "gp");
}
TEST_F(GuitarPro_AllTests, gpxMultiVoices) {
    gpReadTest("multivoices", "gpx");
}
TEST_F(GuitarPro_AllTests, gpOttava1) {
    gpReadTest("ottava1", "gp");
}
TEST_F(GuitarPro_AllTests, gpxOttava1) {
    gpReadTest("ottava1", "gpx");
}
TEST_F(GuitarPro_AllTests, gpOttava2) {
    gpReadTest("ottava2", "gp");
}
TEST_F(GuitarPro_AllTests, gpxOttava2) {
    gpReadTest("ottava2", "gpx");
}
TEST_F(GuitarPro_AllTests, gpOttava3) {
    gpReadTest("ottava3", "gp");
}
TEST_F(GuitarPro_AllTests, gpxOttava3) {
    gpReadTest("ottava3", "gpx");
}
TEST_F(GuitarPro_AllTests, gpOttava4) {
    gpReadTest("ottava4", "gp");
}
TEST_F(GuitarPro_AllTests, gpxOttava4) {
    gpReadTest("ottava4", "gpx");
}
TEST_F(GuitarPro_AllTests, gpOttava5) {
    gpReadTest("ottava5", "gp");
}
TEST_F(GuitarPro_AllTests, gpxOttava5) {
    gpReadTest("ottava5", "gpx");
}
TEST_F(GuitarPro_AllTests, gpChornamesKeyboard) {
    gpReadTest("chordnames_keyboard", "gp");
}
TEST_F(GuitarPro_AllTests, gpxChornamesKeyboard) {
    gpReadTest("chordnames_keyboard", "gpx");
}
TEST_F(GuitarPro_AllTests, gpClefs) {
    gpReadTest("clefs", "gp");
}
TEST_F(GuitarPro_AllTests, gpxClefs) {
    gpReadTest("clefs", "gpx");
}
TEST_F(GuitarPro_AllTests, gpxTuplets) {
    gpReadTest("tuplets", "gpx");
}
TEST_F(GuitarPro_AllTests, gpxTuplets2) {
    gpReadTest("tuplets2", "gpx");
}
TEST_F(GuitarPro_AllTests, gp3CapoFret) {
    gpReadTest("capo-fret", "gp3");
}
TEST_F(GuitarPro_AllTests, gp4CapoFret) {
    gpReadTest("capo-fret", "gp4");
}
TEST_F(GuitarPro_AllTests, gp5CapoFret) {
    gpReadTest("capo-fret", "gp5");
}
TEST_F(GuitarPro_AllTests, gpxUncompletedMeasure) {
    gpReadTest("UncompletedMeasure", "gpx");
}
