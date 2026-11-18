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

#include "async/processevents.h"
#include "modularity/ioc.h"

#include "musesounds/internal/musesoundscheckupdatescenario.h"

#include "mocks/musesoundscheckupdateservicemock.h"
#include "mocks/musesoundsconfigurationmock.h"
#include "interactive/tests/mocks/interactivemock.h"
#include "interactive/tests/mocks/platforminteractivemock.h"

using namespace ::testing;
using namespace mu::musesounds;
using namespace muse;

namespace {
const std::string RELEASE_VERSION = "1.2.3";
const std::string RELEASE_URL = "https://example.com/release";

//! NOTE: keep the body queued, don't run it eagerly
//! otherwise resolve() can fire before onResolve is attached
async::Promise<Val> answeredPromise(const Val& val)
{
    return async::make_promise<Val>([val](auto resolve, auto) {
        return resolve(val);
    });
}

async::Promise<Val> rejectedPromise(int code)
{
    return async::make_promise<Val>([code](auto, auto reject) {
        return reject(code, "");
    });
}
}

namespace mu::musesounds {
class MuseSounds_MuseSoundsCheckUpdateScenarioTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_scenario = std::make_shared<MuseSoundsCheckUpdateScenario>(muse::modularity::globalCtx());

        m_configuration = std::make_shared<NiceMock<MuseSoundsConfigurationMock> >();
        m_service = std::make_shared<NiceMock<MuseSoundsCheckUpdateServiceMock> >();
        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();
        m_platformInteractive = std::make_shared<NiceMock<PlatformInteractiveMock> >();

        m_scenario->configuration.set(m_configuration);
        m_scenario->service.set(m_service);
        m_scenario->interactive.set(m_interactive);
        m_scenario->platformInteractive.set(m_platformInteractive);

        update::ReleaseInfo info;
        info.version = RELEASE_VERSION;
        info.additionInfo["features"] = Val(ValList());
        info.actions = { Val(RELEASE_URL) };
        m_lastCheckResult = RetVal<update::ReleaseInfo>::make_ok(info);

        ON_CALL(*m_service, lastCheckResult())
        .WillByDefault(ReturnRef(m_lastCheckResult));
    }

    void givenTheUserAnswers(async::Promise<Val> answer)
    {
        EXPECT_CALL(*m_interactive, open(Matcher<const UriQuery&>(_)))
        .WillOnce(Return(answer));
    }

    std::shared_ptr<MuseSoundsCheckUpdateScenario> m_scenario;
    std::shared_ptr<MuseSoundsConfigurationMock> m_configuration;
    std::shared_ptr<MuseSoundsCheckUpdateServiceMock> m_service;
    std::shared_ptr<InteractiveMock> m_interactive;
    std::shared_ptr<PlatformInteractiveMock> m_platformInteractive;

    RetVal<update::ReleaseInfo> m_lastCheckResult;
};
}

// ==================================================
// showUpdate()
// ==================================================

TEST_F(MuseSounds_MuseSoundsCheckUpdateScenarioTest, ShowUpdate_UserOpensMuseHub_OpensItAndDoesNotShowTheReleaseAgain)
{
    //! [GIVEN] The user chooses to open MuseHub
    givenTheUserAnswers(answeredPromise(Val("openMuseHub")));

    //! [THEN] MuseHub is opened, and the release is not shown again
    EXPECT_CALL(*m_platformInteractive, openUrl(Matcher<const std::string&>(RELEASE_URL)))
    .WillOnce(Return(make_ok()));
    EXPECT_CALL(*m_configuration, setLastShownMuseSoundsReleaseVersion(RELEASE_VERSION));

    //! [WHEN] Showing the update
    m_scenario->showUpdate();
    async::processMessages();
}

TEST_F(MuseSounds_MuseSoundsCheckUpdateScenarioTest, ShowUpdate_UserCancels_DoesNotShowTheReleaseAgain)
{
    //! [GIVEN] The user closes the dialog
    givenTheUserAnswers(rejectedPromise(int(Ret::Code::Cancel)));

    //! [THEN] MuseHub is not opened, and the release is not shown again
    EXPECT_CALL(*m_platformInteractive, openUrl(Matcher<const std::string&>(_))).Times(0);
    EXPECT_CALL(*m_configuration, setLastShownMuseSoundsReleaseVersion(RELEASE_VERSION));

    //! [WHEN] Showing the update
    m_scenario->showUpdate();
    async::processMessages();
}

TEST_F(MuseSounds_MuseSoundsCheckUpdateScenarioTest, ShowUpdate_DialogFails_ShowsTheReleaseAgainLater)
{
    //! [GIVEN] The dialog cannot be opened
    givenTheUserAnswers(rejectedPromise(int(Ret::Code::UnknownError)));

    //! [THEN] The release can be shown again
    EXPECT_CALL(*m_configuration, setLastShownMuseSoundsReleaseVersion(_)).Times(0);

    //! [WHEN] Showing the update
    m_scenario->showUpdate();
    async::processMessages();
}

TEST_F(MuseSounds_MuseSoundsCheckUpdateScenarioTest, ShowUpdate_UserIsStillAnswering_DoesNotMarkTheReleaseYet)
{
    //! [GIVEN] The user has not answered yet
    async::Promise<Val>::Resolve answer;
    givenTheUserAnswers(async::make_promise<Val>([&answer](auto resolve, auto) {
        answer = resolve;
        return async::Promise<Val>::dummy_result();
    }, async::PromiseType::AsyncByBody));

    //! [THEN] The release is not marked as shown until the answer
    EXPECT_CALL(*m_configuration, setLastShownMuseSoundsReleaseVersion(_)).Times(0);

    //! [WHEN] Showing the update
    m_scenario->showUpdate();
    async::processMessages();

    ::testing::Mock::VerifyAndClearExpectations(m_configuration.get());

    //! [THEN] It is marked once the user answers
    EXPECT_CALL(*m_configuration, setLastShownMuseSoundsReleaseVersion(RELEASE_VERSION));
    (void)answer(Val());
    async::processMessages();
}

TEST_F(MuseSounds_MuseSoundsCheckUpdateScenarioTest, ShowUpdate_UserIsStillAnswering_DoesNotShowTheReleaseTwice)
{
    //! [GIVEN] The release is shown, and the user has not answered yet
    ON_CALL(*m_service, needCheckForUpdate()).WillByDefault(Return(make_ok()));

    async::Promise<Val>::Resolve answer;
    givenTheUserAnswers(async::make_promise<Val>([&answer](auto resolve, auto) {
        answer = resolve;
        return async::Promise<Val>::dummy_result();
    }, async::PromiseType::AsyncByBody));

    m_scenario->showUpdate();
    async::processMessages();

    //! [THEN] There is no update to show meanwhile, and showing it again opens nothing
    EXPECT_FALSE(m_scenario->hasUpdate());
    m_scenario->showUpdate();
    async::processMessages();

    //! [WHEN] The user answers
    (void)answer(Val());
    async::processMessages();

    //! [THEN] The update can be shown again, until the release is marked as shown
    EXPECT_TRUE(m_scenario->hasUpdate());
}

TEST_F(MuseSounds_MuseSoundsCheckUpdateScenarioTest, ShowUpdate_NoUpdate_ShowsNothing)
{
    //! [GIVEN] There is no update
    m_lastCheckResult = RetVal<update::ReleaseInfo>(make_ret(Ret::Code::UnknownError));

    //! [THEN] Nothing is shown
    EXPECT_CALL(*m_interactive, open(Matcher<const UriQuery&>(_))).Times(0);

    //! [WHEN] Showing the update
    m_scenario->showUpdate();
}
