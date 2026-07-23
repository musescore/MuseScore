/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "engraving/compat/scoreaccess.h"

#include "engraving/dom/excerpt.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"

#include "modularity/ioc.h"

using namespace mu::engraving;

static const String GUITARPRO_DIR(u"data/");

namespace mu::iex::guitarpro {
extern Err importGTP(MasterScore*, muse::io::IODevice* io, const muse::modularity::ContextPtr& iocCtx, bool experimental = false);

class GuitarPro_Tests : public ::testing::Test
{
public:
    void gpReadTest(const char* file,  const char* ext);

    // Imports data/<file>.<ext> and reports the importer's Err, without
    // comparing against a reference score and without laying the score out.
    // Layout is deliberately skipped: this is for deliberately malformed
    // fixtures, and importer recovery paths can legitimately leave a score
    // that the engraving layout would reject (see ptbTrackWithoutStrings).
    // Returns nullptr when the import failed.
    MasterScore* gpRead(const char* file, const char* ext, Err& err);
};

// Number of elements of the given type across all tracks of a score. Notes are
// reached through their parent chord, everything else is read off the segments.
static size_t collect(Score* score, ElementType type)
{
    size_t count = 0;
    for (const Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (const Segment* segment = measure->first(); segment; segment = segment->next()) {
            for (const EngravingItem* annotation : segment->annotations()) {
                if (annotation->type() == type) {
                    ++count;
                }
            }
            for (const EngravingItem* item : segment->elist()) {
                if (!item) {
                    continue;
                }
                if (item->type() == type) {
                    ++count;
                }
                if (item->isChord() && type == ElementType::NOTE) {
                    const Chord* chord = toChord(item);
                    count += chord->notes().size();
                    for (const Chord* grace : chord->graceNotes()) {
                        count += grace->notes().size();
                    }
                }
            }
        }
    }
    return count;
}

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

MasterScore* GuitarPro_Tests::gpRead(const char* file, const char* ext, Err& err)
{
    String fileName = String::fromUtf8(file) + u'.' + String::fromUtf8(ext);
    muse::io::path_t path = ScoreRW::rootPath() + u"/" + GUITARPRO_DIR + fileName;

    MasterScore* score = compat::ScoreAccess::createMasterScoreWithBaseStyle(nullptr);

    ScoreLoad sl;
    muse::io::File f(path);
    err = importGTP(score, &f, muse::modularity::globalCtx());

    if (err != Err::NoError) {
        delete score;
        return nullptr;
    }

    return score;
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
TEST_F(GuitarPro_Tests, gpTupletEmptyMeasure) {
    gpReadTest("tuplet-empty-measure", "gp");
}
TEST_F(GuitarPro_Tests, gpSkippedTiedNotes) {
    gpReadTest("skipped_tied_notes", "gp5");
}

TEST_F(GuitarPro_Tests, gpBendAndGlissando) {
    gpReadTest("bend_and_glissando", "gp");
}

TEST_F(GuitarPro_Tests, gp5BendAndGlissando) {
    gpReadTest("bend_and_glissando", "gp5");
}

//---------------------------------------------------------
//   malformed files
//
//   The two .gp4 files below are byte patches of sforzato.gp4, which holds a single note
//   carrying a single dynamic. The three header-only files are synthesised: importGTP()
//   picks its reader by sniffing the content, so they need no valid body.
//---------------------------------------------------------

// The counts the .gp4 patches below measure their deltas against.
TEST_F(GuitarPro_Tests, gp4SforzatoCounts) {
    Err err = Err::UnknownError;
    MasterScore* score = gpRead("sforzato", "gp4", err);
    ASSERT_TRUE(score);
    EXPECT_EQ(err, Err::NoError);
    EXPECT_EQ(collect(score, ElementType::CHORD), 1u);
    EXPECT_EQ(collect(score, ElementType::DYNAMIC), 1u);
    delete score;
}

// GuitarPro::addDynamic() indexes a 9 entry table with the note's dynamic field, here
// patched to 100. Only the dynamic is dropped, so the note itself still arrives.
TEST_F(GuitarPro_Tests, gp4DynamicOutOfRange) {
    Err err = Err::UnknownError;
    MasterScore* score = gpRead("dynamic-out-of-range", "gp4", err);
    ASSERT_TRUE(score);
    EXPECT_EQ(err, Err::NoError);
    EXPECT_EQ(collect(score, ElementType::CHORD), 1u);
    EXPECT_EQ(collect(score, ElementType::DYNAMIC), 0u);
    delete score;
}

// channelDefaults[] holds 64 entries and is indexed by the track's midiChannel, here
// patched to 200. GuitarPro4::read() now rejects the track, which importGTP() reports as
// Err::NoError after showing its own message, leaving the staves it had already created
// but no notes.
TEST_F(GuitarPro_Tests, gp4MidiChannelOutOfRange) {
    Err err = Err::UnknownError;
    MasterScore* score = gpRead("midi-channel-out-of-range", "gp4", err);
    ASSERT_TRUE(score);
    EXPECT_EQ(err, Err::NoError);
    EXPECT_EQ(collect(score, ElementType::CHORD), 0u);
    delete score;
}

// A header matching none of the formats importGTP() recognises.
TEST_F(GuitarPro_Tests, gpUnknownHeader) {
    Err err = Err::UnknownError;
    EXPECT_FALSE(gpRead("unknown-header", "gp4", err));
    EXPECT_EQ(err, Err::FileBadFormat);
}

// A "?FIC" header whose version string matches neither "FICHIER GUITAR PRO " nor
// "FICHIER GUITARE PRO ".
TEST_F(GuitarPro_Tests, gpUnknownVersionString) {
    Err err = Err::UnknownError;
    EXPECT_FALSE(gpRead("unknown-version-string", "gp4", err));
    EXPECT_EQ(err, Err::FileBadFormat);
}

// A recognised version string naming a major version with no reader behind it.
TEST_F(GuitarPro_Tests, gpUnsupportedVersion) {
    Err err = Err::UnknownError;
    EXPECT_FALSE(gpRead("unsupported-version", "gp4", err));
    EXPECT_EQ(err, Err::FileBadFormat);
}

//---------------------------------------------------------
//   PowerTab (.ptb) malformed input regression tests
//
//   There are no real-world .ptb fixtures in this repository (and no writer
//   for the format), so the three files used below are minimal synthetic
//   PowerTab v4 streams built by mirroring the parse order of
//   PowerTab::read() in internal/importptb.cpp. All three share the same
//   skeleton and differ only in the two marked fields:
//
//     "ptab", u16 4                     -> readVersion()
//     u8 2                              -> readSongInfo(), classification 2
//                                          takes neither the 0 nor the 1
//                                          branch, so nothing else is read
//     readDataInstruments() x2          -> PowerTab::read() parses the same
//                                          ptTrack twice (the "guitar score"
//                                          and the "bass score"); after the
//                                          first pass staffInc becomes
//                                          infos.size(), after the second
//                                          staves becomes infos.size()
//
//   Each readDataInstruments() block is:
//     Guitar section          - N x readTrackInfo() (name, midi settings,
//                               tuning name, then a u8 string count followed
//                               by that many midi pitches, high string first)
//     Chord diagram / floating text / GuitarIn / tempo / dynamic / symbol
//                             - all empty (u16 0)
//     Section section         - M x readSection()
//
//   readSection() is: 4 x i32 rect, u8 lastBarData, 4 skipped bytes,
//   readBarLine() (u8 pos, u8 type, u8 keysig, then readTimeSignature()'s
//   3 skipped bytes + u8 26 + u8 pulses, which decodes as 4/4 because
//   numerator = ((26 - 26 % 8) / 8) + 1 = 4 and denominator = 2 ^ (26 % 8) = 4,
//   then an empty rehearsal sign), empty direction/chord-text/rhythm-slash
//   sections, a Staff section of one staff, and an empty MusicBar section.
//   The single staff holds one voice-0 position with one note: the note byte
//   is 0x05, so value (fret) = 0x05 & 0x1F = 5 and str = (0x05 & 0xE0) >> 5 = 0,
//   and the position's duration byte is 1, i.e. a whole note that exactly
//   fills the 4/4 measure.
//
//   The generator and a self-validating re-parser (which confirms each file is
//   consumed to exactly its last byte) are throwaway scripts, not part of the
//   build.
//---------------------------------------------------------

// Baseline: a well formed two-staff file. Block 1 declares one six-string
// guitar track (E4 B3 G3 D3 A2 E2) and block 2 one four-string bass track
// (G2 D2 A1 E1), so staffInc = 1 and staves = 2. getStaffMap() hands
// readStaff() index 0 on the first pass and 0 + staffInc = 1 on the second,
// both valid indices into ptTrack::infos. This pins down what a *correct*
// parse of this skeleton produces, so the two tests below can assert against
// it rather than merely asserting "did not crash".
TEST_F(GuitarPro_Tests, ptbBaseline) {
    Err err = Err::NoError;
    MasterScore* score = gpRead("ptb-baseline", "ptb", err);
    ASSERT_TRUE(score);
    EXPECT_EQ(err, Err::NoError);
    EXPECT_EQ(score->nstaves(), 2u);
    // One note per staff. Fret 5 on string 0 of E4 B3 G3 D3 A2 E2 is 64 + 5,
    // and fret 5 on string 0 of G2 D2 A1 E1 is 43 + 5.
    EXPECT_EQ(collect(score, ElementType::NOTE), 2u);
    delete score;
}

// PowerTab::readPosition() must bounds-check the staff index before doing
// curTrack->infos[staff].notes_count += itemCount.
//
// ptb-oob-staff.ptb is the baseline with block 2's Guitar section item
// count changed from 1 to 0 (and its readTrackInfo() bytes removed), so block 2
// contributes no ptTrack::infos entry while still declaring a section with one
// staff. staffInc is therefore 1 but infos.size() stays 1, and because
// lastStaffMap was cleared between the two passes getStaffMap() falls back to
// {0 + staffInc} = {1}. readStaff(1, ...) -> readPosition(1, 0, ...) then
// indexes infos[1], one element past the end of a one-element
// std::vector<TrackInfo>.
//
// Verified pre-fix failure mode (Debug + AddressSanitizer): TrackInfo is 128
// bytes and notes_count sits at offset 120, so the write lands 120 bytes past
// the end of the 128-byte allocation - past ASAN's default redzone and inside
// a neighbouring live object. The run therefore corrupts the heap silently and
// dies later while destroying the score:
//   AddressSanitizer: BUS ... READ ... in __libcpp_atomic_refcount_decrement
//   #12 mu::engraving::ChordList::~ChordList()
//   #14 mu::engraving::Score::~Score()
// Re-running with ASAN_OPTIONS=redzone=256:max_redzone=2048 pins it down
// directly:
//   ERROR: AddressSanitizer: heap-buffer-overflow ... WRITE of size 4
//   #0 mu::iex::guitarpro::PowerTab::readPosition(...) importptb.cpp:483
//   ... is located 120 bytes after 128-byte region allocated in readTrackInfo
//
// With the guard the bogus beat is dropped, so the score keeps exactly the one
// staff and the one note contributed by the valid first pass.
//
// This fixture also covers addToScore()'s "i < tiedNotes.size()" loop bound,
// but only as defence in depth: dropping the beat here is what keeps
// sec.beats.size() from ever exceeding staves, so with this guard present that
// bound is unreachable. Removing the bound on its own leaves this test passing;
// removing both makes fillMeasure() run for staff 1 of a one-staff score and
// trip "ASSERT FAILED: val < m_score->ntracks()" in EngravingItem::setTrack.
TEST_F(GuitarPro_Tests, ptbInvalidStaffIndex) {
    Err err = Err::NoError;
    MasterScore* score = gpRead("ptb-oob-staff", "ptb", err);
    ASSERT_TRUE(score);
    EXPECT_EQ(err, Err::NoError);
    EXPECT_EQ(score->nstaves(), 1u);
    EXPECT_EQ(collect(score, ElementType::NOTE), 1u);
    delete score;
}

// PowerTab::fillMeasure() must reject a track whose tuning is empty before
// using it to index the part's StringData.
//
// ptb-nostrings.ptb is the baseline with block 1's track string count
// changed from 6 to 0 (and its six tuning bytes removed), so staff 0 has an
// empty ptTrack::infos[0].strings while still carrying a note. addToScore()
// builds StringData(32, 0, ...) for that part, i.e. an empty string list, and
// pre-fix fillMeasure() computes
//     k = std::max(int(strings.size()) - n.str - 1, 0) = std::max(-1, 0) = 0
// and then calls sd->stringList().at(0) on that empty vector.
//
// Verified pre-fix failure mode: the std::vector::at() bounds check throws,
// which googletest reports as
//   C++ exception with description "vector" thrown in the test body.
//
// With the guard the unusable note is skipped and staff 1's note survives.
// Note that the fix's three sub-checks are not individually discriminating
// here: n.str is decoded from three bits so it is always < tiedNotes.size()
// (which is fixed at 10), and removing only the trackStrings.empty() check
// still leaves the following "k >= sd->stringList().size()" check to catch this
// input. This test therefore discriminates on the fillMeasure() hunk as a
// whole.
TEST_F(GuitarPro_Tests, ptbTrackWithoutStrings) {
    Err err = Err::NoError;
    MasterScore* score = gpRead("ptb-nostrings", "ptb", err);
    ASSERT_TRUE(score);
    EXPECT_EQ(err, Err::NoError);
    EXPECT_EQ(score->nstaves(), 2u);
    // Staff 0's note cannot be pitched and is dropped; staff 1's bass note
    // is unaffected.
    EXPECT_EQ(collect(score, ElementType::NOTE), 1u);
    delete score;
}
}
