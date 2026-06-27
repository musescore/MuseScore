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

// Tempo import and no-crash smoke loads: ghost-measure truncation and v0xC2 tempo beat-unit decoding.

#include <gtest/gtest.h>

#include <cmath>

#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/tempotext.h"

#include "testbase.h"

static const QString ENC_DIR(QString(iex_encore_tests_DATA_ROOT) + "/data/");

using namespace mu::engraving;

class Tst_Tempo : public ::testing::Test, public MTest
{
protected:
    void SetUp() override { setRootDir(ENC_DIR); }
};

// Files can carry stale MEAS blocks past the rendered count; only the real ones must import.
// See ENCORE_FORMAT.md §Header.
TEST_F(Tst_Tempo, v0c4_header_measure_count_truncates_ghost_measures)
{
    MasterScore* score = readEncoreScore(
        "importer_header_measure_count_truncates_ghost_measures.enc");
    ASSERT_NE(score, nullptr)
        << "Failed to load importer_header_measure_count_truncates_ghost_measures.enc";
    int measureCount = 0;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (mb->isMeasure()) {
            ++measureCount;
        }
    }
    EXPECT_EQ(measureCount, 2)
        << "ghost MEAS blocks past header.measureCount must be dropped";
    delete score;
}

TEST_F(Tst_Tempo, beethoven_no_crash)
{
    MasterScore* score = readEncoreScore("notes_corrupted.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_GT(score->nmeasures(), 0);
    delete score;
}

TEST_F(Tst_Tempo, twelve_instrument_score_no_crash)
{
    MasterScore* score = readEncoreScore("notes_triplets.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_GT(score->nmeasures(), 0);
    delete score;
}

// No sanityCheck: swing timing files produce slight measure shortfalls (by design).
TEST_F(Tst_Tempo, swing_timing_file_no_crash)
{
    MasterScore* score = readEncoreScore("notes_swing.enc");
    ASSERT_NE(score, nullptr);
    EXPECT_GT(score->nmeasures(), 0);
    delete score;
}

// Regression: older-layout v0xC2 tempo marks carry the beat unit in a different slot; without it
// an "eighth = 240" mark in 6/8 was read as a dotted quarter and played 3x too fast.
// See ENCORE_FORMAT.md §Note element (Tempo beat unit).
TEST_F(Tst_Tempo, v0c2_older_layout_tempo_beat_unit_at_plus26)
{
    MasterScore* score = readEncoreScore("tempo_v0c2_eighth_beat_unit.enc");
    ASSERT_NE(score, nullptr) << "Failed to load tempo_v0c2_eighth_beat_unit.enc";

    TempoText* tt = nullptr;
    for (MeasureBase* mb = score->first(); mb && !tt; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        Measure* m = toMeasure(mb);
        for (Segment* s = m->first(SegmentType::ChordRest); s && !tt; s = s->next(SegmentType::ChordRest)) {
            for (EngravingItem* e : s->annotations()) {
                if (e && e->isTempoText()) {
                    tt = toTempoText(e);
                    break;
                }
            }
        }
    }
    ASSERT_NE(tt, nullptr) << "no TempoText emitted for the v0xC2 tempo mark";

    EXPECT_TRUE(tt->xmlText().contains(u"metNote8thUp"))
        << "expected eighth beat unit, got: " << tt->xmlText().toStdString();
    EXPECT_FALSE(tt->xmlText().contains(u"metAugmentationDot"))
        << "beat unit must not be dotted: " << tt->xmlText().toStdString();
    // eighth = 240 -> 120 quarter/min -> 2 beats/sec.
    EXPECT_EQ(std::lround(tt->tempo().val), 2)
        << "playback tempo must be 2 beats/sec, got " << tt->tempo().val;

    delete score;
}
