/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include "dom/part.h"

#include "playback/renderingcontext.h"
#include "playback/renderers/bendsrenderer.h"

using namespace mu::engraving;
using namespace muse;
using namespace muse::mpe;

static constexpr duration_t QUARTER_NOTE_DURATION = 500000; // duration in microseconds for 4/4 120BPM

class Engraving_BendsRendererTests : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        //! NOTE: allows to read test files using their version readers
        //! instead of using 302 (see mscloader.cpp, makeReader)
        bool useRead302 = MScore::useRead302InTestMode;
        MScore::useRead302InTestMode = false;

        s_score = ScoreRW::readScore(u"playback/playbackeventsrenderer_data/guitar_bends/guitar_bends.mscx");

        ASSERT_TRUE(s_score);
        ASSERT_EQ(s_score->parts().size(), 1);
        ASSERT_EQ(s_score->nstaves(), 2);

        s_playbackCtx = std::make_shared<PlaybackContext>();
        s_playbackCtx->update(s_score->parts().front()->id(), s_score);

        MScore::useRead302InTestMode = useRead302;
    }

    static void TearDownTestSuite()
    {
        delete s_score;
        s_score = nullptr;
        s_playbackCtx.reset();
    }

    const Chord* findChord(int tick, track_idx_t track = 0) const
    {
        for (MeasureBase* mb = s_score->first(); mb; mb = mb->next()) {
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

    static Score* s_score;
    static PlaybackContextPtr s_playbackCtx;
};

Score* Engraving_BendsRendererTests::s_score = nullptr;
PlaybackContextPtr Engraving_BendsRendererTests::s_playbackCtx = nullptr;

/*!
 * @details Render a multibend with the following structure:
 * F3 -> bend -> G3 -> bend -> F3 -> bend -> G3 (appoggiatura) -> grace note bend -> A3 -> hold -> A3 -> bend -> G3 -> A3 (grace after) -> G3 (grace after)
 */
TEST_F(Engraving_BendsRendererTests, Multibend)
{
    // [GIVEN] First chord of the multibend
    const Chord* startChord = findChord(480);
    ASSERT_TRUE(startChord);
    ASSERT_EQ(startChord->notes().size(), 1);

    // [GIVEN] First note of the multibend
    const Note* startNote = startChord->notes().front();

    // [THEN] Check that the note is recognised as part of the multibend
    EXPECT_TRUE(BendsRenderer::isMultibendPart(startNote));

    // [GIVEN] Context of the chord
    ArticulationsProfilePtr profile = std::make_shared<ArticulationsProfile>();
    profile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::Multibend, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::PreAppoggiatura, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::PostAppoggiatura, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::Distortion, buildTestArticulationPattern());
    RenderingContext startChordCtx = buildRenderingCtx(startChord, 0, profile, s_playbackCtx);

    // [WHEN] Render the note
    PlaybackEventList events;
    BendsRenderer::render(startNote, startChordCtx, events);

    // [THEN] Render the multibend as a single NoteEvent
    ASSERT_EQ(events.size(), 1);
    ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(events.front()));

    // [THEN] The note event has the correct timestamp and duration
    const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(events.front());
    EXPECT_EQ(noteEvent.arrangementCtx().actualTimestamp, QUARTER_NOTE_DURATION); // starts after a quarter rest
    EXPECT_EQ(noteEvent.arrangementCtx().actualDuration, QUARTER_NOTE_DURATION * 6); // F3 + G3 + F3 + A3 + A3 + G3
    EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::F, 3));

    // [THEN] The note event contains the multibend articulation with the correct timestamp and duration
    auto multibendIt = noteEvent.expressionCtx().articulations.find(ArticulationType::Multibend);
    ASSERT_TRUE(multibendIt != noteEvent.expressionCtx().articulations.end());
    const mpe::ArticulationMeta& multibendMeta = multibendIt->second.meta;
    EXPECT_EQ(multibendMeta.timestamp, noteEvent.arrangementCtx().actualTimestamp);
    EXPECT_EQ(multibendMeta.overallDuration, noteEvent.arrangementCtx().actualDuration);

    // [THEN] The note event contains the distortion articulation (persistent)
    EXPECT_TRUE(noteEvent.expressionCtx().articulations.contains(ArticulationType::Distortion));

    // [THEN] The pitch curve is correct
    PitchCurve expectedPitchCurve;
    expectedPitchCurve.emplace(0, 0); // F3
    expectedPitchCurve.emplace(1600, 100); // F3 -> G3
    expectedPitchCurve.emplace(3300, 0); //  G3 -> F3
    expectedPitchCurve.emplace(5000, 100); // F3 -> G3 (appoggiatura)
    expectedPitchCurve.emplace(5800, 200); // G3 (appoggiatura) -> A3
    expectedPitchCurve.emplace(8300, 100); // A3 -> hold -> G3
    // See: https://github.com/musescore/MuseScore/issues/28645
    expectedPitchCurve.emplace(9100, 200); // G3 -> A3 (grace 8th after)
    expectedPitchCurve.emplace(9500, 100); // A3 -> G3 (grace 8th after)

    EXPECT_EQ(noteEvent.pitchCtx().pitchCurve, expectedPitchCurve);

    // [THEN] Check we don't render other notes of the multibend (only the first one)
    const GuitarBend* bend = startChord->notes().front()->bendFor();
    ASSERT_TRUE(bend);

    while (bend) {
        const Note* note = bend->endNote();
        ASSERT_TRUE(note);
        EXPECT_TRUE(BendsRenderer::isMultibendPart(note));

        RenderingContext ctx = buildRenderingCtx(note->chord(), 0, profile, s_playbackCtx);

        events.clear();
        BendsRenderer::render(note, ctx, events);
        EXPECT_TRUE(events.empty());

        bend = note->bendFor();
    }
}

/*!
 * @details Render a chord with 2 bends and 1 note:
 * 1st bend: D3 -> E3
 * 2nd bend: G3 -> A3
 * note event: C4
 */
TEST_F(Engraving_BendsRendererTests, MultipleBendsOnOneChord)
{
    // [GIVEN] Chord with multiple notes
    const Chord* chord = findChord(3840);
    ASSERT_TRUE(chord);
    ASSERT_EQ(chord->notes().size(), 3);

    // [GIVEN] Context of the chord
    ArticulationsProfilePtr profile = std::make_shared<ArticulationsProfile>();
    profile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::Multibend, buildTestArticulationPattern());
    RenderingContext ctx = buildRenderingCtx(chord, 0, profile, s_playbackCtx);

    // [WHEN] Render the chord
    PlaybackEventList events;
    for (const Note* note : chord->notes()) {
        BendsRenderer::render(note, ctx, events);
    }

    // [THEN] 3 events: 2 bends + 1 note
    ASSERT_EQ(events.size(), 3);
    for (const mpe::PlaybackEvent& event: events) {
        ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(event));
    }

    PitchCurve expectedBendCurve;
    expectedBendCurve.emplace(0, 0);
    expectedBendCurve.emplace(5000, 100);

    const mpe::NoteEvent& bendEvent1 = std::get<mpe::NoteEvent>(events.at(0));
    EXPECT_EQ(bendEvent1.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::D, 3));
    EXPECT_EQ(bendEvent1.pitchCtx().pitchCurve, expectedBendCurve);

    const mpe::NoteEvent& bendEvent2 = std::get<mpe::NoteEvent>(events.at(1));
    EXPECT_EQ(bendEvent2.pitchCtx().pitchCurve, expectedBendCurve);
    EXPECT_EQ(bendEvent2.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::G, 3));

    const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(events.at(2));
    EXPECT_NE(noteEvent.pitchCtx().pitchCurve, expectedBendCurve);
    EXPECT_EQ(noteEvent.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::C, 4));
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

    // [GIVEN] Pre-bend grace note (unplayable)
    ASSERT_EQ(chord->graceNotesBefore().size(), 1);
    const Chord* graceChord = chord->graceNotesBefore().front();
    ASSERT_EQ(graceChord->notes().size(), 1);
    const Note* unplayableGracePreBendNote = graceChord->notes().front();
    ASSERT_TRUE(unplayableGracePreBendNote->isPreBendStart());

    // [GIVEN] Playable principal note
    ASSERT_EQ(chord->notes().size(), 1);
    const Note* playblePrincipalNote = chord->notes().front();
    ASSERT_TRUE(playblePrincipalNote->bendBack());
    ASSERT_EQ(playblePrincipalNote->bendBack()->startNote(), unplayableGracePreBendNote);

    // [GIVEN] Context of the chord
    ArticulationsProfilePtr profile = std::make_shared<ArticulationsProfile>();
    profile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::Multibend, buildTestArticulationPattern());
    RenderingContext ctx = buildRenderingCtx(chord, 0, profile, s_playbackCtx);

    // [WHEN] Render the unplayable pre-bend grace note
    PlaybackEventList events;
    BendsRenderer::render(unplayableGracePreBendNote, ctx, events);

    // [THEN] No events
    EXPECT_TRUE(events.empty());

    // [WHEN] Render the first playable (principal) note
    BendsRenderer::render(playblePrincipalNote, ctx, events);

    // [THEN] Note successfully rendered
    ASSERT_EQ(events.size(), 1);
    ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(events.front()));

    PitchCurve expectedPitchCurve;
    expectedPitchCurve.emplace(0, 0); // A3
    expectedPitchCurve.emplace(3300, 0); // tied A3
    expectedPitchCurve.emplace(6600, -100); // Release down to G3

    const mpe::NoteEvent& event = std::get<mpe::NoteEvent>(events.front());
    EXPECT_EQ(event.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::A, 3)); // A3
    EXPECT_EQ(event.arrangementCtx().actualDuration, QUARTER_NOTE_DURATION * 3); // A3 + A3 + G3
    EXPECT_EQ(event.pitchCtx().pitchCurve, expectedPitchCurve);
}

/*!
 * @details Render a quarter note (A3) with a slight bend
 */
TEST_F(Engraving_BendsRendererTests, SlightBend)
{
    // [GIVEN] Quarter note with a slight bend
    const Chord* chord = findChord(7680);
    ASSERT_TRUE(chord);
    ASSERT_EQ(chord->notes().size(), 1);
    const Note* note = chord->notes().front();

    // [GIVEN] Context of the chord
    ArticulationsProfilePtr profile = std::make_shared<ArticulationsProfile>();
    profile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::Multibend, buildTestArticulationPattern());
    RenderingContext ctx = buildRenderingCtx(chord, 0, profile, s_playbackCtx);

    // [WHEN] Render the note
    PlaybackEventList events;
    BendsRenderer::render(note, ctx, events);

    // [THEN] Note successfully rendered
    ASSERT_EQ(events.size(), 1);
    ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(events.front()));

    PitchCurve expectedPitchCurve;
    expectedPitchCurve.emplace(0, 0); // A3
    expectedPitchCurve.emplace(5000, 25); // 1/4

    const mpe::NoteEvent& event = std::get<mpe::NoteEvent>(events.front());
    EXPECT_EQ(event.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::A, 3));
    EXPECT_EQ(event.pitchCtx().pitchCurve, expectedPitchCurve);

    EXPECT_EQ(event.expressionCtx().articulations.size(), 1);
    auto artIt = event.expressionCtx().articulations.find(mpe::ArticulationType::Multibend);
    EXPECT_TRUE(artIt != event.expressionCtx().articulations.end());

    const ArticulationMeta& meta = artIt->second.meta;
    EXPECT_EQ(meta.timestamp, timestampFromTicks(s_score, note->tick().ticks()));
    EXPECT_EQ(meta.overallDuration, QUARTER_NOTE_DURATION);
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

    // [GIVEN] First (grace) note of the multibend
    ASSERT_EQ(startChord->graceNotesBefore().size(), 1);
    const Chord* graceChord = startChord->graceNotesBefore().front();
    ASSERT_EQ(graceChord->notes().size(), 1);
    const Note* startGraceNote = graceChord->notes().front();

    // [GIVEN] Context of the chord
    ArticulationsProfilePtr profile = std::make_shared<ArticulationsProfile>();
    profile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::Multibend, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::PreAppoggiatura, buildTestArticulationPattern());
    RenderingContext ctx = buildRenderingCtx(startChord, 0, profile, s_playbackCtx);

    // [WHEN] Render the note
    PlaybackEventList events;
    BendsRenderer::render(startGraceNote, ctx, events);

    // [THEN] Note successfully rendered
    ASSERT_EQ(events.size(), 1);
    ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(events.front()));

    PitchCurve expectedPitchCurve;
    expectedPitchCurve.emplace(0, 0);
    expectedPitchCurve.emplace(400, 0); // start offset: 25%
    expectedPitchCurve.emplace(1200, 50); // end offset: 75%
    expectedPitchCurve.emplace(2450, 50); // start offset: 25%
    expectedPitchCurve.emplace(3300, 250); // end offset: 100%
    expectedPitchCurve.emplace(4125, 250); // start offset: 25%
    expectedPitchCurve.emplace(4950, 150); // end offset: 50%

    const mpe::NoteEvent& event = std::get<mpe::NoteEvent>(events.front());
    EXPECT_EQ(event.arrangementCtx().actualDuration, QUARTER_NOTE_DURATION * 3);
    EXPECT_EQ(event.pitchCtx().pitchCurve, expectedPitchCurve);
}

/*!
 * @details Checks we can render a bend starting from a tied note
 * See: https://github.com/musescore/MuseScore/issues/21345
 */
TEST_F(Engraving_BendsRendererTests, BendOnTiedNotes)
{
    // [GIVEN] First chord of the multibend
    const Chord* chord = findChord(11520);
    ASSERT_TRUE(chord);
    ASSERT_EQ(chord->notes().size(), 1);

    // [GIVEN] Context of the chord
    ArticulationsProfilePtr profile = std::make_shared<ArticulationsProfile>();
    profile->setPattern(ArticulationType::Standard, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::Multibend, buildTestArticulationPattern());
    profile->setPattern(ArticulationType::PreAppoggiatura, buildTestArticulationPattern());
    RenderingContext ctx = buildRenderingCtx(chord, 0, profile, s_playbackCtx);

    // [GIVEN] Quarter A3 tied to another quarter A3
    const Note* note = chord->notes().front();
    ASSERT_TRUE(note->tieFor());
    ASSERT_FALSE(note->tieBack());

    // [THEN] Check that the note is recognised as part of the multibend
    EXPECT_TRUE(BendsRenderer::isMultibendPart(note));

    // [WHEN] Render the tied note
    PlaybackEventList events;
    BendsRenderer::render(note, ctx, events);

    // [THEN] Note successfully rendered
    ASSERT_EQ(events.size(), 1);
    ASSERT_TRUE(std::holds_alternative<mpe::NoteEvent>(events.front()));

    PitchCurve expectedPitchCurve;
    expectedPitchCurve.emplace(0, 0);
    expectedPitchCurve.emplace(3300, 0); // tied A3
    expectedPitchCurve.emplace(6600, 100); // B3

    const mpe::NoteEvent& event = std::get<mpe::NoteEvent>(events.front());
    EXPECT_EQ(event.pitchCtx().nominalPitchLevel, pitchLevel(PitchClass::A, 3));
    EXPECT_EQ(event.pitchCtx().pitchCurve, expectedPitchCurve);
    EXPECT_EQ(event.arrangementCtx().actualDuration, QUARTER_NOTE_DURATION * 3); // 2 tied A3 + B3

    // [THEN] The note event contains the multibend articulation with the correct timestamp and duration
    auto multibendIt = event.expressionCtx().articulations.find(ArticulationType::Multibend);
    ASSERT_TRUE(multibendIt != event.expressionCtx().articulations.end());

    const mpe::ArticulationAppliedData& articulationData = multibendIt->second;
    EXPECT_NE(articulationData.occupiedFrom, 0);
    EXPECT_EQ(articulationData.occupiedTo, HUNDRED_PERCENT);

    const mpe::ArticulationMeta& multibendMeta = articulationData.meta;
    EXPECT_EQ(multibendMeta.timestamp, event.arrangementCtx().actualTimestamp);
    EXPECT_EQ(multibendMeta.overallDuration, event.arrangementCtx().actualDuration);
}
