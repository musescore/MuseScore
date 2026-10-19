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

#include <QUrl>

#include "async/processevents.h"
#include "modularity/ioc.h"

#include "musesounds/internal/musesamplercheckstatusscenario.h"

#include "mocks/musesamplercheckstatusservicemock.h"
#include "mocks/musesoundsconfigurationmock.h"
#include "global/tests/mocks/interactivemock.h"
#include "global/tests/mocks/processmock.h"
#include "global/tests/mocks/globalconfigurationmock.h"
#include "actions/tests/mocks/actionsdispatchermock.h"
#include "multiwindows/tests/mocks/multiwindowsprovidermock.h"

using namespace ::testing;
using namespace mu::musesounds;
using namespace muse;

namespace {
constexpr int TROUBLESHOOT_BUTTON = int(IInteractive::Button::CustomButton) + 1;
constexpr int OPEN_MUSEHUB_BUTTON = int(IInteractive::Button::CustomButton) + 2;

//! NOTE: keep the body queued, don't run it eagerly
//! otherwise resolve() can fire before onResolve is attached
template<typename T>
async::Promise<T> resolvedPromise(const T& val)
{
    return async::make_promise<T>([val](auto resolve, auto reject) {
        (void)reject;
        return resolve(val);
    });
}

async::Promise<IInteractive::Result> resolvedResultPromise(const IInteractive::Result& result = IInteractive::Result())
{
    return resolvedPromise<IInteractive::Result>(result);
}

Matcher<const IInteractive::ButtonDatas&> ButtonIdsAre(const std::vector<int>& expectedIds)
{
    return Truly([expectedIds](const IInteractive::ButtonDatas& buttons) {
        if (buttons.size() != expectedIds.size()) {
            return false;
        }
        for (size_t i = 0; i < buttons.size(); ++i) {
            if (buttons[i].btn != expectedIds[i]) {
                return false;
            }
        }
        return true;
    });
}

Matcher<const IInteractive::Text&> TextContains(const std::string& substr)
{
    return Field(&IInteractive::Text::text, HasSubstr(substr));
}

Matcher<const IInteractive::Options&> HasDontShowAgainCheckBox()
{
    return Truly([](const IInteractive::Options& options) {
        return options.testFlag(IInteractive::Option::WithDontShowAgainCheckBox);
    });
}
}

namespace mu::musesounds {
class MuseSounds_MuseSamplerCheckStatusScenarioTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_scenario = std::make_shared<MuseSamplerCheckStatusScenario>(muse::modularity::globalCtx());

        m_multiwindowsProvider = std::make_shared<NiceMock<muse::mi::MultiWindowsProviderMock> >();
        m_globalConfiguration = std::make_shared<NiceMock<GlobalConfigurationMock> >();
        m_process = std::make_shared<NiceMock<ProcessMock> >();
        m_configuration = std::make_shared<NiceMock<MuseSoundsConfigurationMock> >();
        m_service = std::make_shared<NiceMock<MuseSamplerCheckStatusServiceMock> >();
        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();
        m_dispatcher = std::make_shared<NiceMock<muse::actions::ActionsDispatcherMock> >();

        m_scenario->multiwindowsProvider.set(m_multiwindowsProvider);
        m_scenario->globalConfiguration.set(m_globalConfiguration);
        m_scenario->process.set(m_process);
        m_scenario->configuration.set(m_configuration);
        m_scenario->service.set(m_service);
        m_scenario->interactive.set(m_interactive);
        m_scenario->dispatcher.set(m_dispatcher);

        ON_CALL(*m_multiwindowsProvider, windowCount())
        .WillByDefault(Return(1));

        ON_CALL(*m_service, canCheckForUpdate())
        .WillByDefault(Return(true));

        ON_CALL(*m_configuration, showLoadingFailedWarning())
        .WillByDefault(Return(true));
    }

    void expectNoDialogs()
    {
        EXPECT_CALL(*m_interactive, warning(_, _, _, _, _, _)).Times(0);
        EXPECT_CALL(*m_interactive, info(_, _, _, _, _, _)).Times(0);
    }

    void expectLoadingFailedWarning(const IInteractive::Result& result)
    {
#ifdef Q_OS_LINUX
        const std::string appName = "MuseSounds Manager";
#else
        const std::string appName = "MuseHub";
#endif

#ifdef Q_OS_WIN
        const std::vector<int> buttonIds { OPEN_MUSEHUB_BUTTON, TROUBLESHOOT_BUTTON, int(IInteractive::Button::Close) };
#else
        const std::vector<int> buttonIds { int(IInteractive::Button::Close), TROUBLESHOOT_BUTTON, OPEN_MUSEHUB_BUTTON };
#endif

        ON_CALL(*m_interactive, buttonData(IInteractive::Button::Close))
        .WillByDefault(Return(IInteractive::ButtonData(IInteractive::Button::Close, "")));

        EXPECT_CALL(*m_interactive, warning("MuseSounds could not be loaded", TextContains("open " + appName),
                                            ButtonIdsAre(buttonIds), OPEN_MUSEHUB_BUTTON, HasDontShowAgainCheckBox(), _))
        .WillOnce(Invoke([result](auto&&...) {
            return resolvedResultPromise(result);
        }));
    }

    std::shared_ptr<MuseSamplerCheckStatusScenario> m_scenario;
    std::shared_ptr<muse::mi::MultiWindowsProviderMock> m_multiwindowsProvider;
    std::shared_ptr<GlobalConfigurationMock> m_globalConfiguration;
    std::shared_ptr<ProcessMock> m_process;
    std::shared_ptr<MuseSoundsConfigurationMock> m_configuration;
    std::shared_ptr<MuseSamplerCheckStatusServiceMock> m_service;
    std::shared_ptr<InteractiveMock> m_interactive;
    std::shared_ptr<muse::actions::ActionsDispatcherMock> m_dispatcher;
};
}

// ==================================================
// checkAndShowStatusIfNeed()
// ==================================================

TEST_F(MuseSounds_MuseSamplerCheckStatusScenarioTest, CheckAndShowStatus_MultipleWindows_DoesNothing)
{
    // [GIVEN] Several windows are open
    ON_CALL(*m_multiwindowsProvider, windowCount()).WillByDefault(Return(2));

    // [GIVEN] MuseSampler failed to init
    ON_CALL(*m_service, libInitFailed()).WillByDefault(Return(true));

    // [THEN] Nothing is checked or shown
    expectNoDialogs();
    EXPECT_CALL(*m_service, checkForUpdate()).Times(0);

    // [WHEN] Checking the status
    m_scenario->checkAndShowStatusIfNeed();

    // [THEN] The check can be done later
    EXPECT_FALSE(m_scenario->alreadyChecked());
}

TEST_F(MuseSounds_MuseSamplerCheckStatusScenarioTest, CheckAndShowStatus_LibInitFailed_ShowsLoadingFailedWarningOnly)
{
    // [GIVEN] MuseSampler failed to init
    ON_CALL(*m_service, libInitFailed()).WillByDefault(Return(true));

    // [THEN] The loading failed warning is shown, and the update isn't checked
    expectLoadingFailedWarning(IInteractive::Result(int(IInteractive::Button::Close)));
    EXPECT_CALL(*m_interactive, info(_, _, _, _, _, _)).Times(0);
    EXPECT_CALL(*m_service, checkForUpdate()).Times(0);

    // [THEN] Nothing has been checked yet
    EXPECT_FALSE(m_scenario->alreadyChecked());

    // [WHEN] Checking the status
    m_scenario->checkAndShowStatusIfNeed();
    muse::async::processMessages();

    // [THEN] The status is checked
    EXPECT_TRUE(m_scenario->alreadyChecked());
}

TEST_F(MuseSounds_MuseSamplerCheckStatusScenarioTest, CheckAndShowStatus_LibInitFailed_TroubleshootClicked_OpensTroubleshootUrl)
{
    // [GIVEN] MuseSampler failed to init
    ON_CALL(*m_service, libInitFailed()).WillByDefault(Return(true));

    // [GIVEN] The troubleshoot URL
    const QUrl troubleshootUrl("test://troubleshoot");
    ON_CALL(*m_configuration, museSoundsTroubleshootUrl()).WillByDefault(Return(troubleshootUrl));

    // [GIVEN] The user clicks Troubleshoot
    expectLoadingFailedWarning(IInteractive::Result(TROUBLESHOOT_BUTTON));

    // [THEN] The troubleshoot page is opened
    EXPECT_CALL(*m_interactive, openUrl(troubleshootUrl))
    .WillOnce(Return(make_ok()));

    // [WHEN] Checking the status
    m_scenario->checkAndShowStatusIfNeed();
    muse::async::processMessages();
}

TEST_F(MuseSounds_MuseSamplerCheckStatusScenarioTest, CheckAndShowStatus_LibInitFailed_OpenMuseHubClicked_OpensMuseHubWithoutQuitting)
{
    // [GIVEN] MuseSampler failed to init
    ON_CALL(*m_service, libInitFailed()).WillByDefault(Return(true));

    // [GIVEN] The user clicks "Open MuseHub"
    expectLoadingFailedWarning(IInteractive::Result(OPEN_MUSEHUB_BUTTON));

    // [THEN] MuseHub is opened
#ifdef Q_OS_LINUX
    EXPECT_CALL(*m_process, startDetached("muse-sounds-manager", _))
    .WillOnce(Return(true));
#else
    ON_CALL(*m_interactive, canOpenApp(_)).WillByDefault(Return(make_ok()));
    EXPECT_CALL(*m_interactive, openApp(_))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedPromise<Ret>(make_ok());
    }));
#endif

    // [THEN] The app keeps running
    EXPECT_CALL(*m_dispatcher, dispatch(Matcher<const actions::ActionCode&>(_))).Times(0);

    // [WHEN] Checking the status
    m_scenario->checkAndShowStatusIfNeed();
    muse::async::processMessages(); // warning
    muse::async::processMessages(); // openApp
}

TEST_F(MuseSounds_MuseSamplerCheckStatusScenarioTest, CheckAndShowStatus_LibInitFailed_CloseClicked_DoesNotOpenUrl)
{
    // [GIVEN] MuseSampler failed to init
    ON_CALL(*m_service, libInitFailed()).WillByDefault(Return(true));

    // [GIVEN] The user clicks Close
    expectLoadingFailedWarning(IInteractive::Result(int(IInteractive::Button::Close)));

    // [THEN] Nothing is opened
    EXPECT_CALL(*m_interactive, openUrl(Matcher<const QUrl&>(_))).Times(0);
    EXPECT_CALL(*m_interactive, openUrl(Matcher<const std::string&>(_))).Times(0);

    // [THEN] The warning stays enabled
    EXPECT_CALL(*m_configuration, setShowLoadingFailedWarning(_)).Times(0);

    // [WHEN] Checking the status
    m_scenario->checkAndShowStatusIfNeed();
    muse::async::processMessages();
}

TEST_F(MuseSounds_MuseSamplerCheckStatusScenarioTest, CheckAndShowStatus_LibInitFailed_DontShowAgainChecked_DisablesWarning)
{
    // [GIVEN] MuseSampler failed to init
    ON_CALL(*m_service, libInitFailed()).WillByDefault(Return(true));

    // [GIVEN] The user checks "Don't show again" and clicks Close
    expectLoadingFailedWarning(IInteractive::Result(int(IInteractive::Button::Close), false /*showAgain*/));

    // [THEN] The warning is disabled
    EXPECT_CALL(*m_configuration, setShowLoadingFailedWarning(false)).Times(1);

    // [WHEN] Checking the status
    m_scenario->checkAndShowStatusIfNeed();
    muse::async::processMessages();
}

TEST_F(MuseSounds_MuseSamplerCheckStatusScenarioTest, CheckAndShowStatus_LibInitFailed_DontShowAgainAndTroubleshoot_OpensUrl)
{
    // [GIVEN] MuseSampler failed to init
    ON_CALL(*m_service, libInitFailed()).WillByDefault(Return(true));

    // [GIVEN] The troubleshoot URL
    const QUrl troubleshootUrl("test://troubleshoot");
    ON_CALL(*m_configuration, museSoundsTroubleshootUrl()).WillByDefault(Return(troubleshootUrl));

    // [GIVEN] The user checks "Don't show again" and clicks Troubleshoot
    expectLoadingFailedWarning(IInteractive::Result(TROUBLESHOOT_BUTTON, false /*showAgain*/));

    // [THEN] The warning is disabled, and the troubleshoot page is opened
    EXPECT_CALL(*m_configuration, setShowLoadingFailedWarning(false)).Times(1);
    EXPECT_CALL(*m_interactive, openUrl(troubleshootUrl))
    .WillOnce(Return(make_ok()));

    // [WHEN] Checking the status
    m_scenario->checkAndShowStatusIfNeed();
    muse::async::processMessages();
}

TEST_F(MuseSounds_MuseSamplerCheckStatusScenarioTest, CheckAndShowStatus_LibInitFailed_WarningDisabled_ShowsNothing)
{
    // [GIVEN] MuseSampler failed to init
    ON_CALL(*m_service, libInitFailed()).WillByDefault(Return(true));

    // [GIVEN] The user previously chose "Don't show again"
    ON_CALL(*m_configuration, showLoadingFailedWarning()).WillByDefault(Return(false));

    // [THEN] Nothing is checked or shown
    expectNoDialogs();
    EXPECT_CALL(*m_service, checkForUpdate()).Times(0);

    // [WHEN] Checking the status
    m_scenario->checkAndShowStatusIfNeed();
    muse::async::processMessages();

    // [THEN] The status is checked
    EXPECT_TRUE(m_scenario->alreadyChecked());
}

TEST_F(MuseSounds_MuseSamplerCheckStatusScenarioTest, CheckAndShowStatus_CannotCheckForUpdate_DoesNothing)
{
    // [GIVEN] MuseSampler isn't installed
    ON_CALL(*m_service, canCheckForUpdate()).WillByDefault(Return(false));

    // [THEN] Nothing is checked or shown
    expectNoDialogs();
    EXPECT_CALL(*m_service, checkForUpdate()).Times(0);

    // [WHEN] Checking the status
    m_scenario->checkAndShowStatusIfNeed();

    // [THEN] The check can be done later
    EXPECT_FALSE(m_scenario->alreadyChecked());
}

TEST_F(MuseSounds_MuseSamplerCheckStatusScenarioTest, CheckAndShowStatus_IncompatibleLocalVersion_ShowsCriticalUpdate)
{
    // [GIVEN] The local MuseSampler version is no longer supported
    ON_CALL(*m_service, incompatibleLocalVersion()).WillByDefault(Return(true));

    // [THEN] The critical update notification is shown, and the remote version isn't checked
    EXPECT_CALL(*m_interactive, info("MuseSounds needs an update", _, _, _, _, _))
    .WillOnce(Invoke([](auto&&...) { return resolvedResultPromise(); }));
    EXPECT_CALL(*m_service, checkForUpdate()).Times(0);

    // [WHEN] Checking the status
    m_scenario->checkAndShowStatusIfNeed();
    muse::async::processMessages();

    // [THEN] The status is checked
    EXPECT_TRUE(m_scenario->alreadyChecked());
}

TEST_F(MuseSounds_MuseSamplerCheckStatusScenarioTest, CheckAndShowStatus_UpdateAvailable_ShowsNewVersionNotification)
{
    // [GIVEN] A newer MuseSampler version is available
    ON_CALL(*m_service, checkForUpdate())
    .WillByDefault(Invoke([] {
        return resolvedPromise<RetVal<bool> >(RetVal<bool>::make_ok(true));
    }));

    // [THEN] The new version notification is shown
    EXPECT_CALL(*m_interactive, info("An update for MuseSounds is available", _, _, _, _, _))
    .WillOnce(Invoke([](auto&&...) { return resolvedResultPromise(); }));

    // [WHEN] Checking the status
    m_scenario->checkAndShowStatusIfNeed();
    muse::async::processMessages(); // checkForUpdate
    muse::async::processMessages(); // info

    // [THEN] The status is checked
    EXPECT_TRUE(m_scenario->alreadyChecked());
}

TEST_F(MuseSounds_MuseSamplerCheckStatusScenarioTest, CheckAndShowStatus_NoUpdate_ShowsNothing)
{
    // [GIVEN] The local MuseSampler version is the latest one
    ON_CALL(*m_service, checkForUpdate())
    .WillByDefault(Invoke([] {
        return resolvedPromise<RetVal<bool> >(RetVal<bool>::make_ok(false));
    }));

    // [THEN] Nothing is shown
    expectNoDialogs();

    // [WHEN] Checking the status
    m_scenario->checkAndShowStatusIfNeed();
    muse::async::processMessages();

    // [THEN] The status is checked
    EXPECT_TRUE(m_scenario->alreadyChecked());
}

TEST_F(MuseSounds_MuseSamplerCheckStatusScenarioTest, CheckAndShowStatus_UpdateCheckFailed_ShowsNothing)
{
    // [GIVEN] The update check fails
    ON_CALL(*m_service, checkForUpdate())
    .WillByDefault(Invoke([] {
        return resolvedPromise<RetVal<bool> >(RetVal<bool>::make_ret(make_ret(Ret::Code::UnknownError)));
    }));

    // [THEN] Nothing is shown
    expectNoDialogs();

    // [WHEN] Checking the status
    m_scenario->checkAndShowStatusIfNeed();
    muse::async::processMessages();
}

TEST_F(MuseSounds_MuseSamplerCheckStatusScenarioTest, CheckAndShowStatus_UpdateDeclined_DoesNotQuit)
{
    // [GIVEN] A newer MuseSampler version is available
    ON_CALL(*m_service, checkForUpdate())
    .WillByDefault(Invoke([] {
        return resolvedPromise<RetVal<bool> >(RetVal<bool>::make_ok(true));
    }));

    // [GIVEN] The user clicks "Not now"
    EXPECT_CALL(*m_interactive, info("An update for MuseSounds is available", _, _, _, _, _))
    .WillOnce(Invoke([](auto&&...) {
        return resolvedResultPromise(IInteractive::Result(int(IInteractive::Button::CustomButton) + 1));
    }));

    // [THEN] The app keeps running
    EXPECT_CALL(*m_dispatcher, dispatch(Matcher<const actions::ActionCode&>(_))).Times(0);

    // [WHEN] Checking the status
    m_scenario->checkAndShowStatusIfNeed();
    muse::async::processMessages(); // checkForUpdate
    muse::async::processMessages(); // info
}
