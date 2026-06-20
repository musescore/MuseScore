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

// Text import: lyrics (syllable matching, verses, hyphen/melisma, encodings) and staff/rehearsal/tempo text,
// including rich-text runs. See ENCORE_FORMAT.md §Lyric element and ENCORE_FORMAT.md §TEXT block.

#include <gtest/gtest.h>

#include "engraving/dom/arpeggio.h"
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
#include "engraving/dom/harmony.h"
#include "engraving/dom/tuplet.h"

#include "../internal/importer/emitters-internal.h"
#include "testbase.h"

static const QString ENC_DIR(QString(iex_encore_tests_DATA_ROOT) + "/data/");

using namespace mu::engraving;

class Tst_Text : public ::testing::Test, public MTest
{
protected:
    void SetUp() override { setRootDir(ENC_DIR); }
};

// A hyphen opening a measure must promote the previous measure's last syllable (SINGLE -> BEGIN) so the
// connecting hyphen survives across the barline.
TEST_F(Tst_Text, lyrics_hyphen_renders_across_barline)
{
    MasterScore* score = readEncoreScore("text_lyrics_hyphen_across_barline.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck());

    LyricsSyllabic sofSyll = LyricsSyllabic::SINGLE;
    LyricsSyllabic tlySyll = LyricsSyllabic::SINGLE;
    bool sofSeen = false, tlySeen = false;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            EngravingItem* e = s->element(0);
            if (!e || !e->isChord()) {
                continue;
            }
            for (Lyrics* ly : toChord(e)->lyrics()) {
                if (ly->plainText() == u"sof") {
                    sofSyll = ly->syllabic();
                    sofSeen = true;
                } else if (ly->plainText() == u"tly") {
                    tlySyll = ly->syllabic();
                    tlySeen = true;
                }
            }
        }
    }
    ASSERT_TRUE(sofSeen && tlySeen);
    EXPECT_EQ(sofSyll, LyricsSyllabic::BEGIN) << "first syllable must become BEGIN so the hyphen renders";
    EXPECT_EQ(tlySyll, LyricsSyllabic::END) << "continuation syllable after the barline is END";

    delete score;
}

TEST_F(Tst_Text, lyrics_hyphen_separators_dropped_and_set_syllabic)
{
    MasterScore* score = readEncoreScore("text_lyrics_hyphenated_words.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    struct Entry {
        String text;
        LyricsSyllabic syll;
    };
    std::vector<Entry> seen;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            EngravingItem* el = s->element(0);
            if (!el || !el->isChord()) {
                continue;
            }
            for (Lyrics* ly : toChord(el)->lyrics()) {
                seen.push_back({ ly->plainText(), ly->syllabic() });
            }
        }
    }
    ASSERT_EQ(seen.size(), 3u);
    EXPECT_EQ(seen[0].text, String(u"JU"));
    EXPECT_EQ(seen[0].syll, LyricsSyllabic::BEGIN)
        << "JU is followed by a hyphen continuation";
    EXPECT_EQ(seen[1].text, String(u"LIO"));
    EXPECT_EQ(seen[1].syll, LyricsSyllabic::END)
        << "LIO closes the JU-LIO word";
    EXPECT_EQ(seen[2].text, String(u"RO"));
    EXPECT_EQ(seen[2].syll, LyricsSyllabic::BEGIN)
        << "RO is followed by a hyphen continuation past the bar";
    delete score;
}

TEST_F(Tst_Text, lyrics_variable_length_with_empty_placeholder)
{
    MasterScore* score = readEncoreScore("text_lyrics_variable.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    std::vector<String> expected = { u"JU", u"LIO", u"RO" };
    std::vector<String> seen;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            EngravingItem* el = s->element(0);
            if (!el || !el->isChord()) {
                continue;
            }
            for (Lyrics* ly : toChord(el)->lyrics()) {
                seen.push_back(ly->plainText());
            }
        }
    }
    EXPECT_EQ(seen, expected);
    delete score;
}

// A lyric whose tick is slightly after its note's (visual offset) must still attach to that note; the
// note-tick mapping must not halve the Encore tick, which pushed offset lyrics past the match threshold.
TEST_F(Tst_Text, lyrics_offset_ticks_still_attach_correctly)
{
    MasterScore* score = readEncoreScore("text_lyrics_6_8_offset_ticks.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    std::vector<String> expected = { u"do", u"re", u"mi", u"fa" };
    std::vector<String> seen;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            EngravingItem* el = s->element(0);
            if (!el || !el->isChord()) {
                continue;
            }
            for (Lyrics* ly : toChord(el)->lyrics()) {
                seen.push_back(ly->plainText());
            }
        }
    }
    EXPECT_EQ(seen, expected)
        << "All 4 lyrics must attach to their correct note even when lyric encTick "
        "is +50 ticks after the note encTick";
    delete score;
}

// Lyric encoding is detected per element: a Latin-1 (one byte/char) lyric must not be read as UTF-16 LE,
// which produced spurious CJK code units.
TEST_F(Tst_Text, lyrics_latin1_text_decoded_as_one_byte_per_char)
{
    MasterScore* score = readEncoreScore("text_lyrics_latin1.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    std::vector<String> seen;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            EngravingItem* el = s->element(0);
            if (!el || !el->isChord()) {
                continue;
            }
            for (Lyrics* ly : toChord(el)->lyrics()) {
                seen.push_back(ly->plainText());
            }
        }
    }
    ASSERT_EQ(seen.size(), 2u);
    EXPECT_EQ(seen[0], String(u"txã"));
    EXPECT_EQ(seen[1], String(u"nã"));
    delete score;
}

// Lyrics on a grand-staff bottom staff must be matched against that staff's routed notes, not the raw
// encStaff (which grabs another instrument's notes and reverses the syllables).
TEST_F(Tst_Text, lyrics_grandstaff_match_routed_staff_notes)
{
    MasterScore* score = readEncoreScore("text_lyrics_grandstaff_routed_notes.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck());

    // Collect (pitch, syllable, syllabic) for lyrics on MuseScore staff 1 (track 4 = staff 1, voice 0).
    struct Hit {
        int pitch;
        String text;
        LyricsSyllabic syll;
    };
    std::vector<Hit> hits;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            EngravingItem* el = s->element(4);   // staff 1, voice 0
            if (!el || !el->isChord()) {
                continue;
            }
            Chord* c = toChord(el);
            for (Lyrics* ly : c->lyrics()) {
                hits.push_back({ c->upNote()->pitch(), ly->plainText(), ly->syllabic() });
            }
        }
    }
    ASSERT_EQ(hits.size(), 2u) << "Both syllables must land on the bottom-staff notes";
    EXPECT_EQ(hits[0].text, String(u"Sal"));
    EXPECT_EQ(hits[0].pitch, 55) << "First syllable must be on the first bottom-staff note (pitch 55), not reversed";
    EXPECT_EQ(hits[0].syll, LyricsSyllabic::BEGIN);
    EXPECT_EQ(hits[1].text, String(u"ve"));
    EXPECT_EQ(hits[1].pitch, 57) << "Second syllable must be on the second bottom-staff note (pitch 57)";
    EXPECT_EQ(hits[1].syll, LyricsSyllabic::END);

    delete score;
}

// STAFFTEXT matching an Italian tempo term is promoted to TempoText (relative markings like "a tempo" get
// no absolute BPS); non-tempo strings stay StaffText.
TEST_F(Tst_Text, staff_text_promoted_to_tempo_for_italian_terms)
{
    MasterScore* score = readEncoreScore("text_stafftext_tempo_promotion.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    std::vector<String> tempoTexts;
    std::vector<String> staffTexts;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (!e) {
                    continue;
                }
                if (e->isTempoText()) {
                    tempoTexts.push_back(toTempoText(e)->plainText());
                } else if (e->isStaffText()) {
                    staffTexts.push_back(toStaffText(e)->plainText());
                }
            }
        }
    }
    EXPECT_EQ(tempoTexts, (std::vector<String> { u"Allegro", u"a tempo" }))
        << "Allegro and 'a tempo' must reach the score as TempoText, not StaffText";
    EXPECT_EQ(staffTexts, (std::vector<String> { u"ten." }))
        << "Non-tempo words remain plain StaffText";
    delete score;
}

TEST_F(Tst_Text, staff_text_promoted_to_tempo_sets_tempo_map)
{
    MasterScore* score = readEncoreScore("text_stafftext_tempo_promotion.enc");
    ASSERT_NE(score, nullptr);

    // Allegro at measure 0 must use the palette default of 144 BPM (= 2.4 BPS).
    const Fraction tick0(0, 1);
    EXPECT_NEAR(score->tempo(tick0).val, 144.0 / 60.0, 1e-6)
        << "Allegro at tick 0 must set the tempo to 144 BPM";
    delete score;
}

// ===========================================================================
// TITL slots of the same category join with newlines (headers/footers stack by alignment byte), and Encore
// #P/#D/#T tokens are rewritten to the MuseScore macros $P/$D/$m.
TEST_F(Tst_Text, multi_slot_text_joined_with_newlines)
{
    MasterScore* score = readEncoreScore("text_multi_slot_stacked_text.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    EXPECT_EQ(score->metaTag(u"composer"),
              u"Vicente Paiva e Jararáca\nAdapt.: Sgt Solano\nBanda de Música do CRPO/VRS")
        << "the three author slots must join into one newline-separated string";
    // Two center-aligned header lines must stack into a single Sid value.
    EXPECT_EQ(score->style().styleSt(mu::engraving::Sid::oddHeaderC),
              u"Top line one\nTop line two");
    EXPECT_EQ(score->style().styleSt(mu::engraving::Sid::evenHeaderC),
              u"Top line one\nTop line two");
    // The two footers have different alignments (left + right) and must
    // therefore stay on separate Sids.
    EXPECT_EQ(score->style().styleSt(mu::engraving::Sid::oddFooterL),  u"Left footer");
    EXPECT_EQ(score->style().styleSt(mu::engraving::Sid::oddFooterR),  u"Right footer");
    EXPECT_EQ(score->style().styleSt(mu::engraving::Sid::evenFooterL), u"Left footer");
    EXPECT_EQ(score->style().styleSt(mu::engraving::Sid::evenFooterR), u"Right footer");
    delete score;
}

// ===========================================================================
// When a file writes the TITL block twice, the second pass must replace the first, not double the
// composer/header/footer lines.
TEST_F(Tst_Text, duplicate_titl_block_does_not_double_lines)
{
    MasterScore* score = readEncoreScore("text_duplicate_titl_block.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    // The fixture writes the same TITL block twice; the composer must
    // appear exactly once.
    EXPECT_EQ(score->metaTag(u"composer"), u"Sole Composer")
        << "second TITL block must replace the first instead of appending";
    EXPECT_EQ(score->metaTag(u"workTitle"), u"Duped TITL");
    delete score;
}

// ===========================================================================
// A later empty TITL block (Encore writes one per page, page 2+ often blank) must not overwrite the first
// block's title and author with empty strings.
TEST_F(Tst_Text, empty_second_titl_block_preserves_first_block_data)
{
    MasterScore* score = readEncoreScore("text_titl_empty_second_block.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    // The fixture has TITL1 = {title="Multi TITL First", author="Real Author"};
    // TITL2 is completely empty. The first block's data must survive.
    EXPECT_EQ(score->metaTag(u"workTitle"), u"Multi TITL First")
        << "empty second TITL block must not overwrite the title from the first block";
    EXPECT_EQ(score->metaTag(u"composer"), u"Real Author")
        << "empty second TITL block must not overwrite the author from the first block";
    delete score;
}

// The MEAS header BPM must drive the tempo: a post-pass emits TempoText for the first measure and each BPM
// change (and sets Score::setTempo), rather than defaulting every import to 120.
TEST_F(Tst_Text, measure_header_bpm_drives_initial_tempo_and_changes)
{
    MasterScore* score = readEncoreScore("text_tempo_changes.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    struct Found {
        int measureIdx;
        double bps;
        String xmlText;
    };
    std::vector<Found> seen;
    int mi = -1;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        ++mi;
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isTempoText()) {
                    TempoText* tt = toTempoText(e);
                    seen.push_back({ mi, tt->tempo().val, tt->xmlText() });
                }
            }
        }
    }
    // [100,60,100,60,200,200]: emit TempoText for m1..m5 (initial + changes); m6 same as m5, no mark.
    ASSERT_EQ(seen.size(), 5u);
    EXPECT_EQ(seen[0].measureIdx, 0);
    EXPECT_NEAR(seen[0].bps, 100.0 / 60.0, 1e-6);
    EXPECT_EQ(seen[0].xmlText, u"<sym>metNoteQuarterUp</sym> = 100");
    EXPECT_EQ(seen[1].measureIdx, 1);
    EXPECT_NEAR(seen[1].bps, 60.0 / 60.0, 1e-6);
    EXPECT_EQ(seen[2].measureIdx, 2);
    EXPECT_NEAR(seen[2].bps, 100.0 / 60.0, 1e-6);
    EXPECT_EQ(seen[3].measureIdx, 3);
    EXPECT_NEAR(seen[3].bps, 60.0 / 60.0, 1e-6);
    EXPECT_EQ(seen[4].measureIdx, 4);
    EXPECT_NEAR(seen[4].bps, 200.0 / 60.0, 1e-6);

    // Tempo map must reflect the same changes (sampled at each measure start).
    Measure* m = score->firstMeasure();
    std::vector<int> expected = { 100, 60, 100, 60, 200, 200 };
    for (int i = 0; i < 6 && m; ++i, m = m->nextMeasure()) {
        EXPECT_NEAR(score->tempo(m->tick()).val, expected[i] / 60.0, 1e-6)
            << "measure " << i << " expected " << expected[i] << " BPM";
    }
    delete score;
}

// ===========================================================================
// The ORN TEMPO value is beat-unit BPM, so in a compound meter (dotted-quarter beat) it must be scaled by
// 3/2 to quarter-note BPM, not used directly.
TEST_F(Tst_Text, orn_tempo_compound_meter_dotted_quarter_bpm)
{
    MasterScore* score = readEncoreScore("text_tempo_orn_compound_68.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    TempoText* tt = nullptr;
    for (MeasureBase* mb = score->first(); mb && !tt; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s && !tt; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isTempoText()) {
                    tt = toTempoText(e);
                    break;
                }
            }
        }
    }
    ASSERT_NE(tt, nullptr) << "No TempoText found in score";
    EXPECT_NEAR(tt->tempo().val, 120.0 / 60.0, 1e-6)
        << "ORN TEMPO=80 in 6/8 must produce quarterBpm=120 (BPS=2.0), not 80/60";
    EXPECT_EQ(tt->xmlText(),
              u"<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80")
        << "Compound-meter tempo must use dotted-quarter sym tags; displayed value must be 80";

    delete score;
}

// A tempo ORN anchored to a note's tick but drawn (smaller xoffset) over the earlier downbeat rest must
// snap to the chord-rest matching its drawn position, like dynamics do, not sit on the later note.
TEST_F(Tst_Text, orn_tempo_snaps_to_downbeat_by_xoffset)
{
    MasterScore* score = readEncoreScore("text_tempo_orn_xoffset_downbeat.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck());

    TempoText* tt = nullptr;
    Segment* host = nullptr;
    for (MeasureBase* mb = score->first(); mb && !tt; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s && !tt; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isTempoText()) {
                    tt = toTempoText(e);
                    host = s;
                    break;
                }
            }
        }
    }
    ASSERT_NE(tt, nullptr) << "No TempoText found in score";
    EXPECT_TRUE(host->rtick().isZero())
        << "Tempo ORN must snap to the downbeat rest (rtick 0), not the later note; got rtick "
        << host->rtick().toString().toStdString();
    EXPECT_NEAR(tt->tempo().val, 63.0 / 60.0, 1e-6)
        << "5/8 is not compound: ORN TEMPO=63 means quarter=63 (BPS 1.05)";
    EXPECT_EQ(tt->xmlText(), u"<sym>metNoteQuarterUp</sym> = 63")
        << "Simple-meter tempo must display quarter=63";

    delete score;
}

// ===========================================================================
// The tempo mark's beat unit comes from the ORN noto byte, not the meter: a "quarter = 198" mark in 6/8
// must stay a quarter, not be rewritten as the compound-default dotted quarter.
TEST_F(Tst_Text, tempo_beat_unit_from_noto_overrides_compound_meter)
{
    MasterScore* score = readEncoreScore("text_tempo_orn_explicit_quarter_unit.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck());

    TempoText* tt = nullptr;
    for (MeasureBase* mb = score->first(); mb && !tt; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s && !tt; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isTempoText()) {
                    tt = toTempoText(e);
                    break;
                }
            }
        }
    }
    ASSERT_NE(tt, nullptr) << "No TempoText found in score";
    EXPECT_EQ(tt->xmlText(), u"<sym>metNoteQuarterUp</sym> = 198")
        << "noto=2 (quarter) must override the 6/8 compound default; not dotted-quarter=132";
    EXPECT_NEAR(tt->tempo().val, 198.0 / 60.0, 1e-6)
        << "Playback unchanged: 198 quarter/min = 3.3 BPS";

    delete score;
}

// Some v0xC2 files store the tempo the v0xC4 way (beat-unit code at +28, BPM at +30); the reader must keep
// the +30 BPM when +28 is a valid beat-unit code, or a quarter=158 mark imports as quarter=2.
TEST_F(Tst_Text, tempo_orn_v0c2_keeps_bpm_at_offset_30_when_28_is_beat_unit)
{
    MasterScore* score = readEncoreScore("text_tempo_orn_v0c2_v0c4_layout.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck());

    TempoText* tt = nullptr;
    for (MeasureBase* mb = score->first(); mb && !tt; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s && !tt; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isTempoText()) {
                    tt = toTempoText(e);
                    break;
                }
            }
        }
    }
    ASSERT_NE(tt, nullptr) << "No TempoText found in score";
    EXPECT_EQ(tt->xmlText(), u"<sym>metNoteQuarterUp</sym> = 158")
        << "v0xC2 with a beat-unit code at +28 keeps the BPM at +30 (158), not the unit (2)";
    EXPECT_NEAR(tt->tempo().val, 158.0 / 60.0, 1e-6);

    delete score;
}

// Tempo texts must use <sym> note tags (not a raw Unicode note glyph) so TempoText::updateTempo matches,
// and numeric BPM marks get followText=true so editing the displayed BPM keeps the tempo map in sync.
TEST_F(Tst_Text, tempo_text_uses_sym_tags_and_follow_text_enabled)
{
    MasterScore* score = readEncoreScore("ornaments_tempo_sym_followtext.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    struct Found {
        String xmlText;
        bool followText;
        double bps;
    };
    std::vector<Found> seen;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isTempoText()) {
                    TempoText* tt = toTempoText(e);
                    seen.push_back({ tt->xmlText(), tt->followText(), tt->tempo().val });
                }
            }
        }
    }
    // The fixture has 2 custom measures plus fill measures; at least 2 TempoTexts expected.
    ASSERT_GE(seen.size(), 2u);

    // m1: 4/4 bpm=100 -> simple meter -> quarter sym
    EXPECT_EQ(seen[0].xmlText, u"<sym>metNoteQuarterUp</sym> = 100")
        << "Simple-meter tempo must use metNoteQuarterUp sym tag, not raw unicode";
    EXPECT_TRUE(seen[0].followText)
        << "MEAS-header TempoText must have followText=true so BPM edits update playback";
    EXPECT_NEAR(seen[0].bps, 100.0 / 60.0, 1e-6);

    // m2: 6/8 bpm=80 beat-unit (dotted quarter) -> compound meter
    EXPECT_EQ(seen[1].xmlText,
              u"<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80")
        << "Compound-meter tempo must use dotted-quarter sym tags, not raw unicode";
    EXPECT_TRUE(seen[1].followText)
        << "Compound-meter TempoText must have followText=true";
    // MEAS header bpm=120 (quarter-note BPM); BPS = 120/60 = 2.0
    EXPECT_NEAR(seen[1].bps, 120.0 / 60.0, 1e-6);

    delete score;
}

TEST_F(Tst_Text, header_footer_tokens_translated_to_mscore_macros)
{
    MasterScore* score = readEncoreScore("text_header_footer_tokens.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    // header[0] right-aligned -> oddHeaderR + evenHeaderR.
    EXPECT_EQ(score->style().styleSt(mu::engraving::Sid::oddHeaderR), u"Page $P")
        << "#P must be rewritten to $P (page number on every page)";
    EXPECT_EQ(score->style().styleSt(mu::engraving::Sid::evenHeaderR), u"Page $P");
    // header[1] center-aligned -> oddHeaderC + evenHeaderC.
    EXPECT_EQ(score->style().styleSt(mu::engraving::Sid::oddHeaderC), u"Created $D")
        << "#D must be rewritten to $D (creation date)";
    EXPECT_EQ(score->style().styleSt(mu::engraving::Sid::evenHeaderC), u"Created $D");
    // footer[0] left-aligned -> oddFooterL + evenFooterL.
    EXPECT_EQ(score->style().styleSt(mu::engraving::Sid::oddFooterL), u"Time $m")
        << "#T must be rewritten to $m (best MuseScore equivalent for 'time')";
    EXPECT_EQ(score->style().styleSt(mu::engraving::Sid::evenFooterL), u"Time $m");
    delete score;
}

// A STAFFTEXT's tind byte indexes into the TEXT block for its display string; the importer resolves the
// StaffText via that index. See ENCORE_FORMAT.md §TEXT block.
TEST_F(Tst_Text, staff_text_resolved_via_text_block)
{
    MasterScore* score = readEncoreScore("text_staff_text.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    // "Allegretto" is promoted to TempoText; the remaining entries stay as StaffText.
    std::vector<String> expected = { u"cresc.", u"dimin.", u"ten." };
    std::vector<String> seen;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isStaffText()) {
                    seen.push_back(toStaffText(e)->plainText());
                }
            }
        }
    }
    EXPECT_EQ(seen, expected);
    delete score;
}

// A rich-text TEXT entry stores its text after a variable-length run header, so the text offset must be
// derived from the run count; assuming the single-run offset resolves a multi-run entry to garbage.
// See ENCORE_FORMAT.md §TEXT block.
TEST_F(Tst_Text, staff_text_multirun_header)
{
    MasterScore* score = readEncoreScore("text_staff_text_multirun.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    std::vector<String> seen;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isStaffText()) {
                    seen.push_back(toStaffText(e)->plainText());
                }
            }
        }
    }
    std::vector<String> expected = { u"TAN TRAN" };
    EXPECT_EQ(seen, expected)
        << "A multi-run TEXT entry must resolve its text via the run-count-derived offset";
    delete score;
}

// A rich-text TEXT entry can carry more than one formatting descriptor (count at the header), so the text
// offset must account for the descriptor count; assuming a single descriptor reads into a descriptor and
// decodes garbage. See ENCORE_FORMAT.md §TEXT block.
TEST_F(Tst_Text, staff_text_two_descriptors_header)
{
    MasterScore* score = readEncoreScore("text_staff_text_two_descriptors.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    std::vector<String> seen;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isStaffText()) {
                    seen.push_back(toStaffText(e)->plainText());
                }
            }
        }
    }
    std::vector<String> expected = { u"Cajas y Tambores" };
    EXPECT_EQ(seen, expected)
        << "A two-descriptor TEXT entry must resolve its text via the descriptor-count-derived offset";
    delete score;
}

// Multi-part files write one TEXT block per part view in different order, but tind indices match only the
// first (score) block, so the importer must keep the first block, not overwrite it with each one read.
TEST_F(Tst_Text, staff_text_uses_first_text_block)
{
    MasterScore* score = readEncoreScore("text_staff_text_first_block_wins.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    String seen;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isStaffText()) {
                    seen = toStaffText(e)->plainText();
                }
            }
        }
    }
    EXPECT_EQ(seen, String(u"Alpha"))
        << "tind=0 must resolve against the FIRST TEXT block ('Alpha'), not the last ('Beta')";
    delete score;
}

// Encore separates lines inside a TEXT-block entry with U+0004; a multi-line staff text must keep all lines
// (joined with newlines), not stop at the first separator.
TEST_F(Tst_Text, staff_text_multiline_preserved)
{
    MasterScore* score = readEncoreScore("text_staff_text_multiline.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    String seen;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isStaffText()) {
                    seen = toStaffText(e)->plainText();
                }
            }
        }
    }
    EXPECT_EQ(seen, String(u"Notes + change duration\n(third quarter to half)"))
        << "multi-line staff text must keep both lines (was truncated at first U+0004)";
    delete score;
}

// STAFFTEXT placement comes from the ORN yoffset: positive keeps ABOVE, negative (Cartesian below) maps to BELOW.
TEST_F(Tst_Text, staff_text_placement_from_yoffset)
{
    MasterScore* score = readEncoreScore("text_staff_text_placement.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    std::map<String, PlacementV> placements;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isStaffText()) {
                    StaffText* st = toStaffText(e);
                    placements[st->plainText()] = st->placement();
                }
            }
        }
    }
    ASSERT_EQ(placements.size(), 2u) << "two STAFFTEXT elements expected";
    EXPECT_EQ(placements[u"Above"], PlacementV::ABOVE)
        << "yoffset=+10 must keep default ABOVE placement";
    EXPECT_EQ(placements[u"ten"], PlacementV::BELOW)
        << "yoffset=-10 (Cartesian below) must map to PlacementV::BELOW";
    delete score;
}

// Pure-function tests for tempoXmlText(): the note symbol is always a <sym> tag (not raw Unicode), the
// beat-unit variant follows beatTicks, and displayBpm is the beat-unit BPM verbatim (no conversion).
// ===========================================================================
TEST(Tst_TempoXmlText, simple_meter_quarter_sym)
{
    using namespace mu::iex::enc;
    // beatTicks=240 (quarter beat): quarter sym
    EXPECT_EQ(tempoXmlText(120, 240),
              String(u"<sym>metNoteQuarterUp</sym> = 120"));
    EXPECT_EQ(tempoXmlText(80, 240),
              String(u"<sym>metNoteQuarterUp</sym> = 80"));
    EXPECT_EQ(tempoXmlText(100, 240),
              String(u"<sym>metNoteQuarterUp</sym> = 100"));
}

TEST(Tst_TempoXmlText, dotted_quarter_beat_sym)
{
    using namespace mu::iex::enc;
    // beatTicks=360 (dotted-quarter beat): dotted-quarter sym; displayBpm is already the beat-unit value.
    // (For MEAS BPM the caller converts QPM to displayBpm via bpm*2/3 before calling tempoXmlText.)
    EXPECT_EQ(tempoXmlText(80, 360),
              String(u"<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80"));
    // beatTicks=360 for 3/8 (dotted-quarter beat, previously incorrectly treated as quarter)
    EXPECT_EQ(tempoXmlText(80, 360),
              String(u"<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80"));
}

TEST(Tst_TempoXmlText, beat_ticks_select_the_note_symbol)
{
    using namespace mu::iex::enc;
    // beatTicks is the beat duration in display ticks (quarter=240) and selects the note
    // symbol: 120=eighth, 480=half, plus the dotted variants (base x 3/2).
    EXPECT_EQ(tempoXmlText(156, 120),
              String(u"<sym>metNote8thUp</sym> = 156"));
    EXPECT_EQ(tempoXmlText(120, 240),
              String(u"<sym>metNoteQuarterUp</sym> = 120"));
    EXPECT_EQ(tempoXmlText(90, 480),
              String(u"<sym>metNoteHalfUp</sym> = 90"));
    EXPECT_EQ(tempoXmlText(60, 720),
              String(u"<sym>metNoteHalfUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 60"));
    // Unknown/zero falls back to a plain quarter note.
    EXPECT_EQ(tempoXmlText(80, 0),
              String(u"<sym>metNoteQuarterUp</sym> = 80"));
}

// An ORN TEMPO must not be suppressed just because its value disagrees with the MEAS header BPM when the
// beat is not a quarter (the two are in different units); the genuine ORN mark must be used.
TEST_F(Tst_Text, orn_tempo_5_8_not_suppressed_and_uses_quarter_bpm)
{
    MasterScore* score = readEncoreScore("text_orn_tempo_eighth_beat_not_suppressed.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    TempoText* tt = nullptr;
    for (MeasureBase* mb = score->first(); mb && !tt; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s && !tt;
             s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isTempoText()) {
                    tt = toTempoText(e);
                    break;
                }
            }
        }
    }
    ASSERT_NE(tt, nullptr) << "ORN TEMPO=63 in 5/8 must create a TempoText";
    EXPECT_NEAR(tt->tempo().val, 63.0 / 60.0, 1e-5)
        << "ORN tempo=63 is quarter-note BPM regardless of beatTicks=120; BPS=63/60";
    EXPECT_EQ(tt->xmlText(), String(u"<sym>metNoteQuarterUp</sym> = 63"))
        << "Display must show quarter note symbol (negra) matching Encore's 'negra = 63'";

    delete score;
}

// A 3/8 piece with a dotted-quarter beat (beatTicks=360) must get the same 1.5x BPS adjustment as 6/8;
// the compound check must key on beatTicks, not just numerator > 3 (which excludes 3/8).
TEST_F(Tst_Text, orn_tempo_3_8_dotted_quarter_bps_correct)
{
    MasterScore* score = readEncoreScore("text_orn_tempo_3_8_dotted_quarter.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    TempoText* tt = nullptr;
    for (MeasureBase* mb = score->first(); mb && !tt; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s && !tt; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isTempoText()) {
                    tt = toTempoText(e);
                    break;
                }
            }
        }
    }
    ASSERT_NE(tt, nullptr) << "No TempoText found in score";
    EXPECT_NEAR(tt->tempo().val, 80.0 * 1.5 / 60.0, 1e-5)
        << "ORN TEMPO=80 in 3/8 (beatTicks=360) must give BPS=2.0 (dotted-quarter 80), "
        "not 1.333 (plain quarter 80)";
    EXPECT_EQ(tt->xmlText(),
              u"<sym>metNoteQuarterUp</sym><sym>space</sym><sym>metAugmentationDot</sym> = 80")
        << "Display must show dotted-quarter=80, not quarter=80";

    delete score;
}

// ===========================================================================
// An ORN TEMPO that is not a misplaced ornament (no later measure carries its BPM) is the genuine score
// tempo and takes precedence over the MEAS header BPM. The header-BPM guard must scan the whole measure so
// an ORN placed at a later tick (on the first note, not the first rest) does not produce a second mark.
TEST_F(Tst_Text, orn_tempo_wins_over_meas_bpm_when_not_misplaced)
{
    MasterScore* score = readEncoreScore("text_meas_bpm_suppressed_by_orn_tempo_later_tick.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    TempoText* tt = nullptr;
    for (MeasureBase* mb = score->first(); mb && !tt; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s && !tt;
             s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isTempoText()) {
                    tt = toTempoText(e);
                    break;
                }
            }
        }
    }
    ASSERT_NE(tt, nullptr) << "ORN TEMPO=63 must create a TempoText";
    EXPECT_NEAR(tt->tempo().val, 63.0 / 60.0, 1e-5)
        << "ORN TEMPO=63 (quarter BPM) must give BPS=63/60; "
        "ORN overrides MEAS header when it is not a misplaced ornament";

    delete score;
}

// Regression: importTempoTextSemantic=false must suppress ORN TEMPO (visual score
// marking) just like Italian text; only the MEAS header BPM creates a TempoText.
TEST_F(Tst_Text, orn_tempo_suppressed_when_semantic_disabled)
{
    mu::iex::enc::EncImportOptions opts;
    opts.importTempoTextSemantic = false;
    MasterScore* score = readEncoreScoreWithOpts("text_meas_bpm_suppressed_by_orn_tempo_later_tick.enc", opts);
    ASSERT_NE(score, nullptr);

    std::vector<double> bpsValues;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s;
             s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isTempoText()) {
                    bpsValues.push_back(toTempoText(e)->tempo().val);
                }
            }
        }
    }
    EXPECT_EQ(bpsValues.size(), 1u) << "importTempoTextSemantic=false: ORN TEMPO must be suppressed";
    if (!bpsValues.empty()) {
        EXPECT_NEAR(bpsValues[0], 160.0 / 60.0, 1e-5)
            << "With semantic=false only MEAS header BPM=160 must apply";
    }
    delete score;
}

// An ORN TEMPO whose BPM conflicts with its measure's header but matches a later measure is a misplaced
// ornament; it must be suppressed so the later measure gets the correct TempoText from the header path.
TEST_F(Tst_Text, orn_tempo_mismatch_with_header_bpm_suppressed)
{
    MasterScore* score = readEncoreScore("text_orn_tempo_mismatch_suppressed.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    struct TempoAtTick {
        Fraction tick;
        double bps;
    };
    std::vector<TempoAtTick> found;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s;
             s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isTempoText()) {
                    found.push_back({ s->tick(), toTempoText(e)->tempo().val });
                }
            }
        }
    }

    // No TempoText at measure 1 (tick=0) with BPM=80, the misplaced ornament must be suppressed
    for (const auto& t : found) {
        if (t.tick == Fraction(0, 1)) {
            EXPECT_FALSE(std::abs(t.bps - 80.0 / 60.0) < 1e-4)
                << "ORN TEMPO=80 at M1 (header BPM=249) must be suppressed (misplaced ornament)";
        }
    }

    // TempoText BPM=80 must appear somewhere after tick=0 (at measure 2)
    bool foundM2 = false;
    for (const auto& t : found) {
        if (t.tick > Fraction(0, 1) && std::abs(t.bps - 80.0 / 60.0) < 1e-4) {
            foundM2 = true;
        }
    }
    EXPECT_TRUE(foundM2)
        << "Header BPM=80 must create TempoText at M2 when misplaced ORN TEMPO at M1 is suppressed";

    delete score;
}

// Same misplacement quirk, but the tempo change is several measures after the ORN (the old check
// only looked one measure ahead, so it missed this and placed the tempo too early).
TEST_F(Tst_Text, orn_tempo_misplaced_multi_measure_suppressed)
{
    // text_orn_tempo_misplaced_multi_measure.enc:
    // M1: header BPM=249, ORN TEMPO=80 (misplaced); M2-M3: header BPM=249; M4: header BPM=80.
    // Expected: TempoText BPM=80 at M4 (tick=3 whole notes), NOT at M1.
    MasterScore* score = readEncoreScore("text_orn_tempo_misplaced_multi_measure.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck());

    Fraction tickOf80;
    bool found80 = false, eightyAtStart = false;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isTempoText() && std::abs(toTempoText(e)->tempo().val - 80.0 / 60.0) < 1e-4) {
                    found80 = true;
                    tickOf80 = s->tick();
                    if (s->tick() == Fraction(0, 1)) {
                        eightyAtStart = true;
                    }
                }
            }
        }
    }
    EXPECT_TRUE(found80) << "header BPM=80 at M4 must create a TempoText";
    EXPECT_FALSE(eightyAtStart) << "misplaced ORN TEMPO=80 must not place a tempo at M1";
    EXPECT_GT(tickOf80, Fraction(2, 1)) << "tempo 80 must land at M4 (several measures in), not at M1";

    delete score;
}

// An ORN TEMPO whose BPM equals its own measure's header BPM (e.g. an initial "= 230" Encore
// stores at the end of measure 1) is redundant: it must be suppressed so the header places the
// tempo at the MEASURE START, where it actually drives playback, not left on the ORN's late
// segment (which does not set the tempo, leaving the default).
TEST_F(Tst_Text, orn_tempo_equal_to_header_placed_at_measure_start)
{
    MasterScore* score = readEncoreScore("text_orn_tempo_equals_header.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck());

    int count230 = 0;
    bool atStart = false, late = false;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isTempoText() && std::abs(toTempoText(e)->tempo().val - 230.0 / 60.0) < 1e-4) {
                    ++count230;
                    (s->tick() == Fraction(0, 1) ? atStart : late) = true;
                }
            }
        }
    }
    EXPECT_EQ(count230, 1) << "exactly one tempo 230 (no duplicate)";
    EXPECT_TRUE(atStart) << "tempo 230 must be placed at the measure start";
    EXPECT_FALSE(late) << "tempo 230 must not stay at the ORN's late tick";

    delete score;
}

// End-to-end v0xC2 6/8 lyric fixture exercising three fixes together: the shorter post-kie text gap so
// syllables are not truncated (see ENCORE_FORMAT.md §Lyric element), lyrics-first matching, and a
// compound-meter encTicksPerQuarter (beatTicks * 2/3). All 56 syllables must import intact.

static std::vector<String> collectAllLyrics(MasterScore* score)
{
    std::vector<String> lyrics;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (track_idx_t t = 0; t < score->ntracks(); ++t) {
                EngravingItem* el = s->element(t);
                if (!el || !el->isChord()) {
                    continue;
                }
                for (Lyrics* ly : toChord(el)->lyrics()) {
                    lyrics.push_back(ly->plainText());
                }
            }
        }
    }
    return lyrics;
}

TEST_F(Tst_Text, title_frame_created)
{
    // kordorkestro has title "String Orchestra w/Piano"
    MasterScore* score = readEncoreScore("kordorkestro.enc");
    ASSERT_NE(score, nullptr);
    MeasureBase* first = score->first();
    ASSERT_NE(first, nullptr);
    EXPECT_TRUE(first->isVBox()) << "Score with title should start with a VBox frame";
    delete score;
}

TEST_F(Tst_Text, no_title_frame_when_empty)
{
    // bazo.enc has no title, should not have a VBox frame
    MasterScore* score = readEncoreScore("bazo.enc");
    ASSERT_NE(score, nullptr);
    MeasureBase* first = score->first();
    ASSERT_NE(first, nullptr);
    EXPECT_TRUE(first->isMeasure()) << "Score without title should start with a measure";
    delete score;
}

TEST_F(Tst_Text, title_frame_instruction_and_copyright)
{
    MasterScore* score = readEncoreScore("text_title_instruction_copyright.enc");
    ASSERT_NE(score, nullptr);

    MeasureBase* first = score->first();
    ASSERT_NE(first, nullptr);
    ASSERT_TRUE(first->isVBox()) << "TITL with content must produce a VBox frame";

    std::map<TextStyleType, String> texts;
    for (const EngravingItem* el : first->el()) {
        if (el->isText()) {
            const TextBase* tb = toTextBase(el);
            texts[tb->textStyleType()] = tb->plainText();
        }
    }

    EXPECT_EQ(texts[TextStyleType::TITLE],    String(u"Test Title"));
    EXPECT_EQ(texts[TextStyleType::SUBTITLE], String(u"Test Subtitle"));
    EXPECT_EQ(texts[TextStyleType::LYRICIST], String(u"Test Instruction"))
        << "instruction[0] must be added as LYRICIST text";
    EXPECT_EQ(texts[TextStyleType::COMPOSER], String(u"Test Composer"));

    EXPECT_EQ(score->metaTag(u"workTitle"),  String(u"Test Title"))
        << "title must be stored in workTitle metadata";
    EXPECT_EQ(score->metaTag(u"subtitle"),   String(u"Test Subtitle"))
        << "subtitle[0] must be stored in subtitle metadata";
    EXPECT_EQ(score->metaTag(u"lyricist"),   String(u"Test Instruction"))
        << "instruction[0] must be stored in lyricist metadata";
    EXPECT_EQ(score->metaTag(u"composer"),   String(u"Test Composer"))
        << "author[0] must be stored in composer metadata";
    EXPECT_EQ(score->metaTag(u"copyright"),  String(u"(c) 2026 Test"))
        << "copyright[0] must be stored in copyright metadata";

    delete score;
}

TEST_F(Tst_Text, title_frame_headers_footers)
{
    MasterScore* score = readEncoreScore("text_titl_headers_footers.enc");
    ASSERT_NE(score, nullptr);

    auto styleText = [score](Sid sid) -> String {
        return score->style().styleSt(sid);
    };

    EXPECT_EQ(styleText(Sid::oddHeaderR),  String(u"Header Right"));
    EXPECT_EQ(styleText(Sid::evenHeaderR), String(u"Header Right"));
    EXPECT_EQ(styleText(Sid::oddHeaderC),  String(u"Header Center"));
    EXPECT_EQ(styleText(Sid::evenHeaderC), String(u"Header Center"));
    EXPECT_NE(styleText(Sid::oddHeaderL),  String(u"Header Right"));
    EXPECT_NE(styleText(Sid::oddHeaderL),  String(u"Header Center"));
    EXPECT_NE(styleText(Sid::evenHeaderL), String(u"Header Right"));
    EXPECT_NE(styleText(Sid::evenHeaderL), String(u"Header Center"));

    EXPECT_EQ(styleText(Sid::oddFooterC),  String(u"Footer Center"));
    EXPECT_EQ(styleText(Sid::evenFooterC), String(u"Footer Center"));
    EXPECT_EQ(styleText(Sid::oddFooterR),  String(u"Footer Right"));
    EXPECT_EQ(styleText(Sid::evenFooterR), String(u"Footer Right"));
    EXPECT_NE(styleText(Sid::oddFooterL),  String(u"Footer Center"));
    EXPECT_NE(styleText(Sid::oddFooterL),  String(u"Footer Right"));
    EXPECT_NE(styleText(Sid::evenFooterL), String(u"Footer Center"));
    EXPECT_NE(styleText(Sid::evenFooterL), String(u"Footer Right"));

    delete score;
}

// Regression: TITL encoding inherited TK00 charSize; files with large TK offset but Latin-1 TITL mis-decoded.
TEST_F(Tst_Text, v0c4_titl_latin1_small_varsize)
{
    MasterScore* score = readEncoreScore("text_titl_latin1_small_varsize.enc");
    ASSERT_NE(score, nullptr) << "Failed to load text_titl_latin1_small_varsize.enc";

    EXPECT_EQ(score->metaTag(u"workTitle"), String(u"Romeria"))
        << "small-varsize TITL must decode as Latin-1, not as TWO_BYTES UTF-16";
    delete score;
}

// Regression: formula-offset name recovery probed UTF-16 only; Latin-1 names were discarded silently.
TEST_F(Tst_Text, v0c4_recovered_name_latin1)
{
    MasterScore* score = readEncoreScore("text_recovered_name_latin1.enc");
    ASSERT_NE(score, nullptr) << "Failed to load text_recovered_name_latin1.enc";

    ASSERT_GE(score->parts().size(), 1u);
    const Part* part = score->parts()[0];
    ASSERT_NE(part, nullptr);
    EXPECT_EQ(part->partName(), String(u"Tropa"))
        << "Latin-1 name at NAME_BASE must be recovered when TK block name is empty";
    delete score;
}

// Lightweight test macro for Tst_Text
#ifndef ENC_SANITY_TEST_TEXT
#define ENC_SANITY_TEST_TEXT(testName, fileName) \
    TEST_F(Tst_Text, testName) { \
        MasterScore* score = readEncoreScore(fileName); \
        ASSERT_NE(score, nullptr) << "Failed to load " << fileName; \
        EXPECT_GT(score->nmeasures(), 0); \
        muse::Ret ret = score->sanityCheck(); \
        EXPECT_TRUE(ret) << "Corrupted: " << ret.text(); \
        delete score; \
    }
#endif

ENC_SANITY_TEST_TEXT(staff_text,           "text_staff_text.enc")
ENC_SANITY_TEST_TEXT(titl_headers_footers, "text_titl_headers_footers.enc")
ENC_SANITY_TEST_TEXT(staff_text_placement, "text_staff_text_placement.enc")
ENC_SANITY_TEST_TEXT(keychange_to_c,       "structure_keychange_to_c.enc")
