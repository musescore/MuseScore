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

// v0xA6 (Encore 2.x) importer coverage: the format's distinct element offsets, absolute pitch, tuplet byte,
// duplicate-rest dedupe, octave-key clef compensation, grace time-borrowing, and compact lyric/text/stafftext.
// See ENCORE_FORMAT.md §v0xA6 note (size 10, on-disk slot 20).

#include <gtest/gtest.h>

#include <QByteArray>
#include <QDataStream>

#include "../internal/parser/elem.h"

#include "engraving/dom/chord.h"
#include "engraving/dom/clef.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/stafftext.h"
#include "engraving/dom/tremolosinglechord.h"
#include "engraving/dom/tuplet.h"

#include "testbase.h"

static const QString ENC_DIR(QString(iex_encore_tests_DATA_ROOT) + "/data/");

using namespace mu::engraving;

class Tst_ImporterV0xa6 : public ::testing::Test, public MTest
{
protected:
    void SetUp() override
    {
        setRootDir(ENC_DIR);
    }
};

TEST_F(Tst_ImporterV0xa6, very_old_format_v0xa6_sanity_check)
{
    // Regression: the v0xA6 element offset differs from v0xC4; a wrong offset drops all notes silently.
    MasterScore* score = readEncoreScore("structure_v0xa6_basic.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_EQ(score->nmeasures(), 2);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "v0xA6 synthetic file should pass sanityCheck: " << ret.text();
    delete score;
}

TEST_F(Tst_ImporterV0xa6, very_old_format_v0xa6_pitch_encoding)
{
    // v0xA6 pitch is an absolute MIDI value, decoded differently from v0xC4.
    MasterScore* score = readEncoreScore("structure_v0xa6_basic.enc");
    ASSERT_NE(score, nullptr);

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    std::vector<int> pitches;
    for (Segment* s = m->first(SegmentType::ChordRest); s;
         s = s->next(SegmentType::ChordRest)) {
        for (EngravingItem* e : s->elist()) {
            if (e && e->isChord()) {
                for (Note* n : toChord(e)->notes()) {
                    pitches.push_back(n->pitch());
                }
            }
        }
    }
    ASSERT_EQ(pitches.size(), 4u) << "Measure 1 should have 4 notes";
    EXPECT_EQ(pitches[0], 60) << "C4";
    EXPECT_EQ(pitches[1], 62) << "D4";
    EXPECT_EQ(pitches[2], 64) << "E4";
    EXPECT_EQ(pitches[3], 67) << "G4";
    delete score;
}

// End-to-end regression for the full v0xA6 fix chain: 4 instruments, key transposition, tuplets,
// duplicate REST collapse, and no spurious articulation/tremolo/fingering glyphs.
TEST_F(Tst_ImporterV0xa6, v0xa6_boda_like_full_pipeline)
{
    MasterScore* score = readEncoreScore("importer_v0xa6_boda_like.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_v0xa6_boda_like.enc";

    ASSERT_EQ(score->nstaves(), 4u) << "fixture has 4 instruments";

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    auto staffPitches = [m](int staffIdx) {
        std::vector<int> out;
        const track_idx_t base = staffIdx * VOICES;
        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            EngravingItem* el = s->element(base);
            if (el && el->isChord()) {
                for (Note* n : toChord(el)->notes()) {
                    out.push_back(n->pitch());
                }
            }
        }
        return out;
    };
    auto staffElementCount = [m](int staffIdx) {
        int count = 0;
        const track_idx_t base = staffIdx * VOICES;
        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            if (s->element(base)) {
                ++count;
            }
        }
        return count;
    };
    auto staffTupletGroups = [m](int staffIdx) {
        std::set<const Tuplet*> seen;
        const track_idx_t base = staffIdx * VOICES;
        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            EngravingItem* el = s->element(base);
            if (el && el->isChord()) {
                if (const Tuplet* t = toChord(el)->tuplet()) {
                    seen.insert(t);
                }
            }
        }
        return seen.size();
    };

    EXPECT_EQ(staffTupletGroups(0), 2u) << "B1 must hold 2 triplet groups";
    EXPECT_EQ(staffPitches(0), (std::vector<int> { 88, 88, 89, 88, 86, 88, 86 }))
        << "B1 pitches survive without Key shift";

    EXPECT_EQ(staffElementCount(1), 3) << "B2 must hold rest + 2 chords";
    EXPECT_EQ(staffPitches(1), (std::vector<int> { 76, 77 }))
        << "B2 pitches survive without Key shift";

    EXPECT_EQ(staffElementCount(2), 3)
        << "Laud must hold exactly rest + 2 chords after duplicate-REST dedupe";
    EXPECT_EQ(staffPitches(2), (std::vector<int> { 76 - 12, 77 - 12 }))
        << "Laud pitches must drop by Key = -12";

    EXPECT_EQ(staffElementCount(3), 3) << "Bajo holds 3 notes";
    EXPECT_EQ(staffPitches(3), (std::vector<int> { 57 - 12, 60 - 12, 64 - 12 }))
        << "Bajo pitches must drop by Key = -12";

    int v1Count = 0;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        for (size_t st = 0; st < 4; ++st) {
            if (s->element(st * VOICES + 1)) {
                ++v1Count;
            }
        }
    }
    EXPECT_EQ(v1Count, 0) << "no v1 spillover after dedup + correct tuplet handling";

    int tremCount = 0;
    int fingerCount = 0;
    int fermataCount = 0;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* ann : s->annotations()) {
                if (ann && ann->isFermata()) {
                    ++fermataCount;
                }
            }
            for (size_t v = 0; v < score->nstaves() * VOICES; ++v) {
                EngravingItem* el = s->element(v);
                if (el && el->isChord()) {
                    Chord* c = toChord(el);
                    if (c->tremoloSingleChord() || c->tremoloTwoChord()) {
                        ++tremCount;
                    }
                    for (Note* nt : c->notes()) {
                        for (EngravingItem* sub : nt->el()) {
                            if (sub && sub->isFingering()) {
                                ++fingerCount;
                            }
                        }
                    }
                }
            }
        }
    }
    EXPECT_EQ(tremCount, 0) << "no spurious tremolo glyphs";
    EXPECT_EQ(fingerCount, 0) << "no spurious fingering glyphs";
    EXPECT_EQ(fermataCount, 0) << "no spurious fermata glyphs";

    delete score;
}

// Regression: v0xA6 NOTE stores the tuplet byte at offset +7 (not +13 as in v0xC4).
TEST_F(Tst_ImporterV0xa6, v0xa6_triplet_byte_at_offset_7)
{
    MasterScore* score = readEncoreScore("importer_v0xa6_triplet_byte_at_offset_7.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_v0xa6_triplet_byte_at_offset_7.enc";

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    int tupletCount = 0;
    int noteCount = 0;
    std::vector<int> pitches;
    std::set<const Tuplet*> seenTuplets;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (el && el->isChord()) {
            Chord* c = toChord(el);
            ++noteCount;
            for (Note* nt : c->notes()) {
                pitches.push_back(nt->pitch());
            }
            const Tuplet* t = c->tuplet();
            if (t && seenTuplets.insert(t).second) {
                ++tupletCount;
                EXPECT_EQ(t->ratio().numerator(), 3) << "triplet actualNotes";
                EXPECT_EQ(t->ratio().denominator(), 2) << "triplet normalNotes";
            }
        }
    }
    EXPECT_EQ(noteCount, 6) << "6 triplet sixteenths must survive import";
    EXPECT_EQ(tupletCount, 2) << "two 3:2 triplet groups expected";
    const std::vector<int> expected{ 64, 65, 64, 62, 64, 62 };
    EXPECT_EQ(pitches, expected) << "pitches must match the binary in order";

    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EXPECT_EQ(s->element(1), nullptr) << "voice 1 should stay empty";
    }
    delete score;
}

// Regression: a size-11 v0xA6 NOTE carries an articulation byte (0x20 = fermata); the reader must still
// take the pitch from the v0xA6 slot (not the v0xC4 base read) and emit the fermata.
TEST_F(Tst_ImporterV0xa6, v0xa6_note_size11_fermata_pitch_and_glyph)
{
    MasterScore* score = readEncoreScore("structure_v0xa6_fermata.enc");
    ASSERT_NE(score, nullptr) << "Failed to load structure_v0xa6_fermata.enc";

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    std::vector<int> pitches;
    int fermataCount = 0;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        for (EngravingItem* ann : s->annotations()) {
            if (ann && ann->isFermata()) {
                ++fermataCount;
            }
        }
        EngravingItem* el = s->element(0);
        if (el && el->isChord()) {
            for (Note* nt : toChord(el)->notes()) {
                pitches.push_back(nt->pitch());
            }
        }
    }
    const std::vector<int> expected{ 64, 67 };
    EXPECT_EQ(pitches, expected) << "size-11 NOTE pitch comes from +11, not the +15 decoy";
    EXPECT_EQ(fermataCount, 2) << "the +18 articulation byte 0x20 must import as a fermata";

    delete score;
}

// Regression: v0xA6 can store two byte-identical REST elements at the same tick; importer must deduplicate.
TEST_F(Tst_ImporterV0xa6, v0xa6_duplicate_rest_collapse)
{
    MasterScore* score = readEncoreScore("importer_v0xa6_duplicate_rest_collapse.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_v0xa6_duplicate_rest_collapse.enc";

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    std::vector<std::pair<Fraction, bool> > positions;
    std::vector<int> pitches;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (!el) {
            continue;
        }
        positions.emplace_back(s->tick() - m->tick(), el->isRest());
        if (el->isChord()) {
            pitches.push_back(toChord(el)->notes()[0]->pitch());
        }
    }
    ASSERT_EQ(positions.size(), 3u) << "measure must hold 3 elements after dedupe";
    EXPECT_TRUE(positions[0].second) << "beat 1: rest";
    EXPECT_EQ(positions[0].first, Fraction(0, 1));
    EXPECT_FALSE(positions[1].second) << "beat 2: chord";
    EXPECT_EQ(positions[1].first, Fraction(1, 8));
    EXPECT_FALSE(positions[2].second) << "beat 3: chord";
    EXPECT_EQ(positions[2].first, Fraction(2, 8));
    ASSERT_EQ(pitches.size(), 2u);
    EXPECT_EQ(pitches[0], 64);
    EXPECT_EQ(pitches[1], 64);

    int v1Count = 0;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        if (s->element(1)) {
            ++v1Count;
        }
    }
    EXPECT_EQ(v1Count, 0) << "voice 1 must be empty after dedupe";
    delete score;
}

// Regression: v0xA6 header ends at 0xA6 (174 bytes), not 0xC2 (194).
TEST_F(Tst_ImporterV0xa6, v0xa6_header_ends_at_0xa6)
{
    MasterScore* score = readEncoreScore("importer_v0xa6_header_ends_at_0xa6.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_v0xa6_header_ends_at_0xa6.enc";

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);
    Chord* firstChord = nullptr;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (el && el->isChord()) {
            firstChord = toChord(el);
            break;
        }
    }
    ASSERT_NE(firstChord, nullptr);
    ASSERT_EQ(firstChord->notes().size(), 1u);
    EXPECT_EQ(firstChord->notes()[0]->pitch(), 48)
        << "TK00 at the v0xA6 file offset 0xA6 must be parsed so its "
        "Key = -12 actually lowers C4 (60) to C3 (48)";
    delete score;
}

// Regression: Key byte at TK+42 (not PRG_BASE+n*PRG_STEP) in v0xA6 format.
TEST_F(Tst_ImporterV0xa6, v0xa6_key_transposition_octave_lower)
{
    MasterScore* score = readEncoreScore("importer_v0xa6_key_transposition.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_v0xa6_key_transposition.enc";

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);
    Chord* firstChord = nullptr;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (el && el->isChord()) {
            firstChord = toChord(el);
            break;
        }
    }
    ASSERT_NE(firstChord, nullptr);
    ASSERT_EQ(firstChord->notes().size(), 1u);
    EXPECT_EQ(firstChord->notes()[0]->pitch(), 48)
        << "v0xA6 Key = -12 must lower C4 (60) to C3 (48)";
    delete score;
}

// Regression: an octave key shift must add a matching octave-decorated clef so the display stays at the
// written octave; v0xA6 has no LINE clef data, so the compensation was missing (Key=-12 -> G8vb).
TEST_F(Tst_ImporterV0xa6, v0xa6_octave_key_adds_compensating_clef)
{
    MasterScore* score = readEncoreScore("importer_v0xa6_key_transposition.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_v0xa6_key_transposition.enc";
    ASSERT_FALSE(score->staves().empty());
    EXPECT_EQ(score->staff(0)->clef(Fraction(0, 1)), ClefType::G8_VB)
        << "v0xA6 octave Key=-12 must add a compensating G8vb clef, not leave the plain template clef";
    delete score;
}

// Regression: v0xA6 NOTE is 10 bytes but EncNote::read consumed 27, reading garbage as articulation data.
TEST_F(Tst_ImporterV0xa6, v0xa6_no_spurious_articulation_glyphs)
{
    MasterScore* score = readEncoreScore("importer_v0xa6_no_spurious_tremolo.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_v0xa6_no_spurious_tremolo.enc";

    int tremCount = 0;
    int fingerCount = 0;
    int articCount = 0;
    int fermataCount = 0;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* ann : s->annotations()) {
                if (ann && ann->isFermata()) {
                    ++fermataCount;
                }
            }
            for (size_t v = 0; v < score->nstaves() * VOICES; ++v) {
                EngravingItem* el = s->element(v);
                if (el && el->isChord()) {
                    Chord* c = toChord(el);
                    if (c->tremoloSingleChord() || c->tremoloTwoChord()) {
                        ++tremCount;
                    }
                    articCount += static_cast<int>(c->articulations().size());
                    for (Note* n : c->notes()) {
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
    EXPECT_EQ(tremCount, 0) << "v0xA6 NOTEs do not carry tremolo data";
    EXPECT_EQ(fingerCount, 0) << "v0xA6 NOTEs do not carry fingering or open-string data";
    EXPECT_EQ(articCount, 0) << "v0xA6 NOTEs do not carry articulation glyphs";
    EXPECT_EQ(fermataCount, 0) << "v0xA6 NOTEs do not carry fermata data";
    delete score;
}

// Regression: after a grace note, regular notes at exact face-grid ticks triggered spurious gap snap.
TEST_F(Tst_ImporterV0xa6, v0xa6_grace_ongrid_snap_suppressed)
{
    MasterScore* score = readEncoreScore("importer_v0xa6_grace_ongrid_snap_suppressed.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_v0xa6_grace_ongrid_snap_suppressed.enc";

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "sanityCheck failed: " << ret.text();

    Measure* m1 = score->firstMeasure();
    ASSERT_NE(m1, nullptr);

    bool hasSpuriousInterNoteRest = false;
    bool prevWasChord = false;
    int graceCount = 0;
    for (Segment* s = m1->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (!el) {
            continue;
        }
        if (el->isRest()) {
            if (prevWasChord && s->next(SegmentType::ChordRest)) {
                hasSpuriousInterNoteRest = true;
            }
            prevWasChord = false;
        } else if (el->isChord()) {
            Chord* c = toChord(el);
            graceCount += static_cast<int>(c->graceNotes().size());
            prevWasChord = true;
        }
    }
    EXPECT_FALSE(hasSpuriousInterNoteRest)
        << "spurious rest between regular notes; stolenTicks snap suppression missing";
    EXPECT_EQ(graceCount, 1) << "expected exactly 1 grace (leading 32nd)";
    delete score;
}

// Regression: inner grace (g1=0x10) shorter than the leader (g1=0x20) was treated as a regular note.
TEST_F(Tst_ImporterV0xa6, v0xa6_inner_grace_group)
{
    MasterScore* score = readEncoreScore("importer_v0xa6_inner_grace_group.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_v0xa6_inner_grace_group.enc";

    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << "sanityCheck failed (would crash in GUI): " << ret.text();

    Measure* m1 = score->firstMeasure();
    ASSERT_NE(m1, nullptr);

    int graceCount = 0;
    bool hasSpuriousPreGraceRest = false;
    std::vector<DurationType> regularTypes;
    for (Segment* s = m1->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (!el) {
            continue;
        }
        if (el->isRest()) {
            Segment* nx = s->next(SegmentType::ChordRest);
            if (nx) {
                EngravingItem* nxEl = nx->element(0);
                if (nxEl && nxEl->isChord() && !toChord(nxEl)->graceNotes().empty()) {
                    hasSpuriousPreGraceRest = true;
                }
            }
        } else if (el->isChord()) {
            Chord* c = toChord(el);
            graceCount += static_cast<int>(c->graceNotes().size());
            regularTypes.push_back(c->durationType().type());
        }
    }
    EXPECT_FALSE(hasSpuriousPreGraceRest)
        << "Rest found immediately before grace-note chord (crash-inducing structure)";
    EXPECT_EQ(graceCount, 2) << "expected 2 graces (32nd leader + 64th inner)";
    ASSERT_GE(regularTypes.size(), 2u);
    EXPECT_EQ(regularTypes.front(), DurationType::V_EIGHTH);
    EXPECT_EQ(regularTypes.back(), DurationType::V_EIGHTH);
    delete score;
}

// Regression: v0xA6 grace notes shift subsequent real notes forward; calculateRealDurations must restore face duration.
TEST_F(Tst_ImporterV0xa6, v0xa6_grace_restores_face_value)
{
    MasterScore* score = readEncoreScore("importer_v0xa6_grace_restores_face_value.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_v0xa6_grace_restores_face_value.enc";

    Measure* m1 = score->firstMeasure();
    ASSERT_NE(m1, nullptr);

    std::vector<std::pair<DurationType, bool> > elements;
    for (Segment* s = m1->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (!el) {
            continue;
        }
        if (el->isRest()) {
            elements.push_back({ toRest(el)->durationType().type(), false });
        } else if (el->isChord()) {
            Chord* c = toChord(el);
            for (Chord* gc : c->graceNotes()) {
                elements.push_back({ gc->durationType().type(), true });
            }
            elements.push_back({ c->durationType().type(), false });
        }
    }
    ASSERT_EQ(elements.size(), 5u)
        << "grace time-borrowing correction must restore the last 8th; got " << elements.size();
    EXPECT_EQ(elements[0].second, false);
    EXPECT_EQ(elements[0].first, DurationType::V_EIGHTH);
    EXPECT_EQ(elements[1].second, true);
    EXPECT_EQ(elements[1].first, DurationType::V_32ND);
    EXPECT_EQ(elements[2].second, false);
    EXPECT_EQ(elements[2].first, DurationType::V_16TH);
    EXPECT_EQ(elements[3].second, false);
    EXPECT_EQ(elements[3].first, DurationType::V_16TH);
    EXPECT_EQ(elements[4].second, false);
    EXPECT_EQ(elements[4].first, DurationType::V_EIGHTH)
        << "last note must be an eighth (face value), not a 16th from rawGap=90";
    delete score;
}

// v0xA6 stores LYRIC elements in a compact layout the shared newer-format reader would skip past,
// dropping every 2.x lyric. See ENCORE_FORMAT.md §Lyric element. Feeds the raw bytes for "lent".
TEST_F(Tst_ImporterV0xa6, v0xa6_compact_lyric_parses_text)
{
    QByteArray buf;
    buf.append(char(6));        // size (element +3)
    buf.append(char(0x40));     // rawStaff (+4)
    buf.append(char(0x77));     // control byte (+5)
    buf.append("lent");         // text (+6..)
    buf.append(char(0x00));     // NUL terminator
    while (buf.size() < 6 * 2) {
        buf.append(char(0));    // pad out the size*2 slot
    }

    QDataStream ds(buf);
    ds.setByteOrder(QDataStream::LittleEndian);

    mu::iex::enc::EncLyric lyr(0, 6, 1);
    lyr.preKieSkip = 0;         // v0xA6 layout values (from EncFormatReader_V0xA6)
    lyr.textGapAfterKie = 0;
    lyr.spacingFactor = 2;
    lyr.read(ds);

    EXPECT_EQ(lyr.text, QString("lent"));
}

// v0xA6 TEXT-block entries carry no per-entry header (text at offset 0, not the newer +14), so short
// entries read empty at the wrong offset. See ENCORE_FORMAT.md §TEXT block. Feeds a one-entry "Moderato".
TEST_F(Tst_ImporterV0xa6, v0xa6_text_block_entry_text_at_offset_0)
{
    QByteArray buf;
    auto u16 = [&](int v) { buf.append(char(v & 0xff)); buf.append(char((v >> 8) & 0xff)); };
    u16(0);              // sync
    u16(1);              // count = 1 entry
    u16(0);
    u16(0);              // contentSize (4 bytes, unused)
    u16(10);             // entrySize = 10
    buf.append("Moderato");
    buf.append(char(0));
    buf.append(char(0));                                                // text + NUL + pad = 10 bytes

    QDataStream ds(buf);
    ds.setByteOrder(QDataStream::LittleEndian);

    mu::iex::enc::EncTextBlock tb;
    tb.read(ds, static_cast<quint32>(buf.size()), /*textOffset*/ 0);   // v0xA6

    ASSERT_EQ(tb.entries.size(), size_t(1));
    EXPECT_EQ(tb.entries[0], QString("Moderato"));
}

// v0xA6 compact STAFFTEXT ornaments hold the TEXT-entry index (tind) at a fixed offset, not the
// newer size-based one. See ENCORE_FORMAT.md §Ornament element.
TEST_F(Tst_ImporterV0xa6, v0xa6_stafftext_tind_at_offset_26)
{
    QByteArray buf(30, 0);   // size*2 slot
    buf[0] = char(15);       // element size
    buf[2] = char(0x1E);     // tipo = STAFFTEXT (read right after size + rawStaff)
    buf[25] = char(0x04);    // tind: +26 from the type/voice byte, which sits one byte before the buffer

    QDataStream ds(buf);
    ds.setByteOrder(QDataStream::LittleEndian);

    mu::iex::enc::EncOrnament orn(0, 5, 0);
    orn.tindOffset = 26;     // v0xA6
    orn.read(ds);

    EXPECT_EQ(orn.tind, 4);
}

// End-to-end: a real v0xA6 file whose measure carries a compact lyric syllable ("loco") and a
// STAFFTEXT ornament pointing at TEXT entry 0 ("dolce") must import both. Before the compact-layout
// fix the 2.x lyric and staff text were silently dropped.
TEST_F(Tst_ImporterV0xa6, v0xa6_imports_lyrics_and_staff_text)
{
    MasterScore* score = readEncoreScore("importer_v0xa6_lyrics_and_stafftext.enc");
    ASSERT_NE(score, nullptr);

    bool foundLyric = false;
    bool foundStaffText = false;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* a : s->annotations()) {
                if (a && a->isStaffText() && toStaffText(a)->plainText() == String(u"dolce")) {
                    foundStaffText = true;
                }
            }
            for (int ti = 0; ti < static_cast<int>(score->nstaves() * VOICES); ++ti) {
                EngravingItem* el = s->element(static_cast<track_idx_t>(ti));
                if (!el || !el->isChord()) {
                    continue;
                }
                for (Lyrics* ly : toChord(el)->lyrics()) {
                    if (ly && ly->plainText() == String(u"loco")) {
                        foundLyric = true;
                    }
                }
            }
        }
    }
    EXPECT_TRUE(foundLyric) << "v0xA6 compact lyric 'loco' must be imported";
    EXPECT_TRUE(foundStaffText) << "v0xA6 staff text 'dolce' must be imported";
    delete score;
}

// v0xA6 compact STAFFTEXT stores its vertical placement at a different offset than v0xC4; reading the
// v0xC4 offset yields 0 for both texts, so "espressivo" (negative y) wrongly landed above.
TEST_F(Tst_ImporterV0xa6, v0xa6_stafftext_placement_from_offset_6)
{
    MasterScore* score = readEncoreScore("importer_v0xa6_stafftext_placement.enc");
    ASSERT_NE(score, nullptr);

    bool sawAbove = false;
    bool sawBelow = false;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* a : s->annotations()) {
                if (!a || !a->isStaffText()) {
                    continue;
                }
                const StaffText* st = toStaffText(a);
                if (st->plainText() == String(u"cresc.")) {
                    EXPECT_EQ(st->placement(), PlacementV::ABOVE) << "positive y must place above";
                    sawAbove = true;
                } else if (st->plainText() == String(u"espressivo")) {
                    EXPECT_EQ(st->placement(), PlacementV::BELOW) << "negative y must place below";
                    sawBelow = true;
                }
            }
        }
    }
    EXPECT_TRUE(sawAbove) << "staff text 'cresc.' must be imported";
    EXPECT_TRUE(sawBelow) << "staff text 'espressivo' must be imported";
    delete score;
}

// Encore stores every verse-2 syllable at tick=0; its real position is the xoffset. Verse 2 must align
// to the same notes as verse 1 by xoffset, not collapse onto the first note.
TEST_F(Tst_ImporterV0xa6, v0xa6_second_verse_aligns_by_xoffset)
{
    MasterScore* score = readEncoreScore("importer_v0xa6_two_verse_alignment.enc");
    ASSERT_NE(score, nullptr);
    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    std::vector<std::pair<String, String> > perChord;   // (verse 0 text, verse 1 text)
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (!el || !el->isChord()) {
            continue;
        }
        String v0, v1;
        for (Lyrics* ly : toChord(el)->lyrics()) {
            if (!ly) {
                continue;
            }
            if (ly->verse() == 0) {
                v0 = ly->plainText();
            } else if (ly->verse() == 1) {
                v1 = ly->plainText();
            }
        }
        perChord.emplace_back(v0, v1);
    }

    ASSERT_GE(perChord.size(), size_t(3));
    EXPECT_EQ(perChord[0].first,  String(u"A"));
    EXPECT_EQ(perChord[0].second, String(u"X"));
    EXPECT_EQ(perChord[1].first,  String(u"B"));
    EXPECT_EQ(perChord[1].second, String(u"Y"));
    EXPECT_EQ(perChord[2].first,  String(u"C"));
    EXPECT_EQ(perChord[2].second, String(u"Z"));
    delete score;
}

// Melisma word: Encore stores verse 1 at the melisma's END note, but its xoffset matches verse 2 on the
// first note, so xoffset alignment must keep both syllables on note 1 (tick matching wrongly split them).
TEST_F(Tst_ImporterV0xa6, v0xa6_melisma_word_aligns_both_verses_on_first_note)
{
    MasterScore* score = readEncoreScore("importer_v0xa6_melisma_verse_alignment.enc");
    ASSERT_NE(score, nullptr);
    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    std::vector<std::pair<String, String> > perChord;   // (verse 0 text, verse 1 text)
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        EngravingItem* el = s->element(0);
        if (!el || !el->isChord()) {
            continue;
        }
        String v0, v1;
        for (Lyrics* ly : toChord(el)->lyrics()) {
            if (!ly) {
                continue;
            }
            if (ly->verse() == 0) {
                v0 = ly->plainText();
            } else if (ly->verse() == 1) {
                v1 = ly->plainText();
            }
        }
        perChord.emplace_back(v0, v1);
    }

    ASSERT_GE(perChord.size(), size_t(2));
    EXPECT_EQ(perChord[0].first,  String(u"peace"));
    EXPECT_EQ(perChord[0].second, String(u"born"));
    EXPECT_TRUE(perChord[1].first.isEmpty());
    EXPECT_TRUE(perChord[1].second.isEmpty());
    delete score;
}
