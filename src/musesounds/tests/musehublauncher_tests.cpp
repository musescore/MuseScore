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

#include <optional>

#include "async/asyncable.h"
#include "async/processevents.h"
#include "modularity/ioc.h"

#include "musesounds/internal/musehublauncher.h"

#include "mocks/musesoundsconfigurationmock.h"
#include "global/tests/mocks/interactivemock.h"
#include "global/tests/mocks/processmock.h"

using namespace ::testing;
using namespace mu::musesounds;
using namespace muse;

namespace {
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

template<typename T>
async::Promise<T> rejectedPromise(int code, const std::string& msg)
{
    return async::make_promise<T>([code, msg](auto resolve, auto reject) {
        (void)resolve;
        return reject(code, msg);
    });
}
}

namespace mu::musesounds {
class MuseSounds_MuseHubLauncherTest : public ::testing::Test, public muse::async::Asyncable
{
protected:
    void SetUp() override
    {
        m_launcher = std::make_shared<MuseHubLauncher>(muse::modularity::globalCtx());

        m_configuration = std::make_shared<NiceMock<MuseSoundsConfigurationMock> >();
        m_process = std::make_shared<NiceMock<ProcessMock> >();
        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();

        m_launcher->configuration.set(m_configuration);
        m_launcher->process.set(m_process);
        m_launcher->interactive.set(m_interactive);
    }

    std::optional<Ret> waitForResult(async::Promise<Ret> promise)
    {
        std::optional<Ret> result;
        promise.onResolve(this, [&result](const Ret& ret) {
            result = ret;
        });

        muse::async::processMessages(); // launcher
        muse::async::processMessages(); // canOpenApp
        muse::async::processMessages(); // openApp

        return result;
    }

    std::shared_ptr<MuseHubLauncher> m_launcher;
    std::shared_ptr<NiceMock<MuseSoundsConfigurationMock> > m_configuration;
    std::shared_ptr<NiceMock<ProcessMock> > m_process;
    std::shared_ptr<NiceMock<InteractiveMock> > m_interactive;

    const MuseHubUrls m_urls { "test-app://page?utm_content=place", "test://page?utm_content=place" };
    const SoundLibraryInfo m_soundLibrary { String(u"library-id"), String(u"library-code") };
};
}

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
TEST_F(MuseSounds_MuseHubLauncherTest, Open_AppOpened_DoesNotOpenWebUrl)
{
    // [GIVEN] The configuration returns the URLs for the page
    EXPECT_CALL(*m_configuration, museHubUrls(MuseHubPage::MuseSoundsPro, "place", SoundLibraryInfo()))
    .WillOnce(Return(m_urls));

    // [GIVEN] MuseHub can be opened
    EXPECT_CALL(*m_interactive, canOpenApp(UriQuery(m_urls.museHubUri)))
    .WillOnce(InvokeWithoutArgs([]() {
        return resolvedPromise(make_ok());
    }));

    // [THEN] MuseHub is opened
    EXPECT_CALL(*m_interactive, openApp(UriQuery(m_urls.museHubUri)))
    .WillOnce(InvokeWithoutArgs([]() {
        return resolvedPromise(make_ok());
    }));

    // [THEN] The web URL isn't opened
    EXPECT_CALL(*m_interactive, openUrl(Matcher<const std::string&>(_))).Times(0);
    EXPECT_CALL(*m_interactive, openUrl(Matcher<const QUrl&>(_))).Times(0);

    // [WHEN] Opening the page
    const std::optional<Ret> result = waitForResult(m_launcher->open(MuseHubPage::MuseSoundsPro, "place"));

    // [THEN] The promise is resolved successfully
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->success());
}

TEST_F(MuseSounds_MuseHubLauncherTest, Open_AppCanNotBeOpened_OpensWebUrl)
{
    // [GIVEN] The configuration returns the URLs for the page
    ON_CALL(*m_configuration, museHubUrls(MuseHubPage::MuseSounds, "place", SoundLibraryInfo()))
    .WillByDefault(Return(m_urls));

    // [GIVEN] No app can open the MuseHub link
    EXPECT_CALL(*m_interactive, canOpenApp(UriQuery(m_urls.museHubUri)))
    .WillOnce(InvokeWithoutArgs([]() {
        return resolvedPromise(make_ret(Ret::Code::UnknownError));
    }));

    // [THEN] MuseHub isn't opened
    EXPECT_CALL(*m_interactive, openApp(_)).Times(0);

    // [THEN] The web URL is opened instead
    EXPECT_CALL(*m_interactive, openUrl(m_urls.webUrl))
    .WillOnce(Return(make_ok()));

    // [WHEN] Opening the page
    const std::optional<Ret> result = waitForResult(m_launcher->open(MuseHubPage::MuseSounds, "place"));

    // [THEN] The promise is resolved with the web URL result
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->success());
}

TEST_F(MuseSounds_MuseHubLauncherTest, Open_CanOpenAppRejected_OpensWebUrl)
{
    // [GIVEN] The configuration returns the URLs for the page
    ON_CALL(*m_configuration, museHubUrls(MuseHubPage::MuseSounds, "place", SoundLibraryInfo()))
    .WillByDefault(Return(m_urls));

    // [GIVEN] Checking whether MuseHub can be opened is rejected
    EXPECT_CALL(*m_interactive, canOpenApp(UriQuery(m_urls.museHubUri)))
    .WillOnce(InvokeWithoutArgs([]() {
        return rejectedPromise<Ret>(int(Ret::Code::InternalError), "error");
    }));

    // [THEN] MuseHub isn't opened
    EXPECT_CALL(*m_interactive, openApp(_)).Times(0);

    // [THEN] The web URL is opened instead
    EXPECT_CALL(*m_interactive, openUrl(m_urls.webUrl))
    .WillOnce(Return(make_ok()));

    // [WHEN] Opening the page
    const std::optional<Ret> result = waitForResult(m_launcher->open(MuseHubPage::MuseSounds, "place"));

    // [THEN] The promise is resolved with the web URL result
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->success());
}

TEST_F(MuseSounds_MuseHubLauncherTest, Open_AppFailed_OpensWebUrl)
{
    // [GIVEN] The configuration returns the URLs for the page
    ON_CALL(*m_configuration, museHubUrls(MuseHubPage::Plugins, "place", SoundLibraryInfo()))
    .WillByDefault(Return(m_urls));

    // [GIVEN] MuseHub can be opened
    EXPECT_CALL(*m_interactive, canOpenApp(UriQuery(m_urls.museHubUri)))
    .WillOnce(InvokeWithoutArgs([]() {
        return resolvedPromise(make_ok());
    }));

    // [GIVEN] Opening MuseHub fails
    EXPECT_CALL(*m_interactive, openApp(UriQuery(m_urls.museHubUri)))
    .WillOnce(InvokeWithoutArgs([]() {
        return resolvedPromise(make_ret(Ret::Code::UnknownError));
    }));

    // [THEN] The web URL is opened instead
    EXPECT_CALL(*m_interactive, openUrl(m_urls.webUrl))
    .WillOnce(Return(make_ok()));

    // [WHEN] Opening the page
    const std::optional<Ret> result = waitForResult(m_launcher->open(MuseHubPage::Plugins, "place"));

    // [THEN] The promise is resolved with the web URL result
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->success());
}

TEST_F(MuseSounds_MuseHubLauncherTest, Open_AppRejected_OpensWebUrl)
{
    // [GIVEN] The configuration returns the URLs for the page
    ON_CALL(*m_configuration, museHubUrls(MuseHubPage::MuseSounds, "place", SoundLibraryInfo()))
    .WillByDefault(Return(m_urls));

    // [GIVEN] MuseHub can be opened
    EXPECT_CALL(*m_interactive, canOpenApp(UriQuery(m_urls.museHubUri)))
    .WillOnce(InvokeWithoutArgs([]() {
        return resolvedPromise(make_ok());
    }));

    // [GIVEN] Opening MuseHub is rejected
    EXPECT_CALL(*m_interactive, openApp(UriQuery(m_urls.museHubUri)))
    .WillOnce(InvokeWithoutArgs([]() {
        return rejectedPromise<Ret>(int(Ret::Code::UnknownError), "error");
    }));

    // [THEN] The web URL is opened instead
    EXPECT_CALL(*m_interactive, openUrl(m_urls.webUrl))
    .WillOnce(Return(make_ok()));

    // [WHEN] Opening the page
    const std::optional<Ret> result = waitForResult(m_launcher->open(MuseHubPage::MuseSounds, "place"));

    // [THEN] The promise is resolved with the web URL result
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->success());
}

TEST_F(MuseSounds_MuseHubLauncherTest, Open_WebUrlFailed_ResolvesWithError)
{
    // [GIVEN] The configuration returns the URLs for the page
    ON_CALL(*m_configuration, museHubUrls(MuseHubPage::MuseSounds, "place", SoundLibraryInfo()))
    .WillByDefault(Return(m_urls));

    // [GIVEN] No app can open the MuseHub link
    ON_CALL(*m_interactive, canOpenApp(UriQuery(m_urls.museHubUri)))
    .WillByDefault(InvokeWithoutArgs([]() {
        return resolvedPromise(make_ret(Ret::Code::UnknownError));
    }));

    // [GIVEN] Opening the web URL fails
    EXPECT_CALL(*m_interactive, openUrl(m_urls.webUrl))
    .WillOnce(Return(make_ret(Ret::Code::InternalError)));

    // [WHEN] Opening the page
    const std::optional<Ret> result = waitForResult(m_launcher->open(MuseHubPage::MuseSounds, "place"));

    // [THEN] The promise is resolved with the web URL error
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->code(), int(Ret::Code::InternalError));
}

TEST_F(MuseSounds_MuseHubLauncherTest, OpenSoundLibrary_OpensMuseHub)
{
    // [GIVEN] The configuration returns the URLs for the sound library
    EXPECT_CALL(*m_configuration, museHubUrls(MuseHubPage::SoundLibrary, "place", m_soundLibrary))
    .WillOnce(Return(m_urls));

    // [GIVEN] MuseHub can be opened
    EXPECT_CALL(*m_interactive, canOpenApp(UriQuery(m_urls.museHubUri)))
    .WillOnce(InvokeWithoutArgs([]() {
        return resolvedPromise(make_ok());
    }));

    // [THEN] MuseHub is opened on the sound library page
    EXPECT_CALL(*m_interactive, openApp(UriQuery(m_urls.museHubUri)))
    .WillOnce(InvokeWithoutArgs([]() {
        return resolvedPromise(make_ok());
    }));

    // [WHEN] Opening the sound library
    const std::optional<Ret> result = waitForResult(m_launcher->openSoundLibrary(m_soundLibrary, "place"));

    // [THEN] The promise is resolved successfully
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->success());
}

#else

TEST_F(MuseSounds_MuseHubLauncherTest, OpenSoundLibrary_NoMuseHub_OpensMuseSoundsManager)
{
    // [GIVEN] The configuration returns the URLs for the sound library
    EXPECT_CALL(*m_configuration, museHubUrls(MuseHubPage::SoundLibrary, "place", m_soundLibrary))
    .WillOnce(Return(m_urls));

    // [THEN] MuseSounds Manager is launched without installing anything
    EXPECT_CALL(*m_process, startDetached("muse-sounds-manager", std::vector<std::string>()))
    .WillOnce(Return(true));

    // [THEN] Neither MuseHub nor the web URL is opened
    EXPECT_CALL(*m_interactive, canOpenApp(_)).Times(0);
    EXPECT_CALL(*m_interactive, openApp(_)).Times(0);
    EXPECT_CALL(*m_interactive, openUrl(Matcher<const std::string&>(_))).Times(0);

    // [WHEN] Opening the sound library
    const std::optional<Ret> result = waitForResult(m_launcher->openSoundLibrary(m_soundLibrary, "place"));

    // [THEN] The promise is resolved successfully
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->success());
}

TEST_F(MuseSounds_MuseHubLauncherTest, OpenPlugins_NoMuseHub_OpensWebUrl)
{
    // [GIVEN] The configuration returns the URLs for the plugins page
    EXPECT_CALL(*m_configuration, museHubUrls(MuseHubPage::Plugins, "place", SoundLibraryInfo()))
    .WillOnce(Return(m_urls));

    // [THEN] Neither MuseHub nor MuseSounds Manager is opened
    EXPECT_CALL(*m_interactive, canOpenApp(_)).Times(0);
    EXPECT_CALL(*m_interactive, openApp(_)).Times(0);
    EXPECT_CALL(*m_process, startDetached(_, _)).Times(0);

    // [THEN] The plugins page is opened on the website
    EXPECT_CALL(*m_interactive, openUrl(m_urls.webUrl))
    .WillOnce(Return(make_ok()));

    // [WHEN] Opening the plugins page
    const std::optional<Ret> result = waitForResult(m_launcher->open(MuseHubPage::Plugins, "place"));

    // [THEN] The promise is resolved successfully
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->success());
}

TEST_F(MuseSounds_MuseHubLauncherTest, Open_NoMuseHub_OpensMuseSoundsManagerWithPageArgs)
{
    const std::vector<std::pair<MuseHubPage, std::vector<std::string> > > cases {
        { MuseHubPage::MuseSounds, {} },
        { MuseHubPage::FreeMuseSounds, {} },
        { MuseHubPage::MuseSoundsCore, {} },
        { MuseHubPage::MuseSoundsPro, {} },
        { MuseHubPage::MuseSamplerUpdate, { "--show-sampler-update", "--show-hidden-statuses" } },
    };

    for (const auto& [page, args] : cases) {
        SCOPED_TRACE(int(page));

        // [GIVEN] The configuration returns the URLs for the page
        ON_CALL(*m_configuration, museHubUrls(page, "place", SoundLibraryInfo()))
        .WillByDefault(Return(m_urls));

        // [THEN] MuseSounds Manager is launched with the page arguments
        EXPECT_CALL(*m_process, startDetached("muse-sounds-manager", args))
        .WillOnce(Return(true));

        // [THEN] Neither MuseHub nor the web URL is opened
        EXPECT_CALL(*m_interactive, canOpenApp(_)).Times(0);
        EXPECT_CALL(*m_interactive, openApp(_)).Times(0);
        EXPECT_CALL(*m_interactive, openUrl(Matcher<const std::string&>(_))).Times(0);

        // [WHEN] Opening the page
        const std::optional<Ret> result = waitForResult(m_launcher->open(page, "place"));

        // [THEN] The promise is resolved successfully
        ASSERT_TRUE(result.has_value());
        EXPECT_TRUE(result->success());

        Mock::VerifyAndClearExpectations(m_process.get());
        Mock::VerifyAndClearExpectations(m_interactive.get());
    }
}

TEST_F(MuseSounds_MuseHubLauncherTest, Open_NoMuseHub_ManagerFailed_OpensWebUrl)
{
    // [GIVEN] The configuration returns the URLs for the page
    ON_CALL(*m_configuration, museHubUrls(MuseHubPage::MuseSamplerUpdate, "place", SoundLibraryInfo()))
    .WillByDefault(Return(m_urls));

    // [GIVEN] MuseSounds Manager can't be launched
    EXPECT_CALL(*m_process, startDetached("muse-sounds-manager", _))
    .WillOnce(Return(false));

    // [THEN] The web URL of the page is opened instead
    EXPECT_CALL(*m_interactive, openUrl(m_urls.webUrl))
    .WillOnce(Return(make_ok()));

    // [WHEN] Opening the page
    const std::optional<Ret> result = waitForResult(m_launcher->open(MuseHubPage::MuseSamplerUpdate, "place"));

    // [THEN] The promise is resolved with the web URL result
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->success());
}
#endif
