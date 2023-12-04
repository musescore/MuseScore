/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#include "utils/scorerw.h"

#include "dom/guitarbend.h"
#include "dom/tempo.h"

#include "playback/renderers/bendsrenderer.h"

using namespace mu::engraving;
using namespace mu::mpe;

class Engraving_BendsRendererTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        //! NOTE: allows to read test files using their version readers
        //! instead of using 302 (see mscloader.cpp, makeReader)
        bool useRead302 = MScore::useRead302InTestMode;
        MScore::useRead302InTestMode = false;

        m_score = ScoreRW::readScore(u"playbackeventsrenderer_data/guitar_bends/guitar_bends.mscx");

        ASSERT_TRUE(m_score);
        ASSERT_EQ(m_score->parts().size(), 1);
        ASSERT_EQ(m_score->nstaves(), 2);

        MScore::useRead302InTestMode = useRead302;
    }

    void TearDown() override
    {
        delete m_score;
        m_score = nullptr;
    }

    const Chord* findChord(int tick, track_idx_t track = 0) const
    {
        for (MeasureBase* mb = m_score->first(); mb; mb = mb->next()) {
            if (!mb->isMeasure()) {
                continue;
            }

            const Chord* chord = toMeasure(mb)->findChord(Fraction::fromTicks(tick), track);
            if (chord) {
                return chord;
            }
        }

        return nullptr;
    }

    ArticulationPattern buildTestArticulationPattern() const
    {
        ArticulationPatternSegment blankSegment(ArrangementPattern(HUNDRED_PERCENT /*durationFactor*/, 0 /*timestampOffset*/),
                                                PitchPattern(EXPECTED_SIZE, TEN_PERCENT, 0),
                                                ExpressionPattern(EXPECTED_SIZE, TEN_PERCENT, 0));

        ArticulationPattern pattern;
        pattern.emplace(0, std::move(blankSegment));

        return pattern;
    }

    RenderingContext buildCtx(const Chord* chord, ArticulationsProfilePtr profile,
                              ArticulationType persistentArticulationType = ArticulationType::Standard)
    {
        int chordPosTick = chord->tick().ticks();
        int chordDurationTicks = chord->actualTicks().ticks();

        BeatsPerSecond bps = m_score->tempomap()->tempo(chordPosTick);
        TimeSigFrac timeSignatureFraction = m_score->sigmap()->timesig(chordPosTick).timesig();

        RenderingContext ctx(timestampFromTicks(m_score, chordPosTick),
                             durationFromTicks(bps.val, chordDurationTicks),
                             5000,
                             chordPosTick,
                             0,
                             chordDurationTicks,
                             bps,
                             timeSignatureFraction,
                             persistentArticulationType,
                             ArticulationMap(),
                             profile);

        return ctx;
    }

private:
    Score* m_score = nullptr;
};

/*!
 * @details Render a multibend with the following structure:
 * F3 -> bend -> G3 -> bend -> F3 -> bend -> G3 (appoggiatura) -> grace note bend -> A3 -> hold -> A3 -> bend -> G3
 */
TEST_F(Engraving_BendsRendererTests, Multibend)
{
    // [GIVEN] First chord of the multibend
    const Chord* startChord = findChord(480);
    ASSERT_TRUE(startChord);
    ASSERT_FALSE(startChord->notes().empty());

    // [GIVEN] Context of the chord
    ArticulationsProfilePtr profile = std::make_shared<ArticulationsProfile>();
    profile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::Multibend, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::PreAppoggiatura, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::Distortion, buildTestArticulationPattern());
    RenderingContext ctx = buildCtx(startChord, profile, ArticulationType::Distortion);

    // [THEN] BendsRenderer can render the multibend articulation
    EXPECT_TRUE(BendsRenderer::isAbleToRender(ArticulationType::Multibend));

    // [WHEN] Render the chord
    PlaybackEventList events;
    BendsRenderer::render(startChord, ArticulationType::Multibend, ctx, events);

    // [THEN] Render the multibend as a single NoteEvent
    ASSERT_EQ(events.size(), 1);
    ASSERT_TRUE(std::holds_alternative<mu::mpe::NoteEvent>(events.front()));

    const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(events.front());

    EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Multibend));
    EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Distortion)); // persistent articulation applied
    EXPECT_EQ(noteEvent.arrangementCtx().actualTimestamp, 500000); // starts after a quarter rest
    EXPECT_EQ(noteEvent.arrangementCtx().actualDuration, 3000000); // quarters: F3 + G3 + F3 + A3 + A3 + G3
    EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, 2050); // F3

    PitchCurve expectedPitchCurve;
    expectedPitchCurve.emplace(0, 0); // F3
    expectedPitchCurve.emplace(1600, 100); // F3 -> G3
    expectedPitchCurve.emplace(3300, 0); //  G3 -> F3
    expectedPitchCurve.emplace(5000, 100); // F3 -> G3 (appoggiatura)
    expectedPitchCurve.emplace(5800, 200); // G3 (appoggiatura) -> A3
    expectedPitchCurve.emplace(8300, 100); // A3 -> hold -> G3

    EXPECT_EQ(noteEvent.pitchCtx().pitchCurve, expectedPitchCurve);

    // [THEN] Check we don't render other chords of the multibend (only the first one)
    const GuitarBend* bend = startChord->notes().front()->bendFor();
    ASSERT_TRUE(bend);

    while (bend) {
        const Note* note = bend->endNote();
        ASSERT_TRUE(note);

        const Chord* chord = note->chord();
        ctx = buildCtx(chord, profile);

        events.clear();
        BendsRenderer::render(chord, ArticulationType::Multibend, ctx, events);
        EXPECT_TRUE(events.empty());

        bend = note->bendFor();
    }
}

/*!
 * @details Render a chord with 2 bends and 1 note:
 * 1st bend: D3 -> E3
 * 2nd bend: G3 -> A3
 * note event: A3
 */
TEST_F(Engraving_BendsRendererTests, MultipleBendsOnOneChord)
{
    // [GIVEN] Chord with multiple notes
    const Chord* startChord = findChord(3840);
    ASSERT_TRUE(startChord);
    ASSERT_EQ(startChord->notes().size(), 3);

    // [GIVEN] Context of the chord
    ArticulationsProfilePtr profile = std::make_shared<ArticulationsProfile>();
    profile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::Multibend, buildTestArticulationPattern());
    RenderingContext ctx = buildCtx(startChord, profile);

    // [WHEN] Render the chord
    PlaybackEventList events;
    BendsRenderer::render(startChord, ArticulationType::Multibend, ctx, events);

    // [THEN] 3 events: 2 bends + 1 note
    ASSERT_EQ(events.size(), 3);
    for (const mu::mpe::PlaybackEvent& event: events) {
        ASSERT_TRUE(std::holds_alternative<mu::mpe::NoteEvent>(event));
    }

    PitchCurve expectedBendCurve;
    expectedBendCurve.emplace(0, 0);
    expectedBendCurve.emplace(5000, 100);

    const mu::mpe::NoteEvent& bendEvent1 = std::get<mu::mpe::NoteEvent>(events.at(0));
    EXPECT_EQ(bendEvent1.pitchCtx().nominalPitchLevel, 1900);
    EXPECT_EQ(bendEvent1.pitchCtx().pitchCurve, expectedBendCurve);

    const mu::mpe::NoteEvent& bendEvent2 = std::get<mu::mpe::NoteEvent>(events.at(1));
    EXPECT_EQ(bendEvent2.pitchCtx().pitchCurve, expectedBendCurve);
    EXPECT_EQ(bendEvent2.pitchCtx().nominalPitchLevel, 2150);

    const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(events.at(2));
    EXPECT_NE(noteEvent.pitchCtx().pitchCurve, expectedBendCurve);
    EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, 2400);
}

/*!
 * @details Render a pre-bend with the following structure:
 * Pre-bend G3 up to A3 -> hold for 1 quarter -> release down to G3
 * Check that we ignore the grace note (A3) and render only the principal note (G3)
 */
TEST_F(Engraving_BendsRendererTests, PreBend)
{
    // [GIVEN] Chord with a pre-bend
    const Chord* chord = findChord(5760);
    ASSERT_TRUE(chord);

    // [GIVEN] Context of the chord
    ArticulationsProfilePtr profile = std::make_shared<ArticulationsProfile>();
    profile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::Multibend, buildTestArticulationPattern());
    RenderingContext ctx = buildCtx(chord, profile);

    // [WHEN] Render the chord
    PlaybackEventList events;
    BendsRenderer::render(chord, ArticulationType::Multibend, ctx, events);

    // [THEN] Chord successfully rendered
    ASSERT_EQ(events.size(), 1);
    ASSERT_TRUE(std::holds_alternative<mu::mpe::NoteEvent>(events.front()));

    PitchCurve expectedPitchCurve;
    expectedPitchCurve.emplace(0, 0); // A3
    expectedPitchCurve.emplace(6600, -100); // Release down to G3

    const mu::mpe::NoteEvent& event = std::get<mu::mpe::NoteEvent>(events.front());
    EXPECT_EQ(event.pitchCtx().nominalPitchLevel, 2250); // A3
    EXPECT_EQ(event.arrangementCtx().actualDuration, 1500000); // quarters: A3 + A3 + G3
    EXPECT_EQ(event.pitchCtx().pitchCurve, expectedPitchCurve);
}

/*!
 * @details Render a note (A3) with a slight bend
 */
TEST_F(Engraving_BendsRendererTests, SlightBend)
{
    // [GIVEN] Chord with a slight bend
    const Chord* chord = findChord(7680);
    ASSERT_TRUE(chord);

    // [GIVEN] Context of the chord
    ArticulationsProfilePtr profile = std::make_shared<ArticulationsProfile>();
    profile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::Multibend, buildTestArticulationPattern());
    RenderingContext ctx = buildCtx(chord, profile);

    // [WHEN] Render the chord
    PlaybackEventList events;
    BendsRenderer::render(chord, ArticulationType::Multibend, ctx, events);

    // [THEN] Chord successfully rendered
    ASSERT_EQ(events.size(), 1);
    ASSERT_TRUE(std::holds_alternative<mu::mpe::NoteEvent>(events.front()));

    PitchCurve expectedPitchCurve;
    expectedPitchCurve.emplace(0, 0); // A3
    expectedPitchCurve.emplace(5000, 25); // 1/4

    const mu::mpe::NoteEvent& event = std::get<mu::mpe::NoteEvent>(events.front());
    EXPECT_EQ(event.pitchCtx().nominalPitchLevel, 2250); // A3
    EXPECT_EQ(event.pitchCtx().pitchCurve, expectedPitchCurve);
}

/*!
 * @details Render a multibend with custom time offsets
 * E3 (pre-appoggiatura) -> F3, start offset: 25%, end offset: 75%
 * F3 -> G3, start offset: 25%, end offset: 100%
 * G3 -> F3, start offset: 25%, end offset: 50%
 */
TEST_F(Engraving_BendsRendererTests, Multibend_CustomTimeOffsets)
{
    // [GIVEN] First chord of the multibend
    const Chord* startChord = findChord(9600);
    ASSERT_TRUE(startChord);

    // [GIVEN] Context of the chord
    ArticulationsProfilePtr profile = std::make_shared<ArticulationsProfile>();
    profile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::Multibend, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::PreAppoggiatura, buildTestArticulationPattern());
    RenderingContext ctx = buildCtx(startChord, profile);

    // [WHEN] Render the chord
    PlaybackEventList events;
    BendsRenderer::render(startChord, ArticulationType::Multibend, ctx, events);

    // [THEN] Chord successfully rendered
    ASSERT_EQ(events.size(), 1);
    ASSERT_TRUE(std::holds_alternative<mu::mpe::NoteEvent>(events.front()));

    PitchCurve expectedPitchCurve;
    expectedPitchCurve.emplace(0, 0);
    expectedPitchCurve.emplace(400, 0); // start offset: 25%
    expectedPitchCurve.emplace(1200, 50); // end offset: 75%
    expectedPitchCurve.emplace(2450, 50); // start offset: 25%
    expectedPitchCurve.emplace(3300, 250); // end offset: 100%
    expectedPitchCurve.emplace(4125, 250); // start offset: 25%
    expectedPitchCurve.emplace(4950, 150); // end offset: 50%

    const mu::mpe::NoteEvent& event = std::get<mu::mpe::NoteEvent>(events.front());
    EXPECT_EQ(event.arrangementCtx().actualDuration, 1500000);
    EXPECT_EQ(event.pitchCtx().pitchCurve, expectedPitchCurve);
}
