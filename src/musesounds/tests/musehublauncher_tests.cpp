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

#include "musesounds/internal/musehublauncher.h"

#include "mocks/musesoundsconfigurationmock.h"
#include "global/tests/mocks/interactivemock.h"

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
class MuseSounds_MuseHubLauncherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_launcher = std::make_shared<MuseHubLauncher>(muse::modularity::globalCtx());

        m_configuration = std::make_shared<NiceMock<MuseSoundsConfigurationMock> >();
        m_interactive = std::make_shared<NiceMock<InteractiveMock> >();

        m_launcher->configuration.set(m_configuration);
        m_launcher->interactive.set(m_interactive);
    }

    std::shared_ptr<MuseHubLauncher> m_launcher;
    std::shared_ptr<NiceMock<MuseSoundsConfigurationMock> > m_configuration;
    std::shared_ptr<NiceMock<InteractiveMock> > m_interactive;

    const MuseHubUrls m_urls { "test-app://page?utm_content=place", "test://page?utm_content=place" };
};
}

TEST_F(MuseSounds_MuseHubLauncherTest, Open_AppOpened_DoesNotOpenWebUrl)
{
    // [GIVEN] The configuration returns the URLs for the page
    EXPECT_CALL(*m_configuration, museHubUrls(MuseHubPage::MuseSoundsPro, "place", String()))
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
    m_launcher->open(MuseHubPage::MuseSoundsPro, "place");
    muse::async::processMessages();
    muse::async::processMessages();
}

TEST_F(MuseSounds_MuseHubLauncherTest, Open_AppCanNotBeOpened_OpensWebUrl)
{
    // [GIVEN] The configuration returns the URLs for the page
    ON_CALL(*m_configuration, museHubUrls(MuseHubPage::MuseSounds, "place", String()))
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
    m_launcher->open(MuseHubPage::MuseSounds, "place");
    muse::async::processMessages();
}

TEST_F(MuseSounds_MuseHubLauncherTest, Open_CanOpenAppRejected_OpensWebUrl)
{
    // [GIVEN] The configuration returns the URLs for the page
    ON_CALL(*m_configuration, museHubUrls(MuseHubPage::MuseSounds, "place", String()))
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
    m_launcher->open(MuseHubPage::MuseSounds, "place");
    muse::async::processMessages();
}

TEST_F(MuseSounds_MuseHubLauncherTest, Open_AppFailed_OpensWebUrl)
{
    // [GIVEN] The configuration returns the URLs for the page
    ON_CALL(*m_configuration, museHubUrls(MuseHubPage::Plugins, "place", String()))
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
    m_launcher->open(MuseHubPage::Plugins, "place");
    muse::async::processMessages();
    muse::async::processMessages();
}

TEST_F(MuseSounds_MuseHubLauncherTest, Open_AppRejected_OpensWebUrl)
{
    // [GIVEN] The configuration returns the URLs for the page
    ON_CALL(*m_configuration, museHubUrls(MuseHubPage::MuseSounds, "place", String()))
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
    m_launcher->open(MuseHubPage::MuseSounds, "place");
    muse::async::processMessages();
    muse::async::processMessages();
}

TEST_F(MuseSounds_MuseHubLauncherTest, OpenSoundLibrary_PassesCodeToConfiguration)
{
    // [GIVEN] The configuration returns the URLs for the sound library
    EXPECT_CALL(*m_configuration, museHubUrls(MuseHubPage::SoundLibrary, "place", String(u"library-code")))
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
    m_launcher->openSoundLibrary(String(u"library-code"), "place");
    muse::async::processMessages();
    muse::async::processMessages();
}
