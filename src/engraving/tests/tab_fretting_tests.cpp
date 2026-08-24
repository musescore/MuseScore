/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

// Fretting of chords on tablature staves: the per-note assignment done by
// convertPitch(), the drop tuning re-fretting done by fretChords(), and the
// tuning classification which decides whether that re-fretting applies.

#include <gtest/gtest.h>

#include <vector>

#include "engraving/dom/chord.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/stringdata.h"

#include "utils/scorerw.h"

using namespace mu::engraving;

namespace {
// Tunings, internal order: index 0 = lowest pitch.
std::vector<int> TUNING_STANDARD = { 40, 45, 50, 55, 59, 64 };   // E2 A2 D3 G3 B3 E4
std::vector<int> TUNING_DROP_D   = { 38, 45, 50, 55, 59, 64 };   // D2 A2 D3 G3 B3 E4
std::vector<int> TUNING_DADGAD   = { 38, 45, 50, 55, 57, 62 };   // D2 A2 D3 G3 A3 D4
std::vector<int> TUNING_OPEN_G   = { 38, 43, 50, 55, 59, 62 };   // D2 G2 D3 G3 B3 D4

using Placement = std::pair<int, int>;                    // (visual string, fret)

StringData makeTuning(std::vector<int>& pitches, int frets = 24)
{
    return StringData(frets, static_cast<int>(pitches.size()), pitches.data());
}

// Greedy assignment for each pitch, in the order given.
std::vector<Placement> greedy(const StringData& sd, const std::vector<int>& pitches)
{
    std::vector<Placement> out;
    for (int p : pitches) {
        int string = -1;
        int fret = -1;
        sd.convertPitch(p, 0, &string, &fret);
        out.push_back({ string, fret });
    }
    return out;
}

// Adjacent-string gaps in VISUAL order (v0 -> v(n-1)), i.e. highest-pitched first.
std::vector<int> visualGaps(std::vector<int>& pitches)
{
    std::vector<int> gaps;
    for (size_t i = pitches.size() - 1; i > 0; --i) {
        gaps.push_back(pitches[i] - pitches[i - 1]);
    }
    return gaps;
}
}

class Engraving_TabFrettingTests : public ::testing::Test
{
};

// ---------------------------------------------------------------------------
// Drop D. From the 5th fret up, a power chord is spread over open strings
// instead of the barre shape which would actually be played.
// ---------------------------------------------------------------------------

TEST_F(Engraving_TabFrettingTests, dropD_powerChord_fret5)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    // G5 = G2, D3, G3
    std::vector<Placement> expected = { { 5, 5 }, { 3, 0 }, { 2, 0 } };
    EXPECT_EQ(greedy(sd, { 43, 50, 55 }), expected);
}

TEST_F(Engraving_TabFrettingTests, dropD_powerChord_fret6_noOpenStrings)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    // G#5 = G#2, D#3, G#3. Same defect, but nothing lands on an open string.
    std::vector<Placement> expected = { { 5, 6 }, { 3, 1 }, { 2, 1 } };
    EXPECT_EQ(greedy(sd, { 44, 51, 56 }), expected);
}

TEST_F(Engraving_TabFrettingTests, dropD_powerChord_fourNote)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    // G2, D3, G3, D4: the 5-5-5-7 grip. D4 lands on the B string, not the high E.
    std::vector<Placement> expected = { { 5, 5 }, { 3, 0 }, { 2, 0 }, { 1, 3 } };
    EXPECT_EQ(greedy(sd, { 43, 50, 55, 62 }), expected);
}

TEST_F(Engraving_TabFrettingTests, dropD_rootAndOctave)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    // G2, G3. G3 is the open G string, not the D string.
    std::vector<Placement> expected = { { 5, 5 }, { 2, 0 } };
    EXPECT_EQ(greedy(sd, { 43, 55 }), expected);
}

TEST_F(Engraving_TabFrettingTests, dropD_belowFret5_isForced)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    // F5 at fret 3: every note forced onto its only reachable string.
    std::vector<Placement> expected = { { 5, 3 }, { 4, 3 }, { 3, 3 } };
    EXPECT_EQ(greedy(sd, { 41, 48, 53 }), expected);
}

TEST_F(Engraving_TabFrettingTests, dropD_fifthOnAString_notDefective)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    // C5 rooted on the A string: C3, G3. Nothing reaches fret 5.
    std::vector<Placement> expected = { { 4, 3 }, { 2, 0 } };
    EXPECT_EQ(greedy(sd, { 48, 55 }), expected);
}

// ---------------------------------------------------------------------------
// Standard tuning must be unaffected.
// ---------------------------------------------------------------------------

TEST_F(Engraving_TabFrettingTests, standard_openG)
{
    StringData sd = makeTuning(TUNING_STANDARD);
    // G2 B2 D3 G3 B3 G4 -> 3-2-0-0-0-3
    std::vector<Placement> expected = { { 5, 3 }, { 4, 2 }, { 3, 0 }, { 2, 0 }, { 1, 0 }, { 0, 3 } };
    EXPECT_EQ(greedy(sd, { 43, 47, 50, 55, 59, 67 }), expected);
}

TEST_F(Engraving_TabFrettingTests, standard_openC)
{
    StringData sd = makeTuning(TUNING_STANDARD);
    // C3 E3 G3 C4 E4 -> x-3-2-0-1-0
    std::vector<Placement> expected = { { 4, 3 }, { 3, 2 }, { 2, 0 }, { 1, 1 }, { 0, 0 } };
    EXPECT_EQ(greedy(sd, { 48, 52, 55, 60, 64 }), expected);
}

TEST_F(Engraving_TabFrettingTests, standard_openD_withHighMelody)
{
    StringData sd = makeTuning(TUNING_STANDARD);
    // D3 A3 D4 A4 -> x-x-0-2-3-5. The high fret sits on the HIGHEST string,
    // which is why a directional predicate must not fire here.
    std::vector<Placement> expected = { { 3, 0 }, { 2, 2 }, { 1, 3 }, { 0, 5 } };
    EXPECT_EQ(greedy(sd, { 50, 57, 62, 69 }), expected);
}

// ---------------------------------------------------------------------------
// Alternate tunings, where the ringing open strings are the point of the tuning.
// ---------------------------------------------------------------------------

TEST_F(Engraving_TabFrettingTests, dadgad_droneChord)
{
    StringData sd = makeTuning(TUNING_DADGAD);
    // G2 D3 G3 A3 D4 -> one fretted note, four open drones.
    std::vector<Placement> expected = { { 5, 5 }, { 3, 0 }, { 2, 0 }, { 1, 0 }, { 0, 0 } };
    EXPECT_EQ(greedy(sd, { 43, 50, 55, 57, 62 }), expected);
}

TEST_F(Engraving_TabFrettingTests, openG_droneChord)
{
    StringData sd = makeTuning(TUNING_OPEN_G);
    // C3 G3 B3 D4 -> x-5-x-0-0-0
    std::vector<Placement> expected = { { 4, 5 }, { 2, 0 }, { 1, 0 }, { 0, 0 } };
    EXPECT_EQ(greedy(sd, { 48, 55, 59, 62 }), expected);
}

// ---------------------------------------------------------------------------
// Interval structure of the tunings used above, for reference.
// ---------------------------------------------------------------------------

TEST_F(Engraving_TabFrettingTests, gapStructure_distinguishesDropFromDrone)
{
    EXPECT_EQ(visualGaps(TUNING_STANDARD), (std::vector<int> { 5, 4, 5, 5, 5 }));
    EXPECT_EQ(visualGaps(TUNING_DROP_D), (std::vector<int> { 5, 4, 5, 5, 7 }));
    EXPECT_EQ(visualGaps(TUNING_DADGAD), (std::vector<int> { 5, 2, 5, 5, 7 }));
    EXPECT_EQ(visualGaps(TUNING_OPEN_G), (std::vector<int> { 3, 4, 5, 7, 5 }));
}

// ---------------------------------------------------------------------------
// isDropLikeTuning(): at least 4 strings, every gap except the lowest a fourth
// or a major third, and the lowest string dropped below that.
// ---------------------------------------------------------------------------

namespace {
std::vector<int> TUNING_7STR_DROP_A = { 33, 40, 45, 50, 55, 59, 64 };  // A1 E2 A2 D3 G3 B3 E4
std::vector<int> TUNING_7STR_DROP_D = { 33, 38, 45, 50, 55, 59, 64 };  // A1 D2 A2 D3 G3 B3 E4
std::vector<int> TUNING_8STR_DROP_E = { 28, 35, 40, 45, 50, 55, 59, 64 };
std::vector<int> TUNING_BASS        = { 28, 33, 38, 43 };              // E1 A1 D2 G2
std::vector<int> TUNING_BASS_DROP_D = { 26, 33, 38, 43 };              // D1 A1 D2 G2
std::vector<int> TUNING_DULCIMER    = { 50, 57, 62 };                  // D3 A3 D4
std::vector<int> TUNING_BANJO5      = { 67, 50, 55, 59, 62 };          // re-entrant
}

TEST_F(Engraving_TabFrettingTests, dropLike_acceptsDropTunings)
{
    EXPECT_TRUE(makeTuning(TUNING_DROP_D).isDropLikeTuning());
    EXPECT_TRUE(makeTuning(TUNING_7STR_DROP_A).isDropLikeTuning());
    EXPECT_TRUE(makeTuning(TUNING_8STR_DROP_E).isDropLikeTuning());
    EXPECT_TRUE(makeTuning(TUNING_BASS_DROP_D).isDropLikeTuning());
}

TEST_F(Engraving_TabFrettingTests, dropLike_rejectsStandardTunings)
{
    EXPECT_FALSE(makeTuning(TUNING_STANDARD).isDropLikeTuning());
    EXPECT_FALSE(makeTuning(TUNING_BASS).isDropLikeTuning());
}

TEST_F(Engraving_TabFrettingTests, dropLike_rejectsDroneTunings)
{
    EXPECT_FALSE(makeTuning(TUNING_DADGAD).isDropLikeTuning());
    EXPECT_FALSE(makeTuning(TUNING_OPEN_G).isDropLikeTuning());
}

TEST_F(Engraving_TabFrettingTests, dropLike_rejectsThreeStringDroneInstruments)
{
    // Mountain dulcimer D3 A3 D4: gaps [7,5] would otherwise qualify.
    EXPECT_FALSE(makeTuning(TUNING_DULCIMER).isDropLikeTuning());
}

TEST_F(Engraving_TabFrettingTests, dropLike_rejectsDropAboveLowestString)
{
    // The shipped 7 string "Drop D" adds a low A below the dropped D, so the wide
    // gap is second from the bottom rather than at the bottom. Known exclusion:
    // the same pattern is used by open tunings such as Open G6.
    EXPECT_FALSE(makeTuning(TUNING_7STR_DROP_D).isDropLikeTuning());
}

// ---------------------------------------------------------------------------
// isMonotonicTuning(): guards the re-entrant tunings of #31909.
// ---------------------------------------------------------------------------

TEST_F(Engraving_TabFrettingTests, monotonic_acceptsOrdinaryTunings)
{
    EXPECT_TRUE(makeTuning(TUNING_STANDARD).isMonotonicTuning());
    EXPECT_TRUE(makeTuning(TUNING_DROP_D).isMonotonicTuning());
}

TEST_F(Engraving_TabFrettingTests, monotonic_rejectsReentrantTuning)
{
    // 5-string banjo: the 5th string is higher-pitched than the 4th.
    EXPECT_FALSE(makeTuning(TUNING_BANJO5).isMonotonicTuning());
}

// ---------------------------------------------------------------------------
// findCompactFingering(). This does not check the tuning itself; that is done by
// the caller in fretChords(). These tests all use drop D.
// ---------------------------------------------------------------------------

namespace {
// Convenience: run the search and return the placements, or an empty vector if
// the function declined to offer a re-fingering.
std::vector<Placement> compact(const StringData& sd, const std::vector<int>& pitches)
{
    std::vector<Placement> out;
    if (!sd.findCompactFingering(pitches, &out)) {
        return {};
    }
    return out;
}
}

TEST_F(Engraving_TabFrettingTests, compact_dropD_powerChord_becomesBarre)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    std::vector<Placement> expected = { { 5, 5 }, { 4, 5 }, { 3, 5 } };
    EXPECT_EQ(compact(sd, { 43, 50, 55 }), expected);
}

TEST_F(Engraving_TabFrettingTests, compact_dropD_fret6_becomesBarre)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    std::vector<Placement> expected = { { 5, 6 }, { 4, 6 }, { 3, 6 } };
    EXPECT_EQ(compact(sd, { 44, 51, 56 }), expected);
}

TEST_F(Engraving_TabFrettingTests, compact_dropD_fourNoteGrip)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    // 5-5-5-7: a barre plus one finger.
    std::vector<Placement> expected = { { 5, 5 }, { 4, 5 }, { 3, 5 }, { 2, 7 } };
    EXPECT_EQ(compact(sd, { 43, 50, 55, 62 }), expected);
}

TEST_F(Engraving_TabFrettingTests, compact_dropD_rootAndOctave_skipsAString)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    // G3 on the A string would be the 10th fret, out of reach, so it lands on D.
    std::vector<Placement> expected = { { 5, 5 }, { 3, 5 } };
    EXPECT_EQ(compact(sd, { 43, 55 }), expected);
}

TEST_F(Engraving_TabFrettingTests, compact_declinesWhenResultIsNotCompact)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    // G2 + E4 can only be fretted 5 and 9, too far apart to be worth imposing.
    EXPECT_EQ(compact(sd, { 43, 64 }), std::vector<Placement> {});
}

TEST_F(Engraving_TabFrettingTests, compact_declinesSingleNote)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    // Nothing to re-fret.
    EXPECT_EQ(compact(sd, { 43 }), std::vector<Placement> {});
}

TEST_F(Engraving_TabFrettingTests, compact_declinesPitchBelowLowestString)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    // C2 is below the dropped D, so no string can reach it.
    EXPECT_EQ(compact(sd, { 36, 50 }), std::vector<Placement> {});
}

// ---------------------------------------------------------------------------
// needsCompactFingering(). True when a note sits further than a hand span below
// the highest fretted note and on a higher string, so it could have been fretted
// up in position instead.
// ---------------------------------------------------------------------------

TEST_F(Engraving_TabFrettingTests, trigger_firesOnDropDPowerChord)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    EXPECT_TRUE(sd.needsCompactFingering({ { 5, 5 }, { 3, 0 }, { 2, 0 } }));
}

TEST_F(Engraving_TabFrettingTests, trigger_firesAtFret6WithNoOpenStrings)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    // Same defect at the 6th fret, where no open string is involved at all.
    EXPECT_TRUE(sd.needsCompactFingering({ { 5, 6 }, { 3, 1 }, { 2, 1 } }));
}

TEST_F(Engraving_TabFrettingTests, trigger_firesOnFourNoteGrip)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    EXPECT_TRUE(sd.needsCompactFingering({ { 5, 5 }, { 3, 0 }, { 2, 0 }, { 1, 3 } }));
}

TEST_F(Engraving_TabFrettingTests, trigger_silentBelowFret5)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    // F5 at fret 3, and the C5 shape rooted on the A string.
    EXPECT_FALSE(sd.needsCompactFingering({ { 5, 3 }, { 4, 3 }, { 3, 3 } }));
    EXPECT_FALSE(sd.needsCompactFingering({ { 4, 3 }, { 2, 0 } }));
}

TEST_F(Engraving_TabFrettingTests, trigger_silentWhenHighFretIsOnHighestString)
{
    StringData sd = makeTuning(TUNING_STANDARD);
    // Open D chord with an A4 melody. The fretted note is on the highest string,
    // so no open note could have been fretted up in its position.
    EXPECT_FALSE(sd.needsCompactFingering({ { 3, 0 }, { 2, 2 }, { 1, 3 }, { 0, 5 } }));
}

TEST_F(Engraving_TabFrettingTests, trigger_silentOnStandardOpenChords)
{
    StringData sd = makeTuning(TUNING_STANDARD);
    EXPECT_FALSE(sd.needsCompactFingering({ { 5, 3 }, { 4, 2 }, { 3, 0 }, { 2, 0 }, { 1, 0 }, { 0, 3 } }));
    EXPECT_FALSE(sd.needsCompactFingering({ { 4, 3 }, { 3, 2 }, { 2, 0 }, { 1, 1 }, { 0, 0 } }));
}

TEST_F(Engraving_TabFrettingTests, trigger_silentOnStandardDMinor)
{
    StringData sd = makeTuning(TUNING_STANDARD);
    // x-5-3-2-3-x. The 5th fret here comes from conflict resolution, but the lowest
    // fret is 2, which is still within reach of it.
    EXPECT_FALSE(sd.needsCompactFingering({ { 4, 5 }, { 3, 3 }, { 2, 2 }, { 1, 3 } }));
}

TEST_F(Engraving_TabFrettingTests, compact_honoursPitchOffset)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    // Guitar is a transposing instrument, so the caller passes the staff pitch offset
    // through. +2 moves the barre up two frets, as fret() already does.
    std::vector<Placement> out;
    ASSERT_TRUE(sd.findCompactFingering({ 43, 50, 55 }, &out, 2));
    std::vector<Placement> expected = { { 5, 7 }, { 4, 7 }, { 3, 7 } };
    EXPECT_EQ(out, expected);
}

// ---------------------------------------------------------------------------
// The re-fretting as it runs inside fretChords(), on a real score.
// ---------------------------------------------------------------------------

namespace {
const String FRETTING_DATA_DIR(u"tab_fretting_data/");

// Every note's (visual string, fret), sorted by string so comparisons are stable.
std::vector<Placement> tabOf(MasterScore* score)
{
    std::vector<Placement> result;
    Measure* m = score->firstMeasure();
    if (!m) {
        return result;
    }
    for (Segment& seg : m->segments()) {
        if (!seg.isChordRestType()) {
            continue;
        }
        for (track_idx_t trk = 0; trk < VOICES; ++trk) {
            EngravingItem* el = seg.element(trk);
            if (!el || !el->isChord()) {
                continue;
            }
            for (Note* note : toChord(el)->notes()) {
                result.push_back({ note->string(), note->fret() });
            }
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}
}

TEST_F(Engraving_TabFrettingTests, integration_dropDPowerChordBecomesBarre)
{
    MasterScore* score = ScoreRW::readScore(FRETTING_DATA_DIR + u"dropd_power_chord.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    // G2 D3 G3 in drop D, entered with no fretting. Without the re-fretting this
    // comes out as the 5th fret of the low string plus two open strings.
    std::vector<Placement> expected = { { 3, 5 }, { 4, 5 }, { 5, 5 } };
    EXPECT_EQ(tabOf(score), expected);

    delete score;
}

TEST_F(Engraving_TabFrettingTests, integration_dadgadDroneIsUntouched)
{
    MasterScore* score = ScoreRW::readScore(FRETTING_DATA_DIR + u"dadgad_drone.mscx");
    ASSERT_TRUE(score);
    score->doLayout();

    // DADGAD is not a drop tuning, so the open strings are left alone.
    std::vector<Placement> expected = { { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 5, 5 } };
    EXPECT_EQ(tabOf(score), expected);

    delete score;
}

TEST_F(Engraving_TabFrettingTests, dropD_fullChordAgainstOpenTopStrings)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    // G2 D3 G3 B3 E4 in drop D: a fretted bass against the four standard-tuned
    // strings ringing open. Re-fretted as an A shape barre at the 5th fret.
    std::vector<Placement> today = greedy(sd, { 43, 50, 55, 59, 64 });
    EXPECT_EQ(today, (std::vector<Placement> { { 5, 5 }, { 3, 0 }, { 2, 0 }, { 1, 0 }, { 0, 0 } }));

    std::vector<Placement> sorted = today;
    std::sort(sorted.begin(), sorted.end());
    EXPECT_TRUE(sd.needsCompactFingering(today));

    std::vector<Placement> out;
    const bool adopted = sd.findCompactFingering({ 43, 50, 55, 59, 64 }, &out);
    EXPECT_TRUE(adopted);
    EXPECT_EQ(out, (std::vector<Placement> { { 5, 5 }, { 4, 5 }, { 3, 5 }, { 2, 4 }, { 1, 5 } }));
}

TEST_F(Engraving_TabFrettingTests, compact_aloneIsNotEnoughToDecide)
{
    StringData sd = makeTuning(TUNING_DROP_D);
    // C5 rooted on the A string frets perfectly well as a barre at the 10th, so the
    // compact search on its own would move it right up the neck. Note by note it is
    // A string 3rd fret plus the open G, which is what should be kept. This is why
    // fretChordAsUnit() compares against the note by note fretting before replacing
    // anything, rather than just taking a compact shape whenever one exists.
    std::vector<Placement> out;
    ASSERT_TRUE(sd.findCompactFingering({ 48, 55 }, &out));
    EXPECT_EQ(out, (std::vector<Placement> { { 5, 10 }, { 4, 10 } }));

    // and the trigger is what stops it: note by note is already within reach
    EXPECT_FALSE(sd.needsCompactFingering(greedy(sd, { 48, 55 })));
}
