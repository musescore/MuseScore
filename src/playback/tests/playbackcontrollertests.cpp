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

#include <optional>
#include <utility>
#include <vector>

#include "async/processevents.h"
#include "modularity/ioc.h"

#include "internal/playbackcontroller.h"

#include "rcommand/tests/mocks/commanddispatchermock.h"

#include "project/tests/mocks/projectsessionmock.h"
#include "project/tests/mocks/notationprojectmock.h"
#include "project/tests/mocks/projectaudiosettingsmock.h"

#include "notation/internal/notationsolomutestate.h"
#include "notation/tests/mocks/masternotationmock.h"
#include "notation/tests/mocks/notationautomationmock.h"
#include "notation/tests/mocks/notationconfigurationmock.h"
#include "notation/tests/mocks/notationinteractionmock.h"
#include "notation/tests/mocks/notationmock.h"
#include "notation/tests/mocks/notationpartsmock.h"
#include "notation/tests/mocks/notationplaybackmock.h"
#include "notation/tests/mocks/notationselectionmock.h"
#include "notation/tests/mocks/notationselectionrangemock.h"

#include "mocks/playbackconfigurationmock.h"
#include "mocks/playbackmock.h"
#include "mocks/playermock.h"

using ::testing::_;
using ::testing::AllOf;
using ::testing::An;
using ::testing::Field;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::ReturnRefOfCopy;

using namespace muse;

static const std::string TEST_MODULE("playback_utests");
static const muse::modularity::IoCID TEST_CONTEXT_ID = 1;

namespace mu::playback {
static const engraving::InstrumentTrackId TRACK_ID = { muse::ID(1), u"piano" };
static const audio::TrackId AUDIO_TRACK_ID = 7;
static constexpr int TICKS_PER_SEC = 480;

static const notation::AutomationCurveKey MASTER_VOLUME_KEY = notation::AutomationCurveKey::global(notation::AutomationType::Volume);
static const notation::AutomationCurveKey TRACK_VOLUME_KEY
    = notation::AutomationCurveKey::instrument(notation::AutomationType::Volume, TRACK_ID);
static const notation::AutomationCurveKey TRACK_PAN_KEY = notation::AutomationCurveKey::instrument(notation::AutomationType::Pan, TRACK_ID);

using EnvelopePoints = std::vector<std::pair<double, double> >;

//! NOTE (secs, value) pairs, or nullopt for a fixed value
template<typename T>
static std::optional<EnvelopePoints> envelopePoints(const audio::AutomatableValue<T>& value)
{
    const audio::AutomationEnvelope* envelope = std::get_if<audio::AutomationEnvelope>(&value.value());
    if (!envelope) {
        return std::nullopt;
    }

    EnvelopePoints result;
    for (const auto& [secs, point] : *envelope) {
        result.emplace_back(secs.raw(), point.outValue.raw());
    }

    return result;
}

static engraving::AutomationPoint makePoint(double value)
{
    engraving::AutomationPoint point;
    point.value.inValue = mpe::AutomationPoint::ExplicitArrival { muse::real_t(value), {} };
    point.value.outValue = muse::real_t(value);

    return point;
}

static notation::AutomationPointEdits setPoint(engraving::utick_t tick, double value)
{
    return { notation::AutomationPointEdit { tick, notation::AutomationPointEdit::SetPoint { makePoint(value) } } };
}

static notation::AutomationPointEdits erasePoint(engraving::utick_t tick)
{
    return { notation::AutomationPointEdit { tick, notation::AutomationPointEdit::ErasePoint {} } };
}

MATCHER_P(VolumeEnvelopeIs, points, "") {
    return envelopePoints(arg.volume) == points;
}

MATCHER_P(PanEnvelopeIs, points, "") {
    return envelopePoints(arg.balance) == points;
}

MATCHER_P(FixedVolumeIs, volume, "") {
    return arg.volume == audio::AutomatableValue<audio::volume_db_t>(volume);
}

MATCHER_P(FixedPanIs, balance, "") {
    return arg.balance == audio::AutomatableValue<audio::balance_t>(balance);
}

class PlaybackControllerTests : public ::testing::Test, public async::Asyncable
{
protected:
    void SetUp() override
    {
        m_playback = std::make_shared<NiceMock<audio::PlaybackMock> >();
        m_projectSession = std::make_shared<NiceMock<project::ProjectSessionMock> >();
        m_configuration = std::make_shared<NiceMock<PlaybackConfigurationMock> >();
        m_commandDispatcher = std::make_shared<NiceMock<rcommand::CommandDispatcherMock> >();
        m_project = std::make_shared<NiceMock<project::NotationProjectMock> >();
        m_notationConfiguration = std::make_shared<NiceMock<notation::NotationConfigurationMock> >();
        m_audioSettings = std::make_shared<NiceMock<project::ProjectAudioSettingsMock> >();
        m_player = std::make_shared<NiceMock<audio::PlayerMock> >();

        m_notation = std::make_shared<NiceMock<notation::NotationMock> >();
        m_masterNotation = std::make_shared<NiceMock<notation::MasterNotationMock> >();
        m_notationPlayback = std::make_shared<NiceMock<notation::NotationPlaybackMock> >();
        m_notationAutomation = std::make_shared<NiceMock<notation::NotationAutomationMock> >();
        m_notationParts = std::make_shared<NiceMock<notation::NotationPartsMock> >();
        m_notationInteraction = std::make_shared<NiceMock<notation::NotationInteractionMock> >();
        m_notationSelection = std::make_shared<NiceMock<notation::NotationSelectionMock> >();
        m_notationSelectionRange = std::make_shared<NiceMock<notation::NotationSelectionRangeMock> >();

        m_iocCtx = std::make_shared<modularity::Context>(TEST_CONTEXT_ID);

        modularity::ioc(m_iocCtx)->registerExport<audio::IPlayback>(TEST_MODULE, m_playback);
        modularity::ioc(m_iocCtx)->registerExport<project::IProjectSession>(TEST_MODULE, m_projectSession);
        modularity::ioc(m_iocCtx)->registerExport<rcommand::ICommandDispatcher>(TEST_MODULE, m_commandDispatcher);

        modularity::globalIoc()->registerExport<IPlaybackConfiguration>(TEST_MODULE, m_configuration);
        modularity::globalIoc()->registerExport<notation::INotationConfiguration>(TEST_MODULE, m_notationConfiguration);

        ON_CALL(*m_projectSession, projectChanged()).WillByDefault(Return(m_currentProjectChanged));
        ON_CALL(*m_projectSession, currentNotationChanged()).WillByDefault(Return(m_currentNotationChanged));
        ON_CALL(*m_projectSession, project()).WillByDefault(Return(project::INotationProjectPtr()));
        ON_CALL(*m_projectSession, currentNotation()).WillByDefault(Return(notation::INotationPtr()));

        //! NOTE The audio context stays inited until a test resolves it, as it is in the app:
        //! init() is an RPC round trip to the audio engine
        ON_CALL(*m_playback, init()).WillByDefault([this]() { return makePendingInitPromise(); });
        ON_CALL(*m_playback, isInited()).WillByDefault([this]() { return m_isAudioContextInited; });
        ON_CALL(*m_playback, player()).WillByDefault(Return(audio::IPlayerPtr(m_player)));

        ON_CALL(*m_project, audioSettings()).WillByDefault(Return(project::IProjectAudioSettingsPtr(m_audioSettings)));

        ON_CALL(*m_notation, masterNotation()).WillByDefault(Return(notation::IMasterNotationPtr(m_masterNotation)));
        ON_CALL(*m_notation, parts()).WillByDefault(Return(notation::INotationPartsPtr(m_notationParts)));
        ON_CALL(*m_notation, interaction()).WillByDefault(Return(notation::INotationInteractionPtr(m_notationInteraction)));
        ON_CALL(*m_notationInteraction, selection()).WillByDefault(Return(notation::INotationSelectionPtr(m_notationSelection)));
        ON_CALL(*m_notationSelection, range()).WillByDefault(Return(notation::INotationSelectionRangePtr(m_notationSelectionRange)));

        ON_CALL(*m_masterNotation, playback()).WillByDefault(Return(notation::INotationPlaybackPtr(m_notationPlayback)));
        ON_CALL(*m_masterNotation, automation()).WillByDefault(Return(notation::INotationAutomationPtr(m_notationAutomation)));
        ON_CALL(*m_masterNotation, parts()).WillByDefault(Return(notation::INotationPartsPtr(m_notationParts)));

        m_controller = std::make_shared<PlaybackController>(m_iocCtx);
    }

    void TearDown() override
    {
        //! NOTE The controller must go first: it holds the mocks via the IoC
        m_controller = nullptr;

        drainDeferredCalls();

        modularity::globalIoc()->unregister<IPlaybackConfiguration>(TEST_MODULE);
        modularity::globalIoc()->unregister<notation::INotationConfiguration>(TEST_MODULE);
        modularity::removeIoC(m_iocCtx);
    }

    //! NOTE A deferred call may queue another one, hence the repetition
    static void drainDeferredCalls()
    {
        for (int i = 0; i < 10; ++i) {
            async::processMessages();
        }
    }

    async::Promise<Ret> makePendingInitPromise()
    {
        return async::make_promise<Ret>([this](auto resolve, auto) {
            m_resolveAudioContextInit = [resolve]() { (void)resolve(make_ok()); };
            return async::Promise<Ret>::dummy_result();
        }, async::PromiseType::AsyncByBody);
    }

    void resolveAudioContextInit()
    {
        m_isAudioContextInited = true;
        m_resolveAudioContextInit();
    }

    void openProject()
    {
        ON_CALL(*m_projectSession, project()).WillByDefault(Return(project::INotationProjectPtr(m_project)));

        //! NOTE ProjectSession::setProject notifies in this order
        m_currentProjectChanged.notify();
        m_currentNotationChanged.notify();
    }

    void closeProject()
    {
        ON_CALL(*m_projectSession, project()).WillByDefault(Return(project::INotationProjectPtr()));

        m_currentProjectChanged.notify();
        m_currentNotationChanged.notify();
    }

    //! NOTE Wiring only: a notation with a single track,
    //! so the playback can be fully set up without a score
    void setUpProject(const notation::AutomationDataPtr& automationData)
    {
        ON_CALL(*m_projectSession, currentNotation()).WillByDefault(Return(notation::INotationPtr(m_notation)));

        ON_CALL(*m_notation, soloMuteState()).WillByDefault(Return(notation::INotationSoloMuteStatePtr(
                                                                       std::make_shared<notation::NotationSoloMuteState>())));
        ON_CALL(*m_notation, hasVisibleParts()).WillByDefault(Return(true));

        //! NOTE A NotifyList asserts on subscribing without a notifier
        const async::NotifyList<const engraving::Part*> partList(std::make_shared<async::ChangedNotify<const engraving::Part*> >());
        ON_CALL(*m_notationParts, partList()).WillByDefault(Return(partList));

        ON_CALL(*m_masterNotation, excerpts()).WillByDefault(ReturnRefOfCopy(notation::ExcerptNotationList()));

        ON_CALL(*m_notationAutomation, automationData()).WillByDefault(Return(notation::AutomationDataConstPtr(automationData)));

        mpe::PlaybackData playbackData;
        playbackData.setupData = mpe::PlaybackSetupData(mpe::SoundId::Piano, mpe::SoundCategory::Keyboards);

        ON_CALL(*m_notationPlayback, metronomeTrackId()).WillByDefault(ReturnRefOfCopy(engraving::InstrumentTrackId()));
        ON_CALL(*m_notationPlayback, trackName(TRACK_ID)).WillByDefault(Return(std::optional<std::string>("Piano")));
        ON_CALL(*m_notationPlayback, existingTrackIdSet()).WillByDefault(Return(engraving::InstrumentTrackIdSet { TRACK_ID }));
        ON_CALL(*m_notationPlayback, trackPlaybackData(_)).WillByDefault(ReturnRefOfCopy(playbackData));
        ON_CALL(*m_notationPlayback, multipliedTempo(_)).WillByDefault(ReturnRefOfCopy(notation::Tempo()));
        ON_CALL(*m_notationPlayback, playedTickToSec(_)).WillByDefault([](midi::tick_t tick) {
            return audio::secs_t(tick / static_cast<double>(TICKS_PER_SEC));
        });

        //! NOTE A valid input skips the sound profile lookup
        audio::AudioInputParams inputParams;
        inputParams.resourceMeta.id = "test";
        inputParams.resourceMeta.vendor = "test";
        inputParams.resourceMeta.type = std::string(audio::FLUID_SOUNDFONT_TYPE_NAME);

        ON_CALL(*m_audioSettings, masterAudioOutputParams()).WillByDefault(ReturnRefOfCopy(project::AudioOutputParams()));
        ON_CALL(*m_audioSettings, trackOutputParams(_)).WillByDefault(ReturnRefOfCopy(project::AudioOutputParams()));
        ON_CALL(*m_audioSettings, trackInputParams(_)).WillByDefault(ReturnRefOfCopy(inputParams));

        ON_CALL(*m_playback, addTrack(_, An<const mpe::PlaybackData&>(), _))
        .WillByDefault([](const auto&, const auto&, const audio::TrackParams& params) {
            return async::make_promise<audio::TrackId, audio::TrackParams>([params](auto resolve, auto) {
                return resolve(AUDIO_TRACK_ID, params);
            });
        });

        ON_CALL(*m_playback, addAuxTrack(_, _)).WillByDefault([](const auto&, const auto&) {
            return async::make_promise<audio::TrackId, audio::TrackParams>([](auto, auto reject) {
                return reject(int(Ret::Code::UnknownError), "aux tracks are not needed");
            });
        });
    }

    void startPlayback()
    {
        m_controller->init();
        resolveAudioContextInit();

        openProject();

        //! NOTE Resolves the addTrack() promises
        drainDeferredCalls();
    }

    std::shared_ptr<PlaybackController> m_controller;

    std::shared_ptr<NiceMock<audio::PlaybackMock> > m_playback;
    std::shared_ptr<NiceMock<project::ProjectSessionMock> > m_projectSession;
    std::shared_ptr<NiceMock<PlaybackConfigurationMock> > m_configuration;
    std::shared_ptr<NiceMock<rcommand::CommandDispatcherMock> > m_commandDispatcher;
    std::shared_ptr<NiceMock<project::NotationProjectMock> > m_project;
    std::shared_ptr<NiceMock<notation::NotationConfigurationMock> > m_notationConfiguration;
    std::shared_ptr<NiceMock<project::ProjectAudioSettingsMock> > m_audioSettings;
    std::shared_ptr<NiceMock<audio::PlayerMock> > m_player;

    std::shared_ptr<NiceMock<notation::NotationMock> > m_notation;
    std::shared_ptr<NiceMock<notation::MasterNotationMock> > m_masterNotation;
    std::shared_ptr<NiceMock<notation::NotationPlaybackMock> > m_notationPlayback;
    std::shared_ptr<NiceMock<notation::NotationAutomationMock> > m_notationAutomation;
    std::shared_ptr<NiceMock<notation::NotationPartsMock> > m_notationParts;
    std::shared_ptr<NiceMock<notation::NotationInteractionMock> > m_notationInteraction;
    std::shared_ptr<NiceMock<notation::NotationSelectionMock> > m_notationSelection;
    std::shared_ptr<NiceMock<notation::NotationSelectionRangeMock> > m_notationSelectionRange;

    modularity::ContextPtr m_iocCtx;

    async::Notification m_currentProjectChanged;
    async::Notification m_currentNotationChanged;

    bool m_isAudioContextInited = false;
    std::function<void()> m_resolveAudioContextInit;
};

TEST_F(PlaybackControllerTests, InitsTheAudioContextWithoutAProject)
{
    // [GIVEN] No project is open
    // [THEN] The audio context is inited anyway - it does not depend on a project,
    // and the available sounds are requested as soon as it is inited
    EXPECT_CALL(*m_playback, init()).Times(1);

    // [THEN] But nothing is set up for a project
    EXPECT_CALL(*m_playback, player()).Times(0);

    // [WHEN] The controller is inited
    m_controller->init();
    resolveAudioContextInit();
}

TEST_F(PlaybackControllerTests, InitsTheAudioContextOnlyOnce)
{
    // [THEN] The audio context is inited once per controller, not once per project
    EXPECT_CALL(*m_playback, init()).Times(1);

    // [WHEN] The controller is inited and projects are opened and closed
    m_controller->init();
    resolveAudioContextInit();

    openProject();
    closeProject();
    openProject();
}

TEST_F(PlaybackControllerTests, SetsUpThePlaybackOfAProjectOpenedAfterTheAudioContextIsInited)
{
    // [GIVEN] The audio context is inited
    m_controller->init();
    resolveAudioContextInit();

    // [THEN] The playback is set up
    EXPECT_CALL(*m_playback, player()).Times(1);

    // [WHEN] A project is opened
    openProject();
}

TEST_F(PlaybackControllerTests, DefersTheSetupUntilTheAudioContextIsInited)
{
    // [GIVEN] The controller is inited, but the audio context is not inited yet
    m_controller->init();

    // [THEN] The playback is not set up yet
    EXPECT_CALL(*m_playback, player()).Times(0);

    // [WHEN] A project is opened (from the command line, for example)
    openProject();

    Mock::VerifyAndClearExpectations(m_playback.get());

    // [THEN] The playback is set up as soon as the audio context is inited
    EXPECT_CALL(*m_playback, player()).Times(1);

    // [WHEN] The audio context is inited
    resolveAudioContextInit();
}

TEST_F(PlaybackControllerTests, DoesNotSetUpThePlaybackWithoutAProject)
{
    // [GIVEN] The audio context is inited
    m_controller->init();
    resolveAudioContextInit();

    // [THEN] Nothing is set up - there is no project
    EXPECT_CALL(*m_playback, player()).Times(0);

    // [WHEN] The current project is reset to none
    m_currentProjectChanged.notify();
    m_currentNotationChanged.notify();
}

TEST_F(PlaybackControllerTests, DoesNotSetUpThePlaybackAgainOnANotationChange)
{
    // [GIVEN] A project whose playback is already set up
    m_controller->init();
    resolveAudioContextInit();
    openProject();

    Mock::VerifyAndClearExpectations(m_playback.get());

    // [THEN] The playback is not set up again - the notation change is not a project change
    EXPECT_CALL(*m_playback, player()).Times(0);

    // [WHEN] Only the current notation changes (switching to a part, for example)
    m_currentNotationChanged.notify();
}

TEST_F(PlaybackControllerTests, KeepsTheAudioContextWhenTheProjectIsClosed)
{
    // [GIVEN] A project whose playback is already set up
    m_controller->init();
    resolveAudioContextInit();
    openProject();

    Mock::VerifyAndClearExpectations(m_playback.get());

    // [THEN] The audio context stays inited: it outlives the project,
    // so the sounds received for it are not thrown away
    //! NOTE This does not reach resetPlayback(): the playback is not fully set up here
    //! (see the note on the fixture), so it guards the project-changed handler only
    EXPECT_CALL(*m_playback, deinit()).Times(0);

    // [WHEN] The project is closed
    closeProject();
}

TEST_F(PlaybackControllerTests, DeinitsTheAudioContextWithTheController)
{
    // [GIVEN] A project whose playback is already set up
    m_controller->init();
    resolveAudioContextInit();
    openProject();

    Mock::VerifyAndClearExpectations(m_playback.get());

    // [THEN] The audio context is deinited
    EXPECT_CALL(*m_playback, deinit()).Times(1);

    // [WHEN] The controller is deinited
    m_controller->deinit();
}

TEST_F(PlaybackControllerTests, DoesNotSetUpThePlaybackAfterTheDeinit)
{
    // [GIVEN] A project opened while the audio context init is still pending
    m_controller->init();
    openProject();

    // [GIVEN] The controller is deinited
    m_controller->deinit();

    // [THEN] The pending setup is dropped - we are still subscribed to the init
    EXPECT_CALL(*m_playback, player()).Times(0);

    // [WHEN] The audio context init resolves after the deinit
    resolveAudioContextInit();
}

TEST_F(PlaybackControllerTests, SendsEnvelopesOnSetup)
{
    // [GIVEN] Master Volume and track Volume/Pan automation
    notation::AutomationDataPtr automationData = std::make_shared<notation::AutomationData>();
    automationData->editPoints(MASTER_VOLUME_KEY, setPoint(TICKS_PER_SEC, 0.25));
    automationData->editPoints(TRACK_VOLUME_KEY, setPoint(TICKS_PER_SEC, 0.5));
    automationData->editPoints(TRACK_PAN_KEY, setPoint(2 * TICKS_PER_SEC, 0.75));

    setUpProject(automationData);

    // [THEN] The master gets its Volume envelope, ticks converted to seconds
    EXPECT_CALL(*m_playback, setMasterControlParams(AllOf(VolumeEnvelopeIs(EnvelopePoints { { 1.0, 0.25 } }),
                                                          FixedPanIs(audio::balance_t(0.f))))).Times(1);

    // [THEN] The track is added with its Volume and Pan envelopes
    EXPECT_CALL(*m_playback, addTrack(_, An<const mpe::PlaybackData&>(),
                                      Field(&audio::TrackParams::control, AllOf(VolumeEnvelopeIs(EnvelopePoints { { 1.0, 0.5 } }),
                                                                                PanEnvelopeIs(EnvelopePoints { { 2.0, 0.75 } })))))
    .Times(1);

    // [WHEN] The project is opened
    startPlayback();
}

TEST_F(PlaybackControllerTests, SendsFaderValuesWhenThereIsNoAutomation)
{
    // [GIVEN] Mixer fader values and no automation
    setUpProject(std::make_shared<notation::AutomationData>());

    project::AudioOutputParams masterFaders;
    masterFaders.volume = audio::volume_db_t(-6.f);
    masterFaders.balance = audio::balance_t(0.5f);
    ON_CALL(*m_audioSettings, masterAudioOutputParams()).WillByDefault(ReturnRefOfCopy(masterFaders));

    project::AudioOutputParams trackFaders;
    trackFaders.volume = audio::volume_db_t(-3.f);
    trackFaders.balance = audio::balance_t(-0.5f);
    ON_CALL(*m_audioSettings, trackOutputParams(_)).WillByDefault(ReturnRefOfCopy(trackFaders));

    // [THEN] The master and the track get the fader values
    EXPECT_CALL(*m_playback, setMasterControlParams(AllOf(FixedVolumeIs(masterFaders.volume),
                                                          FixedPanIs(masterFaders.balance)))).Times(1);

    EXPECT_CALL(*m_playback, addTrack(_, An<const mpe::PlaybackData&>(),
                                      Field(&audio::TrackParams::control, AllOf(FixedVolumeIs(trackFaders.volume),
                                                                                FixedPanIs(trackFaders.balance))))).Times(1);

    // [WHEN] The project is opened
    startPlayback();
}

TEST_F(PlaybackControllerTests, ResendsOnlyMasterOnMasterVolumeEdit)
{
    // [GIVEN] An opened project
    notation::AutomationDataPtr automationData = std::make_shared<notation::AutomationData>();
    setUpProject(automationData);
    startPlayback();

    // [THEN] Only the master is resent, with the new envelope
    EXPECT_CALL(*m_playback, setMasterControlParams(VolumeEnvelopeIs(EnvelopePoints { { 1.0, 0.25 } }))).Times(1);
    EXPECT_CALL(*m_playback, setControlParams(_, _)).Times(0);

    // [WHEN] A master Volume point is added
    automationData->editPoints(MASTER_VOLUME_KEY, setPoint(TICKS_PER_SEC, 0.25));
}

TEST_F(PlaybackControllerTests, ResendsOnlyTheTrackOnTrackVolumeEdit)
{
    // [GIVEN] An opened project
    notation::AutomationDataPtr automationData = std::make_shared<notation::AutomationData>();
    setUpProject(automationData);
    startPlayback();

    // [THEN] Only the track is resent, with the new envelope
    EXPECT_CALL(*m_playback, setControlParams(AUDIO_TRACK_ID, VolumeEnvelopeIs(EnvelopePoints { { 1.0, 0.5 } }))).Times(1);
    EXPECT_CALL(*m_playback, setMasterControlParams(_)).Times(0);

    // [WHEN] A track Volume point is added
    automationData->editPoints(TRACK_VOLUME_KEY, setPoint(TICKS_PER_SEC, 0.5));
}

TEST_F(PlaybackControllerTests, KeepsPanEnvelopeWhenOnlyVolumeChanges)
{
    // [GIVEN] A track with Pan automation
    notation::AutomationDataPtr automationData = std::make_shared<notation::AutomationData>();
    automationData->editPoints(TRACK_PAN_KEY, setPoint(TICKS_PER_SEC, 0.75));
    setUpProject(automationData);
    startPlayback();

    // [THEN] The cached Pan envelope is sent along with the new Volume one
    EXPECT_CALL(*m_playback, setControlParams(AUDIO_TRACK_ID, AllOf(VolumeEnvelopeIs(EnvelopePoints { { 2.0, 0.5 } }),
                                                                    PanEnvelopeIs(EnvelopePoints { { 1.0, 0.75 } })))).Times(1);

    // [WHEN] Only the track Volume changes
    automationData->editPoints(TRACK_VOLUME_KEY, setPoint(2 * TICKS_PER_SEC, 0.5));
}

TEST_F(PlaybackControllerTests, KeepsEnvelopesWhenTheTrackIsMuted)
{
    // [GIVEN] A track with Volume automation
    notation::AutomationDataPtr automationData = std::make_shared<notation::AutomationData>();
    automationData->editPoints(TRACK_VOLUME_KEY, setPoint(TICKS_PER_SEC, 0.5));
    setUpProject(automationData);
    startPlayback();

    // [THEN] The track is muted, and the envelope is still applied, not replaced by the fader value
    EXPECT_CALL(*m_playback, setControlParams(AUDIO_TRACK_ID, AllOf(VolumeEnvelopeIs(EnvelopePoints { { 1.0, 0.5 } }),
                                                                    Field(&audio::ControlParams::muted, true)))).Times(1);

    // [WHEN] The track is muted
    m_controller->setTrackSoloMuteState(TRACK_ID, { /*mute*/ true, /*solo*/ false });
}

TEST_F(PlaybackControllerTests, ResendsAllEnvelopesOnTempoChange)
{
    // [GIVEN] Master and track Volume automation
    notation::AutomationDataPtr automationData = std::make_shared<notation::AutomationData>();
    automationData->editPoints(MASTER_VOLUME_KEY, setPoint(TICKS_PER_SEC, 0.25));
    automationData->editPoints(TRACK_VOLUME_KEY, setPoint(TICKS_PER_SEC, 0.5));
    setUpProject(automationData);
    startPlayback();

    // [GIVEN] The tempo doubles
    ON_CALL(*m_notationPlayback, playedTickToSec(_)).WillByDefault([](midi::tick_t tick) {
        return audio::secs_t(tick / (2.0 * TICKS_PER_SEC));
    });

    // [THEN] Both envelopes are rebuilt with the new seconds
    EXPECT_CALL(*m_playback, setMasterControlParams(VolumeEnvelopeIs(EnvelopePoints { { 0.5, 0.25 } }))).Times(1);
    EXPECT_CALL(*m_playback, setControlParams(AUDIO_TRACK_ID, VolumeEnvelopeIs(EnvelopePoints { { 0.5, 0.5 } }))).Times(1);

    // [WHEN] The tempo curve changes
    automationData->editPoints(engraving::TEMPO_KEY, setPoint(0, 0.5));
}

TEST_F(PlaybackControllerTests, KeepsSelectionExportMuteOnTempoChange)
{
    // [GIVEN] A selected part is muted for export while its volume is automated
    notation::AutomationDataPtr automationData = std::make_shared<notation::AutomationData>();
    automationData->editPoints(TRACK_VOLUME_KEY, setPoint(TICKS_PER_SEC, 0.5));
    setUpProject(automationData);
    startPlayback();
    m_controller->setSelectionExportTrackVolumes({ { TRACK_ID.partId, 0 } });

    ON_CALL(*m_notationPlayback, playedTickToSec(_)).WillByDefault([](midi::tick_t tick) {
        return audio::secs_t(tick / (2.0 * TICKS_PER_SEC));
    });

    // [THEN] Resending the automation envelope must not undo the export-only mute
    EXPECT_CALL(*m_playback, setControlParams(AUDIO_TRACK_ID,
                                              AllOf(VolumeEnvelopeIs(EnvelopePoints { { 0.5, 0.5 } }),
                                                    Field(&audio::ControlParams::muted, true)))).Times(1);

    // [WHEN] The tempo curve changes during the export
    automationData->editPoints(engraving::TEMPO_KEY, setPoint(0, 0.5));
}

TEST_F(PlaybackControllerTests, ResendsNothingOnTempoChangeWithoutEnvelopes)
{
    // [GIVEN] No Volume/Pan automation
    notation::AutomationDataPtr automationData = std::make_shared<notation::AutomationData>();
    setUpProject(automationData);
    startPlayback();

    // [THEN] Nothing is resent - there is no envelope to move
    EXPECT_CALL(*m_playback, setMasterControlParams(_)).Times(0);
    EXPECT_CALL(*m_playback, setControlParams(_, _)).Times(0);

    // [WHEN] The tempo changes
    automationData->editPoints(engraving::TEMPO_KEY, setPoint(0, 0.5));
}

TEST_F(PlaybackControllerTests, SendsNewTrackEnvelopeOnTempoAndVolumeChange)
{
    // [GIVEN] Master Volume automation only, so the change below is not a full reset
    notation::AutomationDataPtr automationData = std::make_shared<notation::AutomationData>();
    automationData->editPoints(MASTER_VOLUME_KEY, setPoint(TICKS_PER_SEC, 0.25));
    setUpProject(automationData);
    startPlayback();

    // [THEN] The track gets its first Volume envelope
    EXPECT_CALL(*m_playback, setControlParams(AUDIO_TRACK_ID, VolumeEnvelopeIs(EnvelopePoints { { 1.0, 0.5 } }))).Times(1);

    // [WHEN] The tempo and the track Volume change together (an undo, for example)
    engraving::AutomationCurveMap curves;
    curves[engraving::TEMPO_KEY][0] = makePoint(0.5);
    curves[TRACK_VOLUME_KEY][TICKS_PER_SEC] = makePoint(0.5);
    automationData->replaceCurves(curves);
}

TEST_F(PlaybackControllerTests, DropsEnvelopeWhenCurveIsCleared)
{
    // [GIVEN] A track with a single Volume point
    notation::AutomationDataPtr automationData = std::make_shared<notation::AutomationData>();
    automationData->editPoints(TRACK_VOLUME_KEY, setPoint(TICKS_PER_SEC, 0.5));
    setUpProject(automationData);

    project::AudioOutputParams trackFaders;
    trackFaders.volume = audio::volume_db_t(-3.f);
    ON_CALL(*m_audioSettings, trackOutputParams(_)).WillByDefault(ReturnRefOfCopy(trackFaders));

    startPlayback();

    // [THEN] The track goes back to the fader value
    EXPECT_CALL(*m_playback, setControlParams(AUDIO_TRACK_ID, FixedVolumeIs(trackFaders.volume))).Times(1);

    // [WHEN] The point is erased
    automationData->editPoints(TRACK_VOLUME_KEY, erasePoint(TICKS_PER_SEC));
}
}
