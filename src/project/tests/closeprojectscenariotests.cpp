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

#include <QTimer>

#include "async/processevents.h"

#include "project/internal/closeprojectscenario.h"

#include "context/tests/mocks/globalcontextmock.h"
#include "context/tests/mocks/playbackstatemock.h"
#include "interactive/tests/mocks/interactivemock.h"

#include "mocks/commanddispatchermock.h"
#include "mocks/notationprojectmock.h"
#include "mocks/saveprojectscenariomock.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Truly;

using namespace muse;
using namespace mu::project;

namespace mu::project {
class CloseProjectScenarioTests : public ::testing::Test, public async::Asyncable
{
protected:
    void SetUp() override
    {
        m_scenario = std::make_shared<CloseProjectScenario>(modularity::globalCtx());

        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();
        m_globalContext = std::make_shared<NiceMock<context::GlobalContextMock> >();
        m_commandDispatcher = std::make_shared<NiceMock<rcommand::CommandDispatcherMock> >();
        m_saveScenario = std::make_shared<NiceMock<SaveProjectScenarioMock> >();

        m_scenario->interactive.set(m_interactive);
        m_scenario->globalContext.set(m_globalContext);
        m_scenario->commandDispatcher.set(m_commandDispatcher);
        m_scenario->saveProjectScenario.set(m_saveScenario);

        m_project = std::make_shared<NiceMock<NotationProjectMock> >();
        m_playbackState = std::make_shared<NiceMock<context::PlaybackStateMock> >();

        // An opened score with no pending changes, and silence, unless a test says otherwise.
        ON_CALL(*m_globalContext, currentProject()).WillByDefault(Return(m_project));
        ON_CALL(*m_globalContext, playbackState()).WillByDefault(Return(m_playbackState));
        ON_CALL(*m_playbackState, isPlaying()).WillByDefault(Return(false));
        ON_CALL(*m_project, isNeedSave()).WillByDefault(Return(false));

        ON_CALL(*m_saveScenario, saveProject(_)).WillByDefault([]() { return resolvedPromise(make_ok()); });

        // Dialogs and commands settle immediately so that unstubbed paths do not abort the test.
        ON_CALL(*m_interactive, buttonData(_)).WillByDefault([](IInteractive::Button btn) {
            return IInteractive::ButtonData(btn, "");
        });
        ON_CALL(*m_interactive, open(_)).WillByDefault([] {
            return async::make_promise<Val>([](auto resolve) {
                return resolve(Val());
            });
        });
        ON_CALL(*m_commandDispatcher, dispatch(_)).WillByDefault([](const rcommand::Request& request) {
            return async::make_promise<rcommand::Response>([request](auto resolve) {
                return resolve(rcommand::make_response(request, make_ok()));
            });
        });

        //! NOTE The close flow waits for the save in a nested event loop; in the application the async
        //! queue is pumped by a ticker, so do the same here, or that loop would never see the result.
        m_asyncPump.setInterval(1);
        QObject::connect(&m_asyncPump, &QTimer::timeout, []() { async::processMessages(); });
        m_asyncPump.start();
    }

    void TearDown() override
    {
        m_asyncPump.stop();

        // Let whatever the flow queued after its result run, so that nothing outlives the mocks
        drainDeferredCalls();

        release(m_globalContext);
        release(m_project);
    }

    template<typename T>
    static void release(const std::shared_ptr<T>& mock)
    {
        ::testing::Mock::VerifyAndClearExpectations(mock.get());
        ::testing::Mock::AllowLeak(mock.get());
    }

    //! A deferred call may queue another one, hence the repetition.
    static void drainDeferredCalls()
    {
        for (int i = 0; i < 10; ++i) {
            async::processMessages();
        }
    }

    static async::Promise<Ret> resolvedPromise(const Ret& ret)
    {
        return async::make_promise<Ret>([ret](auto resolve) {
            return resolve(ret);
        });
    }

    static ::testing::Matcher<const UriQuery&> IsHomePage()
    {
        return ::testing::Truly([](const UriQuery& q) {
            return q.uri().toString() == "musescore://home";
        });
    }

    static ::testing::Matcher<const rcommand::Request&> IsStopPlayback()
    {
        return ::testing::Truly([](const rcommand::Request& request) {
            return request.command.toString() == "command://playback/stop";
        });
    }

    //! The score has changes that have not been written yet, and the user answers the question about them with `btn`.
    void givenUnsavedChangesAnsweredWith(IInteractive::Button btn)
    {
        ON_CALL(*m_project, isNeedSave()).WillByDefault(Return(true));
        ON_CALL(*m_interactive, warningSync(_, _, _, _, _, _))
        .WillByDefault(Return(IInteractive::Result(int(btn))));
    }

    void givenSaveFinishesWith(const Ret& ret)
    {
        ON_CALL(*m_saveScenario, saveProject(_)).WillByDefault([ret]() { return resolvedPromise(ret); });
    }

    std::shared_ptr<CloseProjectScenario> m_scenario;

    std::shared_ptr<InteractiveMock> m_interactive;
    std::shared_ptr<context::GlobalContextMock> m_globalContext;
    std::shared_ptr<rcommand::CommandDispatcherMock> m_commandDispatcher;
    std::shared_ptr<SaveProjectScenarioMock> m_saveScenario;

    std::shared_ptr<NotationProjectMock> m_project;
    std::shared_ptr<context::PlaybackStateMock> m_playbackState;

    QTimer m_asyncPump;
};

// ─── Nothing to close ────────────────────────────────────────────────────────

TEST_F(CloseProjectScenarioTests, CloseOpenedProject_NoProject_Succeeds)
{
    //! [GIVEN] No score is opened
    ON_CALL(*m_globalContext, currentProject()).WillByDefault(Return(nullptr));

    //! [THEN] There is nothing to ask about, and nothing to let go of
    EXPECT_CALL(*m_interactive, warningSync(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(*m_globalContext, setCurrentProject(_)).Times(0);

    //! [WHEN] Closing...
    bool ok = m_scenario->closeOpenedProject();

    //! [THEN] ...succeeds, there was nothing in the way
    EXPECT_TRUE(ok);
}

// ─── A score with no pending changes ─────────────────────────────────────────

TEST_F(CloseProjectScenarioTests, CloseOpenedProject_SavedScore_ClosesAndGoesHome)
{
    //! [GIVEN] An opened score whose changes are all written already
    ON_CALL(*m_project, isNeedSave()).WillByDefault(Return(false));
    ON_CALL(*m_interactive, isOpened(::testing::An<const Uri&>())).WillByDefault(Return(RetVal<bool>::make_ok(false)));

    //! [THEN] Nothing is asked, the dialogs left over from the score are closed with it,
    //! the score is let go of, and the home page takes its place
    EXPECT_CALL(*m_interactive, warningSync(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(*m_interactive, closeAllDialogsSync()).Times(1);
    EXPECT_CALL(*m_globalContext, setCurrentProject(INotationProjectPtr())).Times(1);
    EXPECT_CALL(*m_interactive, open(IsHomePage())).Times(1);

    //! [WHEN] Closing...
    bool ok = m_scenario->closeOpenedProject();

    EXPECT_TRUE(ok);
}

TEST_F(CloseProjectScenarioTests, CloseOpenedProject_NotGoingHome_LeavesThePageAlone)
{
    //! [THEN] The caller has its own plans for the page, so the home page is not opened...
    EXPECT_CALL(*m_interactive, open(IsHomePage())).Times(0);

    //! [THEN] ...but the score is still let go of
    EXPECT_CALL(*m_globalContext, setCurrentProject(INotationProjectPtr())).Times(1);

    //! [WHEN] Closing without going home...
    bool ok = m_scenario->closeOpenedProject(false);

    EXPECT_TRUE(ok);
}

TEST_F(CloseProjectScenarioTests, CloseOpenedProject_HomeAlreadyOpen_DoesNotOpenItAgain)
{
    //! [GIVEN] The home page is already on screen
    ON_CALL(*m_interactive, isOpened(::testing::An<const Uri&>())).WillByDefault(Return(RetVal<bool>::make_ok(true)));

    //! [THEN] It is not opened a second time
    EXPECT_CALL(*m_interactive, open(IsHomePage())).Times(0);

    //! [WHEN] Closing...
    bool ok = m_scenario->closeOpenedProject();

    EXPECT_TRUE(ok);
}

// ─── A score with unsaved changes ────────────────────────────────────────────

TEST_F(CloseProjectScenarioTests, CloseOpenedProject_UnsavedChanges_AsksAboutThem)
{
    //! [GIVEN] An opened score with changes that were never written
    givenUnsavedChangesAnsweredWith(IInteractive::Button::DontSave);

    //! [THEN] The user is asked what to do with them
    EXPECT_CALL(*m_interactive, warningSync(_, _, _, _, _, _)).Times(1);

    //! [WHEN] Closing...
    m_scenario->closeOpenedProject();
}

TEST_F(CloseProjectScenarioTests, CloseOpenedProject_UnsavedChanges_UserCancels_KeepsScoreOpen)
{
    //! [GIVEN] An opened score with unsaved changes, and a user who changes their mind
    givenUnsavedChangesAnsweredWith(IInteractive::Button::Cancel);

    //! [THEN] Nothing is saved, and the score stays where it was
    EXPECT_CALL(*m_saveScenario, saveProject(_)).Times(0);
    EXPECT_CALL(*m_globalContext, setCurrentProject(_)).Times(0);
    EXPECT_CALL(*m_interactive, closeAllDialogsSync()).Times(0);
    EXPECT_CALL(*m_interactive, open(IsHomePage())).Times(0);

    //! [WHEN] Closing...
    bool ok = m_scenario->closeOpenedProject();

    //! [THEN] ...is refused
    EXPECT_FALSE(ok);
}

TEST_F(CloseProjectScenarioTests, CloseOpenedProject_UnsavedChanges_UserDeclinesToSave_ClosesAnyway)
{
    //! [GIVEN] An opened score with unsaved changes the user is willing to lose
    givenUnsavedChangesAnsweredWith(IInteractive::Button::DontSave);

    //! [THEN] The changes are not written, and the score is closed regardless
    EXPECT_CALL(*m_saveScenario, saveProject(_)).Times(0);
    EXPECT_CALL(*m_globalContext, setCurrentProject(INotationProjectPtr())).Times(1);

    //! [WHEN] Closing...
    bool ok = m_scenario->closeOpenedProject();

    EXPECT_TRUE(ok);
}

TEST_F(CloseProjectScenarioTests, CloseOpenedProject_UnsavedChanges_UserSaves_SavesThenCloses)
{
    //! [GIVEN] An opened score with unsaved changes the user wants to keep
    givenUnsavedChangesAnsweredWith(IInteractive::Button::Save);
    givenSaveFinishesWith(make_ok());

    //! [THEN] The score is saved over itself - an empty path is what tells the save to keep the file it has -
    //! and only then let go of
    EXPECT_CALL(*m_saveScenario, saveProject(io::path_t())).Times(1);
    EXPECT_CALL(*m_globalContext, setCurrentProject(INotationProjectPtr())).Times(1);

    //! [WHEN] Closing...
    bool ok = m_scenario->closeOpenedProject();

    EXPECT_TRUE(ok);
}

TEST_F(CloseProjectScenarioTests, CloseOpenedProject_UnsavedChanges_SaveFails_KeepsScoreOpen)
{
    //! [GIVEN] An opened score the user wants to save, but the save does not go through
    givenUnsavedChangesAnsweredWith(IInteractive::Button::Save);
    givenSaveFinishesWith(make_ret(Ret::Code::UnknownError));

    //! [THEN] The score is not let go of, so that the work is not lost with it
    EXPECT_CALL(*m_globalContext, setCurrentProject(_)).Times(0);

    //! [WHEN] Closing...
    bool ok = m_scenario->closeOpenedProject();

    //! [THEN] ...is refused
    EXPECT_FALSE(ok);
}

// ─── Playback ────────────────────────────────────────────────────────────────

TEST_F(CloseProjectScenarioTests, CloseOpenedProject_WhilePlaying_StopsPlayback)
{
    //! [GIVEN] The score is being played
    ON_CALL(*m_playbackState, isPlaying()).WillByDefault(Return(true));

    //! [THEN] Playback is stopped, rather than left running over a score that is no longer there
    EXPECT_CALL(*m_commandDispatcher, dispatch(_)).Times(AnyNumber());
    EXPECT_CALL(*m_commandDispatcher, dispatch(IsStopPlayback())).Times(1);

    //! [WHEN] Closing...
    m_scenario->closeOpenedProject();
}

TEST_F(CloseProjectScenarioTests, CloseOpenedProject_NotPlaying_DispatchesNothing)
{
    //! [GIVEN] The score is not being played
    ON_CALL(*m_playbackState, isPlaying()).WillByDefault(Return(false));

    //! [THEN] There is nothing to stop
    EXPECT_CALL(*m_commandDispatcher, dispatch(_)).Times(0);

    //! [WHEN] Closing...
    m_scenario->closeOpenedProject();
}

// ─── One close at a time ─────────────────────────────────────────────────────

TEST_F(CloseProjectScenarioTests, CloseOpenedProject_WhileAlreadyClosing_IsRefused)
{
    //! [GIVEN] An opened score with unsaved changes; the question about them is where a second
    //! close can arrive from, since the dialog is answered from a nested event loop
    ON_CALL(*m_project, isNeedSave()).WillByDefault(Return(true));

    bool secondCloseResult = true;
    bool busyWhileClosing = false;
    ON_CALL(*m_interactive, warningSync(_, _, _, _, _, _))
    .WillByDefault([this, &secondCloseResult, &busyWhileClosing]() {
        busyWhileClosing = m_scenario->isBusy(BusyStatus::Closing);
        secondCloseResult = m_scenario->closeOpenedProject();
        return IInteractive::Result(int(IInteractive::Button::DontSave));
    });

    //! [THEN] The question is asked once: the second close bails out before reaching it
    EXPECT_CALL(*m_interactive, warningSync(_, _, _, _, _, _)).Times(1);

    //! [WHEN] Closing while a close is already under way...
    bool ok = m_scenario->closeOpenedProject();

    //! [THEN] The close in progress reports itself as busy, the second one is refused,
    //! and the first one still finishes
    EXPECT_TRUE(busyWhileClosing);
    EXPECT_FALSE(secondCloseResult);
    EXPECT_TRUE(ok);

    //! [THEN] Once it is over, closing is possible again
    EXPECT_FALSE(m_scenario->isBusy(BusyStatus::Closing));
}
}
