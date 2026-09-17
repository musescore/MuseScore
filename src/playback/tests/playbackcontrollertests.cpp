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

#include "async/processevents.h"
#include "modularity/ioc.h"

#include "internal/playbackcontroller.h"

#include "rcommand/tests/mocks/commanddispatchermock.h"

#include "context/tests/mocks/globalcontextmock.h"
#include "project/tests/mocks/notationprojectmock.h"

#include "mocks/playbackconfigurationmock.h"
#include "mocks/playbackmock.h"

using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::Return;

using namespace muse;

static const std::string TEST_MODULE("playback_utests");
static const muse::modularity::IoCID TEST_CONTEXT_ID = 1;

namespace mu::playback {
class PlaybackControllerTests : public ::testing::Test, public async::Asyncable
{
protected:
    void SetUp() override
    {
        m_playback = std::make_shared<NiceMock<audio::PlaybackMock> >();
        m_globalContext = std::make_shared<NiceMock<context::GlobalContextMock> >();
        m_configuration = std::make_shared<NiceMock<PlaybackConfigurationMock> >();
        m_commandDispatcher = std::make_shared<NiceMock<rcommand::CommandDispatcherMock> >();
        m_project = std::make_shared<NiceMock<project::NotationProjectMock> >();

        m_iocCtx = std::make_shared<modularity::Context>(TEST_CONTEXT_ID);

        modularity::ioc(m_iocCtx)->registerExport<audio::IPlayback>(TEST_MODULE, m_playback);
        modularity::ioc(m_iocCtx)->registerExport<context::IGlobalContext>(TEST_MODULE, m_globalContext);
        modularity::ioc(m_iocCtx)->registerExport<rcommand::ICommandDispatcher>(TEST_MODULE, m_commandDispatcher);

        modularity::globalIoc()->registerExport<IPlaybackConfiguration>(TEST_MODULE, m_configuration);

        ON_CALL(*m_globalContext, currentProjectChanged()).WillByDefault(Return(m_currentProjectChanged));
        ON_CALL(*m_globalContext, currentNotationChanged()).WillByDefault(Return(m_currentNotationChanged));
        ON_CALL(*m_globalContext, currentProject()).WillByDefault(Return(project::INotationProjectPtr()));
        ON_CALL(*m_globalContext, currentNotation()).WillByDefault(Return(notation::INotationPtr()));

        //! NOTE The audio context stays inited until a test resolves it, as it is in the app:
        //! init() is an RPC round trip to the audio engine
        ON_CALL(*m_playback, init()).WillByDefault([this]() { return makePendingInitPromise(); });
        ON_CALL(*m_playback, isInited()).WillByDefault([this]() { return m_isAudioContextInited; });

        m_controller = std::make_shared<PlaybackController>(m_iocCtx);
    }

    void TearDown() override
    {
        //! NOTE The controller must go first: it holds the mocks via the IoC
        m_controller = nullptr;

        drainDeferredCalls();

        modularity::globalIoc()->unregister<IPlaybackConfiguration>(TEST_MODULE);
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
        ON_CALL(*m_globalContext, currentProject()).WillByDefault(Return(project::INotationProjectPtr(m_project)));

        //! NOTE GlobalContext::setCurrentProject notifies in this order
        m_currentProjectChanged.notify();
        m_currentNotationChanged.notify();
    }

    void closeProject()
    {
        ON_CALL(*m_globalContext, currentProject()).WillByDefault(Return(project::INotationProjectPtr()));

        m_currentProjectChanged.notify();
        m_currentNotationChanged.notify();
    }

    std::shared_ptr<PlaybackController> m_controller;

    std::shared_ptr<NiceMock<audio::PlaybackMock> > m_playback;
    std::shared_ptr<NiceMock<context::GlobalContextMock> > m_globalContext;
    std::shared_ptr<NiceMock<PlaybackConfigurationMock> > m_configuration;
    std::shared_ptr<NiceMock<rcommand::CommandDispatcherMock> > m_commandDispatcher;
    std::shared_ptr<NiceMock<project::NotationProjectMock> > m_project;

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
}
