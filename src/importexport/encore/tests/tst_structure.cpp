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

// Score structure import: measures, systems and page breaks, clefs, key/time signatures, pickup measures,
// and page layout (margins, spatium, staff spacing).

#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>

#include <QByteArray>
#include <QFile>
#include <QTemporaryDir>

#include "engraving/dom/system.h"
#include "engraving/dom/barline.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/tuplet.h"
#include "engraving/dom/jump.h"
#include "engraving/dom/marker.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/note.h"
#include "engraving/dom/page.h"
#include "engraving/dom/part.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/tempotext.h"
#include "engraving/dom/timesig.h"
#include "engraving/style/style.h"
#include "engraving/types/fraction.h"

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

class Tst_Structure : public ::testing::Test, public MTest
{
protected:
    void SetUp() override { setRootDir(ENC_DIR); }
};

static int firstPageBreakPage(MasterScore* score)
{
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        bool hasPageBreak = false;
        for (EngravingItem* e : mb->el()) {
            if (e && e->isLayoutBreak() && toLayoutBreak(e)->isPageBreak()) {
                hasPageBreak = true;
                break;
            }
        }
        if (!hasPageBreak) {
            continue;
        }
        System* sys = toMeasure(mb)->system();
        if (sys && sys->page()) {
            for (size_t i = 0; i < score->pages().size(); ++i) {
                if (score->pages()[i] == sys->page()) {
                    return (int)i;
                }
            }
        }
        return -1;
    }
    return -2;
}

TEST_F(Tst_Structure, basic_measure_count)
{
    MasterScore* score = readEncoreScore("bazo.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_GT(score->nmeasures(), 0);
    delete score;
}

TEST_F(Tst_Structure, basic_single_part)
{
    MasterScore* score = readEncoreScore("bazo.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_EQ(score->parts().size(), 1u);
    EXPECT_EQ(score->nstaves(), 1u);
    delete score;
}

TEST_F(Tst_Structure, multipart_score)
{
    MasterScore* score = readEncoreScore("bando.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_GT(score->parts().size(), 1u) << "bando.enc should have multiple parts";
    EXPECT_GT(score->nstaves(), 1u);
    delete score;
}

TEST_F(Tst_Structure, time_sig_4_4)
{
    MasterScore* score = readEncoreScore("chord_parsing.enc");
    ASSERT_NE(score, nullptr);
    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(4, 4)) << "First measure should be 4/4";
    delete score;
}

TEST_F(Tst_Structure, time_sig_3_4)
{
    MasterScore* score = readEncoreScore("notes_triplets.enc");
    ASSERT_NE(score, nullptr);
    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(3, 4)) << "Synthetic triplet file should be 3/4";
    delete score;
}

TEST_F(Tst_Structure, time_sig_2_4)
{
    MasterScore* score = readEncoreScore("notes_swing.enc");
    ASSERT_NE(score, nullptr);
    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(2, 4)) << "First measure should be 2/4";
    delete score;
}

TEST_F(Tst_Structure, key_sig_no_accidentals)
{
    MasterScore* score = readEncoreScore("bazo.enc");
    ASSERT_NE(score, nullptr);
    Staff* st = score->staff(0);
    ASSERT_NE(st, nullptr);
    Key k = st->key(Fraction(0, 1));
    EXPECT_EQ(int(k), 0) << "bazo.enc should be in C major (0 accidentals)";
    delete score;
}

TEST_F(Tst_Structure, key_sig_no_invalid_large_values)
{
    // encKeyToFifths wrapping was broken before (key index 8 mapped to -248); verify -7..7 range.
    MasterScore* score = readEncoreScore("bando.enc");
    ASSERT_NE(score, nullptr);
    Fraction tick(0, 1);
    for (size_t i = 0; i < score->nstaves(); ++i) {
        Staff* st = score->staff(i);
        int keyVal = int(st->key(tick));
        EXPECT_GE(keyVal, -7) << "Staff " << i << " key should be >= -7";
        EXPECT_LE(keyVal, 7) << "Staff " << i << " key should be <= 7";
    }
    delete score;
}

TEST_F(Tst_Structure, intermediate_time_sig_7_8)
{
    MasterScore* score = readEncoreScore("paloteos_7x8.enc");
    ASSERT_NE(score, nullptr);

    Measure* m0 = measureAt(score, 0);
    ASSERT_NE(m0, nullptr);
    EXPECT_EQ(m0->timesig(), Fraction(4, 4)) << "M0 should be 4/4";

    Measure* m16 = measureAt(score, 16);
    ASSERT_NE(m16, nullptr);
    EXPECT_EQ(m16->timesig(), Fraction(7, 8)) << "M16 should be 7/8";
    EXPECT_EQ(m16->ticks(), Fraction(7, 8)) << "M16 duration should be 7/8";

    Segment* tsSeg = m16->findSegment(SegmentType::TimeSig, m16->tick());
    EXPECT_NE(tsSeg, nullptr) << "M16 must have a TimeSig segment";
    if (tsSeg) {
        bool found7_8 = false;
        for (EngravingItem* el : tsSeg->elist()) {
            if (el && el->isTimeSig()) {
                TimeSig* ts = toTimeSig(el);
                if (ts->sig() == Fraction(7, 8)) {
                    found7_8 = true;
                }
            }
        }
        EXPECT_TRUE(found7_8) << "TimeSig segment at M16 must contain a 7/8 element";
    }

    Measure* m15 = measureAt(score, 15);
    ASSERT_NE(m15, nullptr);
    EXPECT_EQ(m15->timesig(), Fraction(4, 4)) << "M15 should still be 4/4";

    delete score;
}

// A 6/8 <-> 3/4 change must be detected: comparing time signatures by value treats 6/8 == 3/4, so the
// change is swallowed; it must be compared by numerator/denominator (Fraction::identical).
TEST_F(Tst_Structure, time_sig_change_6_8_to_3_4_and_back)
{
    MasterScore* score = readEncoreScore("timesig_change_6_8_to_3_4.enc");
    ASSERT_NE(score, nullptr);

    Measure* m0 = measureAt(score, 0);
    ASSERT_NE(m0, nullptr);
    EXPECT_TRUE(m0->timesig().identical(Fraction(6, 8))) << "M0 should be 6/8";

    // Measure 2: 6/8 → 3/4. Must have a visible TimeSig element.
    Measure* m2 = measureAt(score, 2);
    ASSERT_NE(m2, nullptr);
    EXPECT_TRUE(m2->timesig().identical(Fraction(3, 4))) << "M2 should be 3/4";
    {
        Segment* tsSeg = m2->findSegment(SegmentType::TimeSig, m2->tick());
        ASSERT_NE(tsSeg, nullptr) << "M2 must have a TimeSig segment (6/8 → 3/4 change)";
        bool found = false;
        for (EngravingItem* el : tsSeg->elist()) {
            if (el && el->isTimeSig()) {
                TimeSig* ts = toTimeSig(el);
                if (ts->sig().identical(Fraction(3, 4))) {
                    found = true;
                }
            }
        }
        EXPECT_TRUE(found) << "TimeSig at M2 must carry 3/4, not be merged silently with 6/8";
    }

    // Measure 5: 3/4 → 6/8. Must have a visible TimeSig element.
    Measure* m5 = measureAt(score, 5);
    ASSERT_NE(m5, nullptr);
    EXPECT_TRUE(m5->timesig().identical(Fraction(6, 8))) << "M5 should be 6/8";
    {
        Segment* tsSeg = m5->findSegment(SegmentType::TimeSig, m5->tick());
        ASSERT_NE(tsSeg, nullptr) << "M5 must have a TimeSig segment (3/4 → 6/8 change)";
        bool found = false;
        for (EngravingItem* el : tsSeg->elist()) {
            if (el && el->isTimeSig()) {
                TimeSig* ts = toTimeSig(el);
                if (ts->sig().identical(Fraction(6, 8))) {
                    found = true;
                }
            }
        }
        EXPECT_TRUE(found) << "TimeSig at M5 must carry 6/8, not be merged silently with 3/4";
    }

    delete score;
}

// Same value-vs-identical issue for 2/2 <-> 4/4: the change must be detected, not swallowed.
TEST_F(Tst_Structure, time_sig_change_2_2_to_4_4_and_back)
{
    MasterScore* score = readEncoreScore("timesig_change_2_2_to_4_4.enc");
    ASSERT_NE(score, nullptr);

    Measure* m0 = measureAt(score, 0);
    ASSERT_NE(m0, nullptr);
    EXPECT_TRUE(m0->timesig().identical(Fraction(2, 2))) << "M0 should be 2/2";

    Measure* m2 = measureAt(score, 2);
    ASSERT_NE(m2, nullptr);
    EXPECT_TRUE(m2->timesig().identical(Fraction(4, 4))) << "M2 should be 4/4";
    {
        Segment* tsSeg = m2->findSegment(SegmentType::TimeSig, m2->tick());
        ASSERT_NE(tsSeg, nullptr) << "M2 must have a TimeSig segment (2/2 -> 4/4)";
        bool found = false;
        for (EngravingItem* el : tsSeg->elist()) {
            if (el && el->isTimeSig() && toTimeSig(el)->sig().identical(Fraction(4, 4))) {
                found = true;
            }
        }
        EXPECT_TRUE(found) << "TimeSig at M2 must carry 4/4";
    }

    Measure* m4 = measureAt(score, 4);
    ASSERT_NE(m4, nullptr);
    EXPECT_TRUE(m4->timesig().identical(Fraction(2, 2))) << "M4 should be 2/2";
    {
        Segment* tsSeg = m4->findSegment(SegmentType::TimeSig, m4->tick());
        ASSERT_NE(tsSeg, nullptr) << "M4 must have a TimeSig segment (4/4 -> 2/2)";
        bool found = false;
        for (EngravingItem* el : tsSeg->elist()) {
            if (el && el->isTimeSig() && toTimeSig(el)->sig().identical(Fraction(2, 2))) {
                found = true;
            }
        }
        EXPECT_TRUE(found) << "TimeSig at M4 must carry 2/2";
    }

    delete score;
}

// ===========================================================================
// WINI block / page margin tests
// ===========================================================================

// WINI pts format must set the page size explicitly (from its edges), not rely on the MuseScore default,
// so an A4 file still produces an A4 score on a Letter-default machine.
TEST_F(Tst_Structure, page_size_detected_from_wini_pts_format)
{
    MasterScore* score = readEncoreScore("ornaments_fingering_grandstaff.enc");
    ASSERT_NE(score, nullptr);

    const double kA4W = 210.0 / 25.4;   // 8.2677"
    const double kA4H = 297.0 / 25.4;   // 11.6929"
    EXPECT_NEAR(score->style().styleD(Sid::pageWidth),  kA4W, 0.01)
        << "pts-format WINI with A4 boundary values must set A4 page width";
    EXPECT_NEAR(score->style().styleD(Sid::pageHeight), kA4H, 0.01)
        << "pts-format WINI with A4 boundary values must set A4 page height";

    delete score;
}

// ===========================================================================
// WINI / page margin: no-WINI file must leave MuseScore default margins intact.
// ===========================================================================

// File with no WINI block must leave MuseScore default margins intact.
// text_tempo_orn_compound_68.enc has no WINI block.
TEST_F(Tst_Structure, page_margins_no_wini_uses_defaults)
{
    MasterScore* score = readEncoreScore("text_tempo_orn_compound_68.enc");
    ASSERT_NE(score, nullptr);

    const double defaultLeftIn = 15.0 / INCH;
    EXPECT_NEAR(score->style().styleD(Sid::pageOddLeftMargin),  defaultLeftIn, 0.001)
        << "no-WINI file must keep default left margin";
    EXPECT_NEAR(score->style().styleD(Sid::pageEvenLeftMargin), defaultLeftIn, 0.001);
    EXPECT_NEAR(score->style().styleD(Sid::pageOddTopMargin),   defaultLeftIn, 0.001)
        << "no-WINI file must keep default top margin";

    delete score;
}

// ===========================================================================
// A KEYCHANGE to C major (tipo=0) must be emitted; the previous guard silently dropped it.
TEST_F(Tst_Structure, keychange_to_c_major_emitted)
{
    MasterScore* score = readEncoreScore("structure_keychange_to_c.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    int keySigCount = 0;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        for (Segment* s = m->first(SegmentType::KeySig); s; s = s->next(SegmentType::KeySig)) {
            if (s->element(0)) {
                ++keySigCount;
            }
        }
    }
    // Initial key sig (m0 G major) + tipo=0 modulation sig (m1); both must be present.
    EXPECT_GE(keySigCount, 2);
    delete score;
}

// ===========================================================================
// v0xC2 stores the MIDI pitch in the tuplet field, not semiTonePitch; it must be swapped back on import.
// See ENCORE_FORMAT.md §v0xC2 note (size 22 or 24).
TEST_F(Tst_Structure, old_format_v0c2_correct_pitches)
{
    MasterScore* score = readEncoreScore("structure_v0c2_pitches.enc");
    ASSERT_NE(score, nullptr);

    std::vector<int> pitches;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s;
             s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->elist()) {
                if (e && e->isChord()) {
                    for (Note* n : toChord(e)->notes()) {
                        pitches.push_back(n->pitch());
                    }
                }
            }
        }
    }
    ASSERT_EQ(pitches.size(), 4u) << "Should have 4 notes";
    EXPECT_EQ(pitches[0], 60) << "First note should be C4 (60)";
    EXPECT_EQ(pitches[1], 64) << "Second note should be E4 (64)";
    EXPECT_EQ(pitches[2], 67) << "Third note should be G4 (67)";
    EXPECT_EQ(pitches[3], 72) << "Fourth note should be C5 (72)";
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "v0xC2 pitch-fixed score should pass sanityCheck: " << ret.text();
    delete score;
}

TEST_F(Tst_Structure, old_format_v0c2_spurious_semitone_flag_uses_pitch_at_13)
{
    // A small stray flag (1 or 3) in the semiTonePitch slot is not a pitch: the discriminator must treat
    // +15 as a pitch only when it holds a plausible MIDI value, else notes import as MIDI 1 and collapse.
    MasterScore* score = readEncoreScore("structure_v0c2_spurious_semitone_flag.enc");
    ASSERT_NE(score, nullptr);

    std::vector<int> pitches;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s;
             s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->elist()) {
                if (e && e->isChord()) {
                    for (Note* n : toChord(e)->notes()) {
                        pitches.push_back(n->pitch());
                    }
                }
            }
        }
    }
    ASSERT_EQ(pitches.size(), 4u) << "Chord must keep all three notes, not collapse to one";
    EXPECT_EQ(pitches[0], 60) << "chord note 1 must be C4 (60), not the +15 flag";
    EXPECT_EQ(pitches[1], 64) << "chord note 2 must be E4 (64)";
    EXPECT_EQ(pitches[2], 67) << "chord note 3 must be G4 (67)";
    EXPECT_EQ(pitches[3], 72) << "C5 (72) must read from +13 even though +15 == 3";
    delete score;
}

// ===========================================================================
// A pickup measure imports shortened (actual ticks < nominal) while still displaying the nominal time
// signature; the following measure starts right after it.
TEST_F(Tst_Structure, pickup_measure_shortened)
{
    MasterScore* score = readEncoreScore("structure_pickup_measure.enc");
    ASSERT_NE(score, nullptr);

    Measure* m0 = measureAt(score, 0);
    Measure* m1 = measureAt(score, 1);
    ASSERT_NE(m0, nullptr);
    ASSERT_NE(m1, nullptr);

    EXPECT_EQ(m0->timesig(), Fraction(4, 4)) << "Pickup m0 must display the nominal 4/4 time signature";
    EXPECT_EQ(m0->ticks(), Fraction(1, 4)) << "Pickup m0 must be shortened to the pickup duration";
    EXPECT_EQ(m1->tick(), Fraction(1, 4)) << "m1 must start immediately after the shortened m0";

    // The pickup note must be at offset 0 within the short measure.
    Fraction noteOffset { -1, 1 };
    for (Segment* s = m0->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (el && el->isChord()) {
            noteOffset = s->tick() - m0->tick();
            break;
        }
    }
    EXPECT_EQ(noteOffset, Fraction(0, 1)) << "Pickup note must be at offset 0 within the shortened m0";

    delete score;
}

// Case B (pure cumTick): same timeSig=4/4, 8 32nd notes from tick=0.
// No gap-snap (notes at exact cumTick positions). cumTick = 8/32 = 1/4.
// Measure 0 must be shortened to 1/4 based purely on cumTick, no barline needed.
TEST_F(Tst_Structure, pickup_caseb_reduces_to_max_content)
{
    MasterScore* score = readEncoreScore("structure_pickup_caseb_reduces.enc");
    ASSERT_NE(score, nullptr);

    Measure* m0 = measureAt(score, 0);
    Measure* m1 = measureAt(score, 1);
    ASSERT_NE(m0, nullptr);
    ASSERT_NE(m1, nullptr);

    EXPECT_EQ(m0->timesig(), Fraction(4, 4)) << "Pickup m0 must display nominal 4/4";
    EXPECT_EQ(m0->ticks(), Fraction(1, 4)) << "Pickup m0 must be shortened to cumTick=8/32=1/4";
    EXPECT_EQ(m1->tick(), Fraction(1, 4)) << "m1 must start immediately after the shortened m0";

    delete score;
}

// Case B: whole note (fv=1) at tick=0 fills the measure completely.
// cumTick = 1 = measure->ticks() -> NOT less than -> no shortening.
TEST_F(Tst_Structure, pickup_caseb_no_reduce_when_full_content)
{
    MasterScore* score = readEncoreScore("structure_pickup_caseb_no_reduce_full.enc");
    ASSERT_NE(score, nullptr);

    Measure* m0 = measureAt(score, 0);
    ASSERT_NE(m0, nullptr);

    EXPECT_EQ(m0->ticks(), Fraction(4, 4)) << "Measure 0 must NOT be shortened: whole note cumTick=1=measure->ticks()";

    delete score;
}

// A Case A pickup (short ts[0], full ts[1]) whose content is shorter than the pickup must not be
// double-shortened by the Case B path; measure 0 stays at the pickup length.
TEST_F(Tst_Structure, pickup_casea_guard_prevents_double_shortening)
{
    MasterScore* score = readEncoreScore("structure_pickup_casea_sparse.enc");
    ASSERT_NE(score, nullptr);

    Measure* m0 = measureAt(score, 0);
    Measure* m1 = measureAt(score, 1);
    ASSERT_NE(m0, nullptr);
    ASSERT_NE(m1, nullptr);

    EXPECT_EQ(m0->timesig(), Fraction(4, 4)) << "Case A pickup must display nominal 4/4";
    EXPECT_EQ(m0->ticks(), Fraction(2, 4)) << "Case A pickup must stay at its explicit 2/4, not be further shortened by Case B";
    EXPECT_EQ(m1->tick(), Fraction(2, 4)) << "m1 must start at 2/4, not be shifted by a spurious Case B delta";

    delete score;
}

// ===========================================================================
// A v0xC2 4/4 whose time-sig glyph byte marks common time must import as TimeSigType::FOUR_FOUR (the "C"
// symbol), not a numeric 4/4.
TEST_F(Tst_Structure, timesig_v0c2_common_time_glyph_preserved)
{
    MasterScore* score = readEncoreScore("notes_v0c2_common_time_glyph.enc");
    ASSERT_NE(score, nullptr);

    Measure* m0 = measureAt(score, 0);
    ASSERT_NE(m0, nullptr);

    Segment* tsSeg = m0->findSegment(SegmentType::TimeSig, m0->tick());
    ASSERT_NE(tsSeg, nullptr) << "Measure 0 must have a TimeSig segment";

    bool foundFourFour = false;
    for (EngravingItem* el : tsSeg->elist()) {
        if (el && el->isTimeSig()) {
            TimeSig* ts = toTimeSig(el);
            if (ts->timeSigType() == TimeSigType::FOUR_FOUR) {
                foundFourFour = true;
            }
        }
    }
    EXPECT_TRUE(foundFourFour) << "TimeSig glyph 0x63 must produce TimeSigType::FOUR_FOUR (common time C), not NORMAL";

    delete score;
}

// Same as above but for glyph=0x43 ('C', uppercase ASCII), the variant produced by
// older Encore versions (e.g. Encore 3.x/4.x files vs. 5.x files with 0x63).
TEST_F(Tst_Structure, timesig_v0c2_common_time_glyph_uppercase_preserved)
{
    MasterScore* score = readEncoreScore("notes_v0c2_common_time_glyph_uc.enc");
    ASSERT_NE(score, nullptr);

    Measure* m0 = measureAt(score, 0);
    ASSERT_NE(m0, nullptr);

    Segment* tsSeg = m0->findSegment(SegmentType::TimeSig, m0->tick());
    ASSERT_NE(tsSeg, nullptr) << "Measure 0 must have a TimeSig segment";

    bool foundFourFour = false;
    for (EngravingItem* el : tsSeg->elist()) {
        if (el && el->isTimeSig()) {
            TimeSig* ts = toTimeSig(el);
            if (ts->timeSigType() == TimeSigType::FOUR_FOUR) {
                foundFourFour = true;
            }
        }
    }
    EXPECT_TRUE(foundFourFour) << "TimeSig glyph 0x43 must produce TimeSigType::FOUR_FOUR (common time C), not NORMAL";

    delete score;
}

// All ten Encore navigation options (Segno/Coda/ToCoda/Fine + 6 DC/DS variants) survive import.
TEST_F(Tst_Structure, all_encore_navigation_options)
{
    MasterScore* score = readEncoreScore("structure_jump_marks_all.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    int segnoMarkers = 0;
    int codaMarkers = 0;
    int toCodaMarkers = 0;
    int fineMarkers = 0;
    std::set<JumpType> jumpTypes;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (EngravingItem* e : mb->el()) {
            if (e && e->isMarker()) {
                MarkerType mt = toMarker(e)->markerType();
                if (mt == MarkerType::SEGNO) {
                    ++segnoMarkers;
                } else if (mt == MarkerType::CODA) {
                    ++codaMarkers;
                } else if (mt == MarkerType::TOCODA) {
                    ++toCodaMarkers;
                } else if (mt == MarkerType::FINE) {
                    ++fineMarkers;
                }
            } else if (e && e->isJump()) {
                jumpTypes.insert(toJump(e)->jumpType());
            }
        }
    }
    // Segno comes from ORN 0xA2 AND coda byte 0x88; both add a Marker.
    EXPECT_GE(segnoMarkers, 1) << "ORN 0xA2 must produce a Segno Marker";
    // Coda from ORN 0xA6 + byte 0x89; byte 0x85 produces TOCODA instead.
    EXPECT_GE(codaMarkers, 1) << "ORN 0xA6 must produce a Coda Marker";
    // "To Coda" comes from ORN 0xA5 AND coda byte 0x85 (CODA1).
    EXPECT_GE(toCodaMarkers, 1) << "ORN 0xA5 must produce a TOCODA Marker";
    // Fine comes from coda byte 0x86.
    EXPECT_EQ(fineMarkers, 1) << "coda byte 0x86 must produce a FINE Marker";
    // Every Jump variant must appear at least once.
    const std::set<JumpType> expectedJumps = {
        JumpType::DC, JumpType::DS,
        JumpType::DC_AL_FINE, JumpType::DS_AL_FINE,
        JumpType::DC_AL_CODA, JumpType::DS_AL_CODA,
    };
    for (JumpType j : expectedJumps) {
        EXPECT_TRUE(jumpTypes.count(j) > 0)
            << "missing Jump variant for the Encore-UI option";
    }
    delete score;
}

// Jump marks come from the MEAS coda byte and To Coda from an ORN tipo; both must import as markers/jumps.
TEST_F(Tst_Structure, jump_marks_dc_ds_tocoda)
{
    MasterScore* score = readEncoreScore("structure_jump_marks.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    std::vector<std::pair<int, String> > seen;  // measure number (1-based), text
    int measIdx = 0;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        ++measIdx;
        for (EngravingItem* e : mb->el()) {
            if (e && e->isMarker()) {
                seen.emplace_back(measIdx, toMarker(e)->plainText());
            } else if (e && e->isJump()) {
                seen.emplace_back(measIdx, toJump(e)->plainText());
            }
        }
    }
    // Marker (TOCODA) lands on m1; Jumps land on m2 and m3.
    ASSERT_EQ(seen.size(), 3u);
    EXPECT_EQ(seen[0].first, 1);
    EXPECT_TRUE(seen[0].second.contains(u"Coda"))
        << "expected To Coda Marker on m1";
    EXPECT_EQ(seen[1].first, 2);
    EXPECT_TRUE(seen[1].second.contains(u"D.S."))
        << "expected D.S. al Coda Jump on m2";
    EXPECT_EQ(seen[2].first, 3);
    EXPECT_TRUE(seen[2].second.contains(u"D.C."))
        << "expected D.C. Jump on m3";
    delete score;
}

// Section markers (Segno/Coda) from ORN tipos and a dotted end barline must import.
TEST_F(Tst_Structure, section_markers_and_dotted_barline)
{
    MasterScore* score = readEncoreScore("structure_section_markers.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    std::vector<MarkerType> seenMarkers;
    BarLineType m3Bar = BarLineType::NORMAL;
    int measIdx = 0;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        ++measIdx;
        for (EngravingItem* e : mb->el()) {
            if (e && e->isMarker()) {
                seenMarkers.push_back(toMarker(e)->markerType());
            }
        }
        if (measIdx == 3) {
            Measure* m3 = toMeasure(mb);
            Segment* seg = m3->findSegment(SegmentType::EndBarLine, m3->endTick());
            if (seg) {
                if (EngravingItem* el = seg->element(0)) {
                    if (el->isBarLine()) {
                        m3Bar = toBarLine(el)->barLineType();
                    }
                }
            }
        }
    }
    const std::vector<MarkerType> expectedMarkers = {
        MarkerType::SEGNO, MarkerType::CODA,
    };
    EXPECT_EQ(seenMarkers, expectedMarkers);
    EXPECT_EQ(m3Bar, BarLineType::DOTTED)
        << "m3 end barline must be DOTTED (barTypeEnd=0x08)";
    delete score;
}

// Regression: after a Case B pickup shift, a hairpin's search boundary must use the post-shift tick, or a
// stale (too-large) boundary resolves its endpoint in the wrong measure.
TEST_F(Tst_Structure, pickup_caseb_hairpin_maxendtick_not_stale)
{
    MasterScore* score = readEncoreScore("structure_pickup_caseb_hairpin.enc");
    ASSERT_NE(score, nullptr);

    Measure* m1 = measureAt(score, 1);
    ASSERT_NE(m1, nullptr);

    int hairpinCount = 0;
    bool hairpinEndsInM1 = false;
    for (const auto& kv : score->spanner()) {
        Spanner* sp = kv.second;
        if (sp && sp->isHairpin()) {
            ++hairpinCount;
            const Fraction tick2 = sp->tick2();
            if (tick2 >= m1->tick() && tick2 <= m1->endTick()) {
                hairpinEndsInM1 = true;
            }
        }
    }

    EXPECT_GE(hairpinCount, 1) << "At least one hairpin must be imported";
    EXPECT_TRUE(hairpinEndsInM1) << "Hairpin must end within measure 1 (stale maxEndTick would push it past)";

    delete score;
}

// SystemLocks lock each Encore system to exactly its LINE measureCount.
TEST_F(Tst_Structure, fit_spatium_first_system_measure_count)
{
    MasterScore* score = readEncoreScore("text_tempo_orn_compound_68.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    int firstSystemMeasureCount = 0;
    for (const System* sys : score->systems()) {
        int mc = 0;
        for (const MeasureBase* mb : sys->measures()) {
            if (mb->isMeasure()) {
                ++mc;
            }
        }
        if (mc > 0) {
            firstSystemMeasureCount = mc;
            break;
        }
    }
    EXPECT_GE(firstSystemMeasureCount, 3)
        << "first system must fit at least enc.lines[0].measureCount (3) measures";

    delete score;
}

// A mid-measure CLEF anchors to the note that physically follows it in the stream, not to its own stored
// tick, so it lands before the next note rather than mid-beat.
TEST_F(Tst_Structure, mid_measure_clef_change_imported)
{
    MasterScore* score = readEncoreScore("structure_clef_change_mid_measure.enc");
    ASSERT_NE(score, nullptr);

    bool foundC4 = false;
    bool foundEarly = false;
    for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
        for (Segment* s = m->first(SegmentType::Clef); s; s = s->next(SegmentType::Clef)) {
            if (s->tick() <= m->tick()) {
                continue;  // skip header clef at measure start
            }
            EngravingItem* el = s->element(0);
            if (!el || !el->isClef() || toClef(el)->clefType() != ClefType::C4) {
                continue;
            }
            if (s->tick() == m->tick() + Fraction(1, 4)) {
                foundC4 = true;
            } else if (s->tick() == m->tick() + Fraction(3, 16)) {
                foundEarly = true;
            }
        }
    }
    EXPECT_TRUE(foundC4)
        << "mid-measure CLEF(C4L) must anchor to the following note at beat-2 offset (1/4)";
    EXPECT_FALSE(foundEarly)
        << "CLEF must not be placed at its own stored tick (3/16); it follows the next note";
    delete score;
}

// A trailing CLEF (last element of a measure, no note after it) is cautionary: it takes effect on the next
// measure's downbeat, not before the current measure's final note.
TEST_F(Tst_Structure, trailing_clef_change_moves_to_next_measure)
{
    MasterScore* score = readEncoreScore("structure_clef_trailing_cautionary.enc");
    ASSERT_NE(score, nullptr);

    Measure* m1 = score->firstMeasure();
    ASSERT_NE(m1, nullptr);
    Measure* m2 = m1->nextMeasure();
    ASSERT_NE(m2, nullptr);
    const Fraction barline = m2->tick();   // = end of measure 1 = downbeat of measure 2

    // A cautionary clef on the m1/m2 barline is serialized as a trailing Clef segment of m1,
    // so check by absolute tick rather than by measure ownership.
    bool clefAtBarline = false;
    bool clefMidM1 = false;
    for (Measure* m = m1; m; m = m->nextMeasure()) {
        for (Segment* s = m->first(SegmentType::Clef); s; s = s->next(SegmentType::Clef)) {
            EngravingItem* el = s->element(0);
            if (!el || !el->isClef() || toClef(el)->clefType() != ClefType::F) {
                continue;
            }
            if (s->tick() == barline) {
                clefAtBarline = true;
            } else if (s->tick() > m1->tick() && s->tick() < barline) {
                clefMidM1 = true;
            }
        }
    }
    EXPECT_TRUE(clefAtBarline)
        << "trailing CLEF must take effect on the downbeat of the next measure";
    EXPECT_FALSE(clefMidM1)
        << "trailing CLEF must not land inside measure 1";
    delete score;
}

// Malformed-input robustness: a .enc file is untrusted binary and must never crash, hang, or read out of
// bounds. These tests derive corrupt variants from a good fixture and assert the importer returns a bounded
// result (a valid score or a clean null). On a debug build, running to completion is itself the assertion.

static QByteArray readFixtureBytes(const QString& name)
{
    QFile f(ENC_DIR + name);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

// A grand-staff WEDGESTART on the bass sub-staff, paired with a note whose voice nibble is far
// above VOICES, made the wedge resolver derive an out-of-range hairpin track. Before the validTrack
// guard the Hairpin was created at a track beyond ntracks() and crashed at layout. The import must
// drop that hairpin, produce no out-of-range spanner, and lay out cleanly.
TEST_F(Tst_Structure, grandstaff_wedge_out_of_range_voice_does_not_crash)
{
    MasterScore* score = readEncoreScore("structure_grandstaff_wedge_out_of_range_voice.enc");
    ASSERT_NE(score, nullptr);

    const size_t ntracks = score->ntracks();
    for (const auto& pair : score->spanner()) {
        const Spanner* sp = pair.second;
        ASSERT_NE(sp, nullptr);
        EXPECT_LT(sp->track(), ntracks) << "spanner track out of range";
        EXPECT_LT(sp->track2(), ntracks) << "spanner track2 out of range";
    }

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "out-of-range-wedge score should pass sanityCheck: " << ret.text();
}

TEST_F(Tst_Structure, malformed_truncated_input_does_not_crash)
{
    const QByteArray good = readFixtureBytes("bando.enc");
    ASSERT_GT(good.size(), 0);

    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    setRootDir(tmp.path());

    // Every prefix of a valid file, cut at many points, must import without crashing/hanging.
    for (int frac = 1; frac < 10; ++frac) {
        const QByteArray cut = good.left(good.size() * frac / 10);
        const QString name = QString("trunc_%1.enc").arg(frac);
        QFile f(tmp.path() + "/" + name);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write(cut);
        f.close();
        // May be null (clean reject) or a bounded score; the point is it returns at all.
        delete readEncoreScore(name);
    }

    setRootDir(ENC_DIR);
}

TEST_F(Tst_Structure, malformed_oversized_block_size_does_not_crash)
{
    QByteArray data = readFixtureBytes("bando.enc");
    ASSERT_GT(data.size(), 0);

    // Overwrite the 4-byte little-endian size that follows the first MEAS magic with a value
    // far larger than the file. The parser must clamp it to the device instead of seeking past
    // EOF or wrapping the size negative when skipping.
    const int idx = data.indexOf("MEAS");
    ASSERT_GE(idx, 0);
    ASSERT_LE(idx + 8, data.size());
    for (int k = 0; k < 4; ++k) {
        data[idx + 4 + k] = static_cast<char>(0xFF);
    }

    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    setRootDir(tmp.path());
    QFile f(tmp.path() + "/oversized.enc");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write(data);
    f.close();

    delete readEncoreScore("oversized.enc");
    setRootDir(ENC_DIR);
}

TEST_F(Tst_Structure, malformed_truncated_at_byte_boundaries_does_not_crash)
{
    // Finer-grained companion to malformed_truncated_input_does_not_crash: truncate a known-good
    // file at every 64-byte boundary so a cut landing in the middle of any block header, size
    // field, or element stream exercises the parser's bounds guards. Every prefix must import
    // (as a null or a bounded score) without crashing, aborting, or hanging.
    const QByteArray good = readFixtureBytes("bazo.enc");
    ASSERT_GT(good.size(), 0);

    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    setRootDir(tmp.path());

    for (int len = 0; len <= good.size(); len += 64) {
        const QByteArray cut = good.left(len);
        const QString name = QString("trunc_at_%1.enc").arg(len);
        QFile f(tmp.path() + "/" + name);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write(cut);
        f.close();
        delete readEncoreScore(name);
    }

    setRootDir(ENC_DIR);
}

TEST_F(Tst_Structure, malformed_truncated_at_block_boundaries_does_not_crash)
{
    // Cut a known-good file a few bytes past each 4-char block magic (SCOW/TK00/PAGE/LINE/MEAS/PREC/
    // TITL/TEXT/WINI/...), so a truncation lands inside every block type's header or size field. Each
    // prefix must import (null or bounded score) without crashing.
    const QByteArray good = readFixtureBytes("bando.enc");
    ASSERT_GT(good.size(), 0);

    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    setRootDir(tmp.path());

    int caseIdx = 0;
    for (int i = 0; i + 4 <= good.size(); ++i) {
        const char c0 = good[i], c1 = good[i + 1], c2 = good[i + 2], c3 = good[i + 3];
        const bool looksLikeMagic = std::isupper(static_cast<unsigned char>(c0))
                                    && (std::isupper(static_cast<unsigned char>(c1)) || std::isdigit(static_cast<unsigned char>(c1)))
                                    && (std::isupper(static_cast<unsigned char>(c2)) || std::isdigit(static_cast<unsigned char>(c2)))
                                    && (std::isupper(static_cast<unsigned char>(c3)) || std::isdigit(static_cast<unsigned char>(c3)));
        if (!looksLikeMagic) {
            continue;
        }
        // Truncate a few bytes into the block: inside the size field (magic + 0..8 bytes).
        for (int extra : { 2, 6 }) {
            const int len = std::min(i + 4 + extra, static_cast<int>(good.size()));
            const QString name = QString("trunc_block_%1.enc").arg(caseIdx++);
            QFile f(tmp.path() + "/" + name);
            ASSERT_TRUE(f.open(QIODevice::WriteOnly));
            f.write(good.left(len));
            f.close();
            delete readEncoreScore(name);
        }
    }

    setRootDir(ENC_DIR);
}

// A live-recorded chord's notes carry a small per-note tick "strum"; since they share one notated column
// (xoffset), same-column runs must collapse onto the anchor tick into a single chord, not split into extras.
TEST_F(Tst_Structure, chord_strum_xoffset_collapses_to_single_chord)
{
    MasterScore* score = readEncoreScore("notes_chord_strum_xoffset.enc");
    ASSERT_NE(score, nullptr);

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "chord-strum score should pass sanityCheck: " << ret.text();

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    std::vector<Chord*> chords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            chords.push_back(toChord(e));
        }
    }

    ASSERT_EQ(chords.size(), 8u) << "Expected exactly 8 chords (one per notated column)";

    const std::vector<DurationType> expectedDur = {
        DurationType::V_EIGHTH, DurationType::V_16TH, DurationType::V_16TH, DurationType::V_EIGHTH,
        DurationType::V_16TH, DurationType::V_16TH, DurationType::V_EIGHTH, DurationType::V_EIGHTH
    };
    for (size_t i = 0; i < chords.size(); ++i) {
        Chord* c = chords[i];
        std::vector<int> pitches;
        for (Note* n : c->notes()) {
            pitches.push_back(n->pitch());
        }
        std::sort(pitches.begin(), pitches.end());
        ASSERT_EQ(pitches.size(), 4u) << "Chord " << i << " must have 4 notes";
        EXPECT_EQ(pitches[0], 59) << "Chord " << i;
        EXPECT_EQ(pitches[1], 62) << "Chord " << i;
        EXPECT_EQ(pitches[2], 67) << "Chord " << i;
        EXPECT_EQ(pitches[3], 71) << "Chord " << i;
        EXPECT_EQ(c->durationType().type(), expectedDur[i]) << "Chord " << i << " duration";
    }

    delete score;
}

// Inverse of the collapse: two notes a few ticks apart in different columns must stay separate events, not
// merge into one chord, so tightly played tuplet members remain distinct.
TEST_F(Tst_Structure, diff_column_notes_do_not_merge)
{
    MasterScore* score = readEncoreScore("notes_diff_column_no_merge.enc");
    ASSERT_NE(score, nullptr);

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "diff-column score should pass sanityCheck: " << ret.text();

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    std::vector<Chord*> chords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            chords.push_back(toChord(e));
        }
    }

    ASSERT_GE(chords.size(), 2u) << "The two eighths must not collapse into a single chord";
    EXPECT_EQ(chords[0]->notes().size(), 1u) << "First eighth must be a single note (60)";
    EXPECT_EQ(chords[0]->notes().front()->pitch(), 60);
    EXPECT_EQ(chords[1]->notes().size(), 1u) << "Second eighth must be a single note (64)";
    EXPECT_EQ(chords[1]->notes().front()->pitch(), 64);

    delete score;
}

// A 3:2 triplet whose 2nd and 3rd positions were played a few ticks apart in different columns must keep
// all three members separate; without the column check they merge into a two-member triplet.
TEST_F(Tst_Structure, tuplet_diff_column_keeps_all_members)
{
    MasterScore* score = readEncoreScore("notes_tuplet_diff_column_keeps_members.enc");
    ASSERT_NE(score, nullptr);

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    Tuplet* tuplet = nullptr;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChordRest() && toChordRest(e)->tuplet()) {
            tuplet = toChordRest(e)->tuplet();
            break;
        }
    }
    ASSERT_NE(tuplet, nullptr) << "Expected a tuplet on staff 0";
    EXPECT_EQ(tuplet->ratio(), Fraction(3, 2)) << "Expected a 3:2 triplet";

    // Count note-bearing members. Merging positions 2 and 3 would give 2 members
    // (one of them a two-note chord [64,67]) instead of the three single notes.
    std::vector<int> memberPitches;
    int noteMembers = 0;
    for (DurationElement* de : tuplet->elements()) {
        if (de->isChord()) {
            ++noteMembers;
            EXPECT_EQ(toChord(de)->notes().size(), 1u)
                << "Each triplet member must be a single note, not a merged chord";
            memberPitches.push_back(toChord(de)->notes().front()->pitch());
        }
    }
    ASSERT_EQ(noteMembers, 3)
        << "Triplet must keep all 3 members; merging positions 2 and 3 leaves 2";
    std::sort(memberPitches.begin(), memberPitches.end());
    EXPECT_EQ(memberPitches[0], 60);
    EXPECT_EQ(memberPitches[1], 64);
    EXPECT_EQ(memberPitches[2], 67);

    delete score;
}

// Regression: a double barline between two measures is stored by Encore as the SECOND
// measure's start barline (byte 0x0C = DOUBLEL), not the first measure's end barline.
// The importer handled only end barlines, so the divider was dropped. It must map a
// special start barline onto the previous measure's end barline.
TEST_F(Tst_Structure, v0c4_start_double_barline_maps_to_previous_end)
{
    MasterScore* score = readEncoreScore("structure_start_double_barline.enc");
    ASSERT_NE(score, nullptr) << "Failed to load structure_start_double_barline.enc";
    Measure* first = score->firstMeasure();
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first->endBarLineType(), BarLineType::DOUBLE)
        << "the double bar before measure 2 must appear as measure 1's end barline";
    delete score;
}

// A voice that carries a real note can also carry a redundant placeholder REST stored at the SAME
// tick (Encore writes a rest slot for the voice even where the note sits). The non-tuplet rest must
// be dropped so the note keeps beat 1; with the bug the rest was emitted first and pushed the note
// off the beat, overflowing the bar. Fixture: 2/4, voice 0 = eighth rest @0 + quarter @0 + quarter.
TEST_F(Tst_Structure, coincident_placeholder_rest_dropped_note_keeps_beat)
{
    MasterScore* score = readEncoreScore("structure_rest_coincident_with_note.enc");
    ASSERT_NE(score, nullptr) << "Failed to load structure_rest_coincident_with_note.enc";
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Corrupted: " << ret.text();

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);
    Segment* first = m->first(SegmentType::ChordRest);
    ASSERT_NE(first, nullptr);
    EngravingItem* e = first->element(0);
    ASSERT_NE(e, nullptr);
    EXPECT_TRUE(e->isChord())
        << "beat 1 must be the note, not a placeholder rest pushed ahead of it";
    EXPECT_EQ(first->tick(), m->tick());
}
