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

// TIE element import: the many direction/flag/arc-extent/source-position encodings that mark a real tie start,
// consecutive-receiver matching, and a comprehensive all-features smoke fixture.
// See ENCORE_FORMAT.md §TIE element and ENCORE_IMPORTER.md §TIE element handling.

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

class Tst_NotesTies : public ::testing::Test, public MTest
{
protected:
    void SetUp() override { setRootDir(ENC_DIR); }
};

TEST_F(Tst_NotesTies, tie_direction_fc_creates_tie)
{
    MasterScore* score = readEncoreScore("notes_tie_dir_fc.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    int tieCount = 0;
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
            for (Note* n : toChord(el)->notes()) {
                if (n->tieFor()) {
                    ++tieCount;
                }
            }
        }
    }
    EXPECT_EQ(tieCount, 1) << "expected one tie from the 0xfc TIE element";
    delete score;
}

TEST_F(Tst_NotesTies, tie_direction_02_creates_tie)
{
    MasterScore* score = readEncoreScore("notes_tie_dir_02.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    int tieCount = 0;
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
            for (Note* n : toChord(el)->notes()) {
                if (n->tieFor()) {
                    ++tieCount;
                }
            }
        }
    }
    EXPECT_EQ(tieCount, 1) << "expected one tie from the 0x02 TIE dir element";
    delete score;
}

TEST_F(Tst_NotesTies, tie_18byte_intra_chord_arc_no_spurious_tie)
{
    // An 18-byte TIE with arcX1==arcX2 is an intra-chord arc, not a forward tie, so no ties are created.
    MasterScore* score = readEncoreScore("notes_tie_intra_chord_arc_no_spurious.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "intra-chord arc test must produce clean score: " << ret.text();

    int tieCount = 0;
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
            for (Note* n : toChord(el)->notes()) {
                if (n->tieFor()) {
                    ++tieCount;
                }
            }
        }
    }
    EXPECT_EQ(tieCount, 0)
        << "18-byte TIE with arcX1==arcX2 is an intra-chord arc; must produce NO forward ties";
    delete score;
}

TEST_F(Tst_NotesTies, tie_18byte_real_forward_still_creates_tie)
{
    // arcX1 != arcX2 is a real forward tie; C4@0 must tie to C4@480.
    MasterScore* score = readEncoreScore("notes_tie_18byte_real_forward.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "18-byte real forward tie must produce clean score: " << ret.text();

    int tieCount = 0;
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
            for (Note* n : toChord(el)->notes()) {
                if (n->tieFor()) {
                    ++tieCount;
                }
            }
        }
    }
    EXPECT_EQ(tieCount, 1)
        << "18-byte TIE with arcX1!=arcX2 is a real forward tie; must still create one Tie";
    delete score;
}

TEST_F(Tst_NotesTies, tie_direction_04_forward_creates_tie)
{
    // Regression: the +5 byte is a signed arc curvature, not a bitfield, so 0x04 (curves down) is a real
    // tie just like 0xFC. A forward tie is marked by arcX1 < arcX2 regardless of the +5 sign; the old
    // bit-based rule dropped every downward-curving tie.
    MasterScore* score = readEncoreScore("notes_tie_dir_04_forward.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    int tieCount = 0;
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
            for (Note* n : toChord(el)->notes()) {
                if (n->tieFor()) {
                    ++tieCount;
                }
            }
        }
    }
    EXPECT_EQ(tieCount, 1) << "expected one forward tie from the +5=0x04 (arcX1<arcX2) TIE element";
    delete score;
}

TEST_F(Tst_NotesTies, tie_direction_03_creates_tie)
{
    MasterScore* score = readEncoreScore("notes_tie_dir_03.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    int tieCount = 0;
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
            for (Note* n : toChord(el)->notes()) {
                if (n->tieFor()) {
                    ++tieCount;
                }
            }
        }
    }
    EXPECT_EQ(tieCount, 1) << "expected one tie from the 0x03 TIE dir element";
    delete score;
}

TEST_F(Tst_NotesTies, tie_start_flag_on_byte6_creates_tie)
{
    MasterScore* score = readEncoreScore("notes_tie_start_flag_byte6.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    int tieCount = 0;
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
            for (Note* n : toChord(el)->notes()) {
                if (n->tieFor()) {
                    ++tieCount;
                }
            }
        }
    }
    EXPECT_EQ(tieCount, 1)
        << "expected one tie from the +6=0x80 TIE-start-flag element";
    delete score;
}

TEST_F(Tst_NotesTies, tie_source_position_partial_chord)
{
    // Regression: a TIE carries a sourcePosition selecting which chord note it starts from. It must tie
    // only that note (A4 at pos=5), not every note at the chord tick (C#4 at pos=0 must stay untied).
    MasterScore* score = readEncoreScore("notes_tie_partial_chord_source_position.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "partial-chord source-position tie must produce clean score: " << ret.text();

    std::map<int, int> tiesByPitch;
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
            for (Note* n : toChord(el)->notes()) {
                if (n->tieFor()) {
                    tiesByPitch[n->pitch()]++;
                }
            }
        }
    }

    EXPECT_EQ(tiesByPitch.count(61), 0)
        << "C#4 (pitch=61, pos=0) must NOT get a tie: sourcePosition=5 does not match pos=0";
    EXPECT_EQ(tiesByPitch.count(69), 1)
        << "A4 (pitch=69, pos=5) must get exactly one tie: sourcePosition=5 matches pos=5";

    delete score;
}

TEST_F(Tst_NotesTies, tie_crossmeasure_arcxx_equal_with_startflag)
{
    // Regression: an explicit startFlag=0x80 must create a cross-measure tie even when arcX1==arcX2
    // (zero horizontal extent) would otherwise suppress it.
    MasterScore* score = readEncoreScore("notes_tie_crossmeasure_arcxx_equal.enc");
    ASSERT_NE(score, nullptr);

    std::vector<Note*> notes;
    for (MeasureBase* mb = score->first(); mb && notes.size() < 2; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            EngravingItem* el = s->element(0);
            if (el && el->isChord()) {
                Chord* c = toChord(el);
                if (!c->notes().empty()) {
                    notes.push_back(c->notes().front());
                }
            }
        }
    }
    ASSERT_GE(notes.size(), 2u);
    ASSERT_EQ(notes[0]->pitch(), 60);
    ASSERT_EQ(notes[1]->pitch(), 60);

    ASSERT_NE(notes[0]->tieFor(), nullptr)
        << "cross-measure tie (arcX1==arcX2, startFlag=0x80) must not be suppressed";
    EXPECT_EQ(notes[0]->tieFor()->endNote(), notes[1])
        << "tie must connect the whole note in measure 1 to the whole note in measure 2";

    delete score;
}

TEST_F(Tst_NotesTies, tie_spurious_far_receiver_dropped)
{
    // Regression: a tie completes only when the receiver begins exactly where the tie-start note ends.
    // A same-pitch note later in the measure (C4@720 after C4@0 ends at 240) is not consecutive, so the
    // tie must be dropped rather than arcing across the bar.
    MasterScore* score = readEncoreScore("notes_tie_spurious_far_receiver.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "spurious far-receiver test must produce clean score: " << ret.text();

    int tieCount = 0;
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
            for (Note* n : toChord(el)->notes()) {
                if (n->tieFor()) {
                    ++tieCount;
                }
            }
        }
    }
    EXPECT_EQ(tieCount, 0)
        << "tie-start with no consecutive same-pitch note must not arc to a far receiver";
    delete score;
}

TEST_F(Tst_NotesTies, tie_element_creates_mscore_tie)
{
    // A basic TIE element must link two consecutive same-pitch notes with a real Tie (they used to be dropped).
    MasterScore* score = readEncoreScore("notes_tie.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Tie test must produce clean score: " << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    Chord* first = nullptr;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            first = toChord(e);
            break;
        }
    }
    ASSERT_NE(first, nullptr) << "Must have a first chord";
    ASSERT_GE(first->notes().size(), 1u);

    Note* startNote = first->notes()[0];
    EXPECT_EQ(startNote->pitch(), 60) << "First note must be C4 (pitch=60)";

    ASSERT_NE(startNote->tieFor(), nullptr)
        << "TIE element must create a Tie from the first C4 to the second C4";
    EXPECT_NE(startNote->tieFor()->endNote(), nullptr)
        << "Tie must link to an end note";
    if (startNote->tieFor()->endNote()) {
        EXPECT_EQ(startNote->tieFor()->endNote()->pitch(), 60)
            << "Tie end note must also be C4";
    }
    delete score;
}

TEST_F(Tst_NotesTies, sf_tiestart_not_filtered_by_rdur)
{
    // A 64th/128th note that starts a tie must bypass the short-rdur (<15) MIDI-artifact filter.
    MasterScore* score = readEncoreScore("notes_sf_tiestart.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "sf tiestart test must produce clean score: " << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    Chord* first = nullptr;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            first = toChord(e);
            break;
        }
    }
    ASSERT_NE(first, nullptr) << "Must have a first chord (the 64th)";
    EXPECT_EQ(first->durationType().type(), DurationType::V_64TH)
        << "64th note with TIE element must not be filtered by rdur<15";
    ASSERT_GE(first->notes().size(), 1u);

    Note* sfNote = first->notes()[0];
    ASSERT_NE(sfNote->tieFor(), nullptr)
        << "64th note must have an outgoing tie to the Q note";
    if (sfNote->tieFor()) {
        Note* endNote = sfNote->tieFor()->endNote();
        ASSERT_NE(endNote, nullptr);
        EXPECT_EQ(endNote->chord()->durationType().type(), DurationType::V_QUARTER)
            << "Tie end must be the Q note";
    }
    delete score;
}

#ifndef ENC_SANITY_TEST_TIES
#define ENC_SANITY_TEST_TIES(testName, fileName) \
    TEST_F(Tst_NotesTies, testName) { \
        MasterScore* score = readEncoreScore(fileName); \
        ASSERT_NE(score, nullptr) << "Failed to load " << fileName; \
        EXPECT_GT(score->nmeasures(), 0); \
        muse::Ret ret = score->sanityCheck(); \
        EXPECT_TRUE(ret) << "Corrupted: " << ret.text(); \
        delete score; \
    }
#endif

ENC_SANITY_TEST_TIES(tie_start_flag_byte6, "notes_tie_start_flag_byte6.enc")
ENC_SANITY_TEST_TIES(tie_direction_fc,     "notes_tie_dir_fc.enc")

// Comprehensive synthetic fixture exercising every reader/importer feature; checks clean import and counts.
TEST_F(Tst_NotesTies, sintetico_all_features_imports_cleanly)
{
    MasterScore* score = readEncoreScore("sintetico_all_features.enc");
    ASSERT_NE(score, nullptr) << "Failed to load sintetico_all_features.enc";

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "sanityCheck failed: " << ret.text();

    EXPECT_EQ(score->parts().size(), 2u) << "2 instruments expected";
    EXPECT_EQ(score->nmeasures(), 20) << "20 measures expected";

    int fermatas=0, tuplets=0, lyrics_count=0, hairpins=0, spanners=0;
    int tremolos=0, arpeggios=0, tempos=0, dynamics=0, markers=0;

    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        if (m->repeatStart()) {
            ++spanners;
        }
        for (EngravingItem* me : m->el()) {
            if (me && me->isMarker()) {
                ++markers;
            }
        }
        for (Segment* s = m->first(SegmentType::ChordRest); s;
             s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (!e) {
                    continue;
                }
                if (e->isFermata()) {
                    ++fermatas;
                }
                if (e->isDynamic()) {
                    ++dynamics;
                }
                if (e->isTempoText()) {
                    ++tempos;
                }
            }
            for (int ti = 0; ti < static_cast<int>(score->nstaves() * VOICES); ++ti) {
                EngravingItem* el = s->element(static_cast<track_idx_t>(ti));
                if (!el || !el->isChord()) {
                    continue;
                }
                Chord* c = toChord(el);
                if (c->tremoloSingleChord()) {
                    ++tremolos;
                }
                if (c->arpeggio()) {
                    ++arpeggios;
                }
                for (Lyrics* ly : c->lyrics()) {
                    if (ly) {
                        ++lyrics_count;
                    }
                }
            }
            if (s->isChordRestType()) {
                Tuplet* tup = nullptr;
                EngravingItem* el = s->element(0);
                if (el && el->isChordRest()) {
                    tup = toChordRest(el)->tuplet();
                }
                if (tup && tup->elements().front() == s->element(0)) {
                    ++tuplets;
                }
            }
        }
    }
    for (const auto& kv : score->spanner()) {
        Spanner* sp = kv.second;
        if (sp && sp->isHairpin()) {
            ++hairpins;
        }
    }

    EXPECT_GE(fermatas,     1) << "at least 1 fermata (non-tuplet note with articUp=0x20)";
    EXPECT_GE(tuplets,      2) << "at least 2 tuplet groups (triplets in m3)";
    EXPECT_GE(lyrics_count, 4) << "4 lyrics syllables (do re mi fa)";
    EXPECT_GE(dynamics,     8) << "at least 8 of the 13 dynamics";
    EXPECT_GE(tempos,       3) << "at least 3 TempoText marks";
    EXPECT_GE(hairpins,     1) << "at least 1 hairpin";
    EXPECT_GE(tremolos,     1) << "at least 1 TremoloSingleChord";
    EXPECT_GE(arpeggios,    1) << "at least 1 arpeggio";
    EXPECT_GE(markers,      2) << "at least 2 section markers";
    EXPECT_GE(spanners,     1) << "at least 1 repeat-start barline";

    delete score;
}
