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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include "mpe/tests/mocks/articulationprofilesrepositorymock.h"

#include "engraving/automation/automationdata.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/note.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/staff.h"

#include "engraving/playback/noteplaybackinfo.h"
#include "engraving/playback/playbackmarkings.h"
#include "engraving/playback/playbackmodel.h"

#include "utils/scorerw.h"
#include "automation/utils/automationtestutils.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;

using namespace mu::engraving;
using namespace muse;

static const String NOTE_INFO_DIR(u"playback/noteplaybackinfo_data/");
static const String SPANNERS_SCORE(u"playback/playbackmodel_data/spanners/spanners.mscx");
static const String TEMPO_SCORE(u"automation/data/tempo.mscx");

static constexpr int QUARTER_TICKS = 480;
static constexpr int MEASURE_TICKS = 4 * QUARTER_TICKS; // all the scores here are in 4/4

//! influences.mscx: Piano (staves 1-2) and Flute (staff 3), 4/4, quarter notes throughout.
//! m1: tempo 120 and mp for the whole piano on staff 1
//! m2: staff 1 crescendo from beat 2 to beat 4; staff 2 ff for that staff only on beat 1
//! m3: staff 1 f + diminuendo from beat 1 to beat 3, p on beat 3, fermata on beat 4
//! m4-m5: repeated twice; staff 1 pp on m5 beat 4; flute 8va over m4
static const String INFLUENCES_SCORE(NOTE_INFO_DIR + u"influences.mscx");

static constexpr staff_idx_t PIANO_RH = 0;
static constexpr staff_idx_t PIANO_LH = 1;
static constexpr staff_idx_t FLUTE = 2;

namespace noteplaybackinfo_tests {
int tickAt(int measureIdx, int beatIdx)
{
    return (measureIdx * MEASURE_TICKS) + (beatIdx * QUARTER_TICKS);
}

//! The top note of the chord at (0-based) measureIdx, beatIdx on the staff
const Note* noteAt(
    const Score* score,
    staff_idx_t staffIdx,
    int measureIdx,
    int beatIdx)
{
    const Segment* segment = score->tick2segment(Fraction::fromTicks(tickAt(measureIdx, beatIdx)), true, SegmentType::ChordRest);

    if (!segment) {
        return nullptr;
    }

    const EngravingItem* item = segment->element(staff2track(staffIdx));

    return (item && item->isChord())
           ? toChord(item)->upNote()
           : nullptr;
}

//! The annotation of the given type at the tick on the staff
const EngravingItem* annotationAt(
    const Score* score,
    staff_idx_t staffIdx,
    int tick,
    ElementType type)
{
    const Segment* segment = score->tick2segment(Fraction::fromTicks(tick), true, SegmentType::ChordRest);

    if (!segment) {
        return nullptr;
    }

    for (const EngravingItem* annotation : segment->annotations()) {
        if ((annotation->type() == type) && (annotation->staffIdx() == staffIdx)) {
            return annotation;
        }
    }

    return nullptr;
}

//! The spanner of the given type starting at the tick on the staff
const Spanner* spannerAt(
    const Score* score,
    staff_idx_t staffIdx,
    int tick,
    ElementType type)
{
    for (const auto& interval : score->spannerMap().findOverlapping(tick, tick)) {
        const Spanner* spanner = interval.value;

        if ((spanner->type() == type) && (spanner->staffIdx() == staffIdx) && (spanner->tick().ticks() == tick)) {
            return spanner;
        }
    }

    return nullptr;
}

std::vector<const EngravingItem*> itemsOf(const std::vector<NotePlaybackSource>& sources)
{
    std::vector<const EngravingItem*> items;

    for (const NotePlaybackSource& source : sources) {
        items.push_back(source.item);
    }

    return items;
}

bool hasMarkingOfType(const NotePlaybackInfo& info, ElementType type)
{
    return std::ranges::any_of(info.markings, [type](const NotePlaybackMarking& marking) {
        return marking.item && (marking.item->type() == type);
    });
}
}

using namespace noteplaybackinfo_tests;

class Engraving_NotePlaybackInfoTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        using namespace muse::mpe;

        // Playback skips articulation types that have no pattern in the profile, so give every type one
        ArticulationPatternSegment segment(ArrangementPattern(HUNDRED_PERCENT /*durationFactor*/, 0 /*timestampOffset*/),
                                           PitchPattern(ArticulationMap::EXPECTED_SIZE, TEN_PERCENT, 0),
                                           ExpressionPattern(ArticulationMap::EXPECTED_SIZE, TEN_PERCENT, 0));

        ArticulationPattern pattern;
        pattern.emplace(0, std::move(segment));

        const ArticulationsProfilePtr profile = std::make_shared<ArticulationsProfile>();

        for (int type = static_cast<int>(ArticulationType::Standard); type < static_cast<int>(ArticulationType::Last); ++type) {
            profile->setPattern(static_cast<ArticulationType>(type), pattern);
        }

        m_repositoryMock = std::make_shared<NiceMock<ArticulationProfilesRepositoryMock> >();
        ON_CALL(*m_repositoryMock, defaultProfile(_)).WillByDefault(Return(profile));
    }

    void TearDown() override
    {
        // The model refers to the score, so it goes first
        m_model.reset();
        m_score.reset();
    }

    MasterScore* load(const String& path)
    {
        m_score.reset(ScoreRW::readScore(path));

        if (!m_score) {
            return nullptr;
        }

        m_score->initAutomation();

        m_model = std::make_unique<PlaybackModel>(modularity::globalCtx());
        m_model->profilesRepository.set(m_repositoryMock);
        m_model->load(m_score.get());

        return m_score.get();
    }

    NotePlaybackInfo infoAt(
        staff_idx_t staffIdx,
        int measureIdx,
        int beatIdx)
    {
        const Note* note = noteAt(m_score.get(), staffIdx, measureIdx, beatIdx);
        EXPECT_TRUE(note) << "no note on staff " << staffIdx << " at m" << (measureIdx + 1) << " beat " << (beatIdx + 1);

        return m_model->notePlaybackInfo(note);
    }

    std::unique_ptr<MasterScore> m_score;
    std::unique_ptr<PlaybackModel> m_model;
    std::shared_ptr<NiceMock<muse::mpe::ArticulationProfilesRepositoryMock> > m_repositoryMock;
};

//! A dynamic that applies to the whole instrument sets the level on the instrument's other staves too
TEST_F(Engraving_NotePlaybackInfoTests, Dynamic_WholeInstrument_AppliesToOtherStaff)
{
    // [GIVEN] mp on the right-hand staff, for the whole piano
    const MasterScore* score = load(INFLUENCES_SCORE);
    ASSERT_TRUE(score);

    const EngravingItem* mp = annotationAt(score, PIANO_RH, tickAt(0, 0), ElementType::DYNAMIC);
    ASSERT_TRUE(mp);

    // [WHEN] A left-hand note under it
    const NotePlaybackInfo info = infoAt(PIANO_LH, 0, 1);
    ASSERT_TRUE(info.isValid());
    ASSERT_EQ(info.passes.size(), 1);

    // [THEN] The mp sets its level
    EXPECT_FALSE(info.passes.front().isDefaultDynamic);
    EXPECT_EQ(itemsOf(info.passes.front().dynamicSources), std::vector<const EngravingItem*> { mp });
}

//! A dynamic for one staff only doesn't reach the instrument's other staves, nor another instrument
TEST_F(Engraving_NotePlaybackInfoTests, Dynamic_OneStaff_OtherStaffUnaffected)
{
    // [GIVEN] mp for the whole piano (m1), then ff for the left hand only (m2)
    const MasterScore* score = load(INFLUENCES_SCORE);
    ASSERT_TRUE(score);

    const EngravingItem* mp = annotationAt(score, PIANO_RH, tickAt(0, 0), ElementType::DYNAMIC);
    const EngravingItem* ff = annotationAt(score, PIANO_LH, tickAt(1, 0), ElementType::DYNAMIC);
    ASSERT_TRUE(mp);
    ASSERT_TRUE(ff);

    // [THEN] The left hand plays ff, the right hand stays at mp
    const NotePlaybackInfo leftHand = infoAt(PIANO_LH, 1, 0);
    ASSERT_TRUE(leftHand.isValid());
    EXPECT_EQ(itemsOf(leftHand.passes.front().dynamicSources), std::vector<const EngravingItem*> { ff });

    const NotePlaybackInfo rightHand = infoAt(PIANO_RH, 1, 0);
    ASSERT_TRUE(rightHand.isValid());
    EXPECT_EQ(itemsOf(rightHand.passes.front().dynamicSources), std::vector<const EngravingItem*> { mp });

    // [THEN] The flute has no dynamics of its own: default level, no source
    const NotePlaybackInfo flute = infoAt(FLUTE, 1, 0);
    ASSERT_TRUE(flute.isValid());
    EXPECT_TRUE(flute.passes.front().isDefaultDynamic);
    EXPECT_TRUE(flute.passes.front().dynamicSources.empty());
}

//! A hairpin has no effect yet on the note it starts at, with or without a dynamic there
TEST_F(Engraving_NotePlaybackInfoTests, Hairpin_NotListedAtItsStart)
{
    const MasterScore* score = load(INFLUENCES_SCORE);
    ASSERT_TRUE(score);

    const EngravingItem* mp = annotationAt(score, PIANO_RH, tickAt(0, 0), ElementType::DYNAMIC);
    const EngravingItem* f = annotationAt(score, PIANO_RH, tickAt(2, 0), ElementType::DYNAMIC);
    ASSERT_TRUE(mp);
    ASSERT_TRUE(f);
    ASSERT_TRUE(spannerAt(score, PIANO_RH, tickAt(1, 1), ElementType::HAIRPIN));
    ASSERT_TRUE(spannerAt(score, PIANO_RH, tickAt(2, 0), ElementType::HAIRPIN));

    // [THEN] m2 beat 2, where the crescendo starts with no dynamic: still the mp before it
    const NotePlaybackInfo crescStart = infoAt(PIANO_RH, 1, 1);
    ASSERT_TRUE(crescStart.isValid());
    EXPECT_EQ(itemsOf(crescStart.passes.front().dynamicSources), std::vector<const EngravingItem*> { mp });
    EXPECT_TRUE(crescStart.passes.front().dynamicOrigins.empty());

    // [THEN] m3 beat 1, where the diminuendo starts on an f: just the f
    const NotePlaybackInfo dimStart = infoAt(PIANO_RH, 2, 0);
    ASSERT_TRUE(dimStart.isValid());
    EXPECT_EQ(itemsOf(dimStart.passes.front().dynamicSources), std::vector<const EngravingItem*> { f });
    EXPECT_TRUE(dimStart.passes.front().dynamicOrigins.empty());
}

//! Inside a hairpin: the hairpin sets the level, starting from the dynamic before it
TEST_F(Engraving_NotePlaybackInfoTests, Hairpin_InProgress_ListsHairpinAndOrigin)
{
    const MasterScore* score = load(INFLUENCES_SCORE);
    ASSERT_TRUE(score);

    const EngravingItem* mp = annotationAt(score, PIANO_RH, tickAt(0, 0), ElementType::DYNAMIC);
    const EngravingItem* f = annotationAt(score, PIANO_RH, tickAt(2, 0), ElementType::DYNAMIC);
    const Spanner* cresc = spannerAt(score, PIANO_RH, tickAt(1, 1), ElementType::HAIRPIN);
    const Spanner* dim = spannerAt(score, PIANO_RH, tickAt(2, 0), ElementType::HAIRPIN);
    ASSERT_TRUE(mp && f && cresc && dim);

    const NotePlaybackInfo inCresc = infoAt(PIANO_RH, 1, 2);
    ASSERT_TRUE(inCresc.isValid());
    EXPECT_THAT(itemsOf(inCresc.passes.front().dynamicSources), ::testing::Contains(cresc));
    EXPECT_EQ(itemsOf(inCresc.passes.front().dynamicOrigins), std::vector<const EngravingItem*> { mp });

    const NotePlaybackInfo inDim = infoAt(PIANO_RH, 2, 1);
    ASSERT_TRUE(inDim.isValid());
    EXPECT_THAT(itemsOf(inDim.passes.front().dynamicSources), ::testing::Contains(dim));
    EXPECT_EQ(itemsOf(inDim.passes.front().dynamicOrigins), std::vector<const EngravingItem*> { f });
}

//! At a hairpin's end, the dynamic it ends on sets the level
TEST_F(Engraving_NotePlaybackInfoTests, Hairpin_End_ListsEndDynamic)
{
    const MasterScore* score = load(INFLUENCES_SCORE);
    ASSERT_TRUE(score);

    const EngravingItem* p = annotationAt(score, PIANO_RH, tickAt(2, 2), ElementType::DYNAMIC);
    ASSERT_TRUE(p);

    const NotePlaybackInfo info = infoAt(PIANO_RH, 2, 2);
    ASSERT_TRUE(info.isValid());
    EXPECT_EQ(itemsOf(info.passes.front().dynamicSources), std::vector<const EngravingItem*> { p });
    EXPECT_TRUE(info.passes.front().dynamicOrigins.empty());
}

//! A repeated note can get its level from different dynamics on each pass
TEST_F(Engraving_NotePlaybackInfoTests, Repeats_DifferentDynamicPerPass)
{
    // [GIVEN] m4-m5 repeated; p (m3) before the repeat, pp at the end of it (m5 beat 4)
    const MasterScore* score = load(INFLUENCES_SCORE);
    ASSERT_TRUE(score);

    const EngravingItem* p = annotationAt(score, PIANO_RH, tickAt(2, 2), ElementType::DYNAMIC);
    const EngravingItem* pp = annotationAt(score, PIANO_RH, tickAt(4, 3), ElementType::DYNAMIC);
    ASSERT_TRUE(p && pp);

    // [WHEN] The first note of the repeat
    const NotePlaybackInfo info = infoAt(PIANO_RH, 3, 0);
    ASSERT_TRUE(info.isValid());
    ASSERT_EQ(info.passes.size(), 2);

    // [THEN] p on the first pass, pp on the second, and the second pass is a repeat later
    EXPECT_EQ(info.passes[0].passNumber, 1);
    EXPECT_EQ(info.passes[1].passNumber, 2);
    EXPECT_EQ(info.passes[1].utick - info.passes[0].utick, 2 * MEASURE_TICKS);

    EXPECT_EQ(itemsOf(info.passes[0].dynamicSources), std::vector<const EngravingItem*> { p });
    EXPECT_EQ(itemsOf(info.passes[1].dynamicSources), std::vector<const EngravingItem*> { pp });
    EXPECT_GT(info.passes[0].dynamicValue, info.passes[1].dynamicValue);
}

//! An ottava shifts the sounding pitch and is listed as the reason
TEST_F(Engraving_NotePlaybackInfoTests, Ottava_ShiftsPitch)
{
    const MasterScore* score = load(INFLUENCES_SCORE);
    ASSERT_TRUE(score);

    const Spanner* ottava = spannerAt(score, FLUTE, tickAt(3, 0), ElementType::OTTAVA);
    ASSERT_TRUE(ottava);

    const NotePlaybackInfo shifted = infoAt(FLUTE, 3, 1);
    ASSERT_TRUE(shifted.isValid());
    ASSERT_EQ(shifted.pitchShifts.size(), 1);
    EXPECT_EQ(shifted.pitchShifts.front().item, ottava);
    EXPECT_EQ(shifted.pitchShifts.front().semitones, 12);
    EXPECT_EQ(shifted.soundingPitch, shifted.note->pitch() + 12);

    // [THEN] Nothing shifts the note before it
    const NotePlaybackInfo unshifted = infoAt(FLUTE, 2, 3);
    ASSERT_TRUE(unshifted.isValid());
    EXPECT_TRUE(unshifted.pitchShifts.empty());
    EXPECT_EQ(unshifted.soundingPitch, unshifted.note->pitch());
}

//! The tempo marking sets the tempo, and a fermata is listed among the note's markings
TEST_F(Engraving_NotePlaybackInfoTests, Tempo_And_Fermata)
{
    const MasterScore* score = load(INFLUENCES_SCORE);
    ASSERT_TRUE(score);

    const EngravingItem* tempo = annotationAt(score, PIANO_RH, tickAt(0, 0), ElementType::TEMPO_TEXT);
    const EngravingItem* fermata = annotationAt(score, PIANO_RH, tickAt(2, 3), ElementType::FERMATA);
    ASSERT_TRUE(tempo && fermata);

    // [THEN] A plain note: the tempo marking at 120 bpm
    const NotePlaybackInfo plain = infoAt(PIANO_RH, 1, 0);
    ASSERT_TRUE(plain.isValid());
    EXPECT_FALSE(plain.passes.front().isDefaultTempo);
    EXPECT_EQ(itemsOf(plain.passes.front().tempoSources), std::vector<const EngravingItem*> { tempo });
    EXPECT_NEAR(plain.passes.front().tempoValue.val, 2.0, 0.001);

    // [THEN] The note under the fermata lists it
    const NotePlaybackInfo held = infoAt(PIANO_RH, 2, 3);
    ASSERT_TRUE(held.isValid());
    EXPECT_TRUE(hasMarkingOfType(held, ElementType::FERMATA));

    const std::vector<const EngravingItem*> listed = listedItems(held);
    EXPECT_THAT(listed, ::testing::Contains(fermata));
    EXPECT_FALSE(hasMarkingOfType(plain, ElementType::FERMATA));
}

//! A ritardando has no effect on the note it starts at; the tempo before it is what plays
TEST_F(Engraving_NotePlaybackInfoTests, Ritardando_NotListedAtItsStart)
{
    // [GIVEN] tempo.mscx: 180 bpm in m1, ritardando over m2 (whole notes)
    const MasterScore* score = load(TEMPO_SCORE);
    ASSERT_TRUE(score);

    const EngravingItem* tempo = annotationAt(score, 0, tickAt(0, 0), ElementType::TEMPO_TEXT);
    ASSERT_TRUE(tempo);
    ASSERT_TRUE(spannerAt(score, 0, tickAt(1, 0), ElementType::GRADUAL_TEMPO_CHANGE));

    const NotePlaybackInfo info = infoAt(0, 1, 0);
    ASSERT_TRUE(info.isValid());
    ASSERT_FALSE(info.passes.empty());
    EXPECT_EQ(itemsOf(info.passes.front().tempoSources), std::vector<const EngravingItem*> { tempo });
    EXPECT_TRUE(info.passes.front().tempoOrigins.empty());
    EXPECT_NEAR(info.passes.front().tempoValue.val, 3.0, 0.001);
}

//! Pedal, trill and slur are listed as markings of the notes under them
TEST_F(Engraving_NotePlaybackInfoTests, Spanners_ListedAsMarkings)
{
    // [GIVEN] spanners.mscx: pedal over m1 beats 2-3, trill over m2 beats 2-3, slur from m3 beat 2 to beat 3
    const MasterScore* score = load(SPANNERS_SCORE);
    ASSERT_TRUE(score);

    const NotePlaybackInfo pedalled = infoAt(0, 0, 1);
    ASSERT_TRUE(pedalled.isValid());
    EXPECT_TRUE(hasMarkingOfType(pedalled, ElementType::PEDAL));

    const NotePlaybackInfo trilled = infoAt(0, 1, 1);
    ASSERT_TRUE(trilled.isValid());
    EXPECT_TRUE(hasMarkingOfType(trilled, ElementType::TRILL));

    const NotePlaybackInfo slurred = infoAt(0, 2, 1);
    ASSERT_TRUE(slurred.isValid());
    EXPECT_TRUE(hasMarkingOfType(slurred, ElementType::SLUR));

    // [THEN] The first note is under none of them
    const NotePlaybackInfo plain = infoAt(0, 0, 0);
    ASSERT_TRUE(plain.isValid());
    EXPECT_FALSE(hasMarkingOfType(plain, ElementType::PEDAL));
    EXPECT_FALSE(hasMarkingOfType(plain, ElementType::TRILL));
    EXPECT_FALSE(hasMarkingOfType(plain, ElementType::SLUR));
}

//! A point drawn in the automation lane, and a marking's point changed there, are both listed as such
TEST_F(Engraving_NotePlaybackInfoTests, AutomationLaneEdits)
{
    MasterScore* score = load(INFLUENCES_SCORE);
    ASSERT_TRUE(score);

    const EngravingItem* mp = annotationAt(score, PIANO_RH, tickAt(0, 0), ElementType::DYNAMIC);
    ASSERT_TRUE(mp);

    const AutomationCurveKey key = AutomationCurveKey::staff(AutomationType::Dynamics, score->staff(PIANO_RH)->id());
    const AutomationCurve& curve = score->automationData()->curve(key);

    const auto mpPoint = curve.find(tickAt(0, 0));
    ASSERT_TRUE(mpPoint != curve.cend());

    // [WHEN] The mp's point is changed by hand, and a new point is drawn on m1 beat 3
    AutomationPoint changedMp = mpPoint->second;
    changedMp.value.outValue = 0.3;
    changedMp.generated = false;

    AutomationPointEdits edits {
        { tickAt(0, 0), AutomationPointEdit::SetPoint { changedMp } },
        { tickAt(0, 2), AutomationPointEdit::SetPoint { customPoint(0.3, 0.6) } },
    };

    score->startCmd(TranslatableString::untranslatable("NotePlaybackInfo tests"));
    score->editAutomationPoints(key, edits);
    score->endCmd();

    // [THEN] m1 beat 2: the mp, marked as edited in the automation lane
    const NotePlaybackInfo edited = infoAt(PIANO_RH, 0, 1);
    ASSERT_TRUE(edited.isValid());
    ASSERT_EQ(edited.passes.front().dynamicSources.size(), 1);
    EXPECT_EQ(edited.passes.front().dynamicSources.front(), (NotePlaybackSource { mp, true }));

    // [THEN] m1 beat 3: the drawn point, which has no score item
    const NotePlaybackInfo drawn = infoAt(PIANO_RH, 0, 2);
    ASSERT_TRUE(drawn.isValid());
    ASSERT_EQ(drawn.passes.front().dynamicSources.size(), 1);
    EXPECT_EQ(drawn.passes.front().dynamicSources.front(), (NotePlaybackSource { nullptr, false }));
    EXPECT_NEAR(drawn.passes.front().dynamicValue, 0.6, 0.001);
}

//! listedItems: each item once, tempo before dynamic before markings
TEST_F(Engraving_NotePlaybackInfoTests, ListedItems_OrderAndUniqueness)
{
    const MasterScore* score = load(INFLUENCES_SCORE);
    ASSERT_TRUE(score);

    const EngravingItem* tempo = annotationAt(score, PIANO_RH, tickAt(0, 0), ElementType::TEMPO_TEXT);
    const EngravingItem* f = annotationAt(score, PIANO_RH, tickAt(2, 0), ElementType::DYNAMIC);
    const Spanner* dim = spannerAt(score, PIANO_RH, tickAt(2, 0), ElementType::HAIRPIN);
    ASSERT_TRUE(tempo && f && dim);

    // [WHEN] A note inside the diminuendo
    const std::vector<const EngravingItem*> listed = listedItems(infoAt(PIANO_RH, 2, 1));

    // [THEN] Tempo before the dynamics (the hairpin and the f it starts from), each once
    const auto position = [&listed](const EngravingItem* item) {
        return std::ranges::find(listed, item) - listed.begin();
    };

    ASSERT_THAT(listed, ::testing::Contains(tempo));
    ASSERT_THAT(listed, ::testing::Contains(dim));
    ASSERT_THAT(listed, ::testing::Contains(f));
    EXPECT_LT(position(tempo), position(dim));
    EXPECT_LT(position(tempo), position(f));

    for (const EngravingItem* item : listed) {
        EXPECT_EQ(std::ranges::count(listed, item), 1);
    }

    // [THEN] Dynamics have their own row rather than a marking description, and are always listed
    EXPECT_FALSE(markingDescription(f));
    EXPECT_TRUE(isListedMarking(f));
    EXPECT_TRUE(isListedMarking(dim));
}
