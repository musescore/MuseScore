/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "async/asyncable.h"
#include "async/channel.h"
#include "mpe/tests/utils/articulationutils.h"
#include "mpe/tests/mocks/articulationprofilesrepositorymock.h"

#include "utils/scorerw.h"
#include "libmscore/part.h"
#include "libmscore/measure.h"
#include "libmscore/chord.h"

#include "playback/playbackmodel.h"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;

using namespace mu::engraving;
using namespace mu::mpe;
using namespace mu;

static const String PLAYBACK_MODEL_TEST_FILES_DIR("playbackmodel_data/");

class Engraving_PlaybackModelTests : public ::testing::Test, public async::Asyncable
{
protected:
    void SetUp() override
    {
        m_dummyPatternSegment.arrangementPattern
            = tests::createArrangementPattern(HUNDRED_PERCENT /*duration_factor*/, 0 /*timestamp_offset*/);
        m_dummyPatternSegment.pitchPattern = tests::createSimplePitchPattern(0 /*increment_pitch_diff*/);
        m_dummyPatternSegment.expressionPattern = tests::createSimpleExpressionPattern(dynamicLevelFromType(mu::mpe::DynamicType::Natural));
        m_dummyPattern.emplace(0, m_dummyPatternSegment);

        m_defaultProfile = std::make_shared<ArticulationsProfile>();

        m_repositoryMock = std::make_shared<NiceMock<ArticulationProfilesRepositoryMock> >();
    }

    ArticulationsProfilePtr m_defaultProfile = nullptr;

    ArticulationPattern m_dummyPattern;
    ArticulationPatternSegment m_dummyPatternSegment;

    std::shared_ptr<NiceMock<ArticulationProfilesRepositoryMock> > m_repositoryMock = nullptr;
};

/**
 * @brief PlaybackModelTests_SimpleRepeat
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 4 measures
 *          Additionally, there is a simple repeat from measure 2 up to measure 3. In total, we'll be playing 6 measures overall
 */
TEST_F(Engraving_PlaybackModelTests, SimpleRepeat)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "repeat_range/repeat_range.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 6 overall measures which should be played
    int expectedSize = 24;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId().toStdString()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Two_Ending_Repeat
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 6 measures
 *          Additionally, there is a repeat at the end of the second measure. Measure 2 is the first ending of the repeat.
 *          Measure 3 is the second ending of the repeat. In total, we'll be playing 7 measures overall
 */
TEST_F(Engraving_PlaybackModelTests, Two_Ending_Repeat)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "repeat_with_2_voltas/repeat_with_2_voltas.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 7 measures which should be played
    int expectedSize = 28;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId().toStdString()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Da_Capo_Al_Fine
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 6 measures
 *          Additionally, there is a "D.C. Al Fine" marking at the end of the 6-th measure. Measure 2 is marked by "Fine"
 *          In total, we'll be playing 8 measures overall
 */
TEST_F(Engraving_PlaybackModelTests, Da_Capo_Al_Fine)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "da_capo_al_fine/da_capo_al_fine.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 8 measures which should be played
    int expectedSize = 32;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId().toStdString()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Dal_Segno_Al_Coda
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 6 measures
 *          Additionally, there is a "D.S. Al Coda" marking at the end of the 4-th measure. Measure 2 is marked by "Segno" marking.
 *          The end of the 3-rd measure is marked by "To Coda" marking. The beginning of the 5-th measure is marked by "Coda" sign
 *          In total, we'll be playing 8 measures overall
 */
TEST_F(Engraving_PlaybackModelTests, Dal_Segno_Al_Coda)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "dal_segno_al_coda/dal_segno_al_coda.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 8 measures which should be played
    int expectedSize = 32;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId().toStdString()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Dal_Segno_Al_Fine
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 6 measures
 *          Additionally, there is a "D.S. Al Fine" marking at the end of the 6-th measure. Measure 2 is marked by "Segno" marking.
 *          The end of the 4-th measure is marked by "Fine" marking. In total, we'll be playing 9 measures overall
 */
TEST_F(Engraving_PlaybackModelTests, Dal_Segno_Al_Fine)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "dal_segno_al_fine/dal_segno_al_fine.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 9 measures which should be played
    int expectedSize = 36;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId().toStdString()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Da_Capo_Al_Coda
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 6 measures
 *          Additionally, there is a "D.C. Al Coda" marking at the end of the 6-th measure. The end of the measure 2 is marked by "To Coda".
 *          The beginning of the 4-th measure is marked by "Coda" sign. In total, we'll be playing 11 measures overall
 */
TEST_F(Engraving_PlaybackModelTests, Da_Capo_Al_Coda)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "da_capo_al_coda/da_capo_al_coda.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 11 measures which should be played
    int expectedSize = 44;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId().toStdString()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Da_Capo_Al_Coda
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 1 measure
 *          Additionally, the first note is marked by "pizzicato" + "stacattissimo". The 3-rd note is marked by "arco"
 *          We'll be playing 4 events overall
 */
TEST_F(Engraving_PlaybackModelTests, Pizz_To_Arco_Technique)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "pizz_to_arco/pizz_to_arco.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events
    int expectedSize = 4;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId().toStdString()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);

    // [THEN] The first note has Pizzicato and Staccatissimo articulations applied
    const mu::mpe::NoteEvent& firstNoteEvent = std::get<mu::mpe::NoteEvent>(result.at(0).at(0));
    EXPECT_EQ(firstNoteEvent.expressionCtx().articulations.size(), 2);
    EXPECT_TRUE(firstNoteEvent.expressionCtx().articulations.contains(ArticulationType::Pizzicato));
    EXPECT_TRUE(firstNoteEvent.expressionCtx().articulations.contains(ArticulationType::Staccatissimo));

    // [THEN] The second note has only Pizzicato articulation applied
    const mu::mpe::NoteEvent& secondNoteEvent = std::get<mu::mpe::NoteEvent>(result.at(500000).at(0));
    EXPECT_EQ(secondNoteEvent.expressionCtx().articulations.size(), 1);
    EXPECT_TRUE(secondNoteEvent.expressionCtx().articulations.contains(ArticulationType::Pizzicato));

    // [THEN] The third note has only Standard articulation applied
    const mu::mpe::NoteEvent& thirdNoteEvent = std::get<mu::mpe::NoteEvent>(result.at(1000000).at(0));
    EXPECT_EQ(thirdNoteEvent.expressionCtx().articulations.size(), 1);
    EXPECT_TRUE(thirdNoteEvent.expressionCtx().articulations.contains(ArticulationType::Standard));

    // [THEN] The fourth note has only Standard articulation applied
    const mu::mpe::NoteEvent& fourthNoteEvent = std::get<mu::mpe::NoteEvent>(result.at(1500000).at(0));
    EXPECT_EQ(fourthNoteEvent.expressionCtx().articulations.size(), 1);
    EXPECT_TRUE(fourthNoteEvent.expressionCtx().articulations.contains(ArticulationType::Standard));
}

/**
 * @brief PlaybackModelTests_Repeat_Last_Measure
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 6 measures
 *          Additionally, there is a "repeat last measure" sign on the 6-th measure. In total, we'll be playing 7 measures overall
 *
 * @bug The test is currently disabled. At the moment it shows a flaw in libmscore - repeatSegments. RepeatSegments calculations don't
 *      take into account MeasureRepeat elements which leads to issues with playback model. Whenever the root issue will be finished, this test
 *      will be enabled
 */
TEST_F(Engraving_PlaybackModelTests, Repeat_Last_Measure)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "repeat_last_measure/repeat_last_measure.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of events - 4 quarter notes on every measure * 7 measures which should be played
    int expectedSize = 24;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(part->id(), part->instrumentId().toStdString()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_SimpleRepeat_Changes_Notification
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 4 measures
 *          Additionally, there is a simple repeat from measure 2 up to measure 3. In total, we'll be playing 6 measures overall
 *
 *          When the model will be loaded we'll emulate a change notification on the 2-nd measure, so that there will be updated events
 *          on the main stream channel
 */
TEST_F(Engraving_PlaybackModelTests, SimpleRepeat_Changes_Notification)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "repeat_range/repeat_range.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    ON_CALL(*m_repositoryMock, defaultProfile(ArticulationFamily::Strings)).WillByDefault(Return(m_defaultProfile));

    // [GIVEN] Expected amount of changed events
    int expectedChangedEventsCount = 24;

    // [GIVEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score);

    PlaybackData result = model.resolveTrackPlaybackData(part->id(), part->instrumentId().toStdString());

    // [THEN] Updated events map will match our expectations
    result.mainStream.onReceive(this, [expectedChangedEventsCount](const PlaybackEventsMap& updatedEvents) {
        EXPECT_EQ(updatedEvents.size(), expectedChangedEventsCount);
    });

    // [WHEN] Notation has been changed on the 2-nd measure
    ScoreChangesRange range;
    range.tickFrom = 1920;
    range.tickTo = 3840;
    range.staffIdxFrom = 0;
    range.staffIdxTo = 0;
    range.changedTypes = { ElementType::NOTE };

    score->changesChannel().send(range);
}

/**
 * @brief PlaybackModelTests_Metronome_4_4
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 4 measures
 *          Measure 1: 4 quarter notes, Measure 2: full measure rest, Measure 3: 8 eighth-notes, Measure 4: 16 sixteen-notes
 *          So that we'll be playing 16 beats in total
 */
TEST_F(Engraving_PlaybackModelTests, Metronome_4_4)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(PLAYBACK_MODEL_TEST_FILES_DIR + "metronome_4_4/metronome_4_4.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of metronome events - 4 beats on every measure * 4 measures which should be played
    int expectedSize = 16;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(model.metronomeTrackId()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Metronome_6_4_Repeat
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 4 measures
 *          Additionally, there is a simple repeat on measure 2. In total, we'll be playing 30 beats, including repeated measure
 */
TEST_F(Engraving_PlaybackModelTests, Metronome_6_4_Repeat)
{
    // [GIVEN] Simple piece of score (Violin, 6/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_MODEL_TEST_FILES_DIR + "metronome_6_4_with_repeat/metronome_6_4_with_repeat.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] Expected amount of metronome events - 6 beats on every measure * 5 measures which should be played
    int expectedSize = 30;

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score);

    const PlaybackEventsMap& result = model.resolveTrackPlaybackData(model.metronomeTrackId()).originEvents;

    // [THEN] Amount of events does match expectations
    EXPECT_EQ(result.size(), expectedSize);
}

/**
 * @brief PlaybackModelTests_Note_Entry_Playback_Note
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 1 measure
 *          Additionally, we'll emulate the situation where user clicks on the first note, so that we should playback it
 *
 */
TEST_F(Engraving_PlaybackModelTests, Note_Entry_Playback_Note)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_MODEL_TEST_FILES_DIR + "note_entry_playback_note/note_entry_playback_note.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] The articulation profiles repository will be returning required profiles
    ON_CALL(*m_repositoryMock, defaultProfile(_)).WillByDefault(Return(m_defaultProfile));

    // [GIVEN] The very first note of the score
    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    Segment* firstSegment = firstMeasure->segments().firstCRSegment();
    ASSERT_TRUE(firstSegment);

    const Chord* chord = toChord(firstSegment->nextChordRest(0));
    ASSERT_TRUE(chord);
    ASSERT_TRUE(chord->notes().size() == 1);

    const Note* firstNote = chord->notes().front();
    mpe::timestamp_t firstNoteTimestamp = 0;

    // [GIVEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score);

    PlaybackData result = model.resolveTrackPlaybackData(part->id(), part->instrumentId().toStdString());

    // [GIVEN] Expected note event
    const mu::mpe::NoteEvent& expectedEvent = std::get<mu::mpe::NoteEvent>(result.originEvents.at(firstNoteTimestamp).front());

    // [THEN] Triggered events map will match our expectations
    result.offStream.onReceive(this, [firstNoteTimestamp, expectedEvent](const PlaybackEventsMap& triggeredEvents) {
        EXPECT_EQ(triggeredEvents.size(), 1);

        const PlaybackEventList& eventList = triggeredEvents.at(firstNoteTimestamp);

        EXPECT_EQ(eventList.size(), 1);

        const mu::mpe::NoteEvent& noteEvent = std::get<mu::mpe::NoteEvent>(eventList.front());

        EXPECT_TRUE(noteEvent.arrangementCtx().actualTimestamp == expectedEvent.arrangementCtx().actualTimestamp);
        EXPECT_FALSE(noteEvent.expressionCtx() == expectedEvent.expressionCtx());
        EXPECT_TRUE(noteEvent.pitchCtx() == expectedEvent.pitchCtx());
    });

    // [WHEN] User has clicked on the first note
    model.triggerEventsForItems({ firstNote });
}

/**
 * @brief PlaybackModelTests_Note_Entry_Playback_Chord
 * @details In this case we're building up a playback model of a simple score - Violin, 4/4, 120bpm, Treble Cleff, 1 measure
 *          Additionally, we'll emulate the situation where user wants to playback the third chord and make sure that we'll sent
 *          all the necessary events into audio-engine
 *
 */
TEST_F(Engraving_PlaybackModelTests, Note_Entry_Playback_Chord)
{
    // [GIVEN] Simple piece of score (Violin, 4/4, 120 bpm, Treble Cleff)
    Score* score = ScoreRW::readScore(
        PLAYBACK_MODEL_TEST_FILES_DIR + "note_entry_playback_chord/note_entry_playback_chord.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 1);

    const Part* part = score->parts().at(0);
    ASSERT_TRUE(part);
    ASSERT_EQ(part->instruments().size(), 1);

    // [GIVEN] The articulation profiles repository will be returning required profiles
    ON_CALL(*m_repositoryMock, defaultProfile(_)).WillByDefault(Return(m_defaultProfile));

    // [GIVEN] The third chord of the score
    Measure* firstMeasure = score->firstMeasure();
    ASSERT_TRUE(firstMeasure);

    size_t expectedNoteCount = 3;
    int thirdChordPositionTick = 480 * 2;
    timestamp_t thirdChordTimestamp = 500000 * 2;

    const Chord* thirdChord = firstMeasure->findChord(Fraction::fromTicks(thirdChordPositionTick), 0);
    ASSERT_TRUE(thirdChord);
    ASSERT_TRUE(thirdChord->notes().size() == expectedNoteCount);

    // [GIVEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score);

    PlaybackData result = model.resolveTrackPlaybackData(part->id(), part->instrumentId().toStdString());

    // [GIVEN] Expected note event
    const PlaybackEventList& expectedEvents = result.originEvents.at(thirdChordTimestamp);

    // [THEN] Triggered events map will match our expectations
    result.offStream.onReceive(this, [expectedEvents](const PlaybackEventsMap& triggeredEvents) {
        EXPECT_EQ(triggeredEvents.size(), 1);

        const PlaybackEventList& actualEvents = triggeredEvents.at(0);
        EXPECT_EQ(actualEvents.size(), expectedEvents.size());

        for (size_t i = 0; i < expectedEvents.size(); ++i) {
            const mu::mpe::NoteEvent expectedNoteEvent = std::get<mu::mpe::NoteEvent>(expectedEvents.at(i));
            const mu::mpe::NoteEvent actualNoteEvent = std::get<mu::mpe::NoteEvent>(actualEvents.at(i));

            EXPECT_TRUE(actualNoteEvent.arrangementCtx().actualTimestamp == 0);
            EXPECT_FALSE(actualNoteEvent.expressionCtx() == expectedNoteEvent.expressionCtx());
            EXPECT_TRUE(actualNoteEvent.pitchCtx() == expectedNoteEvent.pitchCtx());
        }
    });

    // [WHEN] User has clicked on the first note
    model.triggerEventsForItems({ thirdChord });
}

/**
 * @brief PlaybackModelTests_Playback_Setup_Data_MultiInstrument
 * @details In this case we're building up a playback model of a score with 12 instruments:
 *          - sopranissimo-saxophone
 *          - marching-tenor-drums
 *          - hand-clap
 *          - guitar-steel
 *          - bass-steel-drums
 *          - alto-viol
 *          - f-wagner-tuba
 *          - bass-harmonica-hohner
 *          - chinese-tom-toms
 *          - electric-piano
 *          - boy-soprano
 *
 *          We need to make sure that playback setup data will be properly prepared, so that the score will be played with appropriate sounds
 */
TEST_F(Engraving_PlaybackModelTests, Playback_Setup_Data_MultiInstrument)
{
    // [GIVEN] Score with 12 instruments
    Score* score = ScoreRW::readScore(
        PLAYBACK_MODEL_TEST_FILES_DIR + "playback_setup_instruments/playback_setup_instruments.mscx");

    ASSERT_TRUE(score);
    ASSERT_EQ(score->parts().size(), 12);

    // [GIVEN] Expected setup data for each instrument
    std::unordered_map<std::string, mpe::PlaybackSetupData> expectedSetupData = {
        { "sopranissimo-saxophone", { SoundId::Saxophone, SoundCategory::Winds, { SoundSubCategory::Sopranissimo }, {} } },
        { "marching-tenor-drums", { SoundId::Drum, SoundCategory::Percussions, { SoundSubCategory::Marching,
                                                                                 SoundSubCategory::Snare,
                                                                                 SoundSubCategory::Tenor }, {} } },
        { "hand-clap", { SoundId::Clap, SoundCategory::Percussions, { SoundSubCategory::Hand }, {} } },
        { "guitar-steel", { SoundId::Guitar, SoundCategory::Strings, { SoundSubCategory::Acoustic,
                                                                       SoundSubCategory::Steel,
                                                                       SoundSubCategory::Plucked }, {} } },
        { "bass-steel-drums", { SoundId::SteelDrums, SoundCategory::Percussions, { SoundSubCategory::Metal,
                                                                                   SoundSubCategory::Steel,
                                                                                   SoundSubCategory::Bass }, {} } },
        { "alto-viol", { SoundId::Viol, SoundCategory::Strings, { SoundSubCategory::Alto }, {} } },
        { "f-wagner-tuba", { SoundId::Tuba, SoundCategory::Winds, { SoundSubCategory::Wagner }, {} } },
        { "bass-harmonica-hohner", { SoundId::Harmonica, SoundCategory::Winds, { SoundSubCategory::Bass,
                                                                                 SoundSubCategory::Hohner }, {} } },
        { "chinese-tom-toms", { SoundId::TomToms, SoundCategory::Percussions, { SoundSubCategory::Chinese }, {} } },
        { "electric-piano", { SoundId::Piano, SoundCategory::Keyboards, { SoundSubCategory::Electric }, {} } },
        { "crystal-synth", { SoundId::Synthesizer, SoundCategory::Keyboards, { SoundSubCategory::Electric,
                                                                               SoundSubCategory::FX_Crystal }, {} } },
        { "boy-soprano", { SoundId::Choir, SoundCategory::Voices, { SoundSubCategory::Soprano,
                                                                    SoundSubCategory::Boy }, {} } },
    };

    // [WHEN] The articulation profiles repository will be returning profiles for StringsArticulation family
    EXPECT_CALL(*m_repositoryMock, defaultProfile(_)).WillRepeatedly(Return(m_defaultProfile));

    // [WHEN] The playback model requested to be loaded
    PlaybackModel model;
    model.setprofilesRepository(m_repositoryMock);
    model.load(score);

    // [THEN] Result matches with our expectations
    for (const Part* part : score->parts()) {
        for (const auto& pair : part->instruments()) {
            const std::string& instrumentId = pair.second->id().toStdString();
            const PlaybackData& result = model.resolveTrackPlaybackData(part->id(), instrumentId);

            EXPECT_EQ(result.setupData, expectedSetupData.at(instrumentId));
        }
    }
}
