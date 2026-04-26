/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include <gtest/gtest.h>

#include <algorithm>

#include "engraving/dom/chord.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/segment.h"
#include "engraving/editing/transpose.h"

#include "mocks/engravingconfigurationmock.h"

#include "utils/scorerw.h"

#include "global/modularity/ioc.h"

using namespace mu::engraving;
using ECMock = ::testing::NiceMock<EngravingConfigurationMock>;

static const String DATA_DIR("tab_transpose_data/");

struct TabNote {
    int string = -1;
    int fret = -1;
    int pitch = -1;
    bool dead = false;
};

static std::vector<TabNote> collectTabNotes(MasterScore* score)
{
    std::vector<TabNote> result;
    Measure* m = score->firstMeasure();
    if (!m) {
        return result;
    }
    for (Segment& seg : m->segments()) {
        if (!seg.isChordRestType()) {
            continue;
        }
        for (track_idx_t trk = 0; trk < VOICES; ++trk) {
            EngravingItem* el = seg.element(trk);
            if (!el || !el->isChord()) {
                continue;
            }
            Chord* chord = toChord(el);
            for (Note* note : chord->notes()) {
                result.push_back({ note->string(), note->fret(), note->pitch(), note->deadNote() });
            }
        }
    }
    return result;
}

static void transposeScore(MasterScore* score, int semitones)
{
    // allIntervals indices for semitones 0-11: P1 m2 M2 m3 M3 P4 TT P5 m6 M6 m7 M7
    // Mirrors the internal lookup table used by Interval::chromatic2diatonic.
    static constexpr int kIntervalIdx[12] = { 0, 3, 4, 7, 8, 11, 12, 14, 17, 18, 21, 22 };

    score->cmdSelectAll();
    score->startCmd(TranslatableString::untranslatable("fretting test"));

    const TransposeDirection dir = semitones < 0 ? TransposeDirection::DOWN : TransposeDirection::UP;
    const int abs = std::abs(semitones);
    const int remainder = abs % 12;
    const int octaves = abs / 12;

    if (remainder != 0) {
        Transpose::transpose(score, TransposeMode::BY_INTERVAL, dir, Key::C,
                             kIntervalIdx[remainder], true, true, true);
    }
    for (int i = 0; i < octaves; i++) {
        Transpose::transpose(score, TransposeMode::BY_INTERVAL, dir, Key::C, 25, true, true, true);
    }

    score->endCmd();
    score->doLayout();
}

class Engraving_TabTransposeTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto mock = muse::modularity::globalIoc()->resolve<IEngravingConfiguration>("utests");
        m_mock = dynamic_cast<ECMock*>(mock.get());
        ASSERT_NE(m_mock, nullptr);
    }

    void TearDown() override
    {
        if (m_mock) {
            ON_CALL(*m_mock, preferSameStringForTranspose()).WillByDefault(::testing::Return(false));
            ON_CALL(*m_mock, negativeFretsAllowed()).WillByDefault(::testing::Return(false));
        }
    }

    void setFrettingFlags(bool preferSameStringForTranspose, bool negativeFretsAllowed)
    {
        ASSERT_NE(m_mock, nullptr);
        ON_CALL(*m_mock, preferSameStringForTranspose()).WillByDefault(::testing::Return(preferSameStringForTranspose));
        ON_CALL(*m_mock, negativeFretsAllowed()).WillByDefault(::testing::Return(negativeFretsAllowed));
    }

    ECMock* m_mock = nullptr;
};

static void assertDeadNotesUnchangedByTransposeImpl(MasterScore* score)
{
    ASSERT_TRUE(score);

    auto notesBefore = collectTabNotes(score);
    ASSERT_EQ(notesBefore.size(), 4u);

    // Snapshot all dead notes before transpose.
    std::vector<TabNote> deadBefore;
    std::vector<TabNote> liveBefore;
    for (const auto& n : notesBefore) {
        if (n.dead) {
            deadBefore.push_back(n);
        } else {
            liveBefore.push_back(n);
        }
    }
    ASSERT_EQ(deadBefore.size(), 2u) << "fixture must contain 2 dead notes";
    ASSERT_EQ(liveBefore.size(), 2u) << "fixture must contain 2 live notes";

    constexpr int kSemitones = 2;
    transposeScore(score, kSemitones);

    auto notesAfter = collectTabNotes(score);
    ASSERT_EQ(notesAfter.size(), 4u);

    // All dead notes must still exist and remain unchanged.
    std::vector<TabNote> deadAfter;
    std::vector<TabNote> liveAfter;
    for (const auto& n : notesAfter) {
        if (n.dead) {
            deadAfter.push_back(n);
        } else {
            liveAfter.push_back(n);
        }
    }
    ASSERT_EQ(deadAfter.size(), deadBefore.size()) << "dead notes count changed after transpose";
    ASSERT_EQ(liveAfter.size(), liveBefore.size()) << "live notes count changed after transpose";

    std::vector<TabNote> deadAfterRemaining = deadAfter;
    for (const auto& db : deadBefore) {
        auto it = std::find_if(deadAfterRemaining.begin(), deadAfterRemaining.end(), [&](const TabNote& da) {
            return da.string == db.string && da.fret == db.fret && da.pitch == db.pitch && da.dead == db.dead;
        });
        EXPECT_NE(it, deadAfterRemaining.end())
            << "dead note changed or disappeared after transpose (string=" << db.string
            << ", fret=" << db.fret << ", pitch=" << db.pitch << ")";
        if (it != deadAfterRemaining.end()) {
            deadAfterRemaining.erase(it);
        }
    }

    // Live notes must all be transposed by the expected interval.
    std::vector<int> livePitchesBefore, livePitchesAfter;
    for (const auto& n : liveBefore) {
        livePitchesBefore.push_back(n.pitch);
    }
    for (const auto& n : liveAfter) {
        livePitchesAfter.push_back(n.pitch);
    }
    std::sort(livePitchesBefore.begin(), livePitchesBefore.end());
    std::sort(livePitchesAfter.begin(),  livePitchesAfter.end());
    for (size_t i = 0; i < livePitchesBefore.size(); ++i) {
        EXPECT_EQ(livePitchesAfter[i], livePitchesBefore[i] + kSemitones)
            << "live note pitch did not change by expected transpose interval";
    }
}

// Dead notes must be completely ignored during transpose — their string,
// fret, and pitch must not change. Verify for both the typical configuration
// (flags false) and the alternate behavior (flags true).
TEST_F(Engraving_TabTransposeTests, deadNotesUnchangedByTranspose)
{
    struct FlagCase {
        bool preferSameString;
        bool negativeFretsAllowed;
    };
    constexpr FlagCase flagCases[] = {
        { false, false },
        { true,  true },
    };

    for (const auto& c : flagCases) {
        setFrettingFlags(c.preferSameString, c.negativeFretsAllowed);
        MasterScore* score = ScoreRW::readScore(DATA_DIR + "dead_notes.mscx");
        assertDeadNotesUnchangedByTransposeImpl(score);
        delete score;
    }
}
