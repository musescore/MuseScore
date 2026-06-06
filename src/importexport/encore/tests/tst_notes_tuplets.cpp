/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

// Tuplet import: ratio and actual-tick reconstruction, nested/irregular groups, and orphan-member repair.
// See ENCORE_IMPORTER.md §Rhythm: face value, dots, tuplets.

#include <gtest/gtest.h>

#include "engraving/dom/arpeggio.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/barline.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/textbase.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/tuplet.h"

#include "testbase.h"

static const QString ENC_DIR(QString(iex_encore_tests_DATA_ROOT) + "/data/");

using namespace mu::engraving;

static Measure* measureAt(MasterScore* score, int n)
{
    int idx = 0;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        if (idx == n) {
            return toMeasure(mb);
        }
        ++idx;
    }
    return nullptr;
}

class Tst_NotesTuplets : public ::testing::Test, public MTest
{
protected:
    void SetUp() override { setRootDir(ENC_DIR); }
};

TEST_F(Tst_NotesTuplets, explicit_triplets_in_score)
{
    // notes_triplets.enc: measure 1 has 9 explicit triplet eighths (tup=0x32)
    // forming three 3:2 groups.  Verifies tuplets are parsed and have non-zero ticks.
    MasterScore* score = readEncoreScore("notes_triplets.enc");
    ASSERT_NE(score, nullptr);

    int measWithTuplets = 0;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (EngravingItem* e : toMeasure(mb)->el()) {
            if (e->isTuplet()) {
                Tuplet* t = toTuplet(e);
                EXPECT_NE(t->ticks(), Fraction(0, 1)) << "Tuplet ticks must be non-zero";
                EXPECT_EQ(t->ratio().reduced(), Fraction(3, 2)) << "Should be 3:2 triplet";
                ++measWithTuplets;
                break;
            }
        }
    }
    EXPECT_GT(measWithTuplets, 0) << "Should have at least one measure with triplets";
    delete score;
}

TEST_F(Tst_NotesTuplets, tuplet_notes_have_correct_actual_ticks)
{
    // For a 3:2 triplet of eighth notes, actualTicks = (1/8) / (3/2) = 1/12.
    MasterScore* score = readEncoreScore("notes_triplets.enc");
    ASSERT_NE(score, nullptr);

    bool foundTripletNote = false;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (EngravingItem* e : toMeasure(mb)->el()) {
            if (!e->isTuplet()) {
                continue;
            }
            Tuplet* t = toTuplet(e);
            if (t->ratio().reduced() != Fraction(3, 2)) {
                continue;
            }
            for (DurationElement* de : t->elements()) {
                if (!de->isChordRest()) {
                    continue;
                }
                EXPECT_EQ(toChordRest(de)->actualTicks(), Fraction(1, 12))
                    << "Triplet eighth note should have actualTicks = 1/12";
                foundTripletNote = true;
                break;
            }
            if (foundTripletNote) {
                break;
            }
        }
        if (foundTripletNote) {
            break;
        }
    }
    EXPECT_TRUE(foundTripletNote) << "Should find a 3:2 triplet note";
    delete score;
}

TEST_F(Tst_NotesTuplets, tuplet_measure_fills_correctly)
{
    // Both measures of notes_triplets.enc must pass sanityCheck.
    MasterScore* score = readEncoreScore("notes_triplets.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Triplet score should pass sanityCheck: " << ret.text();
    delete score;
}

TEST_F(Tst_NotesTuplets, tuplet_ticks_not_zero)
{
    // Tuplet::ticks() must be non-zero (setTicks called), or checkMeasure sees duration 0 and adds rests.
    MasterScore* score = readEncoreScore("notes_triplets.enc");
    ASSERT_NE(score, nullptr);
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (EngravingItem* e : toMeasure(mb)->el()) {
            if (e->isTuplet()) {
                EXPECT_NE(toTuplet(e)->ticks(), Fraction(0, 1))
                    << "No tuplet should have zero ticks after fix";
            }
        }
    }
    delete score;
}

TEST_F(Tst_NotesTuplets, tuplet_state_cleared_between_measures)
{
    // Tuplet state must be cleared between measures, or a triplet opened in one measure stays active in the
    // next and wrongly absorbs its plain notes.
    MasterScore* score = readEncoreScore("notes_triplets.enc");
    ASSERT_NE(score, nullptr);

    int measIdx = 0;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        ++measIdx;
        for (Segment* seg = m->first(SegmentType::ChordRest); seg;
             seg = seg->next(SegmentType::ChordRest)) {
            EngravingItem* el = seg->element(0);
            if (!el || !el->isChord()) {
                continue;
            }
            Chord* ch = toChord(el);
            if (!ch->tuplet()) {
                int den = ch->durationType().fraction().denominator();
                bool isPow2 = (den > 0) && ((den & (den - 1)) == 0);
                EXPECT_TRUE(isPow2)
                    << "Measure " << measIdx << " non-tuplet chord has denominator "
                    << den << ", tuplet state may have bled from the previous measure";
            }
        }
    }
    delete score;
}

TEST_F(Tst_NotesTuplets, tuplet_note_sorts_before_non_tuplet_at_same_tick)
{
    // notes_tuplet_sort.enc has, at tick=0, a non-tuplet quarter written
    // BEFORE a tuplet eighth in the binary stream.
    // Without the sort fix: the quarter creates the chord → no tuplet started →
    // voice sum = 3/4 (wrong for a 2/4 measure).
    // With the fix: the tuplet eighth sorts first → V_EIGHTH + 3:2 tuplet group →
    // voice sum = 1/4 (triplet) + 1/4 (trailing quarter) = 2/4.
    MasterScore* score = readEncoreScore("notes_tuplet_sort.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Tuplet-sort file should pass sanityCheck: " << ret.text();
    delete score;
}

TEST_F(Tst_NotesTuplets, no_degenerate_tuplet_ratios)
{
    // A degenerate tuplet byte (0xFF -> 15:15) must be skipped; no tuplet may reduce to 1:1.
    MasterScore* score = readEncoreScore("notes_corrupted.enc");
    ASSERT_NE(score, nullptr);
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (EngravingItem* e : toMeasure(mb)->el()) {
            if (!e->isTuplet()) {
                continue;
            }
            Tuplet* t = toTuplet(e);
            Fraction r = t->ratio().reduced();
            EXPECT_NE(r, Fraction(1, 1)) << "No tuplet should have 1:1 ratio";
            EXPECT_LE(r.numerator(), 9) << "Tuplet numerator should be <= 9";
            EXPECT_LE(r.denominator(), 9) << "Tuplet denominator should be <= 9";
        }
    }
    delete score;
}

TEST_F(Tst_NotesTuplets, swing_offgrid_spurious_triplet_removed)
{
    // Note at tick=560 (rdur=160) triggers implied 3:2 triplet but MS offset is off canonical grid (swing artifact).
    // adjustMeasureTuplets removes the spurious triplet; 3 plain quarters = 3/4. Without this: sum overflows 3/4.
    MasterScore* score = readEncoreScore("notes_swing_offgrid.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Off-grid swing file should pass sanityCheck after tuplet removal: "
                     << ret.text();
    delete score;
}

TEST_F(Tst_NotesTuplets, canonical_implied_triplet_preserved)
{
    // notes_canonical_triplet.enc: 3/4, 3 EXPLICIT 3:2 triplet quarter notes
    // (tuplet=0x32) + 1 plain quarter.
    // With faceValue-cumulative placement: tuplets advance by 1/4*2/3=1/6 each.
    // Sum: 3*(1/6) + 1/4 = 1/2 + 1/4 = 3/4 = mLen → sanityCheck passes.
    // Tuplets are explicitly encoded → always detected regardless of implied-detection logic.
    MasterScore* score = readEncoreScore("notes_canonical_triplet.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Explicit triplets should produce correct 3/4 measure: " << ret.text();
    bool hasTuplet = false;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (EngravingItem* e : toMeasure(mb)->el()) {
            if (e->isTuplet()) {
                hasTuplet = true;
                break;
            }
        }
        if (hasTuplet) {
            break;
        }
    }
    EXPECT_TRUE(hasTuplet) << "Explicit 3:2 triplet notes should produce a Tuplet element";
    delete score;
}

TEST_F(Tst_NotesTuplets, overflow_measure_extended)
{
    // notes_overflow_extend.enc: 2/4 measure with 2 notes (fv=2, half).
    // With faceValue-cumulative placement:
    //   Note 1 (fv=2=half): cumTick=0, advance by 1/2. cumTick = 1/2 = mLen.
    //   Note 2: cumTick already = mLen → skipped (voice full).
    // Result: 1 half note = 1/2 = mLen. sanityCheck passes cleanly.
    // Time signature unchanged at 2/4.
    MasterScore* score = readEncoreScore("notes_overflow_extend.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Measure with face-value overflow should pass sanityCheck "
                        "(second note skipped when voice full): " << ret.text();
    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(2, 4)) << "Time signature should stay 2/4";
    delete score;
}

TEST_F(Tst_NotesTuplets, whole_rest_in_partial_measure)
{
    // notes_whole_rest_2_4.enc: 2/4 measure with a single rest (faceValue=1).
    // Encore encodes a whole-measure rest as fv=1 regardless of the time signature.
    // realDuration2DurationType(480, 1) must return V_HALF (not V_WHOLE) because
    // A whole-measure rest in 2/4 must import as a half rest (from rdur), not a whole rest that overfills.
    MasterScore* score = readEncoreScore("notes_whole_rest_2_4.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "2/4 whole-measure rest should pass sanityCheck: " << ret.text();
    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(2, 4)) << "Time signature should be 2/4";
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isRest()) {
            EXPECT_EQ(toRest(e)->durationType().type(), DurationType::V_HALF)
                << "Whole-measure rest in 2/4 should be a half rest (rdur=480 maps to V_HALF)";
            break;
        }
    }
    delete score;
}

TEST_F(Tst_NotesTuplets, explicit_tuplet_facevalue_not_rdur)
{
    // Explicitly-marked tuplet notes must take their duration from the face value, not a truncated rdur
    // (a following rest can shorten the last member's rdur and wrongly demote it).
    MasterScore* score = readEncoreScore("notes_explicit_tup_rdur_truncated.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Explicit triplet with truncated rdur should pass sanityCheck: " << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    int tupletEighthCount = 0;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (!e || !e->isChord()) {
            continue;
        }
        Chord* ch = toChord(e);
        if (ch->tuplet()) {
            EXPECT_EQ(ch->durationType().type(), DurationType::V_EIGHTH)
                << "Explicit tuplet notes should have V_EIGHTH (face value), not V_32ND from rdur";
            ++tupletEighthCount;
        }
    }
    EXPECT_EQ(tupletEighthCount, 3) << "Should have exactly 3 tuplet notes";
    delete score;
}

TEST_F(Tst_NotesTuplets, partial_explicit_group_treated_as_plain)
{
    // A 4th note carrying the tuplet byte after a complete 3:2 group is not a valid group member and must
    // be treated as a plain note, or it starts a partial tuplet and the measure overshoots 4/4.
    MasterScore* score = readEncoreScore("notes_partial_explicit_group.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Partial explicit group should pass sanityCheck: " << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    std::vector<Chord*> chords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            chords.push_back(toChord(e));
        }
    }
    ASSERT_EQ(chords.size(), 5u) << "Should have 5 chords";
    EXPECT_NE(chords[0]->tuplet(), nullptr) << "Note 1 should be in tuplet";
    EXPECT_NE(chords[1]->tuplet(), nullptr) << "Note 2 should be in tuplet";
    EXPECT_NE(chords[2]->tuplet(), nullptr) << "Note 3 should be in tuplet";
    // Chord 4: isolated tup=0x32, must be treated as plain (no tuplet)
    EXPECT_EQ(chords[3]->tuplet(), nullptr) << "Note 4 (isolated tup byte) should NOT be in tuplet";
    // Chord 5: plain
    EXPECT_EQ(chords[4]->tuplet(), nullptr) << "Note 5 (plain) should NOT be in tuplet";
    delete score;
}

TEST_F(Tst_NotesTuplets, dotted_note_capped_to_remaining_space)
{
    // notes_dotted_note_capping.enc: 2/4 measure with 3 sixteenth notes
    // (ticks 0, 40, 80) followed by a quarter note (tick=120, rdur=360).
    // rdur=360 → realDuration2DurationType gives V_QUARTER; calcDots gives dots=1
    // (dotted quarter, 3/8 = 6/16). cumTick after 3 sixteenths = 3/16.
    // remaining = 2/4 - 3/16 = 5/16.
    //
    // Bug: TDuration(V_QUARTER).fraction() = 1/4 = 4/16 ≤ 5/16 → capping does
    //   NOT fire → chord placed as dotted quarter (3/8 = 6/16) → sum=9/16 > 2/4.
    // Fix: include dots in comparison: 6/16 > 5/16 → capping fires → chord placed
    //   as plain quarter (1/4 = 4/16) → sum = 3/16+1/4+1/16(fill) = 8/16 = 2/4. PASS.
    MasterScore* score = readEncoreScore("notes_dotted_note_capping.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Dotted note past measure end should be capped: " << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(2, 4));

    // The 4th chord (dotted Q, rdur=360) must be capped to V_QUARTER (no dots).
    std::vector<Chord*> chords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            chords.push_back(toChord(e));
        }
    }
    ASSERT_GE(chords.size(), 4u) << "Should have at least 4 chords";
    // First 3: V_16TH
    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(chords[i]->durationType().type(), DurationType::V_16TH)
            << "First 3 chords should be sixteenth notes";
    }
    // 4th: capped to V_QUARTER (not dotted quarter which would overflow)
    EXPECT_EQ(chords[3]->durationType().type(), DurationType::V_QUARTER)
        << "Dotted quarter must be capped to plain quarter when it overflows";
    EXPECT_EQ(chords[3]->dots(), 0) << "Capped chord must have 0 dots";
    delete score;
}

TEST_F(Tst_NotesTuplets, dotted_note_dotctrl_bit0_with_rdur_drift)
{
    // When MIDI drift exceeds the snap tolerance (calcDots and calcDotsSnap both return 0), the dotControl
    // bit-0 flag must still force the dot, or a dotted eighth imports plain and leaves a phantom rest.
    MasterScore* score = readEncoreScore("notes_dotted_ctrl_bit0_drift.enc");
    ASSERT_NE(score, nullptr) << "Failed to load notes_dotted_ctrl_bit0_drift.enc";
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Dotted note with rdur drift must not corrupt measure: " << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(2, 4));

    std::vector<Chord*> chords;
    std::vector<Rest*> rests;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (!e) {
            continue;
        }
        if (e->isChord()) {
            chords.push_back(toChord(e));
        } else if (e->isRest()) {
            Rest* r = toRest(e);
            if (!r->isGap()) {
                rests.push_back(r);
            }
        }
    }
    ASSERT_EQ(chords.size(), 4u) << "Measure must have exactly 4 chords";
    EXPECT_EQ(rests.size(), 0u) << "No phantom rests: measure must fill exactly 2/4";

    EXPECT_EQ(chords[0]->durationType().type(), DurationType::V_EIGHTH)
        << "Note 0 base type must be eighth";
    EXPECT_EQ(chords[0]->dots(), 1)
        << "Note 0 must have 1 dot (dotControl bit 0 = dotted flag)";
    EXPECT_EQ(chords[1]->durationType().type(), DurationType::V_16TH);
    EXPECT_EQ(chords[1]->dots(), 0);
    EXPECT_EQ(chords[2]->durationType().type(), DurationType::V_EIGHTH);
    EXPECT_EQ(chords[2]->dots(), 0);
    EXPECT_EQ(chords[3]->durationType().type(), DurationType::V_EIGHTH);
    EXPECT_EQ(chords[3]->dots(), 0);
    delete score;
}

// The dotted-eighth pattern fix must not fire in an already-full measure: the 8th+16th binary pattern is
// ambiguous, so it only applies when faceSum + 60 == durTicks (measure short by an eighth's dot).
TEST_F(Tst_NotesTuplets, v0c2_full_measure_eighth_plus_sixteenth_no_false_dot)
{
    MasterScore* score = readEncoreScore("notes_v0c2_full_measure_no_false_dot.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Full measure must pass sanityCheck: " << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(4, 4));

    std::vector<Chord*> chords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            chords.push_back(toChord(e));
        }
    }
    ASSERT_EQ(chords.size(), 5u)
        << "5 chords expected (8th+16th+16th+8th+8th); spurious dot on first 8th "
        "would overflow the measure and truncate/reshape later notes";
    EXPECT_EQ(chords[0]->durationType().type(), DurationType::V_EIGHTH);
    EXPECT_EQ(chords[0]->dots(), 0) << "First 8th must NOT be dotted (full measure: faceSum guard)";
    EXPECT_EQ(chords[1]->durationType().type(), DurationType::V_16TH);
    EXPECT_EQ(chords[1]->dots(), 0);
    EXPECT_EQ(chords[2]->durationType().type(), DurationType::V_16TH);
    EXPECT_EQ(chords[2]->dots(), 0);
    EXPECT_EQ(chords[3]->durationType().type(), DurationType::V_EIGHTH);
    EXPECT_EQ(chords[4]->durationType().type(), DurationType::V_EIGHTH);
    delete score;
}

TEST_F(Tst_NotesTuplets, mixed_value_tuplet_exact_ticks_and_isolated_partial)
{
    // A mixed-value tuplet group (Q,Q,8th,8th with tup=0x32) is grouped by face-value sum reaching the
    // threshold, closing as one complete group; exact-ticks correction keeps the following plain notes valid.
    MasterScore* score = readEncoreScore("notes_mixed_value_tuplet.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Mixed-value tuplet group should produce clean 2/4: " << ret.text();
    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(2, 4));

    std::vector<Chord*> chords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            chords.push_back(toChord(e));
        }
    }
    ASSERT_EQ(chords.size(), 4u) << "Should have 4 chords (one complete 4-element group)";
    EXPECT_NE(chords[0]->tuplet(), nullptr) << "Q 1 in tuplet";
    EXPECT_NE(chords[1]->tuplet(), nullptr) << "Q 2 in tuplet";
    EXPECT_NE(chords[2]->tuplet(), nullptr) << "8th 3 in tuplet";
    EXPECT_NE(chords[3]->tuplet(), nullptr) << "8th 4 in same complete group";
    EXPECT_EQ(chords[0]->tuplet(), chords[3]->tuplet()) << "All 4 in same tuplet";
    EXPECT_EQ(chords[3]->actualTicks(), Fraction(1, 12))
        << "8th actualTicks = (1/8)*(2/3) = 1/12";
    delete score;
}

TEST_F(Tst_NotesTuplets, truncate_overfull_tuplet_no_partial_tuplet)
{
    // notes_capped_tuplet_note.enc: 4/4 measure with 3 plain quarters
    // (cumTick=3/4) followed by 3 explicit 3:2 triplet quarters (tup=0x32). The full
    // content overflows 4/4.
    //
    // Default overfill strategy is "Remove extra notes" (Truncate): a tuplet is atomic, so
    // the trailing tuplet is DISSOLVED to plain quarters, then trailing notes are removed
    // until the bar is filled, and the last survivor is dotted to fill exactly (here the
    // 3 originals + 1 dissolved quarter fill 4/4 exactly, so no dots and no rest).
    //
    // Regression: the old note-loop cap ripped one note out of the tuplet, leaving an
    // INVALID partial tuplet (3:2 with <3 members) that broke copy/paste with
    // "Tuplet cannot cross barlines". The measure must contain NO tuplet at all.
    MasterScore* score = readEncoreScore("notes_capped_tuplet_note.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Truncated measure must pass sanity check: " << ret.text();
    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(4, 4));
    EXPECT_EQ(m->ticks(), Fraction(4, 4)) << "Truncate keeps a standard 4/4 measure";
    Fraction sum(0, 1);
    int tupletChords = 0;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChordRest()) {
            ChordRest* cr = toChordRest(e);
            sum += cr->actualTicks();
            if (cr->tuplet()) {
                ++tupletChords;
            }
        }
    }
    EXPECT_EQ(tupletChords, 0) << "Truncate must leave NO tuplet (no partial tuplet)";
    EXPECT_EQ(sum, Fraction(4, 4)) << "Voice 0 must sum to exactly 4/4";
    delete score;
}

TEST_F(Tst_NotesTuplets, truncate_overfull_messy_precontent_fills_to_4_4)
{
    // notes_overfull_messy_precontent_tuplet.enc mirrors a real overfull measure: 4/4 with
    // 17/32 of pre-content (16th+32nd+16th+dotted-quarter) then a 3:2 quarter triplet,
    // total 33/32. Default Truncate must dissolve the triplet, drop trailing notes, dot
    // the survivor, and leave an EXACT 4/4 (no underfull gap, no partial tuplet).
    MasterScore* score = readEncoreScore("notes_overfull_messy_precontent_tuplet.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "Truncated messy measure must pass sanity check";
    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->ticks(), Fraction(4, 4)) << "Truncate keeps a standard 4/4 measure";
    Fraction sum(0, 1);
    int tupletChords = 0;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChordRest()) {
            ChordRest* cr = toChordRest(e);
            sum += cr->actualTicks();
            if (cr->tuplet()) {
                ++tupletChords;
            }
        }
    }
    EXPECT_EQ(tupletChords, 0) << "No partial tuplet left";
    EXPECT_EQ(sum, Fraction(4, 4)) << "Voice 0 must fill exactly 4/4 (no underfull gap)";
    delete score;
}

TEST_F(Tst_NotesTuplets, truncate_overfull_tuplet_with_slur_no_crash)
{
    // notes_overfull_tuplet_with_slur.enc: overfull 4/4 with a 3:2 quarter triplet and a
    // SLURSTART spanning into it. Truncate dissolves and removes tuplet members; a slur
    // whose endpoint resolves into the modified region must not leave a dangling reference
    // that crashes during layout or at score teardown.
    MasterScore* score = readEncoreScore("notes_overfull_tuplet_with_slur.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck());
    delete score;
}

TEST_F(Tst_NotesTuplets, mixed_duration_tuplet_boundary_fill)
{
    // Encore omits a tuplet group's final note when it lands on the measure boundary; closeTupletWithFill
    // must add an invisible fill rest for the missing face duration so the measure sums correctly.
    MasterScore* score = readEncoreScore("notes_mixed_duration_tuplet_boundary_fill.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Mixed-duration tuplet with boundary-omitted note should pass sanityCheck: "
                     << ret.text();
    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(4, 4));

    std::vector<ChordRest*> crs;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChordRest()) {
            crs.push_back(toChordRest(e));
        }
    }
    // half + 3 tuplet notes + 1 invisible fill rest = 5 elements
    ASSERT_GE(crs.size(), 5u) << "Expected half + 3 tuplet notes + fill rest";

    EXPECT_EQ(crs[0]->tuplet(), nullptr) << "Half not in tuplet";
    EXPECT_EQ(crs[0]->durationType().type(), DurationType::V_HALF);

    ASSERT_NE(crs[1]->tuplet(), nullptr) << "qtr 1 in tuplet";
    ASSERT_NE(crs[2]->tuplet(), nullptr) << "qtr 2 in tuplet";
    ASSERT_NE(crs[3]->tuplet(), nullptr) << "8th note in tuplet";
    EXPECT_EQ(crs[1]->tuplet(), crs[2]->tuplet()) << "qtrs share bracket";
    EXPECT_EQ(crs[2]->tuplet(), crs[3]->tuplet()) << "8th shares bracket with qtrs";

    ASSERT_NE(crs[4]->tuplet(), nullptr) << "Fill rest in tuplet";
    EXPECT_EQ(crs[3]->tuplet(), crs[4]->tuplet()) << "Fill rest shares bracket";
    EXPECT_TRUE(crs[4]->isRest()) << "Fill is a rest";
    EXPECT_FALSE(crs[4]->visible()) << "Fill rest is invisible";
    EXPECT_EQ(crs[4]->durationType().type(), DurationType::V_EIGHTH) << "Fill rest is 8th";

    delete score;
}

TEST_F(Tst_NotesTuplets, partial_triplet_at_measure_end_no_voice_overflow)
{
    // A 2-note partial 3:2 triplet at the measure end must fit the remaining space (bracket shortened, the
    // last member reduced to a 16th) rather than advancing by full face value and overflowing into voice 1.
    MasterScore* score = readEncoreScore("notes_partial_triplet_measure_end.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Partial measure-end triplet must not corrupt score: " << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(2, 4));

    std::vector<Chord*> chords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            chords.push_back(toChord(e));
        }
    }
    ASSERT_GE(chords.size(), 5u) << "Should have 3 plain eighths + 2 triplet notes";

    // Plain eighths are not in a tuplet.
    EXPECT_EQ(chords[0]->tuplet(), nullptr) << "Plain 8th 1 no tuplet";
    EXPECT_EQ(chords[1]->tuplet(), nullptr) << "Plain 8th 2 no tuplet";
    EXPECT_EQ(chords[2]->tuplet(), nullptr) << "Plain 8th 3 no tuplet";

    // Both partial-triplet notes are in the same tuplet (not overflowed to voice 1).
    EXPECT_NE(chords[3]->tuplet(), nullptr) << "Triplet note 1 (eighth) should be in tuplet";
    EXPECT_NE(chords[4]->tuplet(), nullptr) << "Triplet note 2 (sixteenth) should be in tuplet";
    EXPECT_EQ(chords[3]->tuplet(), chords[4]->tuplet()) << "Both triplet notes in same bracket";

    // Note 1 displays as eighth (2 triplet slots), note 2 as sixteenth (1 slot).
    EXPECT_EQ(chords[3]->durationType().type(), DurationType::V_EIGHTH)
        << "First triplet note: V_EIGHTH (2-slot face value)";
    EXPECT_EQ(chords[4]->durationType().type(), DurationType::V_16TH)
        << "Second triplet note: V_16TH (1-slot, shortened by dt-reduction fix)";

    delete score;
}

TEST_F(Tst_NotesTuplets, cascade_fill_residual_filled_by_vmeasure_rest)
{
    // The post-checkMeasure micro-fill handles cascade-fill residuals:
    // toRhythmicDurationList decomposes a non-standard gap G with standard durations
    // (1/64 + 1/256 + 1/1024 = 21/1024 for G=1/48) but leaves a residual
    // (1/3072 = 1/48 - 21/1024) that no standard duration can represent.
    // A V_MEASURE gap rest with ticks = residual bridges the last 1/3072.
    //
    // We verify this property through two synthetic files that exercise the
    // preconditions: exact-ticks tuplet correction and isolated-partial-tuplet
    // fill. Both must pass sanityCheck with no large denominators from residuals.
    auto checkNoResidual = [&](MasterScore* score) {
        ASSERT_NE(score, nullptr);
        muse::Ret ret = score->sanityCheck();
        EXPECT_TRUE(ret) << "Should pass sanityCheck: " << ret.text();
        for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
            if (!mb->isMeasure()) {
                continue;
            }
            Measure* m = toMeasure(mb);
            for (track_idx_t t = 0; t < static_cast<track_idx_t>(score->nstaves()) * 4; ++t) {
                Fraction sum(0, 1);
                for (Segment* s = m->first(SegmentType::ChordRest);
                     s; s = s->next(SegmentType::ChordRest)) {
                    EngravingItem* e = s->element(t);
                    if (e) {
                        sum += toChordRest(e)->actualTicks();
                    }
                }
                EXPECT_LE(sum.denominator(), 3072)
                    << "Voice sum denom > 3072 indicates unfilled cascade residual";
            }
        }
        delete score;
    };

    // File 1: implied-group boundary (triggers !tt.groupFull() guard)
    checkNoResidual(readEncoreScore("notes_v0c2_implied_group_boundary.enc"));

    // File 2: mixed-value tuplet + isolated partial fill (triggers exact-ticks correction
    // and isolated-partial-tuplet fill; together these may create non-standard gaps)
    checkNoResidual(readEncoreScore("notes_mixed_value_tuplet.enc"));
}

TEST_F(Tst_NotesTuplets, mixed_duration_triplet_face_value_sum_grouping)
{
    // notes_mixed_duration_triplet.enc: 2/4 measure with two 3:2 triplet
    // brackets, the first containing mixed note values (8+8_rest+16_rest+16).
    //
    // The first bracket: face values 1/8+1/8+1/16+1/16 = 3/8 = 3×(1/8) = threshold.
    // The second bracket: 3 equal 8ths, face sum = 3/8 = threshold.
    // Together: 2 × (3/8 × 2/3) = 2 × 1/4 = 1/2 = 2/4. PASS.
    //
    // Bug: with count-based grouping (close after actualN=3 notes), the first
    //   bracket closes after 8+8_rest+16_rest (only 3 elements), then the 16th
    //   note at tick=200 starts a new incomplete group → sum ≠ 2/4 → FAIL.
    // Fix: close when face-value sum reaches actualN × baseLen (= 3/8 for 3:2 8th).
    //   First bracket spans all 4 elements; both brackets fill exactly 2/4.
    MasterScore* score = readEncoreScore("notes_mixed_duration_triplet.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Mixed-duration triplet should produce clean 2/4: " << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(2, 4));

    // Count chords/rests in the measure, should have 7 elements (plus fills)
    // First 4 should be in the same tuplet, next 3 in another.
    std::vector<ChordRest*> crs;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (!e) {
            continue;
        }
        ChordRest* cr = toChordRest(e);
        bool gap = cr->isRest() && toRest(cr)->isGap();
        if (!gap) {
            crs.push_back(cr);
        }
    }
    ASSERT_GE(crs.size(), 7u) << "Should have 7 non-gap elements";
    // Elements 0-3: first mixed-value bracket (all in same tuplet)
    EXPECT_NE(crs[0]->tuplet(), nullptr) << "Element 0 (8th note) in tuplet";
    EXPECT_NE(crs[1]->tuplet(), nullptr) << "Element 1 (8th rest) in tuplet";
    EXPECT_NE(crs[2]->tuplet(), nullptr) << "Element 2 (16th rest) in tuplet";
    EXPECT_NE(crs[3]->tuplet(), nullptr) << "Element 3 (16th note) in tuplet";
    EXPECT_EQ(crs[0]->tuplet(), crs[1]->tuplet()) << "All 4 in same first tuplet";
    EXPECT_EQ(crs[0]->tuplet(), crs[2]->tuplet());
    EXPECT_EQ(crs[0]->tuplet(), crs[3]->tuplet());
    // Elements 4-6: second bracket (3 equal 8ths)
    EXPECT_NE(crs[4]->tuplet(), nullptr) << "Element 4 in second tuplet";
    EXPECT_NE(crs[5]->tuplet(), nullptr) << "Element 5 in second tuplet";
    EXPECT_NE(crs[6]->tuplet(), nullptr) << "Element 6 in second tuplet";
    EXPECT_NE(crs[0]->tuplet(), crs[4]->tuplet()) << "Two different tuplets";
    delete score;
}

TEST_F(Tst_NotesTuplets, mixed_baseLen_QE_bracket_closes_after_two_notes)
{
    // ornaments_tuplet_mixed_baseLen.enc: 4/4 measure with structure:
    //   plain-Q  +  {Q,E} triplet bracket  +  {Q,Q,Q} triplet bracket
    //
    // Bug: the old threshold algorithm used baseLen=Q so the target was 3Q=3/4.
    // faceSum(Q+E)=3/8 never reached it, pulling the following Q into the same
    // bracket and causing a measure overrun.
    //
    // Fix: close the group when faceSum/actualN is a valid TDuration.
    // {Q,E}/3 = E, valid → closes after 2 notes.
    // {Q,Q,Q}/3 = Q, valid → closes after 3 notes.
    // Sum: Q + Q*(2/3) + E*(2/3) + 3*Q*(2/3) = 1/4+1/6+1/12+1/2 = 4/4. PASS.
    MasterScore* score = readEncoreScore("ornaments_tuplet_mixed_baseLen.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Mixed baseLen brackets should produce clean 4/4: " << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(4, 4));

    std::vector<Chord*> chords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            chords.push_back(toChord(e));
        }
    }
    ASSERT_EQ(chords.size(), 6u) << "plain-Q + 2 in bracket1 + 3 in bracket2";

    // chords[0]: plain quarter, not in a tuplet
    EXPECT_EQ(chords[0]->tuplet(), nullptr) << "Plain Q must not be in a tuplet";

    // chords[1] (Q) and chords[2] (E): first bracket {Q,E}
    ASSERT_NE(chords[1]->tuplet(), nullptr) << "First Q in bracket 1";
    ASSERT_NE(chords[2]->tuplet(), nullptr) << "E in bracket 1";
    EXPECT_EQ(chords[1]->tuplet(), chords[2]->tuplet()) << "Q and E in same bracket";

    // chords[3-5]: second bracket {Q,Q,Q}
    ASSERT_NE(chords[3]->tuplet(), nullptr) << "Q 1 in bracket 2";
    ASSERT_NE(chords[4]->tuplet(), nullptr) << "Q 2 in bracket 2";
    ASSERT_NE(chords[5]->tuplet(), nullptr) << "Q 3 in bracket 2";
    EXPECT_EQ(chords[3]->tuplet(), chords[4]->tuplet()) << "All three in same bracket";
    EXPECT_EQ(chords[3]->tuplet(), chords[5]->tuplet());

    // The two brackets must be distinct
    EXPECT_NE(chords[1]->tuplet(), chords[3]->tuplet()) << "Two separate brackets";

    // Verify actual advances: Q-face rdur=160 in 3:2 → Q*(2/3)=1/6; E-face rdur=80 → E*(2/3)=1/12.
    EXPECT_EQ(chords[0]->actualTicks(), Fraction(1, 4)) << "Plain Q = 1/4";
    EXPECT_EQ(chords[1]->actualTicks(), Fraction(1, 6)) << "Q in 3:2 = Q*(2/3) = 1/6";
    EXPECT_EQ(chords[2]->actualTicks(), Fraction(1, 12)) << "E in 3:2 = E*(2/3) = 1/12";
    EXPECT_EQ(chords[3]->actualTicks(), Fraction(1, 6)) << "Q in 3:2 = 1/6";
    delete score;
}

TEST_F(Tst_NotesTuplets, mixed_value_tuplet_ticks_corrected_for_overshoot)
{
    // notes_mixed_duration_triplet.enc: first bracket {16,16,Q} in a 3:2 group.
    // faceSum = 1/16+1/16+1/4 = 3/8 >= threshold = 3/16 (= baseLen(16th)*3).
    // placedTicks = 3*(2/3) advance = 1/12+1/12+1/6 = 5/24.
    // expected = baseLen*normalN = (1/16)*2 = 1/8.
    // placedTicks(5/24) > expected(1/8) AND faceTicks(3/8) > fullFaceSum(3/16):
    //   mixedValueOvershoot = true → tuplet->setTicks(5/24).
    // Without the ticks correction, tuplet->ticks()=1/8 (too small), checkMeasure
    // sees next chord at P+5/24 > P+1/8 → inserts fill → sum > 2/4 → corrupted.
    MasterScore* score = readEncoreScore("notes_mixed_duration_triplet.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Mixed-value tuplet ticks correction should produce clean 2/4: "
                     << ret.text();
    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    Tuplet* firstTuplet = nullptr;
    for (EngravingItem* e : m->el()) {
        if (e->isTuplet()) {
            firstTuplet = toTuplet(e);
            break;
        }
    }
    ASSERT_NE(firstTuplet, nullptr) << "Must have at least one tuplet";
    // First tuplet spans {16+16+Q} with placedTicks = 1/24+1/24+1/6 = 1/4.
    // Its ticks must be 1/4 (not the default baseLen*normalN = (1/16)*2 = 1/8)
    // for checkMeasure to correctly advance past the group.
    EXPECT_EQ(firstTuplet->ticks(), Fraction(1, 4))
        << "Tuplet ticks must equal placedTicks (1/4) not default 1/8";
    delete score;
}

TEST_F(Tst_NotesTuplets, partial_triplet_unreduced_cumtick_no_crash)
{
    // Large gap triggers gap-snap storing cumTick as Fraction(800,960) (unreduced). Fix-3 must call .reduced()
    // before constructing TDuration; otherwise TDuration(Fraction(160,1920), truncate=false) asserts.
    MasterScore* score = readEncoreScore("notes_partial_triplet_unreduced_cumtick.enc");
    ASSERT_NE(score, nullptr) << "File must import without TDuration assertion failure";
    EXPECT_GT(score->nmeasures(), 0);
    delete score;
}

TEST_F(Tst_NotesTuplets, no_spurious_rests_inside_active_tuplet_gapsnap_suppressed)
{
    MasterScore* score = readEncoreScore("notes_tuplet_no_gapsnap_spurious_rest.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "Triplet + half must not corrupt";

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    int chordCount = 0;
    bool anyVisibleRestInsideTriplet = false;
    Tuplet* activeTup = nullptr;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (!el) {
            continue;
        }
        if (el->isChord()) {
            ++chordCount;
            activeTup = toChord(el)->tuplet();
        } else if (el->isRest()) {
            Rest* r = toRest(el);
            if (!r->isGap() && activeTup != nullptr) {
                anyVisibleRestInsideTriplet = true;
            }
        }
    }
    EXPECT_EQ(chordCount, 4) << "3 triplet quarters + 1 half = 4 chords";
    EXPECT_FALSE(anyVisibleRestInsideTriplet)
        << "No visible rests must appear inside the tuplet bracket (gap-snap suppressed)";

    delete score;
}

TEST_F(Tst_NotesTuplets, segment_override_15notes_becomes_15_8)
{
    MasterScore* score = readEncoreScore("notes_segment_override_15notes.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "15-note segment override must not corrupt";

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    std::vector<Chord*> chords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (el && el->isChord()) {
            chords.push_back(toChord(el));
        }
    }
    EXPECT_EQ(chords.size(), 15u) << "All 15 notes must be placed (none dropped)";

    // All must be in the same Tuplet with ratio 15:8
    Tuplet* tup = chords.empty() ? nullptr : chords[0]->tuplet();
    ASSERT_NE(tup, nullptr) << "Notes must be in a Tuplet bracket";
    EXPECT_EQ(tup->ratio().numerator(), 15) << "Override actualN must be 15";
    EXPECT_EQ(tup->ratio().denominator(), 8) << "Override normalN must be 8";
    EXPECT_EQ(tup->baseLen().type(), DurationType::V_EIGHTH);
    for (size_t i = 1; i < chords.size(); ++i) {
        EXPECT_EQ(chords[i]->tuplet(), tup) << "All 15 notes must be in the same Tuplet";
    }

    // No second voice, no rests outside the bracket
    int voice1Chords = 0;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        if (s->element(1)) {
            ++voice1Chords;
        }
    }
    EXPECT_EQ(voice1Chords, 0) << "Overflow notes must be dropped, not routed to voice 2";

    delete score;
}

TEST_F(Tst_NotesTuplets, segment_override_12notes_plus_2plain_becomes_12_6)
{
    MasterScore* score = readEncoreScore("notes_segment_override_12plus2.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "12+2 segment override must not corrupt";

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    std::vector<Chord*> allChords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (el && el->isChord()) {
            allChords.push_back(toChord(el));
        }
    }
    EXPECT_EQ(allChords.size(), 14u) << "12 tuplet + 2 plain = 14 notes total";

    // First 12 in [12:6]
    Tuplet* tup = allChords.empty() ? nullptr : allChords[0]->tuplet();
    ASSERT_NE(tup, nullptr);
    EXPECT_EQ(tup->ratio().numerator(), 12);
    EXPECT_EQ(tup->ratio().denominator(), 6);
    for (int i = 0; i < 12; ++i) {
        EXPECT_EQ(allChords[i]->tuplet(), tup) << "Note " << i + 1 << " must be in the [12:6] bracket";
    }
    // Last 2 are plain (not in any tuplet)
    EXPECT_EQ(allChords[12]->tuplet(), nullptr) << "Trailing note 13 must be plain";
    EXPECT_EQ(allChords[13]->tuplet(), nullptr) << "Trailing note 14 must be plain";

    delete score;
}

TEST_F(Tst_NotesTuplets, segment_override_does_not_fire_for_clean_multiple)
{
    MasterScore* score = readEncoreScore("notes_segment_no_override_clean_multiple.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "Two standard 3:2 groups must not corrupt";

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    std::vector<Chord*> chords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (el && el->isChord()) {
            chords.push_back(toChord(el));
        }
    }
    ASSERT_EQ(chords.size(), 6u) << "All 6 notes must be placed";

    // Must form two separate Tuplets, each [3:2]
    Tuplet* t1 = chords[0]->tuplet();
    Tuplet* t2 = chords[3]->tuplet();
    ASSERT_NE(t1, nullptr);
    ASSERT_NE(t2, nullptr);
    EXPECT_NE(t1, t2) << "6 notes with 3:2 must form TWO separate groups, not one [6:m]";
    EXPECT_EQ(t1->ratio().numerator(), 3);
    EXPECT_EQ(t1->ratio().denominator(), 2);
    EXPECT_EQ(t2->ratio().numerator(), 3);
    EXPECT_EQ(t2->ratio().denominator(), 2);
    // Notes 1-3 in group 1, notes 4-6 in group 2
    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(chords[i]->tuplet(), t1);
    }
    for (int i = 3; i < 6; ++i) {
        EXPECT_EQ(chords[i]->tuplet(), t2);
    }

    delete score;
}

TEST_F(Tst_NotesTuplets, non_standard_tuplet_dosillo_2_1)
{
    MasterScore* score = readEncoreScore("notes_tuplet_dosillo_2_1.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "Dosillo 2:1 must not corrupt";

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    std::vector<Chord*> chords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (el && el->isChord()) {
            chords.push_back(toChord(el));
        }
    }
    ASSERT_EQ(chords.size(), 2u) << "Dosillo must produce exactly 2 notes";

    Tuplet* tup = chords[0]->tuplet();
    ASSERT_NE(tup, nullptr) << "Both notes must be inside a 2:1 bracket";
    EXPECT_EQ(tup->ratio().numerator(), 2);
    EXPECT_EQ(tup->ratio().denominator(), 1);
    EXPECT_EQ(chords[0]->tuplet(), chords[1]->tuplet());

    delete score;
}

TEST_F(Tst_NotesTuplets, non_standard_tuplet_9_4_nontuplet)
{
    MasterScore* score = readEncoreScore("notes_tuplet_9_4_nontuplet.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "9:4 nontuplet must not corrupt";

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    std::vector<Chord*> chords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (el && el->isChord()) {
            chords.push_back(toChord(el));
        }
    }
    EXPECT_EQ(chords.size(), 9u) << "All 9 notes must be in the 9:4 bracket";

    Tuplet* tup = chords.empty() ? nullptr : chords[0]->tuplet();
    ASSERT_NE(tup, nullptr);
    EXPECT_EQ(tup->ratio().numerator(), 9);
    EXPECT_EQ(tup->ratio().denominator(), 4);
    for (auto* c : chords) {
        EXPECT_EQ(c->tuplet(), tup);
    }

    delete score;
}

TEST_F(Tst_NotesTuplets, last_tuplet_note_short_rdur_not_dropped)
{
    MasterScore* score = readEncoreScore("notes_tuplet_last_note_short_rdur.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "Last note with tiny rdur must not corrupt";

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    std::vector<Chord*> chords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (el && el->isChord()) {
            chords.push_back(toChord(el));
        }
    }
    EXPECT_EQ(chords.size(), 10u)
        << "Note 10 with rdur=6 (< 15) must NOT be dropped by the MIDI artifact filter";

    Tuplet* tup = chords.empty() ? nullptr : chords[0]->tuplet();
    ASSERT_NE(tup, nullptr);
    EXPECT_EQ(tup->ratio().numerator(), 10);
    EXPECT_EQ(tup->ratio().denominator(), 4);
    EXPECT_EQ(chords[9]->tuplet(), tup) << "Note 10 must be inside the tuplet bracket";

    delete score;
}

TEST_F(Tst_NotesTuplets, triplet_orphan_middle_note_missing_tup_byte)
{
    MasterScore* score = readEncoreScore("notes_triplet_orphan_missing_tup.enc");
    ASSERT_NE(score, nullptr);

    Measure* m0 = measureAt(score, 0);
    ASSERT_NE(m0, nullptr);

    // The first segment must carry a chord that is inside a 3:2 tuplet.
    Segment* firstSeg = m0->first(SegmentType::ChordRest);
    ASSERT_NE(firstSeg, nullptr);
    EngravingItem* el = firstSeg->element(0);
    ASSERT_NE(el, nullptr);
    ASSERT_TRUE(el->isChord()) << "First element should be a chord (triplet note 1)";

    Chord* firstChord = toChord(el);
    ASSERT_NE(firstChord->tuplet(), nullptr) << "Triplet note 1 must be inside a tuplet";

    Tuplet* tup = firstChord->tuplet();
    EXPECT_EQ(tup->ratio().reduced(), Fraction(3, 2)) << "Tuplet must be 3:2";
    EXPECT_EQ(static_cast<int>(tup->elements().size()), 3)
        << "Triplet must have 3 elements (orphan middle note must NOT be dropped)";

    // The measure must be properly full (no overflow from treating orphan as plain 8th).
    Fraction measLen = m0->ticks();
    Fraction usedTicks(0, 1);
    for (Segment* s = m0->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        ChordRest* cr = e && e->isChordRest() ? toChordRest(e) : nullptr;
        if (cr && !(cr->isRest() && toRest(cr)->isGap())) {
            usedTicks += cr->actualTicks();
        }
    }
    EXPECT_EQ(usedTicks, measLen) << "Measure must be exactly full (no overflow or gap)";

    delete score;
}

TEST_F(Tst_NotesTuplets, triplet_orphan_with_prior_complete_group)
{
    MasterScore* score = readEncoreScore("notes_triplet_orphan_prior_complete_group.enc");
    ASSERT_NE(score, nullptr);

    Measure* m0 = measureAt(score, 0);
    ASSERT_NE(m0, nullptr);

    std::vector<Tuplet*> tuplets;
    for (EngravingItem* e : m0->el()) {
        if (e->isTuplet()) {
            tuplets.push_back(toTuplet(e));
        }
    }
    ASSERT_EQ(tuplets.size(), 2u) << "Measure must contain exactly two 3:2 tuplets";
    for (int t = 0; t < 2; ++t) {
        EXPECT_EQ(static_cast<int>(tuplets[t]->elements().size()), 3)
            << "Tuplet " << t << " must have 3 elements (orphan must not be dropped)";
        EXPECT_EQ(tuplets[t]->ratio().reduced(), Fraction(3, 2))
            << "Tuplet " << t << " must be 3:2";
    }

    delete score;
}

// Regression: isolated explicit tuplet note placed with face value but cumTick advance was capped;
// voice overran by face - capped. Fix: always set chord duration to the capped value.
TEST_F(Tst_NotesTuplets, isolated_explicit_tuplet_caps_chord_ticks)
{
    MasterScore* score = readEncoreScore("importer_isolated_explicit_tuplet_capped.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_isolated_explicit_tuplet_capped.enc";
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Corrupted: " << ret.text();
    delete score;
}

// Regression: note-level path-A cap deleted a chord that belonged to an inner (nested) tuplet.
// Old code called tt.currentTuplet->remove(chord), the outer tuplet, which does not contain the
// chord. Fix: use chord->tuplet() (the actual owning tuplet) rather than tt.currentTuplet.
TEST_F(Tst_NotesTuplets, inner_tuplet_note_level_cap_no_crash)
{
    MasterScore* score = readEncoreScore("importer_inner_tuplet_note_level_cap.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_inner_tuplet_note_level_cap.enc";
    EXPECT_GT(score->nmeasures(), 0);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Corrupted: " << ret.text();
    delete score;
}

// ===========================================================================
// BUG FIX: mixed-duration explicit tuplet bracket {Q,E} in a 3:2 group
// was not closing correctly. faceSum(Q+E)=3/8 never reached the old
// threshold 3Q=3/4, pulling subsequent notes into the same bracket.
// Fix: close a group when faceSum/actualN is a valid standard TDuration.
// ===========================================================================
TEST_F(Tst_NotesTuplets, v0c4_mixed_duration_tuplet_bracket_closes_correctly)
{
    MasterScore* score = readEncoreScore("ornaments_tuplet_mixed_baseLen.enc");
    ASSERT_NE(score, nullptr) << "Failed to load ornaments_tuplet_mixed_baseLen.enc";

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Measure is corrupt (overrun): " << ret.text();

    Measure* m1 = score->firstMeasure();
    ASSERT_NE(m1, nullptr);

    std::set<Tuplet*> tuplets;
    int noteCount = 0;
    for (Segment* s = m1->first(SegmentType::ChordRest); s;
         s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (!el || !el->isChord()) {
            continue;
        }
        ++noteCount;
        Chord* c = toChord(el);
        if (c->tuplet()) {
            tuplets.insert(c->tuplet());
        }
    }

    EXPECT_EQ(noteCount, 6);
    EXPECT_EQ(tuplets.size(), 2u)
        << "Must form 2 tuplet brackets: {Q,E} and {Q,Q,Q}, not one big group";
    delete score;
}

// ===========================================================================
// BUG FIX: 4:3 quadruplet (tup=0x43) was not recognized; notes appeared
// as plain, with wrong advance (Q instead of E per slot).
// ===========================================================================
TEST_F(Tst_NotesTuplets, v0c4_4to3_quadruplet_correct_advance)
{
    MasterScore* score = readEncoreScore("tuplet_4to3_quadruplet.enc");
    ASSERT_NE(score, nullptr) << "Failed to load tuplet_4to3_quadruplet.enc";
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "4:3 quadruplet must import without measure corruption: " << ret.text();

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    std::vector<Chord*> chords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            chords.push_back(toChord(e));
        }
    }
    ASSERT_EQ(chords.size(), 4u) << "Must have 4 chords in the 4:3 quadruplet";

    for (int i = 0; i < 4; ++i) {
        ASSERT_NE(chords[i]->tuplet(), nullptr) << "Chord " << i << " must be in a 4:3 tuplet";
        EXPECT_EQ(chords[i]->tuplet(), chords[0]->tuplet()) << "All 4 in same bracket";
    }
    EXPECT_EQ(chords[0]->tuplet()->ratio(), Fraction(4, 3)) << "Ratio must be 4:3";
    EXPECT_EQ(chords[0]->actualTicks(), Fraction(3, 32)) << "E in 4:3 = E*(3/4) = 3/32";
    delete score;
}

// Nested-tuplet detection must not spuriously fire on a plain 3:2 group, and it must not pull another
// staff's notes into an inner group (both corrupted the drum staff's brackets and overflowed the measure).
TEST_F(Tst_NotesTuplets, cross_staff_false_nesting_and_drum_corruption)
{
    MasterScore* score = readEncoreScore("notes_cross_staff_false_nesting.enc");
    ASSERT_NE(score, nullptr) << "Failed to load notes_cross_staff_false_nesting.enc";

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Drum staff corruption or false nesting caused measure overflow: " << ret.text();

    ASSERT_GE(score->nstaves(), 2) << "Fixture must have at least 2 staves";

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    std::vector<Chord*> drumChords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        for (int v = 0; v < static_cast<int>(VOICES); ++v) {
            EngravingItem* e = s->element(static_cast<track_idx_t>(1 * VOICES + v));
            if (e && e->isChord()) {
                Chord* c = toChord(e);
                if (!c->isRest() || !toRest(c)->isGap()) {
                    drumChords.push_back(c);
                }
            }
        }
    }

    EXPECT_EQ(drumChords.size(), 12u)
        << "Staff 1 must have 12 eighth notes (4 triplets x 3); "
        "drum corruption shifts notes outside measure, reducing the count";

    // All 12 drum chords must be in tuplets.
    for (size_t i = 0; i < drumChords.size(); ++i) {
        EXPECT_NE(drumChords[i]->tuplet(), nullptr)
            << "Drum chord " << i << " must be in a 3:2 tuplet";
    }

    // There must be exactly 4 distinct tuplet objects on staff 1.
    std::set<Tuplet*> drumTuplets;
    for (Chord* c : drumChords) {
        if (c->tuplet()) {
            drumTuplets.insert(c->tuplet());
        }
    }
    EXPECT_EQ(drumTuplets.size(), 4u)
        << "Staff 1 must form 4 independent 3:2 triplet brackets; "
        "cross-staff contamination collapses them into fewer groups";

    delete score;
}
