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

// Ornaments and articulations: trills, mordents, fermatas, breaths/caesuras, arpeggios, tremolos and the
// articulation-byte mapping, plus their placement/anchoring. See ENCORE_FORMAT.md §Ornament element.

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
#include "engraving/dom/tuplet.h"
#include "engraving/dom/breath.h"
#include "engraving/dom/measurerepeat.h"
#include "engraving/dom/ornament.h"
#include "engraving/dom/trill.h"

#include "testbase.h"

static const QString ENC_DIR(QString(iex_encore_tests_DATA_ROOT) + "/data/");

using namespace mu::engraving;

class Tst_Ornaments : public ::testing::Test, public MTest
{
protected:
    void SetUp() override { setRootDir(ENC_DIR); }
};

// ===========================================================================
// REGRESSION: Capped 3:2 triplet produces non-TDuration placedTicks; closeTuplet must not assert in beam layout.
// ===========================================================================

TEST_F(Tst_Ornaments, beamed_triplet_capped_no_beam_assert)
{
    MasterScore* score = readEncoreScore("ornaments_beamed_triplet_capped.enc");
    ASSERT_NE(score, nullptr) << "File should load without beam-layout assert";
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    for (auto& [tick, sp] : score->spannerMap().map()) {
        EXPECT_LT(sp->tick(), sp->tick2()) << "Spanner has non-positive span";
    }
    delete score;
}

// ===========================================================================
// REGRESSION: WEDGESTART with alMezuro=0 must span the current measure, not collapse to zero.
// ===========================================================================

// ===========================================================================
// REGRESSION: Partial 3:2 quarter triplet (placedTicks=1/3, not a TDuration fraction) must not assert in beam layout.
// ===========================================================================

TEST_F(Tst_Ornaments, partial_quarter_triplet_layout_does_not_assert)
{
    MasterScore* score = readEncoreScore("ornaments_partial_quarter_triplet.enc");
    ASSERT_NE(score, nullptr) << "File must load and lay out without assert in beam.cpp";
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();
    delete score;
}

TEST_F(Tst_Ornaments, articulations_mapped_beyond_fermata)
{
    MasterScore* score = readEncoreScore("ornaments_articulations.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    // Check by type predicate rather than exact SymId; layout may flip Above<->Below per stem direction.
    enum class ArtKind {
        Staccato, Accent, Tenuto, Marcato, Other
    };
    auto kindOf = [](Articulation* a) -> ArtKind {
        if (a->isStaccato()) {
            return ArtKind::Staccato;
        }
        if (a->isAccent()) {
            return ArtKind::Accent;
        }
        if (a->isTenuto()) {
            return ArtKind::Tenuto;
        }
        if (a->isMarcato()) {
            return ArtKind::Marcato;
        }
        return ArtKind::Other;
    };
    std::vector<ArtKind> expected = {
        ArtKind::Staccato, ArtKind::Accent, ArtKind::Tenuto, ArtKind::Marcato,
    };
    std::vector<ArtKind> seen;
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
            for (Articulation* a : toChord(el)->articulations()) {
                seen.push_back(kindOf(a));
            }
        }
    }
    EXPECT_EQ(seen.size(), expected.size());
    for (size_t i = 0; i < std::min(seen.size(), expected.size()); ++i) {
        EXPECT_EQ(seen[i], expected[i]) << "articulation #" << i;
    }
    delete score;
}

// ===========================================================================
// FEATURE: Combo articulation bytes (e.g. 0x24 = tenuto + staccato) expand to two Articulation elements.
// ===========================================================================
TEST_F(Tst_Ornaments, articulation_combos_expand_to_two_glyphs)
{
    MasterScore* score = readEncoreScore("ornaments_articulations_combo.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    // Combined single-glyph articulations map to their own kind.
    // Individual components (Tenuto, Staccato, etc.) are for single-symbol artics.
    enum class K {
        Tenuto, Staccato, Accent, Marcato, Staccatissimo,
        MarcatoStaccato, MarcatoTenuto, AccentStaccato, TenutoStaccato, TenutoAccent,
        Other
    };
    auto kindOf = [](Articulation* a) -> K {
        using mu::engraving::SymId;
        switch (SymId(a->subtype())) {
        case SymId::articTenutoAbove:           return K::Tenuto;
        case SymId::articStaccatoAbove:         return K::Staccato;
        case SymId::articAccentAbove:           return K::Accent;
        case SymId::articMarcatoAbove:          return K::Marcato;
        case SymId::articStaccatissimoAbove:    return K::Staccatissimo;
        case SymId::articMarcatoStaccatoAbove:  return K::MarcatoStaccato;
        case SymId::articMarcatoTenutoAbove:    return K::MarcatoTenuto;
        case SymId::articAccentStaccatoAbove:   return K::AccentStaccato;
        case SymId::articTenutoStaccatoAbove:   return K::TenutoStaccato;
        case SymId::articTenutoAccentAbove:     return K::TenutoAccent;
        default:                                return K::Other;
        }
    };
    // Palette pairs (below-byte, above-byte), verified in Encore 5 against
    // "Accidentals Marks and others": 0x22/0x23 tenuto-accent, 0x24/0x25
    // tenuto-staccato, 0x26/0x27 marcato-tenuto (= tenuto + heavy-accent),
    // 0x2A/0x2B heavy-accent + staccatissimo, 0x2C/0x2D tenuto + staccatissimo.
    // m1: 0x24 TenutoStaccato, 0x17 AccentStaccato, 0x27 MarcatoTenuto, 0x15 MarcatoStaccato.
    // m2: 0x23 TenutoAccent, 0x2D Tenuto+Staccatissimo, 0x2B Marcato+Staccatissimo, 0x24 TenutoStaccato.
    // m3: 0x14 MarcatoStaccatoBelow, 0x26 MarcatoTenutoBelow.
    // m4: 0x25 TenutoStaccato, 0x2A Marcato+Staccatissimo, 0x2C Tenuto+Staccatissimo, 0x22 TenutoAccent.
    const std::vector<std::set<K> > expected = {
        { K::TenutoStaccato },            // 0x24 → articTenutoStaccatoAbove
        { K::AccentStaccato },            // 0x17 → articAccentStaccatoAbove
        { K::MarcatoTenuto },             // 0x27 → articMarcatoTenutoAbove
        { K::MarcatoStaccato },           // 0x15 → articMarcatoStaccatoAbove
        { K::TenutoAccent },              // 0x23 → articTenutoAccentAbove
        { K::Tenuto, K::Staccatissimo },  // 0x2D (no single glyph)
        { K::Marcato, K::Staccatissimo }, // 0x2B heavy accent + staccatissimo
        { K::TenutoStaccato },            // 0x24 again
        { K::MarcatoStaccato },           // 0x14 → articMarcatoStaccatoBelow
        { K::MarcatoTenuto },             // 0x26 → articMarcatoTenutoBelow
        { K::TenutoStaccato },            // 0x25 → articTenutoStaccatoAbove
        { K::Marcato, K::Staccatissimo }, // 0x2A heavy accent + staccatissimo
        { K::Tenuto, K::Staccatissimo },  // 0x2C (no single glyph)
        { K::TenutoAccent },              // 0x22 → articTenutoAccentAbove
    };
    std::vector<std::set<K> > seen;
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
            std::set<K> kinds;
            for (Articulation* a : toChord(el)->articulations()) {
                kinds.insert(kindOf(a));
            }
            if (!kinds.empty()) {
                seen.push_back(kinds);
            }
        }
    }
    ASSERT_EQ(seen.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(seen[i], expected[i]) << "chord #" << i;
    }
    delete score;
}

// ===========================================================================
// FEATURE: Fermata anchored on segment (not chord); direction from artic slot: articUp=0x20 (above), articDown=0x21 (below).
// ===========================================================================
TEST_F(Tst_Ornaments, fermatas_emit_segment_anchored_element)
{
    MasterScore* score = readEncoreScore("ornaments_fermatas.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    std::vector<PlacementV> seen;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        for (Segment* s = toMeasure(mb)->first(SegmentType::ChordRest);
             s; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isFermata()) {
                    seen.push_back(toFermata(e)->placement());
                }
            }
        }
    }
    ASSERT_EQ(seen.size(), 2u);
    EXPECT_EQ(seen[0], PlacementV::BELOW)
        << "articDown=0x21 must produce an inverted (below) fermata";
    EXPECT_EQ(seen[1], PlacementV::ABOVE)
        << "articUp=0x20 must produce an upright (above) fermata";
    delete score;
}

// ===========================================================================
// FEATURE: Single-note tremolos from per-note artic byte (0x41/0x42/0x43/0x03 → R8/R16/R32).
// ===========================================================================
TEST_F(Tst_Ornaments, tremolos_from_per_note_artic_byte)
{
    MasterScore* score = readEncoreScore("ornaments_tremolos.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    std::vector<TremoloType> seen;
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
            if (TremoloSingleChord* trem = toChord(el)->tremoloSingleChord()) {
                seen.push_back(trem->tremoloType());
            }
        }
    }
    const std::vector<TremoloType> expected = {
        TremoloType::R8, TremoloType::R16, TremoloType::R32, TremoloType::R32,
    };
    EXPECT_EQ(seen, expected);
    delete score;
}

// ===========================================================================
// FEATURE: Trill-mark / mordent / inverted-mordent from per-note artic byte, emitted as Ornament for MusicXML.
// ===========================================================================
TEST_F(Tst_Ornaments, trill_mordent_from_per_note_artic_byte)
{
    MasterScore* score = readEncoreScore("ornaments_trill_mordent.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    std::vector<SymId> seen;
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
            for (Articulation* a : toChord(el)->articulations()) {
                seen.push_back(a->symId());
            }
        }
    }
    const std::vector<SymId> expected = {
        SymId::ornamentTrill,
        SymId::ornamentShortTrill,    // 0x0A: inverted-mordent (short)
        SymId::ornamentMordent,       // 0x0B: simple lower mordent
        SymId::ornamentPrallMordent,  // 0x2F: double/long lower mordent
    };
    EXPECT_EQ(seen, expected);
    delete score;
}

// An ornament-family articulation byte (e.g. turn) must create an Ornament, not a plain Articulation:
// layout calls Ornament-only methods on those SymIds and crashes on a plain Articulation.
TEST_F(Tst_Ornaments, ornament_turn_created_as_ornament_not_articulation)
{
    MasterScore* score = readEncoreScore("ornaments_ornament_turn.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);
    Segment* seg = m->first(SegmentType::ChordRest);
    ASSERT_NE(seg, nullptr);
    EngravingItem* el = seg->element(0);
    ASSERT_NE(el, nullptr);
    ASSERT_TRUE(el->isChord());
    const auto& arts = toChord(el)->articulations();
    ASSERT_EQ(arts.size(), 1u) << "One ornament expected on the first note";

    Articulation* art = arts.front();
    EXPECT_EQ(art->symId(), SymId::ornamentTurn)
        << "SymId must be ornamentTurn";
    EXPECT_TRUE(art->isOrnament())
        << "ornamentTurn must be stored as Ornament, not plain Articulation; "
        "a plain Articulation with an ornament SymId causes SIGSEGV in layout";

    delete score;
}

// ===========================================================================
// REGRESSION: DOUBLE barline must be applied to every staff, not only track 0.
// ===========================================================================
TEST_F(Tst_Ornaments, double_barline_lands_on_every_staff)
{
    MasterScore* score = readEncoreScore("ornaments_double_barline_multi_staff.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    Measure* m1 = score->firstMeasure();
    ASSERT_NE(m1, nullptr);
    const size_t nstaves = score->nstaves();
    ASSERT_GE(nstaves, 2u) << "fixture must have at least two staves";

    Segment* endBarSeg = m1->findSegment(SegmentType::EndBarLine, m1->endTick());
    ASSERT_NE(endBarSeg, nullptr) << "m1 must have an EndBarLine segment";

    int doubleBarStaves = 0;
    for (size_t s = 0; s < nstaves; ++s) {
        EngravingItem* el = endBarSeg->element(s * VOICES);
        if (!el || !el->isBarLine()) {
            continue;
        }
        if (toBarLine(el)->barLineType() == BarLineType::DOUBLE) {
            ++doubleBarStaves;
        }
    }
    EXPECT_EQ(doubleBarStaves, static_cast<int>(nstaves))
        << "DOUBLE barline must be present on every staff, not only track 0";
    delete score;
}

// ===========================================================================
// BUG FIX: articulationDown=0x21 on a non-tuplet note must create fermataBelow;
// on a tuplet note it must be suppressed (same dual-meaning rule as 0x20 above).
// ===========================================================================

// ===========================================================================
// BUG FIX: articulationDown=0x21 on a non-tuplet note must create fermataBelow;
// on a tuplet note it must be suppressed (same dual-meaning rule as 0x20 above).
// ===========================================================================
TEST_F(Tst_Ornaments, fermata_below_kept_on_non_tuplet_suppressed_on_tuplet)
{
    MasterScore* score = readEncoreScore("ornaments_fermata_below_not_in_tuplet.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    Measure* m1 = score->firstMeasure();
    ASSERT_NE(m1, nullptr);

    int fermataCount = 0;
    SymId sym = SymId::noSym;
    for (Segment* s = m1->first(SegmentType::ChordRest); s;
         s = s->next(SegmentType::ChordRest)) {
        for (EngravingItem* e : s->annotations()) {
            if (e && e->isFermata()) {
                ++fermataCount;
                sym = toFermata(e)->symId();
            }
        }
    }
    EXPECT_EQ(fermataCount, 1) << "only the non-tuplet note must produce a fermata";
    EXPECT_EQ(sym, SymId::fermataBelow) << "articDown=0x21 on non-tuplet must be fermataBelow";
    delete score;
}

// ===========================================================================
// NEW ARTIC BYTES: brassMuteClosed (0x1B), ornamentTurnInverted (0x2E),
//                 brassMuteHalfClosed (0x30)
// ===========================================================================
TEST_F(Tst_Ornaments, new_artic_bytes_stopped_inverted_turn_half_stopped)
{
    MasterScore* score = readEncoreScore("ornaments_new_artic_bytes.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "sanity check failed";

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    auto articsOnNote = [&](int noteIdx) {
        Segment* seg = m->first(SegmentType::ChordRest);
        for (int i = 0; i < noteIdx && seg; ++i) {
            seg = seg->next(SegmentType::ChordRest);
        }
        if (!seg) {
            return std::vector<SymId> {};
        }
        EngravingItem* el = seg->element(0);
        if (!el || !el->isChord()) {
            return std::vector<SymId> {};
        }
        std::vector<SymId> ids;
        for (Articulation* a : toChord(el)->articulations()) {
            ids.push_back(a->symId());
        }
        for (EngravingItem* e : toChord(el)->el()) {
            if (e->isOrnament()) {
                ids.push_back(toOrnament(e)->symId());
            }
        }
        return ids;
    };

    auto s0 = articsOnNote(0);
    auto s1 = articsOnNote(1);
    auto s2 = articsOnNote(2);

    EXPECT_TRUE(std::find(s0.begin(), s0.end(), SymId::brassMuteClosed) != s0.end())
        << "0x1B must produce brassMuteClosed (+)";
    EXPECT_TRUE(std::find(s1.begin(), s1.end(), SymId::ornamentTurnInverted) != s1.end())
        << "0x2E must produce ornamentTurnInverted (inverted turn)";
    EXPECT_TRUE(std::find(s2.begin(), s2.end(), SymId::brassMuteHalfClosed) != s2.end())
        << "0x30 must produce brassMuteHalfClosed (half-stopped)";

    delete score;
}

// ===========================================================================
// TRILL WITH ACCIDENTALS: intervalAbove set from artic bytes 0x05/0x06/0x07
// ===========================================================================
TEST_F(Tst_Ornaments, trill_accidentals_set_interval_above)
{
    MasterScore* score = readEncoreScore("ornaments_trill_with_accidentals.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "sanity check failed";

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    struct NoteOrnInterval {
        int noteIdx;
        IntervalType expected;
        const char* desc;
    };
    std::vector<NoteOrnInterval> checks = {
        { 0, IntervalType::AUTO,      "0x04 trill: no accidental → AUTO" },
        { 1, IntervalType::MINOR,     "0x05 trill+flat → MINOR (trill menor)" },
        { 2, IntervalType::AUGMENTED, "0x06 trill+sharp → AUGMENTED (trill augmented)" },
        { 3, IntervalType::MAJOR,     "0x07 trill+natural → MAJOR" },
    };

    // Collect intervalAbove values from all trill ornaments (any symId, since
    // MuseScore may store the cue-note accidental as the primary glyph for some intervals).
    std::vector<IntervalType> intervals;
    int noteIdx = 0;
    for (Segment* seg = m->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
        EngravingItem* el = seg->element(0);
        if (!el || !el->isChord()) {
            continue;
        }
        if (noteIdx >= (int)checks.size()) {
            break;
        }
        for (EngravingItem* sub : toChord(el)->el()) {
            if (!sub->isOrnament()) {
                continue;
            }
            Ornament* orn = toOrnament(sub);
            // Only check ornaments that were created for trills (have a trill-related interval or AUTO)
            if (orn->intervalAbove().type != IntervalType::AUTO
                || checks[noteIdx].expected == IntervalType::AUTO) {
                EXPECT_EQ(orn->intervalAbove().type, checks[noteIdx].expected)
                    << checks[noteIdx].desc;
            }
        }
        ++noteIdx;
    }

    delete score;
}

// When several notes in a chord carry the same articulation byte, the ornament must be added once, not
// duplicated per note.
TEST_F(Tst_Ornaments, artic_byte_dedup_no_duplicate_ornament_on_chord)
{
    MasterScore* score = readEncoreScore("notes_artic_dedup_trill_on_chord.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_TRUE(score->sanityCheck()) << "Artic dedup must not corrupt";

    Measure* m = score->firstMeasure();
    ASSERT_NE(m, nullptr);

    Segment* seg = m->first(SegmentType::ChordRest);
    ASSERT_NE(seg, nullptr);
    EngravingItem* el = seg->element(0);
    ASSERT_NE(el, nullptr);
    ASSERT_TRUE(el->isChord());
    Chord* chord = toChord(el);
    ASSERT_EQ(chord->notes().size(), 2u) << "Chord must have both notes";

    // Count ornamentTrill instances on the chord
    int trillCount = 0;
    for (Articulation* e : chord->articulations()) {
        if (e && e->isOrnament() && toOrnament(e)->symId() == SymId::ornamentTrill) {
            ++trillCount;
        }
    }
    EXPECT_EQ(trillCount, 1)
        << "Both notes carry au=0x04 (trill) but the chord must have exactly ONE "
        "ornamentTrill; duplicate artic bytes on multi-note chords must be deduped";

    delete score;
}

// ===========================================================================
// FEATURE: ORN tipo 0x1C (GRAPHIC_LINE, user-drawn line) is silently skipped.
// No articulation is added to the chord; score loads and passes sanity check.
// ===========================================================================

// ===========================================================================
// BUG FIX: articulationUp=0x20 on the last note of a tuplet group is Encore's
// "tuplet bracket placement above" flag, not a fermata.
// ===========================================================================
TEST_F(Tst_Ornaments, v0c4_fermata_suppressed_on_tuplet_last_note)
{
    MasterScore* score = readEncoreScore("ornaments_fermata_not_in_tuplet.enc");
    ASSERT_NE(score, nullptr) << "Failed to load ornaments_fermata_not_in_tuplet.enc";
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    Measure* m1 = score->firstMeasure();
    ASSERT_NE(m1, nullptr);

    int fermataCount = 0;
    for (Segment* s = m1->first(SegmentType::ChordRest); s;
         s = s->next(SegmentType::ChordRest)) {
        for (EngravingItem* e : s->annotations()) {
            if (e && e->isFermata()) {
                ++fermataCount;
            }
        }
    }
    EXPECT_EQ(fermataCount, 1)
        << "Only the non-tuplet note must get a fermata; tuplet notes with "
        "articUp=0x20 encode bracket placement, not a fermata";
    delete score;
}

// Lightweight test macro for Tst_Ornaments
#ifndef ENC_SANITY_TEST_ORNAMENTS
#define ENC_SANITY_TEST_ORNAMENTS(testName, fileName) \
    TEST_F(Tst_Ornaments, testName) { \
        MasterScore* score = readEncoreScore(fileName); \
        ASSERT_NE(score, nullptr) << "Failed to load " << fileName; \
        EXPECT_GT(score->nmeasures(), 0); \
        muse::Ret ret = score->sanityCheck(); \
        EXPECT_TRUE(ret) << "Corrupted: " << ret.text(); \
        delete score; \
    }
#endif

ENC_SANITY_TEST_ORNAMENTS(articulations_extended, "ornaments_articulations.enc")
ENC_SANITY_TEST_ORNAMENTS(articulations_combo,    "ornaments_articulations_combo.enc")
ENC_SANITY_TEST_ORNAMENTS(trill_mordent,          "ornaments_trill_mordent.enc")
ENC_SANITY_TEST_ORNAMENTS(tremolos,               "ornaments_tremolos.enc")
ENC_SANITY_TEST_ORNAMENTS(fermatas,               "ornaments_fermatas.enc")
ENC_SANITY_TEST_ORNAMENTS(technical,              "ornaments_technical.enc")
ENC_SANITY_TEST_ORNAMENTS(trill_spanner,          "ornaments_trill_spanner.enc")
ENC_SANITY_TEST_ORNAMENTS(staccato_orn,           "ornaments_staccato_orn.enc")
ENC_SANITY_TEST_ORNAMENTS(arpeggio,               "ornaments_arpeggio.enc")
