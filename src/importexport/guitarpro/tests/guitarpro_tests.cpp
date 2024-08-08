/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "engraving/tests/utils/scorerw.h"
#include "engraving/tests/utils/scorecomp.h"

#include "engraving/dom/masterscore.h"
#include "engraving/dom/excerpt.h"

#include "modularity/ioc.h"
#include "importexport/guitarpro/iguitarproconfiguration.h"

using namespace mu;
using namespace mu::engraving;

static const String GUITARPRO_DIR(u"data/");

namespace mu::iex::guitarpro {
extern Err importGTP(MasterScore*, muse::io::IODevice* io, const muse::modularity::ContextPtr& iocCtx, bool createLinkedTabForce = false,
                     bool experimental = false);

class GuitarPro_Tests : public ::testing::Test
{
public:
    void gpReadTest(const char* file,  const char* ext);
};

void GuitarPro_Tests::gpReadTest(const char* file, const char* ext)
{
    String fileName = String::fromUtf8(file) + u'.' + String::fromUtf8(ext);

    auto importFunc = [](MasterScore* score, const muse::io::path_t& path) -> Err {
        muse::io::File file(path);
        return importGTP(score, &file, muse::modularity::globalCtx());
    };

    MasterScore* score = ScoreRW::readScore(GUITARPRO_DIR + fileName, false, importFunc);
    EXPECT_TRUE(score);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, fileName + u".mscx", GUITARPRO_DIR + fileName + u"-ref.mscx"));
    delete score;
}

TEST_F(GuitarPro_Tests, gpSforzato) {
    gpReadTest("sforzato", "gp");
}
TEST_F(GuitarPro_Tests, gpxSforzato) {
    gpReadTest("sforzato", "gpx");
}
TEST_F(GuitarPro_Tests, gp4Sforzato) {
    gpReadTest("sforzato", "gp4");
}
TEST_F(GuitarPro_Tests, gpHeavyAccent) {
    gpReadTest("heavy-accent", "gp");
}
TEST_F(GuitarPro_Tests, gpxHeavyAccent) {
    gpReadTest("heavy-accent", "gpx");
}
TEST_F(GuitarPro_Tests, gp5HeavyAccent) {
    gpReadTest("heavy-accent", "gp5");
}
TEST_F(GuitarPro_Tests, gpTremolos) {
    gpReadTest("tremolos", "gp");
}
TEST_F(GuitarPro_Tests, gpxTremolos) {
    gpReadTest("tremolos", "gpx");
}
TEST_F(GuitarPro_Tests, gp5Tremolos) {
    gpReadTest("tremolos", "gp5");
}
TEST_F(GuitarPro_Tests, gpTrill) {
    gpReadTest("trill", "gp");
}
TEST_F(GuitarPro_Tests, gpxTrill) {
    gpReadTest("trill", "gpx");
}
TEST_F(GuitarPro_Tests, gp4Trill) {
    gpReadTest("trill", "gp4");
}
TEST_F(GuitarPro_Tests, gpChordWithTiedHarmonics) {
    gpReadTest("chord_with_tied_harmonics", "gp");
}
TEST_F(GuitarPro_Tests, gp5ChordWithTiedHarmonics) {
    gpReadTest("chord_with_tied_harmonics", "gp5");
}
TEST_F(GuitarPro_Tests, gpDynamic) {
    gpReadTest("dynamic", "gp");
}
TEST_F(GuitarPro_Tests, gpxDynamic) {
    gpReadTest("dynamic", "gpx");
}
TEST_F(GuitarPro_Tests, gp5Dynamic) {
    gpReadTest("dynamic", "gp5");
}
TEST_F(GuitarPro_Tests, gpGraceNote) {
    gpReadTest("grace", "gp");
}
TEST_F(GuitarPro_Tests, gpxGraceNote) {
    gpReadTest("grace", "gpx");
}
TEST_F(GuitarPro_Tests, gp5GraceNote) {
    gpReadTest("grace", "gp5");
}
TEST_F(GuitarPro_Tests, gpVolta) {
    gpReadTest("volta", "gp");
}
TEST_F(GuitarPro_Tests, gpxVolta) {
    gpReadTest("volta", "gpx");
}
TEST_F(GuitarPro_Tests, gp5Volta) {
    gpReadTest("volta", "gp5");
}
TEST_F(GuitarPro_Tests, gp4Volta) {
    gpReadTest("volta", "gp4");
}
TEST_F(GuitarPro_Tests, gp3Volta) {
    gpReadTest("volta", "gp3");
}
TEST_F(GuitarPro_Tests, gpcopyright) {
    gpReadTest("copyright", "gp");
}
TEST_F(GuitarPro_Tests, gpxcopyright) {
    gpReadTest("copyright", "gpx");
}
TEST_F(GuitarPro_Tests, gp5copyright) {
    gpReadTest("copyright", "gp5");
}
TEST_F(GuitarPro_Tests, gp4copyright) {
    gpReadTest("copyright", "gp4");
}
TEST_F(GuitarPro_Tests, gp3copyright) {
    gpReadTest("copyright", "gp3");
}
TEST_F(GuitarPro_Tests, gpTempo) {
    gpReadTest("tempo", "gp");
}
TEST_F(GuitarPro_Tests, gpxTempo) {
    gpReadTest("tempo", "gpx");
}
TEST_F(GuitarPro_Tests, gp5Tempo) {
    gpReadTest("tempo", "gp5");
}
TEST_F(GuitarPro_Tests, gp4Tempo) {
    gpReadTest("tempo", "gp4");
}
TEST_F(GuitarPro_Tests, gp3Tempo) {
    gpReadTest("tempo", "gp3");
}
TEST_F(GuitarPro_Tests, gpBasicBend) {
    gpReadTest("basic-bend", "gp");
}
TEST_F(GuitarPro_Tests, gpxBasicBend) {
    gpReadTest("basic-bend", "gpx");
}
TEST_F(GuitarPro_Tests, gp5BasicBend) {
    gpReadTest("basic-bend", "gp5");
}
TEST_F(GuitarPro_Tests, gpBend) {
    gpReadTest("bend", "gp");
}
TEST_F(GuitarPro_Tests, gpxBend) {
    gpReadTest("bend", "gpx");
}
TEST_F(GuitarPro_Tests, gp5Bend) {
    gpReadTest("bend", "gp5");
}
TEST_F(GuitarPro_Tests, gp4Bend) {
    gpReadTest("bend", "gp4");
}
TEST_F(GuitarPro_Tests, gp3Bend) {
    gpReadTest("bend", "gp3");
}
TEST_F(GuitarPro_Tests, gpBendAndHarmonic) {
    gpReadTest("bend_and_harmonic", "gp");
}
TEST_F(GuitarPro_Tests, gp5BendAndHarmonic) {
    gpReadTest("bend_and_harmonic", "gp5");
}
TEST_F(GuitarPro_Tests, gpKeysig) {
    gpReadTest("keysig", "gp");
}
TEST_F(GuitarPro_Tests, gpxKeysig) {
    gpReadTest("keysig", "gpx");
}
TEST_F(GuitarPro_Tests, gp5Keysig) {
    gpReadTest("keysig", "gp5");
}
TEST_F(GuitarPro_Tests, gp4Keysig) {
    gpReadTest("keysig", "gp4");
}
TEST_F(GuitarPro_Tests, gpDottedTuplets) {
    gpReadTest("dotted-tuplets", "gp");
}
TEST_F(GuitarPro_Tests, gpxDottedTuplets) {
    gpReadTest("dotted-tuplets", "gpx");
}
TEST_F(GuitarPro_Tests, gp5DottedTuplets) {
    gpReadTest("dotted-tuplets", "gp5");
}
TEST_F(GuitarPro_Tests, gpTupletSlur) {
    gpReadTest("tuplet-with-slur", "gp");
}
TEST_F(GuitarPro_Tests, gpxTupletSlur) {
    gpReadTest("tuplet-with-slur", "gpx");
}
TEST_F(GuitarPro_Tests, gp4TupletSlur) {
    gpReadTest("tuplet-with-slur", "gp4");
}
TEST_F(GuitarPro_Tests, gpBeamsStemsLL) {
    gpReadTest("beams-stems-ledger-lines", "gp");
}
TEST_F(GuitarPro_Tests, gpxBeamsStemsLL) {
    gpReadTest("beams-stems-ledger-lines", "gpx");
}
TEST_F(GuitarPro_Tests, gp5BeamsStemsLL) {
    gpReadTest("beams-stems-ledger-lines", "gp5");
}
TEST_F(GuitarPro_Tests, gpFretDiagram_2Instr) {
    gpReadTest("fret-diagram_2instruments", "gp");
}
TEST_F(GuitarPro_Tests, gpxFretDiagram_2Instr) {
    gpReadTest("fret-diagram_2instruments", "gpx");
}
TEST_F(GuitarPro_Tests, gpFretDiagram) {
    gpReadTest("fret-diagram", "gp");
}
TEST_F(GuitarPro_Tests, gpxFretDiagram) {
    gpReadTest("fret-diagram", "gpx");
}
TEST_F(GuitarPro_Tests, gp5FretDiagram) {
    gpReadTest("fret-diagram", "gp5");
}
TEST_F(GuitarPro_Tests, gp4FretDiagram) {
    gpReadTest("fret-diagram", "gp4");
}
TEST_F(GuitarPro_Tests, gpFadeIn) {
    gpReadTest("fade-in", "gp");
}
TEST_F(GuitarPro_Tests, gpxFadeIn) {
    gpReadTest("fade-in", "gpx");
}
TEST_F(GuitarPro_Tests, gp5FadeIn) {
    gpReadTest("fade-in", "gp5");
}
TEST_F(GuitarPro_Tests, gp4FadeIn) {
    gpReadTest("fade-in", "gp4");
}
TEST_F(GuitarPro_Tests, gpSlurNoteMask) {
    gpReadTest("slur-notes-effect-mask", "gp");
}
TEST_F(GuitarPro_Tests, gpxSlurNoteMask) {
    gpReadTest("slur-notes-effect-mask", "gpx");
}
TEST_F(GuitarPro_Tests, gp5SlurNoteMask) {
    gpReadTest("slur-notes-effect-mask", "gp5");
}
TEST_F(GuitarPro_Tests, gpCentered) {
    gpReadTest("rest-centered", "gp");
}
TEST_F(GuitarPro_Tests, gpxCentered) {
    gpReadTest("rest-centered", "gpx");
}
TEST_F(GuitarPro_Tests, gp5Centered) {
    gpReadTest("rest-centered", "gp5");
}
TEST_F(GuitarPro_Tests, gp4Centered) {
    gpReadTest("rest-centered", "gp4");
}
TEST_F(GuitarPro_Tests, gpSlideInAbove) {
    gpReadTest("slide-in-above", "gp");
}
TEST_F(GuitarPro_Tests, gpxSlideInAbove) {
    gpReadTest("slide-in-above", "gpx");
}
TEST_F(GuitarPro_Tests, gp5SlideInAbove) {
    gpReadTest("slide-in-above", "gp5");
}
TEST_F(GuitarPro_Tests, gp4SlideInAbove) {
    gpReadTest("slide-in-above", "gp4");
}
TEST_F(GuitarPro_Tests, gpSlideInBelow) {
    gpReadTest("slide-in-below", "gp");
}
TEST_F(GuitarPro_Tests, gpxSlideInBelow) {
    gpReadTest("slide-in-below", "gpx");
}
TEST_F(GuitarPro_Tests, gp5SlideInBelow) {
    gpReadTest("slide-in-below", "gp5");
}
TEST_F(GuitarPro_Tests, gp4SlideInBelow) {
    gpReadTest("slide-in-below", "gp4");
}
TEST_F(GuitarPro_Tests, gpSlideOutUp) {
    gpReadTest("slide-out-up", "gp");
}
TEST_F(GuitarPro_Tests, gpxSlideOutUp) {
    gpReadTest("slide-out-up", "gpx");
}
TEST_F(GuitarPro_Tests, gp5SlideOutUp) {
    gpReadTest("slide-out-up", "gp5");
}
TEST_F(GuitarPro_Tests, gp4SlideOutUp) {
    gpReadTest("slide-out-up", "gp4");
}
TEST_F(GuitarPro_Tests, gpSlideOutDown) {
    gpReadTest("slide-out-down", "gp");
}
TEST_F(GuitarPro_Tests, gpxSlideOutDown) {
    gpReadTest("slide-out-down", "gpx");
}
TEST_F(GuitarPro_Tests, gp5SlideOutDown) {
    gpReadTest("slide-out-down", "gp5");
}
TEST_F(GuitarPro_Tests, gp4SlideOutDown) {
    gpReadTest("slide-out-down", "gp4");
}
TEST_F(GuitarPro_Tests, gpLegatoSlide) {
    gpReadTest("legato-slide", "gp");
}
TEST_F(GuitarPro_Tests, gpxLegatoSlide) {
    gpReadTest("legato-slide", "gpx");
}
TEST_F(GuitarPro_Tests, gp5LegatoSlide) {
    gpReadTest("legato-slide", "gp5");
}
TEST_F(GuitarPro_Tests, gp4LegatoSlide) {
    gpReadTest("legato-slide", "gp4");
}
TEST_F(GuitarPro_Tests, gpShiftSlide) {
    gpReadTest("shift-slide", "gp");
}
TEST_F(GuitarPro_Tests, gpxShiftSlide) {
    gpReadTest("shift-slide", "gpx");
}
TEST_F(GuitarPro_Tests, gp5ShiftSlide) {
    gpReadTest("shift-slide", "gp5");
}
TEST_F(GuitarPro_Tests, gp4ShiftSlide) {
    gpReadTest("shift-slide", "gp4");
}
TEST_F(GuitarPro_Tests, gpDoubleBar) {
    gpReadTest("double-bar", "gp");
}
TEST_F(GuitarPro_Tests, gpxDoubleBar) {
    gpReadTest("double-bar", "gpx");
}
TEST_F(GuitarPro_Tests, gpCrecDim) {
    gpReadTest("crescendo-diminuendo", "gp");
}
TEST_F(GuitarPro_Tests, gpxCrecDim) {
    gpReadTest("crescendo-diminuendo", "gpx");
}
TEST_F(GuitarPro_Tests, gpDeadNote) {
    gpReadTest("dead-note", "gp");
}
TEST_F(GuitarPro_Tests, gpxDeadNote) {
    gpReadTest("dead-note", "gpx");
}
TEST_F(GuitarPro_Tests, gpWah) {
    gpReadTest("wah", "gp");
}
TEST_F(GuitarPro_Tests, gpxWah) {
    gpReadTest("wah", "gpx");
}
TEST_F(GuitarPro_Tests, gpAccent) {
    gpReadTest("accent", "gp");
}
TEST_F(GuitarPro_Tests, gpxAccent) {
    gpReadTest("accent", "gpx");
}
TEST_F(GuitarPro_Tests, gpArpeggio) {
    gpReadTest("arpeggio", "gp");
}
TEST_F(GuitarPro_Tests, gpxArpeggio) {
    gpReadTest("arpeggio", "gpx");
}
TEST_F(GuitarPro_Tests, gpTurn) {
    gpReadTest("turn", "gp");
}
TEST_F(GuitarPro_Tests, gpxTurn) {
    gpReadTest("turn", "gpx");
}
TEST_F(GuitarPro_Tests, gpMordents) {
    gpReadTest("mordents", "gp");
}
TEST_F(GuitarPro_Tests, gpxMordents) {
    gpReadTest("mordents", "gpx");
}
TEST_F(GuitarPro_Tests, gpPickUpDown) {
    gpReadTest("pick-up-down", "gp");
}
TEST_F(GuitarPro_Tests, gpxPickUpDown) {
    gpReadTest("pick-up-down", "gpx");
}
TEST_F(GuitarPro_Tests, gp5PickUpDown) {
    gpReadTest("pick-up-down", "gp5");
}
TEST_F(GuitarPro_Tests, gp4PickUpDown) {
    gpReadTest("pick-up-down", "gp4");
}
TEST_F(GuitarPro_Tests, gpFingering) {
    gpReadTest("fingering", "gp");
}
TEST_F(GuitarPro_Tests, gpxFingering) {
    gpReadTest("fingering", "gpx");
}
TEST_F(GuitarPro_Tests, gp5Fingering) {
    gpReadTest("fingering", "gp5");
}
TEST_F(GuitarPro_Tests, gp4Fingering) {
    gpReadTest("fingering", "gp4");
}
TEST_F(GuitarPro_Tests, gpBrush) {
    gpReadTest("brush", "gp");
}
TEST_F(GuitarPro_Tests, gpxBrush) {
    gpReadTest("brush", "gpx");
}
TEST_F(GuitarPro_Tests, gp5Brush) {
    gpReadTest("brush", "gp5");
}
TEST_F(GuitarPro_Tests, gp4Brush) {
    gpReadTest("brush", "gp4");
}
TEST_F(GuitarPro_Tests, gpRepeats) {
    gpReadTest("repeats", "gp");
}
TEST_F(GuitarPro_Tests, gpxRepeats) {
    gpReadTest("repeats", "gpx");
}
TEST_F(GuitarPro_Tests, gpGraceBefore) {
    gpReadTest("grace-before-beat", "gp");
}
TEST_F(GuitarPro_Tests, gpxGraceBefore) {
    gpReadTest("grace-before-beat", "gpx");
}
TEST_F(GuitarPro_Tests, gpGraceOn) {
    gpReadTest("grace-on-beat", "gp");
}
TEST_F(GuitarPro_Tests, gpxGraceOn) {
    gpReadTest("grace-on-beat", "gpx");
}
TEST_F(GuitarPro_Tests, gpGraceDurations) {
    gpReadTest("grace-durations", "gp");
}
TEST_F(GuitarPro_Tests, gpPalmMute) {
    gpReadTest("palm-mute", "gp");
}
TEST_F(GuitarPro_Tests, gpxPalmMute) {
    gpReadTest("palm-mute", "gpx");
}
TEST_F(GuitarPro_Tests, gp5PalmMute) {
    gpReadTest("palm-mute", "gp5");
}
TEST_F(GuitarPro_Tests, gp4PalmMute) {
    gpReadTest("palm-mute", "gp4");
}
TEST_F(GuitarPro_Tests, gpLetRing) {
    gpReadTest("let-ring", "gp");
}
TEST_F(GuitarPro_Tests, gpxLetRing) {
    gpReadTest("let-ring", "gpx");
}
TEST_F(GuitarPro_Tests, gp5LetRing) {
    gpReadTest("let-ring", "gp5");
}
TEST_F(GuitarPro_Tests, gp4LetRing) {
    gpReadTest("let-ring", "gp4");
}
TEST_F(GuitarPro_Tests, gpTapSlapPop) {
    gpReadTest("tap-slap-pop", "gp");
}
TEST_F(GuitarPro_Tests, gpxTapSlapPop) {
    gpReadTest("tap-slap-pop", "gpx");
}
TEST_F(GuitarPro_Tests, gp5TapSlapPop) {
    gpReadTest("tap-slap-pop", "gp5");
}
TEST_F(GuitarPro_Tests, gpBarre) {
    gpReadTest("barre", "gp");
}
TEST_F(GuitarPro_Tests, gpxBarre) {
    gpReadTest("barre", "gpx");
}
TEST_F(GuitarPro_Tests, gpTimer) {
    gpReadTest("timer", "gp");
}
TEST_F(GuitarPro_Tests, gpxTimer) {
    gpReadTest("timer", "gpx");
}
TEST_F(GuitarPro_Tests, gpText) {
    gpReadTest("text", "gp");
}
TEST_F(GuitarPro_Tests, gpxText) {
    gpReadTest("text", "gpx");
}
TEST_F(GuitarPro_Tests, gpArtHarmonic) {
    gpReadTest("artificial-harmonic", "gp");
}
TEST_F(GuitarPro_Tests, gpxArtHarmonic) {
    gpReadTest("artificial-harmonic", "gpx");
}
TEST_F(GuitarPro_Tests, gpGhost) {
    gpReadTest("ghost-note", "gp");
}
TEST_F(GuitarPro_Tests, gpxGhost) {
    gpReadTest("ghost-note", "gpx");
}
TEST_F(GuitarPro_Tests, gp3GhostNote) {
    gpReadTest("ghost_note", "gp3");
}
TEST_F(GuitarPro_Tests, gpRasg) {
    gpReadTest("rasg", "gp");
}
TEST_F(GuitarPro_Tests, gpxRasg) {
    gpReadTest("rasg", "gpx");
}
TEST_F(GuitarPro_Tests, gpPercussion) {
    gpReadTest("all-percussion", "gp");
}
TEST_F(GuitarPro_Tests, DISABLED_gpxPercussion) {
    gpReadTest("all-percussion", "gpx");
}
TEST_F(GuitarPro_Tests, gp5Percussion) {
    gpReadTest("all-percussion", "gp5");
}
TEST_F(GuitarPro_Tests, gpFermata) {
    gpReadTest("fermata", "gp");
}
TEST_F(GuitarPro_Tests, gpxFermata) {
    gpReadTest("fermata", "gpx");
}
TEST_F(GuitarPro_Tests, gpDirections) {
    gpReadTest("directions", "gp");
}
TEST_F(GuitarPro_Tests, gpxDirections) {
    gpReadTest("directions", "gpx");
}
TEST_F(GuitarPro_Tests, gpSlur) {
    gpReadTest("slur", "gp");
}
TEST_F(GuitarPro_Tests, gpxSlur) {
    gpReadTest("slur", "gpx");
}
TEST_F(GuitarPro_Tests, gp4Slur) {
    gpReadTest("slur", "gp4");
}
TEST_F(GuitarPro_Tests, gpSlurHS) {
    gpReadTest("slur_hammer_slur", "gp");
}
TEST_F(GuitarPro_Tests, gpxSlurHS) {
    gpReadTest("slur_hammer_slur", "gpx");
}
TEST_F(GuitarPro_Tests, gpSlur3M) {
    gpReadTest("slur_over_3_measures", "gp");
}
TEST_F(GuitarPro_Tests, gpxSlur3M) {
    gpReadTest("slur_over_3_measures", "gpx");
}
TEST_F(GuitarPro_Tests, gpSlurSH) {
    gpReadTest("slur_slur_hammer", "gp");
}
TEST_F(GuitarPro_Tests, gpxSlurSH) {
    gpReadTest("slur_slur_hammer", "gpx");
}
TEST_F(GuitarPro_Tests, gpSlurV) {
    gpReadTest("slur_voices", "gp");
}
TEST_F(GuitarPro_Tests, gpxSlurV) {
    gpReadTest("slur_voices", "gpx");
}
TEST_F(GuitarPro_Tests, gpVibrato) {
    gpReadTest("vibrato", "gp");
}
TEST_F(GuitarPro_Tests, gpxVibrato) {
    gpReadTest("vibrato", "gpx");
}
TEST_F(GuitarPro_Tests, gp5Vibrato) {
    gpReadTest("vibrato", "gp5");
}
TEST_F(GuitarPro_Tests, gpVolumeSwell) {
    gpReadTest("volume-swell", "gp");
}
TEST_F(GuitarPro_Tests, gpxVolumeSwell) {
    gpReadTest("volume-swell", "gpx");
}
TEST_F(GuitarPro_Tests, gpTremoloBar) {
    gpReadTest("tremolo-bar", "gp");
}
TEST_F(GuitarPro_Tests, gpxTremoloBar) {
    gpReadTest("tremolo-bar", "gpx");
}
TEST_F(GuitarPro_Tests, gpFreeTime) {
    gpReadTest("free-time", "gp");
}
TEST_F(GuitarPro_Tests, gpxFreeTime) {
    gpReadTest("free-time", "gpx");
}
TEST_F(GuitarPro_Tests, gpRepeatBar) {
    gpReadTest("repeated-bars", "gp");
}
TEST_F(GuitarPro_Tests, gpxRepeatBar) {
    gpReadTest("repeated-bars", "gpx");
}
TEST_F(GuitarPro_Tests, gpDottedGliss) {
    gpReadTest("dotted-gliss", "gp");
}
TEST_F(GuitarPro_Tests, gpxDottedGliss) {
    gpReadTest("dotted-gliss", "gpx");
}
TEST_F(GuitarPro_Tests, gp3DottedGliss) {
    gpReadTest("dotted-gliss", "gp3");
}
TEST_F(GuitarPro_Tests, DISABLED_gpHighPitch) {
    gpReadTest("high-pitch", "gp");
}
TEST_F(GuitarPro_Tests, DISABLED_gpxHighPitch) {
    gpReadTest("high-pitch", "gpx");
}
TEST_F(GuitarPro_Tests, DISABLED_gp3HighPitch) {
    gpReadTest("high-pitch", "gp3");
}
TEST_F(GuitarPro_Tests, gpMultiVoices) {
    gpReadTest("multivoices", "gp");
}
TEST_F(GuitarPro_Tests, gpxMultiVoices) {
    gpReadTest("multivoices", "gpx");
}
TEST_F(GuitarPro_Tests, gpOttava1) {
    gpReadTest("ottava1", "gp");
}
TEST_F(GuitarPro_Tests, gpxOttava1) {
    gpReadTest("ottava1", "gpx");
}
TEST_F(GuitarPro_Tests, gpOttava2) {
    gpReadTest("ottava2", "gp");
}
TEST_F(GuitarPro_Tests, gpxOttava2) {
    gpReadTest("ottava2", "gpx");
}
TEST_F(GuitarPro_Tests, gpOttava3) {
    gpReadTest("ottava3", "gp");
}
TEST_F(GuitarPro_Tests, gpxOttava3) {
    gpReadTest("ottava3", "gpx");
}
TEST_F(GuitarPro_Tests, gpOttava4) {
    gpReadTest("ottava4", "gp");
}
TEST_F(GuitarPro_Tests, gpxOttava4) {
    gpReadTest("ottava4", "gpx");
}
TEST_F(GuitarPro_Tests, gpOttava5) {
    gpReadTest("ottava5", "gp");
}
TEST_F(GuitarPro_Tests, gpxOttava5) {
    gpReadTest("ottava5", "gpx");
}
TEST_F(GuitarPro_Tests, gpOttavaSimile) {
    gpReadTest("ottava-simile", "gp");
}
TEST_F(GuitarPro_Tests, gpChornamesKeyboard) {
    gpReadTest("chordnames_keyboard", "gp");
}
TEST_F(GuitarPro_Tests, gpxChornamesKeyboard) {
    gpReadTest("chordnames_keyboard", "gpx");
}
TEST_F(GuitarPro_Tests, gpClefs) {
    gpReadTest("clefs", "gp");
}
TEST_F(GuitarPro_Tests, gpxClefs) {
    gpReadTest("clefs", "gpx");
}
TEST_F(GuitarPro_Tests, gpxTuplets) {
    gpReadTest("tuplets", "gpx");
}
TEST_F(GuitarPro_Tests, gpxTuplets2) {
    gpReadTest("tuplets2", "gpx");
}
TEST_F(GuitarPro_Tests, gp3CapoFret) {
    gpReadTest("capo-fret", "gp3");
}
TEST_F(GuitarPro_Tests, gp4CapoFret) {
    gpReadTest("capo-fret", "gp4");
}
TEST_F(GuitarPro_Tests, gp5CapoFret) {
    gpReadTest("capo-fret", "gp5");
}
TEST_F(GuitarPro_Tests, gpxUncompletedMeasure) {
    gpReadTest("UncompletedMeasure", "gpx");
}
TEST_F(GuitarPro_Tests, gpInstrumentChange) {
    gpReadTest("instr-change", "gp");
}
TEST_F(GuitarPro_Tests, gpxInstrumentChange) {
    gpReadTest("instr-change", "gpx");
}
TEST_F(GuitarPro_Tests, gpInstrumentChange1beat) {
    gpReadTest("instr-change-1-beat", "gp");
}
TEST_F(GuitarPro_Tests, gpxInstrumentChange1beat) {
    gpReadTest("instr-change-1-beat", "gpx");
}
TEST_F(GuitarPro_Tests, gpFixEmptyMeasures) {
    gpReadTest("mmrest", "gp");
}
TEST_F(GuitarPro_Tests, gpLineElements) {
    gpReadTest("line_elements", "gp");
}
TEST_F(GuitarPro_Tests, gp5LineElements) {
    gpReadTest("line_elements", "gp5");
}
TEST_F(GuitarPro_Tests, gp5LetRingTied) {
    gpReadTest("let-ring-tied", "gp5");
}
TEST_F(GuitarPro_Tests, gpPercussionBeams) {
    gpReadTest("percussion-beams", "gp");
}
TEST_F(GuitarPro_Tests, gpSpannerInUncompleteMeasure) {
    gpReadTest("spanner-in-uncomplete-measure", "gp");
}
TEST_F(GuitarPro_Tests, gp5SpannerInUncompleteMeasure) {
    gpReadTest("spanner-in-uncomplete-measure", "gp5");
}
TEST_F(GuitarPro_Tests, gpBarlineLastMeasure) {
    gpReadTest("barline-last-measure", "gp");
}
TEST_F(GuitarPro_Tests, gpBeamModes) {
    gpReadTest("beam-modes", "gp");
}
TEST_F(GuitarPro_Tests, gpHideRests) {
    gpReadTest("hide-rests", "gp");
}
}
