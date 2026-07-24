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

// Grace and cue notes: grace-before/after placement, dangling graces as cue notes, cue small/mute state,
// and ornaments applied to grace chords. See ENCORE_FORMAT.md §Grace and cue notes.

#include <gtest/gtest.h>

#include "engraving/dom/articulation.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/durationtype.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/segment.h"

#include "testbase.h"

static const QString ENC_DIR(QString(iex_encore_tests_DATA_ROOT) + "/data/");

using namespace mu::engraving;

class Tst_Grace : public ::testing::Test, public MTest
{
protected:
    void SetUp() override { setRootDir(ENC_DIR); }
};

#define ENC_SANITY_TEST(testName, fileName) \
    TEST_F(Tst_Grace, testName) { \
        MasterScore* score = readEncoreScore(fileName); \
        ASSERT_NE(score, nullptr) << "Failed to load " << fileName; \
        EXPECT_GT(score->nmeasures(), 0); \
        muse::Ret ret = score->sanityCheck(); \
        EXPECT_TRUE(ret) << "Corrupted: " << ret.text(); \
        delete score; \
    }

// Regression: grace chords wrongly attached to a Segment made beam layout assert. Invariant checked here:
// segment-attached chords are NORMAL and graces live in graceNotes().
TEST_F(Tst_Grace, grace_with_beamed_eighths_no_layout_crash)
{
    MasterScore* score = readEncoreScore("importer_grace_beam.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_grace_beam.enc";
    EXPECT_GT(score->nmeasures(), 0);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Corrupted: " << ret.text();

    bool foundGrace = false;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->elist()) {
                if (!e || !e->isChord()) {
                    continue;
                }
                Chord* c = toChord(e);
                EXPECT_EQ(c->noteType(), NoteType::NORMAL)
                    << "Segment-attached chord must be NORMAL; grace chords belong in graceNotes()";
                for (Chord* gc : c->graceNotes()) {
                    if (gc->noteType() != NoteType::NORMAL) {
                        foundGrace = true;
                    }
                }
            }
        }
    }
    EXPECT_TRUE(foundGrace) << "Grace eighth should be attached as a graceNotes() child";
    delete score;
}

// A dangling group of acciaccatura graces with no principal chord to ornament must survive as small
// cue notes (normal-type chords), not be discarded. Fixture: four such quarter graces.
TEST_F(Tst_Grace, grace1_0x30_dangling_graces_become_cue_notes)
{
    MasterScore* score = readEncoreScore("importer_grace1_0x30_normal_notes.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_GT(score->nmeasures(), 0);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Corrupted: " << ret.text();

    int cueChords = 0;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->elist()) {
                if (e && e->isChord() && toChord(e)->noteType() == NoteType::NORMAL
                    && !toChord(e)->notes().empty() && toChord(e)->notes().front()->isSmall()) {
                    ++cueChords;
                }
            }
        }
    }
    EXPECT_EQ(cueChords, 4)
        << "All four dangling grace notes must survive as small cue chords, not be discarded";
    delete score;
}

// A cue note (grace1 0x20 + grace2 0x01) keeps its full rhythmic value but imports SMALL and MUTED
// (Note::setPlay(false)), in the spare cue voice.
TEST_F(Tst_Grace, cue_note_small_and_muted)
{
    MasterScore* score = readEncoreScore("importer_cue_note.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Corrupted: " << ret.text();

    int noteCount = 0;
    bool smallAndMuted = false;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->elist()) {
                if (!e || !e->isChord()) {
                    continue;
                }
                for (Note* n : toChord(e)->notes()) {
                    ++noteCount;
                    if (n->isSmall() && !n->play()) {
                        smallAndMuted = true;
                    }
                }
            }
        }
    }
    EXPECT_EQ(noteCount, 1) << "the cue note is the only note in the score";
    EXPECT_TRUE(smallAndMuted) << "cue note must be small and muted";
    delete score;
}

// A quarter with an acciaccatura at a tick inside its span (contiguous, no silence between) imports
// the ornament as a grace-AFTER the quarter (a *_AFTER note type), keeping it in the same bar.
TEST_F(Tst_Grace, acciaccatura_after_contiguous_note)
{
    MasterScore* score = readEncoreScore("importer_grace_after_contiguous.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Corrupted: " << ret.text();

    bool foundGraceAfter = false;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->elist()) {
                if (!e || !e->isChord()) {
                    continue;
                }
                for (Chord* gc : toChord(e)->graceNotes()) {
                    const NoteType nt = gc->noteType();
                    if (nt == NoteType::GRACE8_AFTER || nt == NoteType::GRACE16_AFTER
                        || nt == NoteType::GRACE32_AFTER) {
                        foundGraceAfter = true;
                    }
                }
            }
        }
    }
    EXPECT_TRUE(foundGraceAfter)
        << "an acciaccatura contiguous with the preceding note must be a grace-after";
    delete score;
}

// A quarter trailed only by a grace at a dotted-quarter distance must stay a PLAIN quarter: a grace
// has no rhythmic footprint and must not promote the preceding note to a dotted note.
TEST_F(Tst_Grace, trailing_grace_does_not_dot_preceding_note)
{
    MasterScore* score = readEncoreScore("importer_grace_trailing_no_dot.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Corrupted: " << ret.text();

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);
    const Chord* first = nullptr;
    for (Segment* s = m->first(SegmentType::ChordRest); s && !first; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            first = toChord(e);
        }
    }
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first->durationType().type(), DurationType::V_QUARTER)
        << "the quarter must not be inflated by the trailing grace";
    EXPECT_EQ(first->dots(), 0) << "a note trailed only by a grace must not become dotted";
    delete score;
}

// Regression: an articulation byte on a grace note must become an Ornament, not a plain Articulation
// (it was applied before the grace was attached, so ornaments could not be created).
// Fixture: a normal note plus an acciaccatura grace carrying a trill articulation.
TEST_F(Tst_Grace, grace_articulation_becomes_ornament)
{
    MasterScore* score = readEncoreScore("grace_ornament.enc");
    ASSERT_NE(score, nullptr);

    bool foundOrnamentOnGrace = false;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->elist()) {
                if (!e || !e->isChord()) {
                    continue;
                }
                for (Chord* gc : toChord(e)->graceNotes()) {
                    for (Articulation* a : gc->articulations()) {
                        if (a->isOrnament()) {
                            foundOrnamentOnGrace = true;
                        }
                    }
                }
            }
        }
    }
    EXPECT_TRUE(foundOrnamentOnGrace)
        << "trill artic byte on a grace note must become an Ornament, not a plain Articulation";

    delete score;
}

// grace2 bit 0x01 is the Encore per-note MUTE flag (independent of size). grace1 bit 0x20 is small.
// One standalone note per bar: m1 cue muted, m2 cue sounding, m3 normal muted, m4 normal. A small
// note alone in its bar is a cue (small, full value); its play state follows the mute flag.
TEST_F(Tst_Grace, cue_mute_flag_and_sounding_cue)
{
    MasterScore* score = readEncoreScore("importer_cue_mute_flags.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Corrupted: " << ret.text();

    std::vector<std::pair<bool, bool> > perMeasure;   // {isSmall, plays}
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            EngravingItem* e = s->element(0);
            if (e && e->isChord() && !toChord(e)->notes().empty()) {
                const Note* n = toChord(e)->notes().front();
                perMeasure.push_back({ n->isSmall(), n->play() });
                break;
            }
        }
    }
    ASSERT_GE(perMeasure.size(), 4u);
    EXPECT_TRUE(perMeasure[0].first);
    EXPECT_FALSE(perMeasure[0].second);                                       // cue muted: small, silent
    EXPECT_TRUE(perMeasure[1].first);
    EXPECT_TRUE(perMeasure[1].second);                                        // cue sounding: small, plays
    EXPECT_FALSE(perMeasure[2].first);
    EXPECT_FALSE(perMeasure[2].second);                                       // normal muted: full, silent
    EXPECT_FALSE(perMeasure[3].first);
    EXPECT_TRUE(perMeasure[3].second);                                        // normal: full, plays
    delete score;
}

// Covers: grace note filtering (fv>=4 only), ACCIACCATURA
ENC_SANITY_TEST(grace_notes, "notes_grace.enc")
