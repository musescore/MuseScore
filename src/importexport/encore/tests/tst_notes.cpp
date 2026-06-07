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

// Note import: pitch and tick scaling, dotted values, boundary/overflow handling, and the general
// note-element decoding shared across formats. See ENCORE_FORMAT.md §Note element.

#include <gtest/gtest.h>

#include "engraving/dom/arpeggio.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/barline.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/dynamic.h"
#include "engraving/dom/fermata.h"
#include "engraving/dom/fingering.h"
#include "engraving/dom/fret.h"
#include "engraving/dom/hairpin.h"
#include "engraving/dom/harmony.h"
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

class Tst_Notes : public ::testing::Test, public MTest
{
protected:
    void SetUp() override { setRootDir(ENC_DIR); }
};

// Encore ticks (240/quarter) scale to MuseScore (480/quarter), so quarter positions land on 480 multiples.
TEST_F(Tst_Notes, tick_scaling_quarter_positions)
{
    MasterScore* score = readEncoreScore("chord_parsing.enc");
    ASSERT_NE(score, nullptr);
    Measure* m = measureAt(score, 1);
    ASSERT_NE(m, nullptr);
    Fraction mTick = m->tick();

    std::vector<int> relTicks;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChordRest()) {
            relTicks.push_back((s->tick() - mTick).ticks());
        }
    }
    for (int t : relTicks) {
        EXPECT_EQ(t % 480, 0) << "Note at rel tick " << t << " should be on a quarter-note boundary";
    }
    delete score;
}

TEST_F(Tst_Notes, note_pitches_whole_note)
{
    MasterScore* score = readEncoreScore("akordo.enc");
    ASSERT_NE(score, nullptr);
    Chord* foundChord = nullptr;
    for (MeasureBase* mb = score->first(); mb && !foundChord; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        for (Segment* s = m->first(SegmentType::ChordRest); s && !foundChord; s = s->next(SegmentType::ChordRest)) {
            EngravingItem* e = s->element(0);
            if (e && e->isChord()) {
                foundChord = toChord(e);
            }
        }
    }
    ASSERT_NE(foundChord, nullptr) << "Should find at least one chord in akordo.enc";
    EXPECT_GT(foundChord->notes().size(), 0u);
    int pitch = foundChord->notes().front()->pitch();
    EXPECT_GE(pitch, 21) << "MIDI pitch should be in valid piano range";
    EXPECT_LE(pitch, 108) << "MIDI pitch should be in valid piano range";
    delete score;
}

// A rest of 180 Encore ticks must import as a dotted eighth (V_EIGHTH + 1 dot).
TEST_F(Tst_Notes, dotted_quarter_note)
{
    MasterScore* score = readEncoreScore("notes_swing.enc");
    ASSERT_NE(score, nullptr);
    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isRest()) {
            Rest* rest = toRest(e);
            if (rest->durationType().type() == DurationType::V_EIGHTH) {
                EXPECT_EQ(rest->dots(), 1) << "Dotted eighth rest should have 1 dot";
                break;
            }
        }
    }
    delete score;
}

// A note at tick == durTicks belongs to the next measure and must not overflow the current one.
TEST_F(Tst_Notes, boundary_notes_not_in_current_measure)
{
    MasterScore* score = readEncoreScore("chord_parsing.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Chord Parsing should pass sanityCheck: " << ret.text();
    delete score;
}

TEST_F(Tst_Notes, measures_do_not_overflow_4_4)
{
    MasterScore* score = readEncoreScore("chord_parsing.enc");
    ASSERT_NE(score, nullptr);
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        if (m->timesig() != Fraction(4, 4)) {
            continue;
        }
        m->setCorrupted(0, false);  // reset first
    }
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "No 4/4 measure should overflow: " << ret.text();
    delete score;
}

// A note at tick == durTicks has zero real duration and must be skipped, so no chord ends up V_MEASURE.
TEST_F(Tst_Notes, last_note_real_duration_not_zero)
{
    MasterScore* score = readEncoreScore("chord_parsing.enc");
    ASSERT_NE(score, nullptr);
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->elist()) {
                if (!e || !e->isChord()) {
                    continue;
                }
                Chord* chord = toChord(e);
                EXPECT_NE(chord->durationType().type(), DurationType::V_MEASURE)
                    << "No chord should have V_MEASURE type (indicates zero real duration)";
            }
        }
    }
    delete score;
}

// After tick scaling every segment must fall within its measure's tick range (no notes placed outside).
TEST_F(Tst_Notes, tick_scaling_no_note_outside_measure)
{
    MasterScore* score = readEncoreScore("bando.enc");
    ASSERT_NE(score, nullptr);
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        Fraction mStart = m->tick();
        Fraction mEnd = m->endTick();
        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            EXPECT_GE(s->tick(), mStart)
                << "Segment tick should be >= measure start";
            EXPECT_LT(s->tick(), mEnd)
                << "Segment tick should be < measure end";
        }
    }
    delete score;
}

// An invalid faceValue (0 or > 8) must be skipped rather than yielding a garbage duration and crashing.
TEST_F(Tst_Notes, invalid_facevalue_no_crash)
{
    MasterScore* score = readEncoreScore("notes_corrupted.enc");
    ASSERT_NE(score, nullptr) << "Opus 27 should load despite faceValue=0/28 corruption";
    EXPECT_GT(score->nmeasures(), 0);
    delete score;
}

TEST_F(Tst_Notes, invalid_facevalue_notes_have_valid_duration_type)
{
    MasterScore* score = readEncoreScore("notes_corrupted.enc");
    ASSERT_NE(score, nullptr);
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->elist()) {
                if (!e) {
                    continue;
                }
                ChordRest* cr = toChordRest(e);
                DurationType dt = cr->durationType().type();
                EXPECT_NE(dt, DurationType::V_ZERO) << "No chord/rest should have V_ZERO duration";
                EXPECT_NE(dt, DurationType::V_INVALID) << "No chord/rest should have V_INVALID duration";
            }
        }
    }
    delete score;
}

// Notes only a few ticks apart are MIDI timing artifacts (realDuration < 15); dropping them must leave no
// two chords sharing a tick in voice 0.
TEST_F(Tst_Notes, tiny_duration_notes_do_not_create_overlaps)
{
    MasterScore* score = readEncoreScore("notes_swing.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_GT(score->nmeasures(), 0);
    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    std::set<Fraction> seenTicks;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (!e) {
            continue;
        }
        EXPECT_EQ(seenTicks.count(s->tick()), 0u)
            << "Tiny-duration note was not skipped: overlap detected";
        seenTicks.insert(s->tick());
    }
    delete score;
}

// A voice >= VOICES must be skipped, not clamped to voice 3 (which collided with real voice-3 elements);
// no element's track may exceed maxTrack.
TEST_F(Tst_Notes, no_voice_conflict_from_clamping)
{
    MasterScore* score = readEncoreScore("notes_corrupted.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_GT(score->nmeasures(), 0);
    track_idx_t maxTrack = static_cast<track_idx_t>(score->nstaves()) * VOICES;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->elist()) {
                if (e) {
                    EXPECT_LT(e->track(), maxTrack)
                        << "Element track " << e->track() << " should be < maxTrack " << maxTrack;
                }
            }
        }
    }
    delete score;
}

// Encore encodes leading silence via an absolute tick offset, not a REST element; the importer snaps to
// that tick so beat positions are preserved.
TEST_F(Tst_Notes, implicit_leading_rest_keeps_note_positions)
{
    MasterScore* score = readEncoreScore("notes_implicit_leading_rest.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->ticks(), Fraction(3, 4)) << "measure must be 3/4";

    // Voice 0 should produce: quarter rest @ beat 1, quarter note @ beat 2,
    // quarter note @ beat 3. Same order as Encore -- no reordering.
    std::vector<Fraction> ticks;
    std::vector<bool> isRest;
    std::vector<int> pitches;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (!el) {
            continue;
        }
        ticks.push_back(s->tick() - m->tick());
        if (el->isRest()) {
            isRest.push_back(true);
            pitches.push_back(-1);
        } else if (el->isChord()) {
            isRest.push_back(false);
            pitches.push_back(toChord(el)->upNote()->pitch());
        }
    }
    ASSERT_EQ(ticks.size(), 3u) << "voice 0 must contain rest + 2 notes";
    EXPECT_TRUE(isRest[0]) << "beat 1 must be a rest, not a note";
    EXPECT_EQ(ticks[0], Fraction(0, 1));
    EXPECT_FALSE(isRest[1]);
    EXPECT_EQ(ticks[1], Fraction(1, 4));
    EXPECT_EQ(pitches[1], 72);
    EXPECT_FALSE(isRest[2]);
    EXPECT_EQ(ticks[2], Fraction(2, 4));
    EXPECT_EQ(pitches[2], 74);
    delete score;
}

// ===========================================================================
// FIX: calculateRealDurations inflates rdur to gap-to-measure-end for isolated notes (e.g. 720 in 3/4).
// The importer rejects dotted-half promotion when rdur exceeds face's tick count AND isn't a real dotted multiple.
// ===========================================================================
TEST_F(Tst_Notes, inflated_rdur_keeps_face_value_quarter_chord)
{
    MasterScore* score = readEncoreScore("notes_inflated_rdur_quarter_chord.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    // Voice 1 (MuseScore track 1 of staff 0) holds the encVoice=1 chord.
    Chord* chord = nullptr;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(1);
        if (el && el->isChord()) {
            chord = toChord(el);
            break;
        }
    }
    ASSERT_NE(chord, nullptr) << "encVoice=1 chord must land on MuseScore voice 1";
    EXPECT_EQ(chord->durationType().type(), DurationType::V_QUARTER)
        << "Face=3 (quarter) must win over rdur=720 (inflated to dotted-half ratio)";
    EXPECT_EQ(chord->durationType().dots(), 0)
        << "rdur=720 is not a real dotted multiple of quarter; no dot";
    EXPECT_EQ(chord->notes().size(), 2u)
        << "the two NOTE elements at tick 0 are merged into one chord";
    delete score;
}

// An eighth chord whose next note kept its original quarter position has an inflated rdur (gap to next);
// the face value (eighth) must win over that rdur so the chord imports as an eighth, not a quarter.
TEST_F(Tst_Notes, inflated_rdur_eighth_chord_keeps_face_value)
{
    MasterScore* score = readEncoreScore("notes_chord_inflated_rdur_keeps_eighth.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    Chord* chordAtQ1 = nullptr;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (el && el->isChord()) {
            Chord* c = toChord(el);
            if (c->tick() == m->tick() + Fraction(1, 4)) {
                chordAtQ1 = c;
                break;
            }
        }
    }
    ASSERT_NE(chordAtQ1, nullptr) << "Chord at beat 2 (tick=1/4) not found";
    EXPECT_EQ(chordAtQ1->durationType().type(), DurationType::V_EIGHTH)
        << "fv=4 (eighth) must win over rdur=240 (inflated by trailing gap); chord must be eighth";
    EXPECT_EQ(chordAtQ1->notes().size(), 2u)
        << "G4 and A4 at same tick must merge into one chord";
    delete score;
}

// A triplet-spaced rdur (e.g. 80) must not promote past the face value: a 16th stays a 16th, not an eighth.
TEST_F(Tst_Notes, note_rdur_80_stays_16th_face_value)
{
    MasterScore* score = readEncoreScore("notes_rdur_80_stays_16th.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    std::vector<DurationType> types;
    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (el && el->isChord()) {
            types.push_back(toChord(el)->durationType().type());
        }
    }
    ASSERT_GE(types.size(), 2u);
    EXPECT_EQ(types[0], DurationType::V_16TH)
        << "First note had rdur=80 (was V_EIGHTH under the old triplet table); must stay 16th";
    EXPECT_EQ(types[1], DurationType::V_16TH);
    delete score;
}

// Off-beat MIDI ticks (a note 1 tick late) must be placed by cumulative face value, not the raw MIDI tick,
// so notes land at canonical positions without spurious gap fills.
TEST_F(Tst_Notes, offbeat_notes_canonical_placement)
{
    MasterScore* score = readEncoreScore("notes_offbeat_canonical.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Off-beat MIDI ticks should produce clean measure via cumTick: "
                     << ret.text();
    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    std::vector<Fraction> noteTicks;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            noteTicks.push_back(s->tick() - m->tick());
        }
    }
    ASSERT_EQ(noteTicks.size(), 2u) << "Should have exactly 2 quarter notes";
    EXPECT_EQ(noteTicks[0], Fraction(0, 1)) << "First note at tick 0";
    EXPECT_EQ(noteTicks[1], Fraction(1, 4)) << "Second note at tick 1/4 (canonical, not MIDI-offset)";
    delete score;
}

// On a PERC staff, note lines come from the Encore position byte and noteheads from the faceValue high
// nibble, so distinct positions map to distinct lines instead of all collapsing to line 0.
TEST_F(Tst_Notes, perc_clef_note_positions_from_encore_position_byte)
{
    MasterScore* score = readEncoreScore("notes_perc_clef_positions.enc");
    ASSERT_NE(score, nullptr);
    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    std::vector<Note*> notes;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            Chord* c = toChord(e);
            if (!c->notes().empty()) {
                notes.push_back(c->notes().front());
            }
        }
    }
    ASSERT_GE(notes.size(), 3u);

    EXPECT_EQ(notes[0]->pitch(), 62);
    EXPECT_EQ(notes[1]->pitch(), 65);
    EXPECT_EQ(notes[2]->pitch(), 81);

    // Line is derived from the Encore position byte as line = max(-4, 10 - position).
    EXPECT_EQ(notes[0]->line(),  9) << "pitch 62 position=1 must be at line 9";
    EXPECT_EQ(notes[1]->line(),  7) << "pitch 65 position=3 must be at line 7";
    EXPECT_EQ(notes[2]->line(), -2) << "pitch 81 position=12 must be at line -2";

    // Verify drumset registration: the visual head is determined by the drumset entry.
    // note->headGroup() is a user-override property (stays HEAD_NORMAL unless the user
    // explicitly changes it); the rendering path uses drumset->noteHead(pitch) directly.
    const Drumset* ds = notes[0]->part()->instrument()->drumset();
    ASSERT_NE(ds, nullptr) << "Staff must have a drumset assigned (PERC clef)";

    EXPECT_EQ(ds->noteHead(81), NoteHeadGroup::HEAD_XCIRCLE)
        << "fv high nibble=5 must register HEAD_XCIRCLE in drumset";

    EXPECT_EQ(ds->noteHead(62), NoteHeadGroup::HEAD_NORMAL)
        << "fv high nibble=0 must register HEAD_NORMAL in drumset";
    EXPECT_EQ(ds->noteHead(65), NoteHeadGroup::HEAD_NORMAL)
        << "fv high nibble=0 must register HEAD_NORMAL in drumset";

    delete score;
}

// The faceValue notehead must override the standard drumset's pre-registered head: a normal-head note on
// pitch 40 (which the standard drumset registers as HEAD_SLASH) must become HEAD_NORMAL.
TEST_F(Tst_Notes, perc_clef_facevalue_overrides_standard_drumset_notehead)
{
    MasterScore* score = readEncoreScore("notes_perc_clef_standard_drumset_notehead.enc");
    ASSERT_NE(score, nullptr);
    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    Note* note = nullptr;
    for (Segment* s = m->first(SegmentType::ChordRest); s && !note; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            Chord* c = toChord(e);
            if (!c->notes().empty()) {
                note = c->notes().front();
            }
        }
    }
    ASSERT_NE(note, nullptr);
    ASSERT_EQ(note->pitch(), 40);

    const Drumset* ds = note->part()->instrument()->drumset();
    ASSERT_NE(ds, nullptr) << "Staff must have a drumset assigned (PERC clef)";
    EXPECT_EQ(ds->noteHead(40), NoteHeadGroup::HEAD_NORMAL)
        << "faceValue normal must override HEAD_SLASH from standard drumset (pitch 40)";

    delete score;
}

// All 10 faceValue high-nibble notehead types map to the correct NoteHeadGroup; non-zero nibbles set
// note->setFixed(true) so layout does not override the head from the shared drumset entry.
TEST_F(Tst_Notes, perc_notehead_all_nibble_types)
{
    MasterScore* score = readEncoreScore("notes_perc_notehead_all_nibbles.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    // Notes are spread across 3 measures (4 per measure), iterate all.
    std::vector<Note*> notes;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            EngravingItem* e = s->element(0);
            if (e && e->isChord()) {
                for (Note* n : toChord(e)->notes()) {
                    notes.push_back(n);
                }
            }
        }
    }
    ASSERT_EQ(notes.size(), 10u) << "Expected 10 notes (one per nibble 0-9) across 3 measures";

    using G = NoteHeadGroup;
    const std::vector<G> expected = {
        G::HEAD_NORMAL,        // nibble 0
        G::HEAD_DIAMOND,       // nibble 1
        G::HEAD_TRIANGLE_UP,   // nibble 2
        G::HEAD_CUSTOM,        // nibble 3 (square)
        G::HEAD_CROSS,         // nibble 4
        G::HEAD_XCIRCLE,       // nibble 5
        G::HEAD_PLUS,          // nibble 6
        G::HEAD_SLASH,         // nibble 7
        G::HEAD_LARGE_DIAMOND, // nibble 8
        G::HEAD_NORMAL,        // nibble 9 (invisible, head=NORMAL)
    };

    for (size_t i = 0; i < notes.size(); ++i) {
        EXPECT_EQ(notes[i]->headGroup(), expected[i])
            << "nibble " << i << " (pitch " << notes[i]->pitch() << ")";
        // nibble=9: note must be invisible (sin_cabeza)
        if (i == 9) {
            EXPECT_FALSE(notes[i]->visible()) << "nibble 9 must be invisible (sin_cabeza)";
        }
    }

    delete score;
}

// Two PERC notes at the same pitch but different faceValue nibbles must each keep their own notehead;
// they cannot both defer to the single shared drumset entry.
TEST_F(Tst_Notes, perc_shared_pitch_two_nibbles_stay_fixed)
{
    MasterScore* score = readEncoreScore("notes_perc_shared_pitch_nibbles.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    std::vector<Note*> notes;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            EngravingItem* e = s->element(0);
            if (e && e->isChord()) {
                for (Note* n : toChord(e)->notes()) {
                    notes.push_back(n);
                }
            }
        }
    }
    ASSERT_GE(notes.size(), 2u);
    EXPECT_EQ(notes[0]->headGroup(), NoteHeadGroup::HEAD_SLASH)
        << "nibble=7 at pitch=60 must keep HEAD_SLASH even when pitch is shared";
    EXPECT_EQ(notes[1]->headGroup(), NoteHeadGroup::HEAD_LARGE_DIAMOND)
        << "nibble=8 at pitch=60 must keep HEAD_LARGE_DIAMOND even when pitch is shared";
    delete score;
}

// Notes a few ticks apart (MIDI drift) must merge into one chord: the near-simultaneous cluster is skipped
// in the rdur calc so the first note is not dropped by the short-rdur filter.
TEST_F(Tst_Notes, near_simultaneous_notes_form_chord)
{
    MasterScore* score = readEncoreScore("notes_v0c2_near_simultaneous_chord.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Near-simultaneous chord should produce clean 2/4: " << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(2, 4));

    Chord* first = nullptr;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            first = toChord(e);
            break;
        }
    }
    ASSERT_NE(first, nullptr) << "Must have at least one chord";
    EXPECT_EQ(first->notes().size(), 2u)
        << "Near-simultaneous notes (ticks 0 and 3) must form a 2-note chord";
    delete score;
}

// A triple-dotted eighth advances by 15/64, not 14/64, so the following chord starts flush with no overrun.
TEST_F(Tst_Notes, triple_dotted_advance_matches_chord_ticks)
{
    MasterScore* score = readEncoreScore("notes_triple_dotted_advance.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Triple-dotted advance must equal chord ticks: " << ret.text();
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
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first->ticks(), Fraction(15, 64)) << "Must be triple-dotted 8th (15/64)";
    EXPECT_EQ(first->dots(), 3) << "Must have 3 augmentation dots";
    Chord* second = nullptr;
    for (Segment* s = first->segment()->next(SegmentType::ChordRest);
         s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChord()) {
            second = toChord(e);
            break;
        }
    }
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(second->segment()->tick() - m->tick(), Fraction(15, 64))
        << "Second chord must start at 15/64 (triple-dotted advance), not 14/64";
    delete score;
}

// The dotControl byte (not the MIDI-drifted realDuration) must decide a note's dot count: a dotted eighth
// stays dotted even when the next note's start makes rdur look undotted.
TEST_F(Tst_Notes, dotted_note_uses_dotcontrol_byte)
{
    MasterScore* score = readEncoreScore("notes_dotted_note.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Dotted note test must produce clean score: " << ret.text();

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
    EXPECT_EQ(first->durationType().type(), DurationType::V_EIGHTH)
        << "First note must be an 8th (dotted)";
    EXPECT_EQ(first->dots(), 1)
        << "dotControl=180=8th*3/2 must produce 1 augmentation dot on the note";
    delete score;
}

// Same dotControl rule for rests: a dotted eighth rest stays dotted despite a MIDI-drifted rdur.
TEST_F(Tst_Notes, dotted_rest_uses_dotcontrol_byte)
{
    MasterScore* score = readEncoreScore("notes_dotted_rest.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Dotted rest test must produce clean score: " << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(3, 4));

    Rest* dottedRest = nullptr;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isRest()) {
            dottedRest = toRest(e);
            break;
        }
    }
    ASSERT_NE(dottedRest, nullptr) << "Must find a rest after the quarter note";
    EXPECT_EQ(dottedRest->durationType().type(), DurationType::V_EIGHTH)
        << "Rest must be an eighth rest (dotted)";
    EXPECT_EQ(dottedRest->dots(), 1)
        << "dotControl=180=8th*3/2 must produce 1 augmentation dot";
    delete score;
}

// With dotControl=0, an rdur 1 tick off a double-dotted value must snap to the dotted count (tolerance 1).
TEST_F(Tst_Notes, rdur_snap_corrects_dot_count)
{
    MasterScore* score = readEncoreScore("notes_rdur_snap.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "rdur snap test must produce clean score: " << ret.text();

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
    EXPECT_EQ(first->durationType().type(), DurationType::V_EIGHTH)
        << "Note must be an 8th (double-dotted)";
    EXPECT_EQ(first->dots(), 2)
        << "rdur=211 is 1 tick away from dd8th=210: must snap to 2 augmentation dots";
    delete score;
}

// A 32nd rest whose rdur is shortened by the next note's MIDI start must be kept in order: for face value
// >= 32nd the face value is trusted over the short rdur, so the rest is not dropped or reordered.
TEST_F(Tst_Notes, rest_before_note_midi_slop_keeps_rest)
{
    MasterScore* score = readEncoreScore("notes_rest_before_note_midi_slop.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "score must be clean: " << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    std::vector<ChordRest*> crs;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* e = s->element(0);
        if (e && e->isChordRest()) {
            crs.push_back(toChordRest(e));
        }
    }
    ASSERT_GE(crs.size(), 6u) << "Must have 6 ChordRest elements in M1";
    EXPECT_TRUE(crs[0]->isChord())
        << "First element must be a chord (eighth note)";
    EXPECT_EQ(crs[0]->durationType().type(), DurationType::V_EIGHTH);
    EXPECT_TRUE(crs[1]->isRest())
        << "Second element must be a rest (32nd); without fix it appears last";
    EXPECT_EQ(crs[1]->durationType().type(), DurationType::V_32ND)
        << "Rest must be a 32nd (face value preserved despite rdur=5)";
    EXPECT_TRUE(crs[2]->isChord())
        << "Third element must be a chord (dotted 16th)";
    EXPECT_EQ(crs[2]->durationType().type(), DurationType::V_16TH);
    delete score;
}

// A short-rdur note that is not a chord extension must still be filtered as a MIDI artifact; the chord-ext
// test must use the previous element's tick so a lone 64th (rdur=11) is not mistaken for a chord tone.
TEST_F(Tst_Notes, rdur_non_chord_ext_filtered)
{
    MasterScore* score = readEncoreScore("notes_rdur_non_chord_ext_filtered.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "score must be clean: " << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    std::vector<Chord*> chords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (el && el->isChord()) {
            chords.push_back(toChord(el));
        }
    }

    ASSERT_EQ(chords.size(), 1u)
        << "Only the Q E4 must be placed; the 64th C4 artifact must be filtered";
    EXPECT_EQ(chords[0]->durationType().type(), DurationType::V_QUARTER)
        << "Placed note must be a quarter";
    ASSERT_GE(chords[0]->notes().size(), 1u);
    EXPECT_EQ(chords[0]->notes()[0]->pitch(), 64)
        << "Placed note must be E4 (pitch=64), not the filtered C4 (pitch=60)";

    delete score;
}

// ===========================================================================
// BUG FIX: grace1 low-nibble cascade filter for MIDI artifact continuation notes
// ===========================================================================

TEST_F(Tst_Notes, grace1_cascade_filter)
{
    // 64th C4 (g1low=1) filtered as artifact; Q C4 (g1low=2) is its tie-receiver and must also be filtered.
    // Fix: when g1low=1 note is filtered, record its pitch; next note with g1low=2 and same pitch is cascade-filtered.
    MasterScore* score = readEncoreScore("notes_grace1_cascade_filter.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "score must be clean: " << ret.text();

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    std::vector<Chord*> chords;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (el && el->isChord()) {
            chords.push_back(toChord(el));
        }
    }

    ASSERT_EQ(chords.size(), 1u)
        << "Both C4 notes must be filtered; only Q E4 must appear";
    ASSERT_GE(chords[0]->notes().size(), 1u);
    EXPECT_EQ(chords[0]->notes()[0]->pitch(), 64)
        << "Only E4 (pitch=64) must appear; C4 (pitch=60) must be cascade-filtered";

    delete score;
}

// A pitch encoded twice in the same chord cluster must collapse to one notehead, regardless of the
// grace1 0x40 chord-extension bit.
TEST_F(Tst_Notes, duplicate_pitch_in_chord_cluster_suppressed)
{
    MasterScore* score = readEncoreScore("notes_chord_duplicate.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    Segment* firstSeg = m->first(SegmentType::ChordRest);
    ASSERT_NE(firstSeg, nullptr);
    EngravingItem* elem = firstSeg->element(0);
    ASSERT_NE(elem, nullptr) << "Expected a chord at tick=0";
    ASSERT_TRUE(elem->isChord()) << "Expected a Chord, got something else";

    Chord* chord = toChord(elem);
    EXPECT_EQ(chord->notes().size(), 1u)
        << "Duplicate pitch (tick=0, pitch=60 encoded twice) must produce exactly one notehead";

    delete score;
}

// Duplicate suppression must also fire when neither copy carries the chord-extension bit (some v0xC2 files).
TEST_F(Tst_Notes, duplicate_pitch_no_ext_bit_suppressed)
{
    MasterScore* score = readEncoreScore("notes_chord_duplicate_no_ext_bit.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);
    Segment* firstSeg = m->first(SegmentType::ChordRest);
    ASSERT_NE(firstSeg, nullptr);
    EngravingItem* elem = firstSeg->element(0);
    ASSERT_NE(elem, nullptr) << "Expected a chord at tick=0";
    ASSERT_TRUE(elem->isChord());

    EXPECT_EQ(toChord(elem)->notes().size(), 1u)
        << "Duplicate pitch (both grace1=0x00, no ext bit) must produce exactly one notehead";

    delete score;
}

// Grand-staff files encode the target staff in the high bits of the raw staff byte (staffWithin), so
// voices marked staffWithin=1 must land on staff 2 while staffWithin=0 stays on staff 1.
TEST_F(Tst_Notes, grandstaff_staffwithin_routes_voices_to_correct_staff)
{
    MasterScore* score = readEncoreScore("notes_grandstaff_bit6_second_staff.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    ASSERT_EQ(score->nstaves(), 2) << "Piano grand staff must have 2 staves";

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    auto notesOnStaff = [&](int staffIdx) {
        std::vector<int> pitches;
        for (Segment* seg = m->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
            for (int v = 0; v < static_cast<int>(VOICES); ++v) {
                EngravingItem* e = seg->element(static_cast<track_idx_t>(staffIdx * VOICES + v));
                if (e && e->isChord()) {
                    for (Note* n : toChord(e)->notes()) {
                        pitches.push_back(n->pitch());
                    }
                }
            }
        }
        return pitches;
    };

    auto s1 = notesOnStaff(0);
    auto s2 = notesOnStaff(1);

    EXPECT_EQ(s1.size(), 2u) << "Treble staff must have 2 notes";
    EXPECT_EQ(s2.size(), 2u) << "Bass staff must have 2 notes (bit6 routing broken)";

    for (int p : s1) {
        EXPECT_GE(p, 60) << "Treble staff note must be middle C or above";
    }
    for (int p : s2) {
        EXPECT_LT(p, 60) << "Bass staff note must be below middle C";
    }

    delete score;
}

// A REST with staffWithin=1 must land on staff 2, like notes do.
TEST_F(Tst_Notes, grandstaff_staffwithin_rest_on_second_staff)
{
    MasterScore* score = readEncoreScore("notes_grandstaff_staffwithin_rest_on_second_staff.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "sanity check failed";
    ASSERT_EQ(score->nstaves(), 2);

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    bool hasRestOnStaff2 = false;
    for (Segment* seg = m->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
        for (int v = 0; v < static_cast<int>(VOICES); ++v) {
            EngravingItem* e = seg->element(static_cast<track_idx_t>(1 * VOICES + v));
            if (e && e->isRest()) {
                hasRestOnStaff2 = true;
            }
        }
    }
    EXPECT_TRUE(hasRestOnStaff2) << "Rest with staffWithin=1 must land on staff 2";

    bool hasNoteOnStaff1 = false;
    for (Segment* seg = m->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
        EngravingItem* e = seg->element(0);
        if (e && e->isChord()) {
            hasNoteOnStaff1 = true;
        }
    }
    EXPECT_TRUE(hasNoteOnStaff1) << "Treble note (staffWithin=0) must stay on staff 1";

    delete score;
}

// A TIE with staffWithin=1 must resolve on staff 2 (with its note), not leave a dangling tie on staff 1.
TEST_F(Tst_Notes, grandstaff_staffwithin_tie_on_second_staff)
{
    // Single 4/4 measure: treble C5 half+half, bass E3 half tied to E3 half.
    // Both bass notes use voice=2, staffWithin=1 (raw staff byte = 0x40).
    // The TIE element also carries staffWithin=1; the tieStartSet routing must
    // key the tie by the ROUTED (staffIdx=1, voice=0) rather than the raw values
    // to correctly link the two E3 chords on staff 2.
    MasterScore* score = readEncoreScore("notes_grandstaff_staffwithin_tie_on_second_staff.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "sanity check failed";
    ASSERT_EQ(score->nstaves(), 2);

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    Note* tieStart = nullptr;
    for (Segment* seg = m->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
        for (int v = 0; v < static_cast<int>(VOICES); ++v) {
            EngravingItem* e = seg->element(static_cast<track_idx_t>(1 * VOICES + v));
            if (e && e->isChord()) {
                for (Note* n : toChord(e)->notes()) {
                    if (n->pitch() == 52 && !tieStart) {
                        tieStart = n;
                    }
                }
            }
        }
    }
    ASSERT_NE(tieStart, nullptr) << "Bass E3 (pitch=52) must be on staff 2";
    EXPECT_NE(tieStart->tieFor(), nullptr)
        << "First E3 on staff 2 must carry tie-for (staffWithin tie routing broken)";

    if (tieStart->tieFor()) {
        Note* tieEnd = tieStart->tieFor()->endNote();
        EXPECT_NE(tieEnd, nullptr) << "Tie must resolve to second E3 on staff 2";
        if (tieEnd) {
            EXPECT_EQ(tieEnd->pitch(), 52) << "Tie end note must also be E3 (pitch=52)";
        }
    }

    delete score;
}

// All four Encore voices distribute correctly across the grand staff: voices 0-1 to the treble staff,
// voices 2-3 to the bass staff.
TEST_F(Tst_Notes, grandstaff_staffwithin_four_voices)
{
    MasterScore* score = readEncoreScore("notes_grandstaff_staffwithin_four_voices.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "sanity check failed";
    ASSERT_EQ(score->nstaves(), 2);

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    auto pitchesOnStaff = [&](int staffIdx) {
        std::vector<int> pitches;
        for (Segment* seg = m->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
            for (int v = 0; v < static_cast<int>(VOICES); ++v) {
                EngravingItem* e = seg->element(static_cast<track_idx_t>(staffIdx * VOICES + v));
                if (e && e->isChord()) {
                    for (Note* n : toChord(e)->notes()) {
                        pitches.push_back(n->pitch());
                    }
                }
            }
        }
        return pitches;
    };

    auto s1 = pitchesOnStaff(0);
    auto s2 = pitchesOnStaff(1);

    EXPECT_EQ(s1.size(), 2u) << "Treble must have 2 notes (C5, E5)";
    EXPECT_EQ(s2.size(), 2u) << "Bass must have 2 notes (G3, B3)";

    for (int p : s1) {
        EXPECT_GE(p, 72) << "Treble note must be >= 72 (C5)";
    }
    for (int p : s2) {
        EXPECT_LT(p, 60) << "Bass note must be < 60 (middle C)";
    }

    delete score;
}

// A voice number above the staff-2 marker (voice 5..7, staffWithin 0) is a genuine extra voice on
// its OWN staff, not a request to move to the next staff. On a grand-staff instrument the old
// voice>=VOICES rule pushed a voice-7 top-staff note onto the bass staff. Fixture: a voice-7 note
// (raw_staff = top staff) on a Piano grand staff must land on staff 0, not staff 1.
TEST_F(Tst_Notes, grandstaff_high_voice_stays_on_own_staff)
{
    MasterScore* score = readEncoreScore("notes_grandstaff_high_voice_own_staff.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "sanity check failed";

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);
    bool onTopStaff = false, onBassStaff = false;
    for (Segment* seg = m->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
        for (int tr = 0; tr < 2 * static_cast<int>(VOICES); ++tr) {
            EngravingItem* e = seg->element(static_cast<track_idx_t>(tr));
            if (e && e->isChord() && !toChord(e)->notes().empty()
                && toChord(e)->notes().front()->pitch() == 67) {
                (tr < static_cast<int>(VOICES) ? onTopStaff : onBassStaff) = true;
            }
        }
    }
    EXPECT_TRUE(onTopStaff) << "the voice-7 note must stay on its own (top) staff";
    EXPECT_FALSE(onBassStaff) << "the voice-7 note must not be pushed onto the bass staff";
    delete score;
}

// On a single-staff instrument, Encore voice nibble 4 is a genuine second melodic voice (not the
// grand-staff silent-voice marker), so it must import as a separate voice 1 rather than being
// concatenated onto voice 0 (which produced overfull, non-dyadic bars that failed to open).
TEST_F(Tst_Notes, singlestaff_voice4_second_voice)
{
    MasterScore* score = readEncoreScore("notes_singlestaff_voice4_second_voice.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "sanity check failed";

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    auto countChordsInVoice = [&](int voice) {
        int count = 0;
        for (Segment* seg = m->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
            EngravingItem* e = seg->element(static_cast<track_idx_t>(voice));
            if (e && e->isChord()) {
                ++count;
            }
        }
        return count;
    };

    EXPECT_EQ(countChordsInVoice(0), 4) << "voice 0 must hold only its own four notes";
    EXPECT_EQ(countChordsInVoice(1), 4) << "the voice-4 second voice must import as a separate voice 1";
    delete score;
}

// ===========================================================================
// grandstaff_staffwithin_sequential
//
// Sequential notes on both staves use independent cumTick accumulators.
// Each staff advances its own position; notes must not bleed across staves.
// Fixture: treble C5 at tick=0, E5 at tick=240; bass C3 at tick=0, E3 at 240.
// Each staff must have exactly 2 notes placed at the correct ticks.
// ===========================================================================
TEST_F(Tst_Notes, grandstaff_staffwithin_sequential)
{
    MasterScore* score = readEncoreScore("notes_grandstaff_staffwithin_sequential.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "sanity check failed";
    ASSERT_EQ(score->nstaves(), 2);

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    auto countNotesOnStaff = [&](int staffIdx) {
        int count = 0;
        for (Segment* seg = m->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
            for (int v = 0; v < static_cast<int>(VOICES); ++v) {
                EngravingItem* e = seg->element(static_cast<track_idx_t>(staffIdx * VOICES + v));
                if (e && e->isChord()) {
                    count += static_cast<int>(toChord(e)->notes().size());
                }
            }
        }
        return count;
    };

    EXPECT_EQ(countNotesOnStaff(0), 2) << "Treble must have 2 notes";
    EXPECT_EQ(countNotesOnStaff(1), 2) << "Bass must have 2 notes";

    delete score;
}

// A transposing instrument's written spelling must use the concert key, so a written F4 does not drift to
// a double-flat (Gbb) spelling.
TEST_F(Tst_Notes, transposing_instrument_written_tpc_not_double_flat)
{
    MasterScore* score = readEncoreScore("notes_transposing_written_tpc.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);
    Segment* seg = m->first(SegmentType::ChordRest);
    ASSERT_NE(seg, nullptr);
    EngravingItem* e = seg->element(0);
    ASSERT_NE(e, nullptr);
    ASSERT_TRUE(e->isChord());
    Note* note = toChord(e)->upNote();
    ASSERT_NE(note, nullptr);

    EXPECT_EQ(note->pitch(), 71)
        << "Concert pitch must be 65 (written F4) + 6 = 71 (B4)";
    EXPECT_EQ(note->tpc2(), 13)
        << "Written TPC must be 13 (F natural), not 1 (Gbb) or 7 (Cb); "
        "double-flat spellings indicate the wrong key context in computeWindow";

    delete score;
}

// score->spell() can drift a whole transposing-staff melody to double-flats when the written key is flat
// but the concert key is sharp; respellTransposingStaves re-derives TPCs from sounding pitch + concert key
// so no note ends up double-flat. The single-note fix does not catch this, hence the melody fixture.
TEST_F(Tst_Notes, transposing_melody_no_double_flat_after_spell)
{
    MasterScore* score = readEncoreScore("notes_transposing_respell_melody.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck());

    const std::vector<int> expectedTpc1 = { 18, 19, 22, 18 };   // E B G# E (concert)
    const std::vector<int> expectedTpc2 = { 12, 13, 16, 12 };   // Bb F D Bb (written)

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);
    size_t i = 0;
    for (Segment* seg = m->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
        EngravingItem* e = seg->element(0);
        if (!e || !e->isChord()) {
            continue;
        }
        Note* note = toChord(e)->upNote();
        ASSERT_NE(note, nullptr);
        ASSERT_LT(i, expectedTpc1.size());
        EXPECT_EQ(note->tpc1(), expectedTpc1[i])
            << "Concert TPC at note " << i << " must follow the A-major concert key, not a flat drift";
        EXPECT_EQ(note->tpc2(), expectedTpc2[i])
            << "Written TPC at note " << i << " must not be a double-flat (spell() drift)";
        ++i;
    }
    EXPECT_EQ(i, expectedTpc1.size()) << "Expected 4 melody notes on the transposing staff";

    delete score;
}

// scale_no_anchor_produces_no_circles
TEST_F(Tst_Notes, scale_no_anchor_produces_no_circles)
{
    // Notes with opt bit 0 and pos in 0-7 but NO au=0x39..0x40 anchor → no circles.
    MasterScore* score = readEncoreScore("notes_scale_no_anchor_no_circles.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "sanity check failed";

    int fingerCount = 0;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* seg = toMeasure(mb)->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
            for (size_t v = 0; v < VOICES; ++v) {
                EngravingItem* el = seg->element(static_cast<track_idx_t>(v));
                if (el && el->isChord()) {
                    for (Note* n : toChord(el)->notes()) {
                        for (EngravingItem* sub : n->el()) {
                            if (sub && sub->isFingering()) {
                                ++fingerCount;
                            }
                        }
                    }
                }
            }
        }
    }
    EXPECT_EQ(fingerCount, 0)
        << "Without 0x39..0x40 anchor bytes, options-bit-0 notes must not show circles";

    delete score;
}

// ===========================================================================
// REGRESSION: Standalone string-number ORN (0xE6 = string 2) must NOT duplicate
// the string number that the per-note hasScaleStringAnchors options-bit-0 path
// already placed on the same note.
// Fixture: n1 artUp=0x39 (string 1, sets anchor); ORN 0xE6 at tick=240 (string 2)
// + n2 with options bit 0 and position=1. Without the dedup guard in the resolver,
// n2 would get TWO "2" string numbers.
// ===========================================================================
TEST_F(Tst_Notes, string_num_orn_does_not_duplicate_anchor_path_number)
{
    MasterScore* score = readEncoreScore("notes_string_num_orn_no_dup.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck());

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    std::map<int, std::vector<int> > numsByBeat;  // beat_index → list of string numbers
    int beat = 0;
    for (Segment* seg = m->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
        EngravingItem* el = seg->element(0);
        if (!el || !el->isChord()) {
            continue;
        }
        for (Note* n : toChord(el)->notes()) {
            for (EngravingItem* sub : n->el()) {
                if (sub && sub->isFingering()
                    && toFingering(sub)->textStyleType() == TextStyleType::STRING_NUMBER) {
                    bool ok;
                    int v = toFingering(sub)->plainText().toInt(&ok);
                    if (ok) {
                        numsByBeat[beat].push_back(v);
                    }
                }
            }
        }
        ++beat;
    }
    EXPECT_EQ(numsByBeat[0].size(), 1u) << "n1 must have exactly one string number (1)";
    EXPECT_EQ(numsByBeat[1].size(), 1u) << "n2 must have exactly one string number (2), not two";
    if (!numsByBeat[0].empty()) {
        EXPECT_EQ(numsByBeat[0][0], 1);
    }
    if (!numsByBeat[1].empty()) {
        EXPECT_EQ(numsByBeat[1][0], 2);
    }

    delete score;
}

// voice_overflow_notes_dropped_not_routed_to_voice2
TEST_F(Tst_Notes, voice_overflow_notes_dropped_not_routed_to_voice2)
{
    MasterScore* score = readEncoreScore("notes_voice_overflow_dropped.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "Overflow must not corrupt";

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    // Voice 0 should have exactly 2 half notes
    int v0Chords = 0;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (el && el->isChord()) {
            ++v0Chords;
        }
    }
    EXPECT_EQ(v0Chords, 2) << "Only notes 1-2 fit; notes 3-5 must be dropped";

    // Voice 1 must be empty (overflow notes are not routed to voice 2)
    int v1Chords = 0;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(1);
        if (el && el->isChord()) {
            ++v1Chords;
        }
    }
    EXPECT_EQ(v1Chords, 0) << "Overflow notes must be dropped, NOT routed to voice 2";

    delete score;
}

// Compact rawStaff encodes staffWithin in the high bits and the instrument index in the low bits; the low
// bits must not be read as a LINE slot, or the second instrument's notes land on the first's staves.
TEST_F(Tst_Notes, notes_multiinstr_compact_routing)
{
    MasterScore* score = readEncoreScore("notes_multiinstr_compact_routing.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_EQ(score->nstaves(), 4) << "score must have 4 staves (2 instruments x 2 each)";

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    Segment* seg = m->first(SegmentType::ChordRest);
    ASSERT_NE(seg, nullptr);

    auto pitchOnStaff = [&](int staffIdx) -> int {
        for (int v = 0; v < static_cast<int>(VOICES); ++v) {
            EngravingItem* e = seg->element(static_cast<track_idx_t>(staffIdx * VOICES + v));
            if (e && e->isChord()) {
                return toChord(e)->notes().front()->pitch();
            }
        }
        return -1;
    };

    EXPECT_EQ(pitchOnStaff(0), 60) << "staff 0 (instr 0 treble) must have C4";
    EXPECT_EQ(pitchOnStaff(1), 48) << "staff 1 (instr 0 bass) must have C3";
    EXPECT_EQ(pitchOnStaff(2), 64) << "staff 2 (instr 1 treble) must have E4";
    EXPECT_EQ(pitchOnStaff(3), 52) << "staff 3 (instr 1 bass) must have E3";

    delete score;
}

TEST_F(Tst_Notes, notes_v0c2_multiinstr_compact_routing)
{
    // v0xC2 counterpart: same compact rawStaff encoding in an older file format.
    // Verifies the lineSlotByRawByte lookup in emitters.cpp works for v0xC2 (size=22 notes).
    MasterScore* score = readEncoreScore("notes_v0c2_multiinstr_compact_routing.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_EQ(score->nstaves(), 4);

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);
    Segment* seg = m->first(SegmentType::ChordRest);
    ASSERT_NE(seg, nullptr);

    auto pitchOnStaff = [&](int staffIdx) -> int {
        for (int v = 0; v < static_cast<int>(VOICES); ++v) {
            EngravingItem* e = seg->element(static_cast<track_idx_t>(staffIdx * VOICES + v));
            if (e && e->isChord()) {
                return toChord(e)->notes().front()->pitch();
            }
        }
        return -1;
    };

    EXPECT_EQ(pitchOnStaff(0), 60) << "staff 0 (instr 0 treble) must have C4";
    EXPECT_EQ(pitchOnStaff(1), 48) << "staff 1 (instr 0 bass) must have C3";
    EXPECT_EQ(pitchOnStaff(2), 64) << "staff 2 (instr 1 treble) must have E4";
    EXPECT_EQ(pitchOnStaff(3), 52) << "staff 3 (instr 1 bass) must have E3";

    delete score;
}

// In some v0xC2 size=24 notes the pitch is already in semiTonePitch (tuplet==0); the pitch-swap must be
// skipped so it is preserved.
TEST_F(Tst_Notes, notes_v0c2_size24_semitone_pitch)
{
    MasterScore* score = readEncoreScore("notes_v0c2_size24_semitonepitch.enc");
    ASSERT_NE(score, nullptr);

    Measure* m = measureAt(score, 0);
    ASSERT_NE(m, nullptr);

    std::vector<int> pitches;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (!el || !el->isChord()) {
            continue;
        }
        pitches.push_back(toChord(el)->notes().front()->pitch());
    }

    ASSERT_EQ(pitches.size(), 2u);
    EXPECT_EQ(pitches[0], 60) << "C4 (60): pitch from semiTonePitch must survive; swap must not fire when tuplet==0";
    EXPECT_EQ(pitches[1], 64) << "E4 (64): pitch from semiTonePitch must survive; swap must not fire when tuplet==0";

    delete score;
}

// When a non-first measure has explicit notes filling only part of the
// duration, the trailing empty space must be filled with invisible gap rests,
// not visible rests. Measure 0 is fully filled (to avoid pickup shortening).
// Measure 1 has two eighth notes (cumTick=1/4); trailing 3/4 must be invisible.
TEST_F(Tst_Notes, trailing_space_uses_invisible_gap_rests)
{
    MasterScore* score = readEncoreScore("notes_implicit_trailing_gap.enc");
    ASSERT_NE(score, nullptr);

    Measure* m1 = measureAt(score, 1);   // measure 1 has partial content
    ASSERT_NE(m1, nullptr);

    int visibleRests = 0;
    int gapRests = 0;
    for (Segment* s = m1->first(SegmentType::ChordRest); s;
         s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (el && el->isRest()) {
            if (toRest(el)->isGap()) {
                ++gapRests;
            } else {
                ++visibleRests;
            }
        }
    }

    EXPECT_EQ(visibleRests, 0) << "Trailing silence must use invisible gap rests, not visible rests";
    EXPECT_GT(gapRests, 0) << "Must have at least one invisible gap rest for the trailing 3/4";

    delete score;
}

// A 16th note with rdur=112 must not become triple-dotted: 112 is the integer-truncated triple-dot
// threshold (true value 112.5), which used to misalign the rest of the measure.
TEST_F(Tst_Notes, rdur112_16th_note_not_triple_dotted)
{
    MasterScore* score = readEncoreScore("notes_16th_rdur112_no_triple_dot.enc");
    ASSERT_NE(score, nullptr);

    Measure* m0 = measureAt(score, 0);
    ASSERT_NE(m0, nullptr);

    Segment* firstSeg = m0->first(SegmentType::ChordRest);
    ASSERT_NE(firstSeg, nullptr);
    EngravingItem* el = firstSeg->element(0);
    ASSERT_NE(el, nullptr);
    ASSERT_TRUE(el->isChord());

    Chord* first = toChord(el);
    EXPECT_EQ(first->durationType(), DurationType::V_16TH)
        << "First chord must be a plain 16th note";
    EXPECT_EQ(first->dots(), 0)
        << "rdur=112 for a 16th must not be interpreted as triple-dotted";

    delete score;
}

// Two explicit RESTs at the same tick (voices routing to the same MuseScore voice) must not both advance
// cumTick: the second is a duplicate at an already-filled position, or subsequent notes shift by an eighth.
TEST_F(Tst_Notes, dual_explicit_rests_same_tick_no_cumtick_drift)
{
    MasterScore* score = readEncoreScore("notes_dual_rests_same_tick_routing.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);
    const Fraction measTick = m->tick();

    std::vector<Fraction> chordTicks;
    int restCount = 0;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (!el) {
            continue;
        }
        if (el->isChord()) {
            chordTicks.push_back(s->tick() - measTick);
        } else if (el->isRest()) {
            ++restCount;
        }
    }

    ASSERT_EQ(static_cast<int>(chordTicks.size()), 2)
        << "Expected exactly two chords: D3+F#3 at start, B2+D3 at half-measure";

    EXPECT_EQ(chordTicks[0].ticks(), 0)
        << "First chord (D3+F#3) must be at the start of the measure";

    EXPECT_EQ(chordTicks[1].ticks(), 960)
        << "Second chord (B2+D3) must be at half-measure (MuseScore tick 960); "
        "the two duplicate rests at enc tick=120 must not shift it to tick 720";

    // At least one rest must appear (for the enc tick=120 eighth rest).
    // A gap-fill rest may also appear between the explicit rest and the quarter
    // note, so we only assert the minimum; the key invariant is the chord tick.
    EXPECT_GE(restCount, 1)
        << "At least one rest must appear for the enc tick=120 explicit rest";

    Segment* seg2 = m->findSegment(SegmentType::ChordRest, measTick + Fraction(960, 1920));
    if (seg2) {
        EngravingItem* el2 = seg2->element(0);
        if (el2 && el2->isChord()) {
            std::set<int> pitches;
            for (Note* n : toChord(el2)->notes()) {
                pitches.insert(n->pitch());
            }
            EXPECT_TRUE(pitches.count(47)) << "B2 (midi=47) must be in the second chord";
            EXPECT_TRUE(pitches.count(50)) << "D3 (midi=50) must be in the second chord";
        }
    }

    delete score;
}

// A v0xC2 note whose dotControl bit 0 is coincidentally set but whose realDuration exactly matches the
// plain face value must not be dotted; the bit-0 fallback must not fire and overflow the measure.
TEST_F(Tst_Notes, v0c2_plain_sixteenth_with_spurious_dotctrl_bit0_no_dot)
{
    MasterScore* score = readEncoreScore("notes_v0c2_plain_sixteenth_no_spurious_dot.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Measure must pass sanityCheck: " << ret.text();

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
        << "5 chords expected (2x16th + 3x8th); fewer means overflow truncated a note";
    EXPECT_EQ(chords[0]->durationType().type(), DurationType::V_16TH);
    EXPECT_EQ(chords[0]->dots(), 0) << "16th with dotControl=0x39 must NOT be dotted";
    EXPECT_EQ(chords[1]->durationType().type(), DurationType::V_16TH);
    EXPECT_EQ(chords[1]->dots(), 0) << "16th with dotControl=0x39 must NOT be dotted";
    EXPECT_EQ(chords[2]->durationType().type(), DurationType::V_EIGHTH);
    EXPECT_EQ(chords[3]->durationType().type(), DurationType::V_EIGHTH);
    EXPECT_EQ(chords[4]->durationType().type(), DurationType::V_EIGHTH);
    delete score;
}
TEST_F(Tst_Notes, bazo)
{
    MasterScore* score = readEncoreScore("bazo.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_GT(score->nmeasures(), 0);
    delete score;
}

TEST_F(Tst_Notes, akordo)
{
    MasterScore* score = readEncoreScore("akordo.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_GT(score->nmeasures(), 0);
    delete score;
}

TEST_F(Tst_Notes, ripetoj)
{
    MasterScore* score = readEncoreScore("ripetoj.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_GT(score->nmeasures(), 0);
    delete score;
}

TEST_F(Tst_Notes, opeco_vochoj)
{
    MasterScore* score = readEncoreScore("opeco_vochoj.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_GT(score->nmeasures(), 0);
    delete score;
}

TEST_F(Tst_Notes, bando)
{
    MasterScore* score = readEncoreScore("bando.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_GT(score->nmeasures(), 0);
    delete score;
}

TEST_F(Tst_Notes, kordorkestro)
{
    MasterScore* score = readEncoreScore("kordorkestro.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_GT(score->nmeasures(), 0);
    delete score;
}

TEST_F(Tst_Notes, chord_parsing)
{
    MasterScore* score = readEncoreScore("chord_parsing.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_GT(score->nmeasures(), 0);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Score has corrupted measures: " << ret.text();
    delete score;
}

TEST_F(Tst_Notes, encore_symbols)
{
    MasterScore* score = readEncoreScore("encore_symbols.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_GT(score->nmeasures(), 0);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Score has corrupted measures: " << ret.text();
    delete score;
}

// No sanityCheck: swing timing files produce slight measure shortfalls (by design).
TEST_F(Tst_Notes, swing_timing)
{
    MasterScore* score = readEncoreScore("notes_swing.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_GT(score->nmeasures(), 0);
    delete score;
}

TEST_F(Tst_Notes, multiple_voices_loaded)
{
    MasterScore* score = readEncoreScore("opeco_vochoj.enc");
    ASSERT_NE(score, nullptr);
    bool foundVoice1 = false;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        for (Segment* s = m->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            if (s->element(1) && s->element(1)->isChordRest()) {
                foundVoice1 = true;
                break;
            }
        }
        if (foundVoice1) {
            break;
        }
    }
    EXPECT_TRUE(foundVoice1) << "opeco_vochoj.enc should have notes in voice 2";
    delete score;
}

// Regression: both notes and rests updated prevMidiTick; a note at the same tick as a rest was mis-detected
// as chord extension, replacing the rest's segment while cumTick was already advanced past it.
TEST_F(Tst_Notes, rest_does_not_anchor_chord_extension)
{
    MasterScore* score = readEncoreScore("importer_rest_not_chord_anchor.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Corrupted: " << ret.text();
    delete score;
}

// Regression: rest with open tuplet skipped first cap; tuplet closed; second cap shortened cumTick advance
// but left rest ticks at uncapped face value. cr->actualTicks() exceeded cumTick advance, overrunning the measure.
TEST_F(Tst_Notes, rest_caps_its_ticks_when_advance_is_capped)
{
    MasterScore* score = readEncoreScore("importer_rest_caps_in_open_tuplet.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Corrupted: " << ret.text();
    delete score;
}

// Regression: duplicate tt.placedTicks += advance in rest path double-counted the rest's contribution,
// masking the undershoot and skipping closeTuplet shrink. checkMeasure reported "Incomplete measure".
TEST_F(Tst_Notes, rest_in_tuplet_does_not_double_count_placed_ticks)
{
    MasterScore* score = readEncoreScore("importer_rest_in_tuplet.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_rest_in_tuplet.enc";
    EXPECT_GT(score->nmeasures(), 0);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Corrupted: " << ret.text();
    delete score;
}

// A dotted rest must be recognized via the realDuration snap fallback (matching the note handler); passing
// the dotControl bitmask as a tick count yielded 0 dots, turning a dotted-quarter rest into a plain one.
TEST_F(Tst_Notes, v0c4_dotted_rest_correct_duration)
{
    MasterScore* score = readEncoreScore("rest_dotted_before_notes.enc");
    ASSERT_NE(score, nullptr) << "Failed to load rest_dotted_before_notes.enc";
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Dotted rest must import without measure corruption: " << ret.text();

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(7, 8));

    Segment* first = m->first(SegmentType::ChordRest);
    ASSERT_NE(first, nullptr);
    EngravingItem* el = first->element(0);
    ASSERT_NE(el, nullptr);
    ASSERT_TRUE(el->isRest()) << "First element must be a rest";
    Rest* rest = toRest(el);
    EXPECT_EQ(rest->durationType().type(), DurationType::V_QUARTER) << "Base type: quarter";
    EXPECT_EQ(rest->dots(), 1) << "Must have 1 dot (dotted-quarter rest)";
    EXPECT_EQ(rest->ticks(), Fraction(3, 8)) << "Dotted-quarter rest spans 3/8";
    delete score;
}

// When MIDI drift puts rdur more than 1 tick off the dotted value, the dotControl bit-0 flag must still
// mark the note dotted (overriding calcDotsSnap's 0).
TEST_F(Tst_Notes, v0c4_dotted_note_dotctrl_bit0_drift)
{
    MasterScore* score = readEncoreScore("notes_dotted_ctrl_bit0_drift.enc");
    ASSERT_NE(score, nullptr) << "Failed to load notes_dotted_ctrl_bit0_drift.enc";
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "dotControl bit-0 dotted note with rdur drift must not corrupt: "
                     << ret.text();

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->timesig(), Fraction(2, 4));

    Segment* first = m->first(SegmentType::ChordRest);
    ASSERT_NE(first, nullptr);
    EngravingItem* el = first->element(0);
    ASSERT_NE(el, nullptr);
    ASSERT_TRUE(el->isChord()) << "First element must be a chord, not a rest";
    Chord* c = toChord(el);
    EXPECT_EQ(c->durationType().type(), DurationType::V_EIGHTH)
        << "First note base type: eighth";
    EXPECT_EQ(c->dots(), 1)
        << "dotControl bit 0 forces 1 dot when calcDotsSnap misses due to rdur drift";
    delete score;
}

// Regression: options byte bit 0 and position field must NOT produce string numbers on plain
// piano/vocal notes.
TEST_F(Tst_Notes, v0c4_no_spurious_string_numbers_from_options_byte)
{
    MasterScore* score = readEncoreScore("notes_no_spurious_string_numbers.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "sanity check failed";

    int fingerCount = 0;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (size_t v = 0; v < score->nstaves() * VOICES; ++v) {
                EngravingItem* el = s->element(static_cast<track_idx_t>(v));
                if (el && el->isChord()) {
                    for (Note* n : toChord(el)->notes()) {
                        for (EngravingItem* sub : n->el()) {
                            if (sub && sub->isFingering()) {
                                ++fingerCount;
                            }
                        }
                    }
                }
            }
        }
    }
    EXPECT_EQ(fingerCount, 0)
        << "Piano/grand-staff notes must not get circled string numbers.";

    delete score;
}

// Regression: gap snap used denominator 4*beatTicks (correct only for x/4). x/8 beatTicks=120 gave
// half the correct whole-note value, overflowing by one beat.
TEST_F(Tst_Notes, v0c4_gap_snap_eighth_meter)
{
    MasterScore* score = readEncoreScore("importer_gap_snap_eighth_meter.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_gap_snap_eighth_meter.enc";
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Corrupted: " << ret.text();

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    std::vector<Fraction> ticks;
    std::vector<bool> isRest;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (!el) {
            continue;
        }
        ticks.push_back(s->tick() - m->tick());
        isRest.push_back(el->isRest());
    }
    ASSERT_GE(ticks.size(), 3u);
    EXPECT_EQ(ticks[0], Fraction(0, 1));
    EXPECT_TRUE(isRest[0]) << "beat 1 must be a rest";
    EXPECT_EQ(ticks[1], Fraction(1, 8));
    EXPECT_TRUE(isRest[1]) << "beat 2 must be a rest";
    EXPECT_EQ(ticks[2], Fraction(2, 8));
    EXPECT_FALSE(isRest[2]) << "beat 3 must carry the chord";
    delete score;
}

// Regression: gap snap + inflated-rdur guard with Key=-12 (Octave Lower). Voice 0 has implicit
// leading silence (tick offsets, no REST); voice 1 has trailing silence; Key shifts all pitches by -12.
TEST_F(Tst_Notes, v0c4_octave_lower_implicit_silences)
{
    MasterScore* score = readEncoreScore("structure_octave_lower_implicit_silences.enc");
    ASSERT_NE(score, nullptr) << "Failed to load structure_octave_lower_implicit_silences.enc";
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Corrupted: " << ret.text();

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    const track_idx_t baseTrack = 0;

    std::vector<Fraction> v0Ticks;
    std::vector<bool> v0IsRest;
    std::vector<int> v0Pitches;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(baseTrack);
        if (!el) {
            continue;
        }
        v0Ticks.push_back(s->tick() - m->tick());
        if (el->isRest()) {
            v0IsRest.push_back(true);
            v0Pitches.push_back(-1);
        } else if (el->isChord()) {
            v0IsRest.push_back(false);
            v0Pitches.push_back(toChord(el)->upNote()->pitch());
        }
    }
    ASSERT_EQ(v0Ticks.size(), 3u) << "voice 0 must contain rest + 2 notes";
    EXPECT_TRUE(v0IsRest[0]) << "beat 1 must be a rest, not a note";
    EXPECT_EQ(v0Ticks[0], Fraction(0, 1));
    EXPECT_FALSE(v0IsRest[1]);
    EXPECT_EQ(v0Ticks[1], Fraction(1, 4));
    EXPECT_EQ(v0Pitches[1], 73 - 12);
    EXPECT_FALSE(v0IsRest[2]);
    EXPECT_EQ(v0Ticks[2], Fraction(2, 4));
    EXPECT_EQ(v0Pitches[2], 74 - 12);

    Chord* v1Chord = nullptr;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(baseTrack + 1);
        if (el && el->isChord()) {
            v1Chord = toChord(el);
            break;
        }
    }
    ASSERT_NE(v1Chord, nullptr) << "encVoice=1 chord must land on MuseScore voice 2";
    EXPECT_EQ(v1Chord->durationType().type(), DurationType::V_QUARTER)
        << "face=quarter must win over rdur inflated to dotted-half ratio (720)";
    EXPECT_EQ(v1Chord->durationType().dots(), 0);
    ASSERT_EQ(v1Chord->notes().size(), 2u);
    std::set<int> pitches{ v1Chord->notes()[0]->pitch(), v1Chord->notes()[1]->pitch() };
    EXPECT_EQ(pitches, (std::set<int> { 64 - 12, 73 - 12 }));
    delete score;
}

// Regression: single-switch voice overflow placed a note into a full voice (e.g. half rest).
TEST_F(Tst_Notes, multi_stream_switch_skips_voice_filled_by_rest)
{
    MasterScore* score = readEncoreScore("importer_full_voice_skipped.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_full_voice_skipped.enc";
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Corrupted: " << ret.text();
    delete score;
}

// Regression: gap snap fired on drift ticks, producing a zero-length rhythmic gap that aborted
// populateRhythmicList.
TEST_F(Tst_Notes, v0c2_multi_stream_drift_imports_cleanly)
{
    MasterScore* score = readEncoreScore("importer_v0c2_multi_stream_drift.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_v0c2_multi_stream_drift.enc";
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "Corrupted: " << ret.text();
    delete score;
}

// Regression guard: 2/2 with the correct beatTicks=480 still imports all notes after wholeTicks became the
// constant 960 (was beatTicks*timeSigDen).
TEST_F(Tst_Notes, v0c4_2_2_beatticks480_correct_encoding_still_works)
{
    MasterScore* score = readEncoreScore("importer_2_2_beatticks480_correct.enc");
    ASSERT_NE(score, nullptr);

    Measure* m0 = score->firstMeasure();
    ASSERT_NE(m0, nullptr);

    std::vector<std::pair<Fraction, bool> > elements;
    for (Segment* s = m0->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (!el) {
            continue;
        }
        bool isGap = el->isRest() && toRest(el)->isGap();
        if (!isGap) {
            elements.emplace_back(s->tick() - m0->tick(), el->isRest());
        }
    }

    ASSERT_EQ(elements.size(), 7u)
        << "Correct 2/2 (beatTicks=480) must also produce 7 elements";
    EXPECT_EQ(elements[2].first, Fraction(3, 8))
        << "Third element must be at 3/8, no regression from wholeTicks fix";
    EXPECT_FALSE(elements[2].second) << "Third element must be a chord, not a rest";

    delete score;
}

// With a non-standard beatTicks=240 in 2/2, gap-snap must use the constant 960 ticks/whole so it does not
// fire at the wrong positions and drop notes.
TEST_F(Tst_Notes, v0c4_2_2_beatticks240_gap_snap_no_false_fire)
{
    MasterScore* score = readEncoreScore("importer_2_2_beatticks240_gap_snap.enc");
    ASSERT_NE(score, nullptr);

    Measure* m0 = score->firstMeasure();
    ASSERT_NE(m0, nullptr);

    std::vector<std::pair<Fraction, bool> > elements;
    for (Segment* s = m0->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (!el) {
            continue;
        }
        bool isGap = el->isRest() && toRest(el)->isGap();
        if (!isGap) {
            elements.emplace_back(s->tick() - m0->tick(), el->isRest());
        }
    }

    ASSERT_EQ(elements.size(), 7u)
        << "Measure must contain exactly 7 elements (rest + Q + 5 eighths); "
        "gap-snap with wrong wholeTicks drops notes 3-7";

    EXPECT_EQ(elements[0].first, Fraction(0, 1));
    EXPECT_TRUE(elements[0].second) << "element 0 must be a rest";

    EXPECT_EQ(elements[1].first, Fraction(1, 8));
    EXPECT_FALSE(elements[1].second) << "element 1 must be a chord (Q)";

    EXPECT_EQ(elements[2].first, Fraction(3, 8))
        << "element 2 must be at 3/8 (tick 360); "
        "false gap-snap would place it at 3/4 (tick 720)";
    EXPECT_FALSE(elements[2].second) << "element 2 must be a chord, not a rest";

    EXPECT_EQ(elements[3].first, Fraction(4, 8));
    EXPECT_EQ(elements[4].first, Fraction(5, 8));
    EXPECT_EQ(elements[5].first, Fraction(6, 8));
    EXPECT_EQ(elements[6].first, Fraction(7, 8));
    for (int i = 3; i <= 6; ++i) {
        EXPECT_FALSE(elements[i].second) << "element " << i << " must be a chord";
    }

    delete score;
}

#ifndef ENC_SANITY_TEST_NOTES
#define ENC_SANITY_TEST_NOTES(testName, fileName) \
    TEST_F(Tst_Notes, testName) { \
        MasterScore* score = readEncoreScore(fileName); \
        ASSERT_NE(score, nullptr) << "Failed to load " << fileName; \
        EXPECT_GT(score->nmeasures(), 0); \
        muse::Ret ret = score->sanityCheck(); \
        EXPECT_TRUE(ret) << "Corrupted: " << ret.text(); \
        delete score; \
    }
#endif

// Covers: tuplet=0xFF (degenerate), faceValue=0 (invalid), voice>=4, open SLURSTART
ENC_SANITY_TEST_NOTES(corrupted_elements,         "notes_corrupted.enc")

// Covers: explicit 3:2 triplets, 3/4 time sig, multi-measure
ENC_SANITY_TEST_NOTES(explicit_triplets_3_4,      "notes_triplets.enc")
