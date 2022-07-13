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
TEST_F(GuitarPro_AllTests, DISABLED_gpSlideInAbove) {
    gpReadTest("slide-in-above", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxSlideInAbove) {
    gpReadTest("slide-in-above", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp5SlideInAbove) {
    gpReadTest("slide-in-above", "gp5");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp4SlideInAbove) {
    gpReadTest("slide-in-above", "gp4");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpSlideInBelow) {
    gpReadTest("slide-in-below", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxSlideInBelow) {
    gpReadTest("slide-in-below", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp5SlideInBelow) {
    gpReadTest("slide-in-below", "gp5");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp4SlideInBelow) {
    gpReadTest("slide-in-below", "gp4");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpSlideOutUp) {
    gpReadTest("slide-out-up", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxSlideOutUp) {
    gpReadTest("slide-out-up", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp5SlideOutUp) {
    gpReadTest("slide-out-up", "gp5");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp4SlideOutUp) {
    gpReadTest("slide-out-up", "gp4");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpSlideOutDown) {
    gpReadTest("slide-out-down", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxSlideOutDown) {
    gpReadTest("slide-out-down", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp5SlideOutDown) {
    gpReadTest("slide-out-down", "gp5");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp4SlideOutDown) {
    gpReadTest("slide-out-down", "gp4");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpLegatoSlide) {
    gpReadTest("legato-slide", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxLegatoSlide) {
    gpReadTest("legato-slide", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp5LegatoSlide) {
    gpReadTest("legato-slide", "gp5");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp4LegatoSlide) {
    gpReadTest("legato-slide", "gp4");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpShiftSlide) {
    gpReadTest("shift-slide", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxShiftSlide) {
    gpReadTest("shift-slide", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp5ShiftSlide) {
    gpReadTest("shift-slide", "gp5");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp4ShiftSlide) {
    gpReadTest("shift-slide", "gp4");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpDoubleBar) {
    gpReadTest("double-bar", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxDoubleBar) {
    gpReadTest("double-bar", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpCrecDim) {
    gpReadTest("crescendo-diminuendo", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxCrecDim) {
    gpReadTest("crescendo-diminuendo", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpDeadNote) {
    gpReadTest("dead-note", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxDeadNote) {
    gpReadTest("dead-note", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpWah) {
    gpReadTest("wah", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxWah) {
    gpReadTest("wah", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpAccent) {
    gpReadTest("accent", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxAccent) {
    gpReadTest("accent", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpArpeggio) {
    gpReadTest("arpeggio", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxArpeggio) {
    gpReadTest("arpeggio", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpTurn) {
    gpReadTest("turn", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxTurn) {
    gpReadTest("turn", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpMordents) {
    gpReadTest("mordents", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxMordents) {
    gpReadTest("mordents", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpPickUpDown) {
    gpReadTest("pick-up-down", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxPickUpDown) {
    gpReadTest("pick-up-down", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp5PickUpDown) {
    gpReadTest("pick-up-down", "gp5");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp4PickUpDown) {
    gpReadTest("pick-up-down", "gp4");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpFingering) {
    gpReadTest("fingering", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxFingering) {
    gpReadTest("fingering", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp5Fingering) {
    gpReadTest("fingering", "gp5");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp4Fingering) {
    gpReadTest("fingering", "gp4");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpBrush) {
    gpReadTest("brush", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxBrush) {
    gpReadTest("brush", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp5Brush) {
    gpReadTest("brush", "gp5");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp4Brush) {
    gpReadTest("brush", "gp4");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpRepeats) {
    gpReadTest("repeats", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxRepeats) {
    gpReadTest("repeats", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpGraceBefore) {
    gpReadTest("grace-before-beat", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxGraceBefore) {
    gpReadTest("grace-before-beat", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpGraceOn) {
    gpReadTest("grace-on-beat", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxGraceOn) {
    gpReadTest("grace-on-beat", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpPalmMute) {
    gpReadTest("palm-mute", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxPalmMute) {
    gpReadTest("palm-mute", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp5PalmMute) {
    gpReadTest("palm-mute", "gp5");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp4PalmMute) {
    gpReadTest("palm-mute", "gp4");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpLetRing) {
    gpReadTest("let-ring", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxLetRing) {
    gpReadTest("let-ring", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp5LetRing) {
    gpReadTest("let-ring", "gp5");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp4LetRing) {
    gpReadTest("let-ring", "gp4");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpTapSlapPop) {
    gpReadTest("tap-slap-pop", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxTapSlapPop) {
    gpReadTest("tap-slap-pop", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp5TapSlapPop) {
    gpReadTest("tap-slap-pop", "gp5");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpBarre) {
    gpReadTest("barre", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxBarre) {
    gpReadTest("barre", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpTimer) {
    gpReadTest("timer", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxTimer) {
    gpReadTest("timer", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpText) {
    gpReadTest("text", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxText) {
    gpReadTest("text", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpArtHarmonic) {
    gpReadTest("artificial-harmonic", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxArtHarmonic) {
    gpReadTest("artificial-harmonic", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpGhost) {
    gpReadTest("ghost-note", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxGhost) {
    gpReadTest("ghost-note", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp3GhostNote) {
    gpReadTest("ghost_note", "gp3");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpRasg) {
    gpReadTest("rasg", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxRasg) {
    gpReadTest("rasg", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpPercussion) {
    gpReadTest("all-percussion", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxPercussion) {
    gpReadTest("all-percussion", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp5Percussion) {
    gpReadTest("all-percussion", "gp5");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpFermata) {
    gpReadTest("fermata", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxFermata) {
    gpReadTest("fermata", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpDirections) {
    gpReadTest("directions", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxDirections) {
    gpReadTest("directions", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpSlur) {
    gpReadTest("slur", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxSlur) {
    gpReadTest("slur", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp4Slur) {
    gpReadTest("slur", "gp4");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpSlurHS) {
    gpReadTest("slur_hammer_slur", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxSlurHS) {
    gpReadTest("slur_hammer_slur", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpSlur3M) {
    gpReadTest("slur_over_3_measures", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxSlur3M) {
    gpReadTest("slur_over_3_measures", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpSlurSH) {
    gpReadTest("slur_slur_hammer", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxSlurSH) {
    gpReadTest("slur_slur_hammer", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpSlurV) {
    gpReadTest("slur_voices", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxSlurV) {
    gpReadTest("slur_voices", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpVibrato) {
    gpReadTest("vibrato", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxVibrato) {
    gpReadTest("vibrato", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp5Vibrato) {
    gpReadTest("vibrato", "gp5");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpVolumeSwell) {
    gpReadTest("volume-swell", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxVolumeSwell) {
    gpReadTest("volume-swell", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpTremoloBar) {
    gpReadTest("tremolo-bar", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxTremoloBar) {
    gpReadTest("tremolo-bar", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpFreeTime) {
    gpReadTest("free-time", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxFreeTime) {
    gpReadTest("free-time", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpRepeatBar) {
    gpReadTest("repeated-bars", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxRepeatBar) {
    gpReadTest("repeated-bars", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpDottedGliss) {
    gpReadTest("dotted-gliss", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxDottedGliss) {
    gpReadTest("dotted-gliss", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp3DottedGliss) {
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
TEST_F(GuitarPro_AllTests, DISABLED_gpMultiVoices) {
    gpReadTest("multivoices", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxMultiVoices) {
    gpReadTest("multivoices", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpOttava1) {
    gpReadTest("ottava1", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxOttava1) {
    gpReadTest("ottava1", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpOttava2) {
    gpReadTest("ottava2", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxOttava2) {
    gpReadTest("ottava2", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpOttava3) {
    gpReadTest("ottava3", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxOttava3) {
    gpReadTest("ottava3", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpOttava4) {
    gpReadTest("ottava4", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxOttava4) {
    gpReadTest("ottava4", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpOttava5) {
    gpReadTest("ottava5", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxOttava5) {
    gpReadTest("ottava5", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpChornamesKeyboard) {
    gpReadTest("chordnames_keyboard", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxChornamesKeyboard) {
    gpReadTest("chordnames_keyboard", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpClefs) {
    gpReadTest("clefs", "gp");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxClefs) {
    gpReadTest("clefs", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxTuplets) {
    gpReadTest("tuplets", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxTuplets2) {
    gpReadTest("tuplets2", "gpx");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp3CapoFret) {
    gpReadTest("capo-fret", "gp3");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp4CapoFret) {
    gpReadTest("capo-fret", "gp4");
}
TEST_F(GuitarPro_AllTests, DISABLED_gp5CapoFret) {
    gpReadTest("capo-fret", "gp5");
}
TEST_F(GuitarPro_AllTests, DISABLED_gpxUncompletedMeasure) {
    gpReadTest("UncompletedMeasure", "gpx");
}
