/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
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

#include "async/processevents.h"
#include "async/promise.h"

#include "interactive/tests/mocks/interactivemock.h"

#include "notationscene/qml/MuseScore/NotationScene/editpercussionshortcutmodel.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

using namespace muse;

namespace mu::notation {
class EditPercussionShortcutModelTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();
        ON_CALL(*m_interactive, buttonData(_)).WillByDefault([](IInteractive::Button btn) {
            return IInteractive::ButtonData(btn, "");
        });

        m_model = new EditPercussionShortcutModel();
        m_model->interactive.set(m_interactive);

        //! The drum whose shortcut is edited, and the shortcuts that are already taken
        QVariantMap originDrum { { "title", "Kick" }, { "shortcut", "A" }, { "pitch", 36 } };
        QVariantMap otherDrum { { "title", "Snare" }, { "shortcut", "S" }, { "pitch", 38 } };
        QVariantMap appShortcut { { "title", "Bold" }, { "shortcut", "B" } };
        m_model->load(originDrum, { originDrum, otherDrum }, { appShortcut });
    }

    void TearDown() override
    {
        drainDeferredCalls();

        delete m_model;
        ::testing::Mock::VerifyAndClearExpectations(m_interactive.get());
    }

    //! Whether the dialog was asked to close, and whether with the shortcut saved
    struct CloseRequest {
        bool isRequested = false;
        bool accepted = false;
    };

    CloseRequest trySave()
    {
        std::vector<bool> requests;
        QMetaObject::Connection connection = QObject::connect(m_model, &EditPercussionShortcutModel::closeRequested,
                                                              [&requests](bool accepted) { requests.push_back(accepted); });

        m_model->trySave();
        drainDeferredCalls();

        QObject::disconnect(connection);

        EXPECT_LE(requests.size(), 1) << "the dialog is asked to close once";
        if (requests.empty()) {
            return {};
        }

        return { true, requests.front() };
    }

    //! The answers are delivered by deferred calls
    static void drainDeferredCalls()
    {
        async::processMessages();
    }

    static async::Promise<IInteractive::Result> dialogResult(IInteractive::Button btn)
    {
        return async::make_promise<IInteractive::Result>([btn](auto resolve) {
            return resolve(IInteractive::Result(int(btn)));
        });
    }

    EditPercussionShortcutModel* m_model = nullptr;
    std::shared_ptr<InteractiveMock> m_interactive;
};
}

using namespace mu::notation;

TEST_F(EditPercussionShortcutModelTests, TrySave_SameShortcut_IsNotSaved)
{
    //! [GIVEN] The drum keeps its shortcut
    m_model->inputKey(Qt::Key_A);

    //! [THEN] Nothing is asked
    EXPECT_CALL(*m_interactive, warning(_, _, _, _, _, _)).Times(0);

    //! [WHEN] Saving...
    CloseRequest close = trySave();

    //! [THEN] ...the dialog closes, there is nothing to save
    EXPECT_TRUE(close.isRequested);
    EXPECT_FALSE(close.accepted);
}

TEST_F(EditPercussionShortcutModelTests, TrySave_FreeShortcut_IsSavedWithoutAsking)
{
    //! [GIVEN] The new shortcut is not taken
    m_model->inputKey(Qt::Key_D);

    //! [THEN] Nothing is asked
    EXPECT_CALL(*m_interactive, warning(_, _, _, _, _, _)).Times(0);

    //! [WHEN] Saving...
    CloseRequest close = trySave();

    //! [THEN] ...it is saved
    EXPECT_TRUE(close.isRequested);
    EXPECT_TRUE(close.accepted);
}

TEST_F(EditPercussionShortcutModelTests, TrySave_TakenByAnotherDrum_UserAgrees_IsSaved)
{
    //! [GIVEN] The new shortcut is taken by another drum, and the user agrees to reassign it
    m_model->inputKey(Qt::Key_S);

    //! [THEN] The user is asked once
    EXPECT_CALL(*m_interactive, warning(_, _, _, _, _, _)).WillOnce(Return(dialogResult(IInteractive::Button::Ok)));

    //! [WHEN] Saving...
    CloseRequest close = trySave();

    //! [THEN] ...it is saved, and the other drum is the one to lose it
    EXPECT_TRUE(close.isRequested);
    EXPECT_TRUE(close.accepted);
    EXPECT_EQ(m_model->conflictDrumPitch(), 38);
}

TEST_F(EditPercussionShortcutModelTests, TrySave_TakenByAnotherDrum_UserCancels_IsNotSaved)
{
    //! [GIVEN] The new shortcut is taken by another drum, and the user does not want to reassign it
    m_model->inputKey(Qt::Key_S);

    //! [THEN] The user is asked once
    EXPECT_CALL(*m_interactive, warning(_, _, _, _, _, _)).WillOnce(Return(dialogResult(IInteractive::Button::Cancel)));

    //! [WHEN] Saving...
    CloseRequest close = trySave();

    //! [THEN] ...it is not saved
    EXPECT_TRUE(close.isRequested);
    EXPECT_FALSE(close.accepted);
}

TEST_F(EditPercussionShortcutModelTests, TrySave_TakenByTheApplication_UserAgrees_IsSaved)
{
    //! [GIVEN] The new shortcut is taken by the application, and the user agrees to take it over
    m_model->inputKey(Qt::Key_B);

    //! [THEN] The user is asked once
    EXPECT_CALL(*m_interactive, warning(_, _, _, _, _, _)).WillOnce(Return(dialogResult(IInteractive::Button::Ok)));

    //! [WHEN] Saving...
    CloseRequest close = trySave();

    //! [THEN] ...it is saved, and no drum loses its shortcut
    EXPECT_TRUE(close.isRequested);
    EXPECT_TRUE(close.accepted);
    EXPECT_EQ(m_model->conflictDrumPitch(), -1);
}

TEST_F(EditPercussionShortcutModelTests, TrySave_UserIsStillAnswering_DoesNotCloseYet)
{
    //! [GIVEN] The new shortcut is taken by another drum, and the user has not answered yet
    m_model->inputKey(Qt::Key_S);

    async::Promise<IInteractive::Result>::Resolve answer;
    EXPECT_CALL(*m_interactive, warning(_, _, _, _, _, _)).WillOnce([&answer] {
        return async::make_promise<IInteractive::Result>([&answer](auto resolve) {
            answer = resolve;
            return async::Promise<IInteractive::Result>::dummy_result();
        }, async::PromiseType::AsyncByBody);
    });

    //! [WHEN] Saving...
    CloseRequest close = trySave();

    //! [THEN] ...waits for the answer
    EXPECT_FALSE(close.isRequested);

    (void)answer(IInteractive::Result(int(IInteractive::Button::Ok)));
}

TEST_F(EditPercussionShortcutModelTests, TrySave_UserIsStillAnswering_KeepsTheShortcutTheUserIsAskedAbout)
{
    //! [GIVEN] The new shortcut is taken by another drum, and the user is asked about it
    m_model->inputKey(Qt::Key_S);

    async::Promise<IInteractive::Result>::Resolve answer;
    EXPECT_CALL(*m_interactive, warning(_, _, _, _, _, _)).WillOnce([&answer] {
        return async::make_promise<IInteractive::Result>([&answer](auto resolve) {
            answer = resolve;
            return async::Promise<IInteractive::Result>::dummy_result();
        }, async::PromiseType::AsyncByBody);
    });

    std::vector<bool> closeRequests;
    QObject::connect(m_model, &EditPercussionShortcutModel::closeRequested, [&closeRequests](bool accepted) {
        closeRequests.push_back(accepted);
    });

    m_model->trySave();

    //! [WHEN] The shortcut is changed, cleared and saved again before the answer
    m_model->inputKey(Qt::Key_D);
    m_model->clear();
    m_model->trySave();

    //! [THEN] Nothing changes and the user is not asked again
    EXPECT_EQ(m_model->newShortcutText(), QString("S"));
    EXPECT_FALSE(m_model->cleared());
    EXPECT_TRUE(closeRequests.empty());

    //! [WHEN] The user agrees
    (void)answer(IInteractive::Result(int(IInteractive::Button::Ok)));
    drainDeferredCalls();

    //! [THEN] The dialog closes with the shortcut the user agreed to
    EXPECT_EQ(closeRequests, std::vector<bool> { true });
    EXPECT_EQ(m_model->newShortcutText(), QString("S"));
}
