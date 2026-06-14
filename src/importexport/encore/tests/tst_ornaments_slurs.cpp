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

// Slur and ottava endpoint resolution: open-slur removal, alMezuro/+16 measure-count resolution, xoffset
// pixel-span heuristics, multi-instrument routing, and grace-to-main/grace-to-later anchoring.
// See ENCORE_IMPORTER.md §Slur endpoint resolution and ENCORE_IMPORTER.md §Grace-to-main and grace-to-later slurs.

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
#include "engraving/dom/ottava.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/trill.h"

#include "testbase.h"
#include "../internal/importer/import-options.h"

static const QString ENC_DIR(QString(iex_encore_tests_DATA_ROOT) + "/data/");

using namespace mu::engraving;

class Tst_OrnamentsSlurs : public ::testing::Test, public MTest
{
protected:
    void SetUp() override { setRootDir(ENC_DIR); }
};

// A SLURSTART with no matching stop has no endpoints and would NaN in Bezier layout; open slurs must be
// dropped so every surviving spanner has a valid tick range.
TEST_F(Tst_OrnamentsSlurs, no_nan_crash_from_open_slurs)
{
    MasterScore* score = readEncoreScore("notes_corrupted.enc");
    ASSERT_NE(score, nullptr) << "Corrupted file should load without NaN crash";
    for (auto& [tick, sp] : score->spannerMap().map()) {
        EXPECT_LT(sp->tick(), sp->tick2())
            << "All spanners should have tick < tick2 (valid range)";
    }
    delete score;
}

TEST_F(Tst_OrnamentsSlurs, no_nan_crash_opus27)
{
    MasterScore* score = readEncoreScore("notes_corrupted.enc");
    ASSERT_NE(score, nullptr);
    delete score;
}

TEST_F(Tst_OrnamentsSlurs, overfull_measure_slur_no_zero_length_arc)
{
    // Overfull-measure tick drift can make a cross-measure slur resolve start==end (a zero-length arc),
    // which NaNs the Bezier layout; such slurs must be dropped. Needs the IrregularMeasure strategy (which
    // extends the bar and produces the drift); the readEncoreScore default Truncate hides it. The fixture
    // is a trimmed real file because a synthetic bar does not reproduce the accumulated drift.
    mu::iex::enc::EncImportOptions opts;
    opts.overfillMeasureStrategy = mu::iex::enc::OverfillStrategy::IrregularMeasure;
    MasterScore* score = readEncoreScoreWithOpts("structure_v0c4_slur_zero_length_overfull.enc", opts);
    ASSERT_NE(score, nullptr) << "must load and lay out without a NaN crash";
    for (auto& [tick, sp] : score->spannerMap().map()) {
        if (!sp->isSlur()) {
            continue;
        }
        EXPECT_NE(sp->startElement(), sp->endElement())
            << "no slur may have a coinciding start and end element (zero-length arc)";
    }
    delete score;
}

// .enc has no SLURSTOP; a SLURSTART resolves its end from the alMezuro measure count after the measure pass.
TEST_F(Tst_OrnamentsSlurs, multi_measure_slur_resolved_from_almezuro)
{
    MasterScore* score = readEncoreScore("ornaments_multi_measure_slur.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    int slurCount = 0;
    for (auto& [tick, sp] : score->spannerMap().map()) {
        if (!sp->isSlur()) {
            continue;
        }
        ++slurCount;
        EXPECT_LT(sp->tick(), sp->tick2()) << "slur span must be positive";
        EXPECT_NE(sp->startElement(), nullptr) << "slur missing start element";
        EXPECT_NE(sp->endElement(), nullptr) << "slur missing end element";
    }
    EXPECT_EQ(slurCount, 2);
    delete score;
}

// Regression: in multi-instrument compact-encoded files the raw instrument index must be translated to
// the routed LINE slot before finding a slur's notes, or staves 1-3 miss and the last-chord fallback
// wrongly picks note3. Each staff's slur must end at note2.
TEST_F(Tst_OrnamentsSlurs, multiinstr_slur_endpoint_on_second_note_not_last_chord)
{
    MasterScore* score = readEncoreScore("ornaments_multiinstr_slur_routing.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_EQ(score->nstaves(), 4);

    std::map<int, int> expectedEndPitch = { { 0, 64 }, { 1, 52 }, { 2, 71 }, { 3, 59 } };
    std::map<int, bool> staffSeen;

    for (auto& [tick, sp] : score->spannerMap().map()) {
        if (!sp->isSlur()) {
            continue;
        }
        EXPECT_NE(sp->startElement(), nullptr) << "slur missing start";
        EXPECT_NE(sp->endElement(),   nullptr) << "slur missing end";
        if (!sp->startElement() || !sp->endElement()) {
            continue;
        }
        const int si = static_cast<int>(sp->staffIdx());
        staffSeen[si] = true;
        EXPECT_LT(sp->tick(), sp->tick2()) << "slur span must be positive, staff " << si;

        const EngravingItem* endEl = sp->endElement();
        ASSERT_TRUE(endEl->isChord()) << "slur end must be a chord, staff " << si;
        const int endPitch = toChord(endEl)->notes().back()->pitch();
        auto it = expectedEndPitch.find(si);
        if (it != expectedEndPitch.end()) {
            EXPECT_EQ(endPitch, it->second)
                << "slur on staff " << si << " must end at note2 (pitch " << it->second
                << "), not note3";
        }
    }

    for (const auto& [si, expected] : expectedEndPitch) {
        EXPECT_TRUE(staffSeen.count(si) > 0) << "missing slur on staff " << si;
    }
    delete score;
}

// v0xC2 xoffset2 is stale, so a tiny-span slur is a note-to-next-note arc ending at note2, regardless of
// a decoy note3 whose xoffset would otherwise match.
TEST_F(Tst_OrnamentsSlurs, v0xc2_slur_ends_at_note2_not_decoy_note3)
{
    MasterScore* score = readEncoreScore("ornaments_v0c2_slur_firstnote_xoff_mismatch.enc");
    ASSERT_NE(score, nullptr);

    const Measure* m0 = score->firstMeasure();
    ASSERT_NE(m0, nullptr);

    bool foundSlur = false;
    for (auto& [tick, sp] : score->spannerMap().map()) {
        if (!sp->isSlur()) {
            continue;
        }
        foundSlur = true;
        ASSERT_NE(sp->endElement(), nullptr) << "slur must have an end element";
        ASSERT_TRUE(sp->endElement()->isChord()) << "slur end must be a chord";
        const int endPitch = toChord(sp->endElement())->notes().back()->pitch();
        EXPECT_EQ(endPitch, 64) << "slur must end at E4 (note2), not C4 (decoy note3)";
    }
    EXPECT_TRUE(foundSlur) << "score must contain a slur";
    delete score;
}

// A v0xC2 within-measure slur (+16 count = 0) must end at the next note in its own bar and not extend to
// a decoy note in the next measure that an xoffset match would prefer.
TEST_F(Tst_OrnamentsSlurs, v0xc2_same_measure_slur_not_extended_to_next_measure)
{
    MasterScore* score = readEncoreScore("ornaments_v0c2_same_measure_slur_no_cross.enc");
    ASSERT_NE(score, nullptr);

    const Measure* m0 = score->firstMeasure();
    ASSERT_NE(m0, nullptr);

    bool foundSlur = false;
    for (auto& [tick, sp] : score->spannerMap().map()) {
        if (!sp->isSlur()) {
            continue;
        }
        foundSlur = true;
        EXPECT_NE(sp->startElement(), nullptr) << "slur must have start element";
        EXPECT_NE(sp->endElement(),   nullptr) << "slur must have end element";
        if (!sp->startElement() || !sp->endElement()) {
            continue;
        }
        const Measure* startMeas = sp->startElement()->findMeasure();
        const Measure* endMeas   = sp->endElement()->findMeasure();
        EXPECT_EQ(startMeas, m0) << "slur must start in measure 0";
        EXPECT_EQ(endMeas, m0) << "slur must end in measure 0, not in the decoy measure 1";
    }
    EXPECT_TRUE(foundSlur) << "score must contain a slur";
    delete score;
}

// v0xC2 multi-instrument slur routing: staff-slot translation must let each staff's slur find its notes,
// and the next-note rule must end each slur at note2 rather than a decoy note3.
TEST_F(Tst_OrnamentsSlurs, v0xc2_multiinstr_slur_endpoint_on_note2_not_decoy)
{
    MasterScore* score = readEncoreScore("ornaments_v0c2_multiinstr_slur_routing.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_EQ(score->nstaves(), 4);

    const std::map<int, int> expectedPitch = { { 0, 60 }, { 1, 52 }, { 2, 71 }, { 3, 59 } };
    std::map<int, bool> staffSeen;

    for (auto& [tick, sp] : score->spannerMap().map()) {
        if (!sp->isSlur()) {
            continue;
        }
        EXPECT_NE(sp->startElement(), nullptr) << "slur missing start";
        EXPECT_NE(sp->endElement(),   nullptr) << "slur missing end";
        if (!sp->startElement() || !sp->endElement()) {
            continue;
        }
        const int si = static_cast<int>(sp->staffIdx());
        staffSeen[si] = true;
        ASSERT_TRUE(sp->endElement()->isChord()) << "slur end not a chord, staff " << si;
        const int endPitch = toChord(sp->endElement())->notes().back()->pitch();
        auto it = expectedPitch.find(si);
        if (it != expectedPitch.end()) {
            EXPECT_EQ(endPitch, it->second)
                << "staff " << si << ": slur must end at note2 (pitch " << it->second
                << "), not note3 (decoy)";
        }
    }
    for (const auto& [si, _] : expectedPitch) {
        EXPECT_TRUE(staffSeen.count(si) > 0) << "missing slur on staff " << si;
    }
    delete score;
}

// A slur from an appoggiatura grace to its co-located main note must not be dropped: the zero-span end is
// detected as grace-to-main with the grace chord as startElement.
TEST_F(Tst_OrnamentsSlurs, grace_slur_to_main_not_dropped)
{
    MasterScore* score = readEncoreScore("ornaments_grace_slur_to_main.enc");
    ASSERT_NE(score, nullptr);

    int slurCount = 0;
    bool graceStart = false;
    for (auto& [tick, sp] : score->spannerMap().map()) {
        if (sp->isSlur()) {
            ++slurCount;
            if (sp->startElement() && sp->startElement()->isChord()) {
                graceStart = toChord(sp->startElement())->isGrace();
            }
        }
    }
    bool endInSameMeasure = false;
    for (auto& [tick, sp] : score->spannerMap().map()) {
        if (sp->isSlur() && sp->startElement() && sp->endElement()
            && sp->startElement()->isChord() && sp->endElement()->isChord()) {
            const Chord* startCh = toChord(sp->startElement());
            const Chord* endCh   = toChord(sp->endElement());
            if (startCh->isGrace() && !endCh->isGrace()) {
                endInSameMeasure = (startCh->measure() == endCh->measure());
            }
        }
    }
    EXPECT_GE(slurCount, 1) << "At least one slur must be imported";
    EXPECT_TRUE(graceStart)
        << "Slur from appoggiatura grace must have a grace chord as startElement";
    EXPECT_TRUE(endInSameMeasure)
        << "Slur endElement must be the main chord in the same measure, not a note in the next measure";

    delete score;
}

// A slur from a grace to a later note must start at the grace chord, not fall back to an earlier note.
TEST_F(Tst_OrnamentsSlurs, grace_slur_to_later_note_starts_from_grace)
{
    MasterScore* score = readEncoreScore("ornaments_grace_slur_to_later.enc");
    ASSERT_NE(score, nullptr);

    int slurCount = 0;
    bool graceStart = false;
    bool endIsQuarter = false;
    for (auto& [tick, sp] : score->spannerMap().map()) {
        if (sp->isSlur()) {
            ++slurCount;
            if (sp->startElement() && sp->startElement()->isChord()) {
                graceStart = toChord(sp->startElement())->isGrace();
            }
            if (sp->endElement() && sp->endElement()->isChord()) {
                endIsQuarter = (toChord(sp->endElement())->durationType().type()
                                == DurationType::V_QUARTER);
            }
        }
    }
    EXPECT_GE(slurCount, 1) << "Slur from grace to later note must be imported";
    EXPECT_TRUE(graceStart)
        << "Slur startElement must be the grace chord, not the half note";
    EXPECT_TRUE(endIsQuarter)
        << "Slur endElement must be the quarter note that follows the graces";

    delete score;
}

// When an ACCIACCATURA grace and its main note share the same Encore tick, a grace-to-main slur must end
// on the co-located main chord in the same measure, not spill to a rest or note in a later measure.
TEST_F(Tst_OrnamentsSlurs, v0c4_grace_slur_to_main_coloc_correct_endpoint)
{
    MasterScore* score = readEncoreScore("ornaments_v0c4_grace_slur_to_main_coloc.enc");
    ASSERT_NE(score, nullptr);

    int slurCount = 0;
    bool graceStart = false;
    bool endIsNonGrace = false;
    bool endInSameMeasure = false;
    for (auto& [tick, sp] : score->spannerMap().map()) {
        if (!sp->isSlur()) {
            continue;
        }
        ++slurCount;
        if (sp->startElement() && sp->startElement()->isChord()) {
            graceStart = toChord(sp->startElement())->isGrace();
        }
        if (sp->endElement() && sp->endElement()->isChord()) {
            const Chord* endCh = toChord(sp->endElement());
            endIsNonGrace = !endCh->isGrace();
            if (sp->startElement() && sp->startElement()->isChord()) {
                const Chord* startCh = toChord(sp->startElement());
                endInSameMeasure = (startCh->measure() == endCh->measure());
            }
        }
    }
    EXPECT_GE(slurCount, 1) << "Grace-to-main slur must be imported";
    EXPECT_TRUE(graceStart) << "startElement must be the grace chord";
    EXPECT_TRUE(endIsNonGrace) << "endElement must be the non-grace main chord";
    EXPECT_TRUE(endInSameMeasure)
        << "endElement must be in the same measure as the grace, not in a later measure";

    delete score;
}

// v0xC2 counterpart: the pixel-span heuristic must detect grace+regular co-location and force a zero-span
// grace-to-main slur rather than being baited to a later note.
TEST_F(Tst_OrnamentsSlurs, v0c2_grace_slur_to_main_coloc_correct_endpoint)
{
    MasterScore* score = readEncoreScore("ornaments_v0c2_grace_slur_to_main_coloc.enc");
    ASSERT_NE(score, nullptr);

    int slurCount = 0;
    bool graceStart = false;
    bool endIsNonGrace = false;
    bool endAtSameTick = false;
    for (auto& [tick, sp] : score->spannerMap().map()) {
        if (!sp->isSlur()) {
            continue;
        }
        ++slurCount;
        if (sp->startElement() && sp->startElement()->isChord()) {
            graceStart = toChord(sp->startElement())->isGrace();
        }
        if (sp->endElement() && sp->endElement()->isChord()) {
            const Chord* endCh = toChord(sp->endElement());
            endIsNonGrace = !endCh->isGrace();
            // Grace-to-main is a zero-span arc: end chord sits at the slur tick.
            endAtSameTick = (endCh->tick() == sp->tick());
        }
    }
    EXPECT_GE(slurCount, 1) << "Grace-to-main slur must be imported";
    EXPECT_TRUE(graceStart) << "startElement must be the grace chord";
    EXPECT_TRUE(endIsNonGrace) << "endElement must be the non-grace main chord";
    EXPECT_TRUE(endAtSameTick)
        << "endElement must be the co-located main chord (same beat as grace), "
        "not the note at the following beat";

    delete score;
}

// In v0xC4 the main note is serialized before the grace at the same tick, so the grace arrives as a chord
// extension and must be retroactively attached to the already-placed main chord; the slur then anchors to
// the grace. See ENCORE_IMPORTER.md §Grace note ordering (multi-grace groups).
TEST_F(Tst_OrnamentsSlurs, v0c4_grace_after_main_in_binary_slur_anchors_to_grace)
{
    MasterScore* score = readEncoreScore("ornaments_v0c4_grace_after_main_in_binary.enc");
    ASSERT_NE(score, nullptr);

    int slurCount = 0;
    bool graceStart = false;
    for (auto& [tick, sp] : score->spannerMap().map()) {
        if (!sp->isSlur()) {
            continue;
        }
        ++slurCount;
        if (sp->startElement() && sp->startElement()->isChord()) {
            graceStart = toChord(sp->startElement())->isGrace();
        }
    }
    EXPECT_GE(slurCount, 1) << "Grace-to-later slur must be imported";
    EXPECT_TRUE(graceStart)
        << "startElement must be the grace chord, not the regular (main) chord at same tick";

    delete score;
}

// v0xC4 grace-after-main where the slur targets a LATER note: retroactive grace attachment plus the
// xoffset shortcut must yield a grace-to-later slur (grace start, end at the later note), not zero-span.
TEST_F(Tst_OrnamentsSlurs, v0c4_grace_after_main_grace_to_later_slur_anchors_to_grace)
{
    MasterScore* score = readEncoreScore("ornaments_v0c4_grace_after_main_grace_to_later.enc");
    ASSERT_NE(score, nullptr);

    int slurCount = 0;
    bool graceStart = false;
    bool endAtLaterTick = false;
    for (auto& [tick, sp] : score->spannerMap().map()) {
        if (!sp->isSlur()) {
            continue;
        }
        ++slurCount;
        if (sp->startElement() && sp->startElement()->isChord()) {
            graceStart = toChord(sp->startElement())->isGrace();
        }
        if (sp->endElement() && sp->endElement()->isChord()) {
            // Grace-to-later ends after the grace tick, not zero-span at the co-located main chord.
            endAtLaterTick = (toChord(sp->endElement())->tick() > sp->tick());
        }
    }
    EXPECT_GE(slurCount, 1) << "Grace-to-later slur must be imported";
    EXPECT_TRUE(graceStart)
        << "startElement must be the grace chord retroactively attached to mainChord";
    EXPECT_TRUE(endAtLaterTick)
        << "Slur must end at the LATER note (grace-to-later), not zero-span at main";

    delete score;
}

// Same grace-after-main case but with preceding notes advancing the tick cursor: the grace must still be
// retroactively attached to its main chord, not carried forward to the later note.
TEST_F(Tst_OrnamentsSlurs, v0c4_grace_after_main_preceding_notes_slur_anchors_to_grace)
{
    MasterScore* score = readEncoreScore("ornaments_v0c4_grace_after_main_preceding_notes.enc");
    ASSERT_NE(score, nullptr);

    int slurCount = 0;
    bool graceStart = false;
    bool endAtLaterTick = false;
    for (auto& [tick, sp] : score->spannerMap().map()) {
        if (!sp->isSlur()) {
            continue;
        }
        ++slurCount;
        if (sp->startElement() && sp->startElement()->isChord()) {
            graceStart = toChord(sp->startElement())->isGrace();
        }
        if (sp->endElement() && sp->endElement()->isChord()) {
            endAtLaterTick = (toChord(sp->endElement())->tick() > sp->tick());
        }
    }
    EXPECT_GE(slurCount, 1) << "Grace-to-later slur must be imported";
    EXPECT_TRUE(graceStart)
        << "startElement must be the grace chord; preceding context must not prevent "
        "retroactive attachment when grace follows main in binary";
    EXPECT_TRUE(endAtLaterTick)
        << "Slur must end at the later note (grace-to-later), not at the main chord";

    delete score;
}

// The pixel-span reference xoffset must be the grace note's, not the main note's: the slur arc begins at
// the grace, so using the main's larger xoffset would inflate the target and pick a far later note.
TEST_F(Tst_OrnamentsSlurs, v0c4_grace_after_main_slur_arc_starts_at_grace_not_regular)
{
    MasterScore* score = readEncoreScore("ornaments_v0c4_grace_after_main_slur_to_main.enc");
    ASSERT_NE(score, nullptr);

    int slurCount = 0;
    bool graceStart = false;
    bool endAtSameTick = false;
    for (auto& [tick, sp] : score->spannerMap().map()) {
        if (!sp->isSlur()) {
            continue;
        }
        ++slurCount;
        if (sp->startElement() && sp->startElement()->isChord()) {
            graceStart = toChord(sp->startElement())->isGrace();
        }
        if (sp->endElement() && sp->endElement()->isChord()) {
            // Grace-to-main: end at the co-located main chord (same tick), not the later note.
            endAtSameTick = (toChord(sp->endElement())->tick() == sp->tick());
        }
    }
    EXPECT_GE(slurCount, 1) << "Grace-to-main slur must be imported";
    EXPECT_TRUE(graceStart) << "startElement must be the grace chord";
    EXPECT_TRUE(endAtSameTick)
        << "Slur must end at the co-located main chord (grace-to-main), not the "
        "later note, firstNoteXoff must use the grace xoffset, not the regular";

    delete score;
}

// A cross-measure slur's endpoint is resolved by comparing xoffset2 against the target measure's note
// xoffsets, so it lands on the matching interior note (D4), not the last-ChordRest fallback (F4).
TEST_F(Tst_OrnamentsSlurs, cross_measure_slur_endpoint_precision)
{
    MasterScore* score = readEncoreScore("ornaments_cross_measure_slur_precision.enc");
    ASSERT_NE(score, nullptr);
    muse::Ret ret = score->sanityCheck();
    EXPECT_TRUE(ret) << ret.text();

    const Spanner* crossSlur = nullptr;
    const Fraction firstMeasTick = score->firstMeasure()->tick();
    for (auto& [tick, sp] : score->spannerMap().map()) {
        if (!sp->isSlur()) {
            continue;
        }
        if (sp->tick() == firstMeasTick && sp->tick2() > sp->tick()) {
            crossSlur = sp;
            break;
        }
    }
    ASSERT_NE(crossSlur, nullptr) << "Cross-measure slur must be created";
    ASSERT_NE(crossSlur->endElement(), nullptr) << "Slur must have a resolved end element";
    ASSERT_TRUE(crossSlur->endElement()->isChord()) << "Slur end element must be a Chord";

    const int endPitch = toChord(crossSlur->endElement())->notes().back()->pitch();
    EXPECT_EQ(endPitch, 62)
        << "slurXoffset2=15 must select D4 (pitch=62, xoff=15), not F4 (last note, xoff=35)";

    delete score;
}
// Regression: slur end was anchored on the last ChordRest of the alMezuro measure, covering all remaining notes.
// Fix: snap firstNote.xoffset + (xoffset2 - xoffset) to the closest note xoffset in the start measure.
TEST_F(Tst_OrnamentsSlurs, v0c4_slur_pixel_span)
{
    MasterScore* score = readEncoreScore("importer_slur_pixel_span.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_slur_pixel_span.enc";

    Slur* found = nullptr;
    for (const auto& kv : score->spanner()) {
        Spanner* sp = kv.second;
        if (sp && sp->isSlur()) {
            found = toSlur(sp);
            break;
        }
    }
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->tick(), Fraction(0, 1))
        << "slur start at the SLURSTART tick (beat 1)";
    EXPECT_EQ(found->tick2(), Fraction(1, 2))
        << "slur end snaps to note 3 at tick=480 (target xoff 70 matches "
        "note xoff 70 exactly); not the last note of the measure";
    delete score;
}

// Regression: pixel-span heuristic in 6/8 (compound meter) used beatTicks*timeSigDen as whole-note
// ticks instead of durTicks*timeSigDen/timeSigNum.
TEST_F(Tst_OrnamentsSlurs, v0c4_slur_pixel_span_6_8)
{
    MasterScore* score = readEncoreScore("importer_slur_pixel_span_6_8.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_slur_pixel_span_6_8.enc";

    Slur* found = nullptr;
    for (const auto& kv : score->spanner()) {
        Spanner* sp = kv.second;
        if (sp && sp->isSlur()) {
            found = toSlur(sp);
            break;
        }
    }
    ASSERT_NE(found, nullptr) << "A slur must be created";
    EXPECT_EQ(found->tick(), Fraction(1, 8))
        << "slur start must be at the 2nd note (enc_tick=120 = 1/8 from measure start)";
    EXPECT_EQ(found->tick2(), Fraction(1, 4))
        << "slur end must be at note 3 (enc_tick=240 = 1/4); "
        "with wrong formula it lands at note 4 (enc_tick=360 = 3/8)";
    delete score;
}

// Regression: SLURSTART xoffset > 127 must be treated as unsigned for pixel-span computation.
TEST_F(Tst_OrnamentsSlurs, v0c4_slur_xoffset_unsigned)
{
    MasterScore* score = readEncoreScore("importer_slur_xoffset_unsigned.enc");
    ASSERT_NE(score, nullptr) << "Failed to load importer_slur_xoffset_unsigned.enc";

    Slur* found = nullptr;
    for (const auto& kv : score->spanner()) {
        Spanner* sp = kv.second;
        if (sp && sp->isSlur()) {
            found = toSlur(sp);
            break;
        }
    }
    ASSERT_NE(found, nullptr) << "A slur must be created";
    EXPECT_EQ(found->tick(), Fraction(1, 4))
        << "slur starts at note 2 (tick=240 = 1/4)";
    EXPECT_EQ(found->tick2(), Fraction(1, 2))
        << "slur ends at note 3 (tick=480 = 1/2); with signed xoffset it lands too late";
    delete score;
}

// Regression: pixel-span heuristic skips cross-measure slurs (alMezuro >= 1).
// Pins the fallback: alMezuro=1 slur must anchor on the last ChordRest of the target measure.
TEST_F(Tst_OrnamentsSlurs, v0c4_slur_cross_measure_fallback)
{
    MasterScore* score = readEncoreScore("importer_slur_cross_measure_fallback.enc");
    ASSERT_NE(score, nullptr)
        << "Failed to load importer_slur_cross_measure_fallback.enc";

    Slur* found = nullptr;
    for (const auto& kv : score->spanner()) {
        Spanner* sp = kv.second;
        if (sp && sp->isSlur()) {
            found = toSlur(sp);
            break;
        }
    }
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->tick(), Fraction(0, 1))
        << "slur start at the SLURSTART tick (m1 beat 1)";
    EXPECT_EQ(found->tick2(), Fraction(7, 4))
        << "cross-measure slur must fall back to the last ChordRest of "
        "the alMezuro target measure (m2 beat 4 = absolute tick 7/4)";
    delete score;
}

// Two ottava spanners: 8va in m0, 8vb in m1.
// resolveOttavas pins each endpoint to the next ottava's startTick, or scoreEnd.
TEST_F(Tst_OrnamentsSlurs, v0c4_ottava_two_spanners)
{
    MasterScore* score = readEncoreScore("ornaments_ottava_two_spanners.enc");
    ASSERT_NE(score, nullptr) << "Failed to load ornaments_ottava_two_spanners.enc";

    std::vector<Ottava*> ottavas;
    for (const auto& kv : score->spanner()) {
        Spanner* sp = kv.second;
        if (sp && sp->isOttava()) {
            ottavas.push_back(toOttava(sp));
        }
    }
    ASSERT_EQ(ottavas.size(), 2u) << "expected exactly 2 ottava spanners";

    std::sort(ottavas.begin(), ottavas.end(), [](Ottava* a, Ottava* b) {
        return a->tick() < b->tick();
    });

    EXPECT_EQ(ottavas[0]->ottavaType(), OttavaType::OTTAVA_8VA);
    EXPECT_EQ(ottavas[0]->tick(),  Fraction(0, 1));
    EXPECT_EQ(ottavas[0]->tick2(), Fraction(1, 1));

    EXPECT_EQ(ottavas[1]->ottavaType(), OttavaType::OTTAVA_8VB);
    EXPECT_EQ(ottavas[1]->tick(),  Fraction(1, 1));
    EXPECT_EQ(ottavas[1]->tick2(), Fraction(6, 1));

    delete score;
}

// Regression: when any slur's +16 measure-count points past the last measure, the whole file's field is
// unreliable, so every slur (even plausible-looking counts) must resolve inside its own bar.
TEST_F(Tst_OrnamentsSlurs, v0c2_unreliable_slur_count_stays_in_measure)
{
    MasterScore* score = readEncoreScore("ornaments_v0c2_unreliable_slur_count.enc");
    ASSERT_NE(score, nullptr) << "Failed to load ornaments_v0c2_unreliable_slur_count.enc";

    int total = 0;
    int crossMeasure = 0;
    for (auto it : score->spanner()) {
        Spanner* sp = it.second;
        if (!sp || !sp->isSlur()) {
            continue;
        }
        ++total;
        Measure* m1 = score->tick2measure(sp->tick());
        Measure* m2 = score->tick2measure(sp->tick2());
        if (m1 && m2 && m1 != m2) {
            ++crossMeasure;
        }
    }
    EXPECT_EQ(total, 2) << "both slurs must import";
    EXPECT_EQ(crossMeasure, 0)
        << "a plausible-looking count must not extend a slur past its bar when the file's "
        "+16 field is unreliable";
    delete score;
}

// Regression: some v0xC2 files store a per-staff CONSTANT in the slur +16 field, so every slur carries the
// same in-range value regardless of start. A repeated large span (>=3) across different start measures
// marks +16 unreliable, so each slur resolves inside its own bar instead of drawing a phantom span.
TEST_F(Tst_OrnamentsSlurs, v0c2_constant_slur_count_stays_in_measure)
{
    MasterScore* score = readEncoreScore("ornaments_v0c2_constant_slur_count.enc");
    ASSERT_NE(score, nullptr) << "Failed to load ornaments_v0c2_constant_slur_count.enc";

    int total = 0;
    int crossMeasure = 0;
    for (auto it : score->spanner()) {
        Spanner* sp = it.second;
        if (!sp || !sp->isSlur()) {
            continue;
        }
        ++total;
        Measure* m1 = score->tick2measure(sp->tick());
        Measure* m2 = score->tick2measure(sp->tick2());
        if (m1 && m2 && m1 != m2) {
            ++crossMeasure;
        }
    }
    EXPECT_EQ(total, 2) << "both slurs must import";
    EXPECT_EQ(crossMeasure, 0)
        << "a constant +16 value repeated across start measures must not extend the slurs "
        "into an 11-measure phantom span";
    delete score;
}
