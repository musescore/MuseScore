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

#include <gmock/gmock.h>

#include "context/iglobalcontext.h"
#include "engraving/compat/scoreaccess.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/note.h"
#include "notation/imasternotation.h"
#include "notation/inotation.h"
#include "notation/inotationmidiinput.h"
#include "notation/inotationplayback.h"
#include "notation/inotationsolomutestate.h"
#include "notation/tests/mocks/notationconfigurationmock.h"
#include "notation/tests/mocks/notationinteractionmock.h"
#include "notation/tests/mocks/notationselectionmock.h"
#include "playback/tests/mocks/playbackcontrollermock.h"

#include "notationscene/qml/MuseScore/NotationScene/pianokeyboard/pianokeyboardcontroller.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

using namespace muse;
using namespace mu;
using namespace mu::notation;

namespace {
class PlaybackStateFake : public context::IPlaybackState
{
public:
    bool isPlaying() const override
    {
        return m_status == audio::PlaybackStatus::Running;
    }

    audio::PlaybackStatus playbackStatus() const override
    {
        return m_status;
    }

    async::Channel<audio::PlaybackStatus> playbackStatusChanged() const override
    {
        return m_statusChanged;
    }

    audio::secs_t playbackPosition() const override
    {
        return m_position;
    }

    async::Channel<audio::secs_t> playbackPositionChanged() const override
    {
        return m_positionChanged;
    }

    void setStatus(audio::PlaybackStatus status)
    {
        m_status = status;
        m_statusChanged.send(status);
    }

    void setPosition(audio::secs_t position)
    {
        m_position = position;
        m_positionChanged.send(position);
    }

private:
    audio::PlaybackStatus m_status = audio::PlaybackStatus::Stopped;
    audio::secs_t m_position = 0.0;
    mutable async::Channel<audio::PlaybackStatus> m_statusChanged;
    mutable async::Channel<audio::secs_t> m_positionChanged;
};

class NotationMidiInputFake : public INotationMidiInput
{
public:
    void onMidiEventReceived(const midi::Event&) override
    {
    }

    async::Channel<std::vector<const engraving::Note*> > notesReceived() const override
    {
        return m_notesReceived;
    }

    void onRealtimeAdvance() override
    {
    }

    void sendNotes(const std::vector<const engraving::Note*>& notes)
    {
        m_notesReceived.send(notes);
    }

private:
    mutable async::Channel<std::vector<const engraving::Note*> > m_notesReceived;
};

class NotationSoloMuteStateFake : public INotationSoloMuteState
{
public:
    Ret read(const engraving::MscReader&, const io::path_t&) override
    {
        return Ret();
    }

    Ret write(io::IODevice*) override
    {
        return Ret();
    }

    bool trackSoloMuteStateExists(const engraving::InstrumentTrackId&) const override
    {
        return false;
    }

    const SoloMuteState& trackSoloMuteState(const engraving::InstrumentTrackId&) const override
    {
        return m_state;
    }

    void setTrackSoloMuteState(const engraving::InstrumentTrackId& trackId, const SoloMuteState& state) override
    {
        m_state = state;
        m_trackSoloMuteStateChanged.send(trackId, state);
    }

    void removeTrackSoloMuteState(const engraving::InstrumentTrackId&) override
    {
    }

    async::Channel<engraving::InstrumentTrackId, SoloMuteState> trackSoloMuteStateChanged() const override
    {
        return m_trackSoloMuteStateChanged;
    }

private:
    SoloMuteState m_state;
    mutable async::Channel<engraving::InstrumentTrackId, SoloMuteState> m_trackSoloMuteStateChanged;
};

class NotationMock : public INotation
{
public:
    MOCK_METHOD(const modularity::ContextPtr&, iocContext, (), (const, override));
    MOCK_METHOD(project::INotationProject*, project, (), (const, override));
    MOCK_METHOD(IMasterNotationPtr, masterNotation, (), (const, override));
    MOCK_METHOD(QString, name, (), (const, override));
    MOCK_METHOD(QString, projectName, (), (const, override));
    MOCK_METHOD(QString, projectNameAndPartName, (), (const, override));
    MOCK_METHOD(QString, workTitle, (), (const, override));
    MOCK_METHOD(QString, projectWorkTitle, (), (const, override));
    MOCK_METHOD(QString, projectWorkTitleAndPartName, (), (const, override));
    MOCK_METHOD(bool, isOpen, (), (const, override));
    MOCK_METHOD(void, setIsOpen, (bool), (override));
    MOCK_METHOD(async::Notification, openChanged, (), (const, override));
    MOCK_METHOD(bool, hasVisibleParts, (), (const, override));
    MOCK_METHOD(bool, isMaster, (), (const, override));
    MOCK_METHOD(ViewMode, viewMode, (), (const, override));
    MOCK_METHOD(void, setViewMode, (const ViewMode&), (override));
    MOCK_METHOD(async::Notification, viewModeChanged, (), (const, override));
    MOCK_METHOD(INotationPaintingPtr, painting, (), (const, override));
    MOCK_METHOD(INotationViewStatePtr, viewState, (), (const, override));
    MOCK_METHOD(INotationSoloMuteStatePtr, soloMuteState, (), (const, override));
    MOCK_METHOD(INotationInteractionPtr, interaction, (), (const, override));
    MOCK_METHOD(INotationMidiInputPtr, midiInput, (), (const, override));
    MOCK_METHOD(INotationUndoStackPtr, undoStack, (), (const, override));
    MOCK_METHOD(INotationStylePtr, style, (), (const, override));
    MOCK_METHOD(INotationElementsPtr, elements, (), (const, override));
    MOCK_METHOD(INotationAccessibilityPtr, accessibility, (), (const, override));
    MOCK_METHOD(INotationPartsPtr, parts, (), (const, override));
    MOCK_METHOD(async::Channel<RectF>, notationChanged, (), (const, override));
};

class GlobalContextFake : public context::IGlobalContext
{
public:
    void setCurrentProject(const project::INotationProjectPtr& project) override
    {
        m_project = project;
        m_projectChanged.notify();
    }

    project::INotationProjectPtr currentProject() const override
    {
        return m_project;
    }

    async::Notification currentProjectChanged() const override
    {
        return m_projectChanged;
    }

    IMasterNotationPtr currentMasterNotation() const override
    {
        return m_masterNotation;
    }

    async::Notification currentMasterNotationChanged() const override
    {
        return m_masterNotationChanged;
    }

    void setCurrentNotation(const INotationPtr& notation) override
    {
        m_notation = notation;
        m_notationChanged.notify();
    }

    INotationPtr currentNotation() const override
    {
        return m_notation;
    }

    async::Notification currentNotationChanged() const override
    {
        return m_notationChanged;
    }

    void setCurrentPlayer(const audio::IPlayerPtr&) override
    {
    }

    context::IPlaybackStatePtr playbackState() const override
    {
        return m_playbackState;
    }

    void setCurrentMasterNotation(const IMasterNotationPtr& notation)
    {
        m_masterNotation = notation;
        m_masterNotationChanged.notify();
    }

    void setPlaybackState(const context::IPlaybackStatePtr& state)
    {
        m_playbackState = state;
    }

private:
    project::INotationProjectPtr m_project;
    IMasterNotationPtr m_masterNotation;
    INotationPtr m_notation;
    context::IPlaybackStatePtr m_playbackState;
    mutable async::Notification m_projectChanged;
    mutable async::Notification m_masterNotationChanged;
    mutable async::Notification m_notationChanged;
};

class NotationPlaybackMock : public INotationPlayback
{
public:
    MOCK_METHOD(void, init, (), (override));
    MOCK_METHOD(void, reload, (), (override));
    MOCK_METHOD(void, setSendEventsOnScoreChange, (const engraving::InstrumentTrackId&, bool), (override));
    MOCK_METHOD(void, sendEventsForChangedTracks, (), (override));
    MOCK_METHOD(async::Channel<engraving::InstrumentTrackIdSet>, tracksDataChanged, (), (const, override));
    MOCK_METHOD(const engraving::InstrumentTrackId&, metronomeTrackId, (), (const, override));
    MOCK_METHOD(engraving::InstrumentTrackId, chordSymbolsTrackId, (const ID&), (const, override));
    MOCK_METHOD(bool, isChordSymbolsTrack, (const engraving::InstrumentTrackId&), (const, override));
    MOCK_METHOD(const mpe::PlaybackData&, trackPlaybackData, (const engraving::InstrumentTrackId&), (const, override));
    MOCK_METHOD(std::vector<midi::note_idx_t>, activePlaybackPitches,
                (audio::secs_t, const engraving::InstrumentTrackIdSet&), (const, override));
    MOCK_METHOD(void, triggerEventsForItems,
                (const std::vector<const engraving::EngravingItem*>&, mpe::duration_t, bool), (override));
    MOCK_METHOD(void, triggerMetronome, (midi::tick_t), (override));
    MOCK_METHOD(void, triggerCountIn, (midi::tick_t, secs_t &), (override));
    MOCK_METHOD(void, triggerControllers, (const mpe::ControllerChangeEventList&, engraving::staff_idx_t, int), (override));
    MOCK_METHOD(engraving::InstrumentTrackIdSet, existingTrackIdSet, (), (const, override));
    MOCK_METHOD(async::Channel<engraving::InstrumentTrackId>, trackAdded, (), (const, override));
    MOCK_METHOD(async::Channel<engraving::InstrumentTrackId>, trackRemoved, (), (const, override));
    MOCK_METHOD(audio::secs_t, totalPlayTime, (), (const, override));
    MOCK_METHOD(async::Channel<audio::secs_t>, totalPlayTimeChanged, (), (const, override));
    MOCK_METHOD(audio::secs_t, playedTickToSec, (midi::tick_t), (const, override));
    MOCK_METHOD(midi::tick_t, secToPlayedTick, (audio::secs_t), (const, override));
    MOCK_METHOD(midi::tick_t, secToTick, (audio::secs_t), (const, override));
    MOCK_METHOD((RetVal<midi::tick_t>), playPositionTickByRawTick, (midi::tick_t), (const, override));
    MOCK_METHOD((RetVal<midi::tick_t>), playPositionTickByElement, (const engraving::EngravingItem*), (const, override));
    MOCK_METHOD(void, addLoopBoundary, (LoopBoundaryType, midi::tick_t), (override));
    MOCK_METHOD(void, setLoopBoundariesEnabled, (bool), (override));
    MOCK_METHOD(bool, isLoopEnabled, (), (const, override));
    MOCK_METHOD(async::Channel<bool>, loopEnabledChanged, (), (const, override));
    MOCK_METHOD(const LoopBoundaries&, loopBoundaries, (), (const, override));
    MOCK_METHOD(async::Notification, loopBoundariesChanged, (), (const, override));
    MOCK_METHOD(const Tempo&, multipliedTempo, (midi::tick_t), (const, override));
    MOCK_METHOD(engraving::MeasureBeat, beat, (midi::tick_t), (const, override));
    MOCK_METHOD(midi::tick_t, beatToRawTick, (int, int), (const, override));
    MOCK_METHOD(double, tempoMultiplier, (), (const, override));
    MOCK_METHOD(void, setTempoMultiplier, (double), (override));
    MOCK_METHOD(void, addSoundFlags, (const std::vector<engraving::StaffText*>&), (override));
    MOCK_METHOD(void, removeSoundFlags, (const engraving::InstrumentTrackIdSet&), (override));
    MOCK_METHOD(bool, hasSoundFlags, (const engraving::InstrumentTrackIdSet&), (override));
};

class MasterNotationMock : public IMasterNotation
{
public:
    MOCK_METHOD(project::INotationProject*, project, (), (const, override));
    MOCK_METHOD(Ret, setupNewScore, (engraving::MasterScore*, const ScoreCreateOptions&), (override));
    MOCK_METHOD(void, applyOptions, (engraving::MasterScore*, const ScoreCreateOptions&, bool), (override));
    MOCK_METHOD(engraving::MasterScore*, masterScore, (), (const, override));
    MOCK_METHOD(void, setMasterScore, (engraving::MasterScore*, bool), (override));
    MOCK_METHOD(INotationPtr, notation, (), (override));
    MOCK_METHOD(int, mscVersion, (), (const, override));
    MOCK_METHOD(IExcerptNotationPtr, createEmptyExcerpt, (const QString&), (const, override));
    MOCK_METHOD(const ExcerptNotationList&, excerpts, (), (const, override));
    MOCK_METHOD(async::Notification, excerptsChanged, (), (const, override));
    MOCK_METHOD(const ExcerptNotationList&, potentialExcerpts, (), (const, override));
    MOCK_METHOD(void, initExcerpts, (const ExcerptNotationList&), (override));
    MOCK_METHOD(void, setExcerpts, (const ExcerptNotationList&), (override));
    MOCK_METHOD(void, resetExcerpt, (IExcerptNotationPtr &), (override));
    MOCK_METHOD(void, sortExcerpts, (ExcerptNotationList &), (override));
    MOCK_METHOD(void, setExcerptIsOpen, (const INotationPtr, bool), (override));
    MOCK_METHOD(INotationPartsPtr, parts, (), (const, override));
    MOCK_METHOD(bool, hasParts, (), (const, override));
    MOCK_METHOD(async::Notification, hasPartsChanged, (), (const, override));
    MOCK_METHOD(INotationPlaybackPtr, playback, (), (const, override));
    MOCK_METHOD(void, initNotationSoloMuteState, (const INotationPtr), (override));
    MOCK_METHOD(INotationAutomationPtr, automation, (), (const, override));
};

engraving::InstrumentTrackId makeTrackId(int partId, const char16_t* instrumentId)
{
    return engraving::InstrumentTrackId { ID(partId), String(instrumentId) };
}
}

class PianoKeyboardControllerTests : public ::testing::Test, public async::Asyncable
{
public:
    void SetUp() override
    {
        static modularity::IoCID nextContextId = 100;
        m_iocContext = std::make_shared<modularity::Context>(nextContextId++);

        m_playbackState = std::make_shared<PlaybackStateFake>();
        m_globalContext = std::make_shared<GlobalContextFake>();
        m_globalContext->setPlaybackState(m_playbackState);

        m_notationPlayback = std::make_shared<NiceMock<NotationPlaybackMock> >();
        m_masterNotation = std::make_shared<NiceMock<MasterNotationMock> >();
        ON_CALL(*m_masterNotation, playback()).WillByDefault(Return(m_notationPlayback));
        m_globalContext->setCurrentMasterNotation(m_masterNotation);

        m_soloMuteState = std::make_shared<NotationSoloMuteStateFake>();
        m_interaction = std::make_shared<NiceMock<NotationInteractionMock> >();
        m_midiInput = std::make_shared<NotationMidiInputFake>();
        m_notation = std::make_shared<NiceMock<NotationMock> >();
        ON_CALL(*m_notation, soloMuteState()).WillByDefault(Return(m_soloMuteState));
        ON_CALL(*m_notation, interaction()).WillByDefault(Return(m_interaction));
        ON_CALL(*m_notation, midiInput()).WillByDefault(Return(m_midiInput));
        ON_CALL(*m_interaction, selectionChanged()).WillByDefault(Return(m_selectionChanged));
        m_globalContext->setCurrentNotation(m_notation);

        m_playbackController = std::make_shared<NiceMock<playback::PlaybackControllerMock> >();
        m_audibleTracks = { makeTrackId(1, u"piano") };
        ON_CALL(*m_playbackController, audibleInstrumentTrackIds()).WillByDefault(Return(m_audibleTracks));

        modularity::ioc(m_iocContext)->registerExport<context::IGlobalContext>("piano_keyboard_tests", m_globalContext);
        modularity::ioc(m_iocContext)->registerExport<playback::IPlaybackController>("piano_keyboard_tests", m_playbackController);

        m_configuration = std::make_shared<NiceMock<NotationConfigurationMock> >();
        ON_CALL(*m_configuration, isCountInEnabled()).WillByDefault(Return(false));
        ON_CALL(*m_configuration, isPlayChordSymbolsChanged()).WillByDefault(Return(m_playChordSymbolsChanged));

        m_controller = std::make_unique<PianoKeyboardController>(m_iocContext);
        m_controller->notationConfiguration.set(m_configuration);
        m_controller->keyStatesChanged().onNotify(this, [this]() {
            ++m_keyStatesChangedCount;
        });
        m_controller->init();
    }

    void TearDown() override
    {
        m_controller->keyStatesChanged().disconnect(this);
        m_controller.reset();
        modularity::removeIoC(m_iocContext);
    }

protected:
    void enableTracking()
    {
        m_controller->setPlaybackTrackingEnabled(true);
    }

    std::unique_ptr<PianoKeyboardController> m_controller;
    modularity::ContextPtr m_iocContext;
    std::shared_ptr<PlaybackStateFake> m_playbackState;
    std::shared_ptr<GlobalContextFake> m_globalContext;
    std::shared_ptr<NiceMock<NotationPlaybackMock> > m_notationPlayback;
    std::shared_ptr<NiceMock<MasterNotationMock> > m_masterNotation;
    std::shared_ptr<NiceMock<NotationMock> > m_notation;
    std::shared_ptr<NotationSoloMuteStateFake> m_soloMuteState;
    std::shared_ptr<NiceMock<NotationInteractionMock> > m_interaction;
    std::shared_ptr<NotationMidiInputFake> m_midiInput;
    std::shared_ptr<NiceMock<playback::PlaybackControllerMock> > m_playbackController;
    std::shared_ptr<NiceMock<NotationConfigurationMock> > m_configuration;
    async::Notification m_selectionChanged;
    async::Notification m_playChordSymbolsChanged;
    engraving::InstrumentTrackIdSet m_audibleTracks;
    int m_keyStatesChangedCount = 0;
};

TEST_F(PianoKeyboardControllerTests, RunningQueriesImmediatelyAndUsesAudibleTracks)
{
    enableTracking();
    m_playbackState->setPosition(1.25);

    EXPECT_CALL(*m_playbackController, audibleInstrumentTrackIds())
    .WillOnce(Return(m_audibleTracks));
    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(1.25), m_audibleTracks))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 60, 64 }));

    m_playbackState->setStatus(audio::PlaybackStatus::Running);

    EXPECT_EQ(m_controller->keyState(60), KeyState::Played);
    EXPECT_EQ(m_controller->keyState(64), KeyState::Played);
    EXPECT_EQ(m_controller->keyState(65), KeyState::None);
    EXPECT_EQ(m_keyStatesChangedCount, 1);
}

TEST_F(PianoKeyboardControllerTests, CountInWaitsForFirstDifferentPosition)
{
    ON_CALL(*m_configuration, isCountInEnabled()).WillByDefault(Return(true));
    enableTracking();
    m_playbackState->setPosition(3.0);

    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(_, _)).Times(0);
    m_playbackState->setStatus(audio::PlaybackStatus::Running);
    m_playbackState->setPosition(3.0);
    EXPECT_EQ(m_keyStatesChangedCount, 0);

    testing::Mock::VerifyAndClearExpectations(m_notationPlayback.get());
    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(3.1), m_audibleTracks))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 67 }));

    m_playbackState->setPosition(3.1);

    EXPECT_EQ(m_controller->keyState(67), KeyState::Played);
    EXPECT_EQ(m_keyStatesChangedCount, 1);
}

TEST_F(PianoKeyboardControllerTests, ResumeFromPauseWaitsForCountInAgain)
{
    ON_CALL(*m_configuration, isCountInEnabled()).WillByDefault(Return(true));
    enableTracking();
    m_playbackState->setPosition(3.0);

    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(_, _)).Times(0);
    m_playbackState->setStatus(audio::PlaybackStatus::Running);
    m_playbackState->setPosition(3.0);

    testing::Mock::VerifyAndClearExpectations(m_notationPlayback.get());
    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(3.1), m_audibleTracks))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 67 }));
    m_playbackState->setPosition(3.1);
    m_playbackState->setStatus(audio::PlaybackStatus::Paused);

    testing::Mock::VerifyAndClearExpectations(m_notationPlayback.get());
    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(_, _)).Times(0);
    m_playbackState->setStatus(audio::PlaybackStatus::Running);
    m_playbackState->setPosition(3.1);

    testing::Mock::VerifyAndClearExpectations(m_notationPlayback.get());
    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(3.2), m_audibleTracks))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 69 }));
    m_playbackState->setPosition(3.2);

    EXPECT_EQ(m_controller->keyState(67), KeyState::None);
    EXPECT_EQ(m_controller->keyState(69), KeyState::Played);
    EXPECT_EQ(m_keyStatesChangedCount, 3);
}

TEST_F(PianoKeyboardControllerTests, PositionChangesAreIgnoredUnlessRunningAndSameKeysDoNotNotify)
{
    enableTracking();

    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(_, _)).Times(0);
    m_playbackState->setPosition(1.0);
    m_playbackState->setStatus(audio::PlaybackStatus::Paused);
    m_playbackState->setPosition(2.0);
    EXPECT_EQ(m_keyStatesChangedCount, 0);

    testing::Mock::VerifyAndClearExpectations(m_notationPlayback.get());
    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(_, _))
    .Times(2)
    .WillRepeatedly(Return(std::vector<midi::note_idx_t> { 60 }));

    m_playbackState->setStatus(audio::PlaybackStatus::Running);
    m_playbackState->setPosition(2.5);
    m_playbackState->setPosition(2.5);

    EXPECT_EQ(m_controller->keyState(60), KeyState::Played);
    EXPECT_EQ(m_keyStatesChangedCount, 1);
}

TEST_F(PianoKeyboardControllerTests, PauseStopAndResumePreservePressedKey)
{
    m_controller->setPressedKey(60);
    enableTracking();
    m_playbackState->setPosition(1.0);
    ON_CALL(*m_notationPlayback, activePlaybackPitches(_, _))
    .WillByDefault(Return(std::vector<midi::note_idx_t> { 64 }));

    m_playbackState->setStatus(audio::PlaybackStatus::Running);
    EXPECT_EQ(m_controller->keyState(60), KeyState::Played);
    EXPECT_EQ(m_controller->keyState(64), KeyState::Played);

    m_playbackState->setStatus(audio::PlaybackStatus::Paused);
    EXPECT_EQ(m_controller->keyState(60), KeyState::Played);
    EXPECT_EQ(m_controller->keyState(64), KeyState::None);

    m_playbackState->setStatus(audio::PlaybackStatus::Running);
    EXPECT_EQ(m_controller->keyState(64), KeyState::Played);

    m_playbackState->setStatus(audio::PlaybackStatus::Stopped);
    EXPECT_EQ(m_controller->keyState(60), KeyState::Played);
    EXPECT_EQ(m_controller->keyState(64), KeyState::None);
}

TEST_F(PianoKeyboardControllerTests, PlaybackPreservesSelectionAndMidiStates)
{
    muse::ValCh<bool> useWrittenPitch;
    useWrittenPitch.val = true;
    ON_CALL(*m_configuration, midiUseWrittenPitch()).WillByDefault(Return(useWrittenPitch));

    std::unique_ptr<engraving::MasterScore> score(engraving::compat::ScoreAccess::createMasterScore(nullptr));
    auto chord = engraving::Factory::makeChord(score->dummy()->segment());
    engraving::Note* note60 = engraving::Factory::createNote(chord.get());
    note60->setPitch(60);
    chord->add(note60);
    engraving::Note* note64 = engraving::Factory::createNote(chord.get());
    note64->setPitch(64);
    chord->add(note64);

    auto selection = std::make_shared<NiceMock<NotationSelectionMock> >();
    ON_CALL(*selection, isNone()).WillByDefault(Return(false));
    ON_CALL(*selection, notes(_)).WillByDefault(Return(std::vector<engraving::Note*> { note60 }));
    ON_CALL(*m_interaction, selection()).WillByDefault(Return(selection));
    m_selectionChanged.notify();

    EXPECT_EQ(m_controller->keyState(60), KeyState::Selected);
    EXPECT_EQ(m_controller->keyState(64), KeyState::OtherInSelectedChord);
    EXPECT_FALSE(m_controller->isFromMidi());

    enableTracking();
    m_playbackState->setPosition(1.0);
    ON_CALL(*m_notationPlayback, activePlaybackPitches(_, _))
    .WillByDefault(Return(std::vector<midi::note_idx_t> { 60, 67 }));
    m_playbackState->setStatus(audio::PlaybackStatus::Running);

    EXPECT_EQ(m_controller->keyState(60), KeyState::Played);
    EXPECT_EQ(m_controller->keyState(64), KeyState::OtherInSelectedChord);
    EXPECT_EQ(m_controller->keyState(67), KeyState::Played);

    m_midiInput->sendNotes({ note64 });
    EXPECT_TRUE(m_controller->isFromMidi());
    EXPECT_EQ(m_controller->keyState(60), KeyState::Played);
    EXPECT_EQ(m_controller->keyState(64), KeyState::Selected);
    EXPECT_EQ(m_controller->keyState(67), KeyState::Played);

    m_playbackState->setStatus(audio::PlaybackStatus::Paused);
    EXPECT_TRUE(m_controller->isFromMidi());
    EXPECT_EQ(m_controller->keyState(60), KeyState::OtherInSelectedChord);
    EXPECT_EQ(m_controller->keyState(64), KeyState::Selected);
    EXPECT_EQ(m_controller->keyState(67), KeyState::None);
}

TEST_F(PianoKeyboardControllerTests, SeekReplacesPlaybackKeys)
{
    enableTracking();
    m_playbackState->setPosition(1.0);
    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(1.0), _))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 60 }));
    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(8.0), _))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 72 }));

    m_playbackState->setStatus(audio::PlaybackStatus::Running);
    m_playbackState->setPosition(8.0);

    EXPECT_EQ(m_controller->keyState(60), KeyState::None);
    EXPECT_EQ(m_controller->keyState(72), KeyState::Played);
    EXPECT_EQ(m_keyStatesChangedCount, 2);
}

TEST_F(PianoKeyboardControllerTests, AudibleTracksChangeRefreshesAtSamePosition)
{
    enableTracking();
    m_playbackState->setPosition(1.0);
    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(1.0), m_audibleTracks))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 60 }));
    m_playbackState->setStatus(audio::PlaybackStatus::Running);

    const engraving::InstrumentTrackIdSet replacementTracks = { makeTrackId(2, u"violin") };
    EXPECT_CALL(*m_playbackController, audibleInstrumentTrackIds())
    .WillOnce(Return(replacementTracks));
    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(1.0), replacementTracks))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 67 }));

    m_soloMuteState->setTrackSoloMuteState(*replacementTracks.cbegin(), { false, true });

    EXPECT_EQ(m_controller->keyState(60), KeyState::None);
    EXPECT_EQ(m_controller->keyState(67), KeyState::Played);
    EXPECT_EQ(m_keyStatesChangedCount, 2);
}

TEST_F(PianoKeyboardControllerTests, ChordSymbolsConfigurationChangeRefreshesAtSamePosition)
{
    enableTracking();
    m_playbackState->setPosition(1.0);
    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(1.0), _))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 60 }));
    m_playbackState->setStatus(audio::PlaybackStatus::Running);

    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(1.0), _))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 64 }));

    m_playChordSymbolsChanged.notify();

    EXPECT_EQ(m_controller->keyState(60), KeyState::None);
    EXPECT_EQ(m_controller->keyState(64), KeyState::Played);
    EXPECT_EQ(m_keyStatesChangedCount, 2);
}

TEST_F(PianoKeyboardControllerTests, SelectionChangeRefreshesRangePlaybackAtSamePosition)
{
    enableTracking();
    m_playbackState->setPosition(1.0);
    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(1.0), _))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 60 }));
    m_playbackState->setStatus(audio::PlaybackStatus::Running);

    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(1.0), _))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 72 }));

    m_selectionChanged.notify();

    EXPECT_EQ(m_controller->keyState(60), KeyState::None);
    EXPECT_EQ(m_controller->keyState(72), KeyState::Played);
    EXPECT_EQ(m_keyStatesChangedCount, 2);
}

TEST_F(PianoKeyboardControllerTests, DisabledTrackingDoesNotQueryOrNotifyAndReopenRefreshes)
{
    m_playbackState->setPosition(2.0);
    ON_CALL(*m_configuration, isCountInEnabled()).WillByDefault(Return(true));

    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(_, _)).Times(0);
    m_playbackState->setStatus(audio::PlaybackStatus::Running);
    m_playbackState->setPosition(2.0);
    m_playbackState->setPosition(2.1);
    m_soloMuteState->setTrackSoloMuteState(*m_audibleTracks.cbegin(), { true, false });
    m_playChordSymbolsChanged.notify();
    m_selectionChanged.notify();
    EXPECT_EQ(m_keyStatesChangedCount, 0);

    testing::Mock::VerifyAndClearExpectations(m_notationPlayback.get());
    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(2.1), m_audibleTracks))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 69 }));

    enableTracking();

    EXPECT_EQ(m_controller->keyState(69), KeyState::Played);
    EXPECT_EQ(m_keyStatesChangedCount, 1);

    m_controller->setPlaybackTrackingEnabled(false);
    EXPECT_EQ(m_controller->keyState(69), KeyState::None);
    EXPECT_EQ(m_keyStatesChangedCount, 1);

    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(2.1), m_audibleTracks))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 69 }));
    enableTracking();
    EXPECT_EQ(m_controller->keyState(69), KeyState::Played);
    EXPECT_EQ(m_keyStatesChangedCount, 2);
}

TEST_F(PianoKeyboardControllerTests, CurrentNotationSwitchRefreshesAndDisconnectsOldSoloMuteState)
{
    enableTracking();
    m_playbackState->setPosition(1.0);
    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(1.0), _))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 60 }));
    m_playbackState->setStatus(audio::PlaybackStatus::Running);

    auto replacementSoloMuteState = std::make_shared<NotationSoloMuteStateFake>();
    auto replacementInteraction = std::make_shared<NiceMock<NotationInteractionMock> >();
    auto replacementMidiInput = std::make_shared<NotationMidiInputFake>();
    auto replacementNotation = std::make_shared<NiceMock<NotationMock> >();
    ON_CALL(*replacementNotation, soloMuteState()).WillByDefault(Return(replacementSoloMuteState));
    ON_CALL(*replacementNotation, interaction()).WillByDefault(Return(replacementInteraction));
    ON_CALL(*replacementNotation, midiInput()).WillByDefault(Return(replacementMidiInput));

    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(1.0), m_audibleTracks))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 65 }));
    m_globalContext->setCurrentNotation(replacementNotation);

    EXPECT_EQ(m_controller->keyState(60), KeyState::None);
    EXPECT_EQ(m_controller->keyState(65), KeyState::Played);

    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(_, _)).Times(0);
    m_soloMuteState->setTrackSoloMuteState(*m_audibleTracks.cbegin(), { true, false });

    testing::Mock::VerifyAndClearExpectations(m_notationPlayback.get());
    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(1.0), m_audibleTracks))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 69 }));
    replacementSoloMuteState->setTrackSoloMuteState(*m_audibleTracks.cbegin(), { false, true });

    EXPECT_EQ(m_controller->keyState(65), KeyState::None);
    EXPECT_EQ(m_controller->keyState(69), KeyState::Played);
}

TEST_F(PianoKeyboardControllerTests, MasterNotationSwitchAndMissingMasterRefreshPlayback)
{
    enableTracking();
    m_playbackState->setPosition(1.0);
    EXPECT_CALL(*m_notationPlayback, activePlaybackPitches(audio::secs_t(1.0), _))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 60 }));
    m_playbackState->setStatus(audio::PlaybackStatus::Running);

    auto replacementPlayback = std::make_shared<NiceMock<NotationPlaybackMock> >();
    auto replacementMaster = std::make_shared<NiceMock<MasterNotationMock> >();
    ON_CALL(*replacementMaster, playback()).WillByDefault(Return(replacementPlayback));
    EXPECT_CALL(*replacementPlayback, activePlaybackPitches(audio::secs_t(1.0), m_audibleTracks))
    .WillOnce(Return(std::vector<midi::note_idx_t> { 65 }));

    m_globalContext->setCurrentMasterNotation(replacementMaster);
    EXPECT_EQ(m_controller->keyState(60), KeyState::None);
    EXPECT_EQ(m_controller->keyState(65), KeyState::Played);

    m_globalContext->setCurrentMasterNotation(nullptr);
    EXPECT_EQ(m_controller->keyState(65), KeyState::None);
}

TEST_F(PianoKeyboardControllerTests, DestructionDisconnectsPlaybackSubscriptions)
{
    m_controller->keyStatesChanged().disconnect(this);
    m_controller.reset();

    m_playbackState->setStatus(audio::PlaybackStatus::Running);
    m_playbackState->setPosition(10.0);
    SUCCEED();
}
