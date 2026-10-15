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

#include <QIODevice>
#include <QUrl>

#include "async/asyncable.h"
#include "async/processevents.h"
#include "modularity/ioc.h"

#include "musesounds/internal/musesamplercheckstatusservice.h"

#include "mocks/musesoundsconfigurationmock.h"
#include "musesampler/tests/mocks/musesamplerinfomock.h"
#include "musesampler/tests/mocks/musesamplerconfigurationmock.h"
#include "network/tests/mocks/networkmanagercreatormock.h"
#include "network/tests/mocks/networkmanagermock.h"

using namespace ::testing;
using namespace mu::musesounds;
using namespace muse;
using namespace muse::network;
using namespace muse::musesampler;

namespace {
QByteArray versionReply(const std::string& version)
{
    return QByteArray::fromStdString(R"({ "data": { "product": { "assets": [ { "version": ")" + version + R"(" } ] } } })");
}
}

namespace mu::musesounds {
class MuseSounds_MuseSamplerCheckStatusServiceTest : public ::testing::Test, public muse::async::Asyncable
{
protected:
    void SetUp() override
    {
        m_service = std::make_shared<MuseSamplerCheckStatusService>(muse::modularity::globalCtx());

        m_configuration = std::make_shared<NiceMock<MuseSoundsConfigurationMock> >();
        m_networkManagerCreator = std::make_shared<NiceMock<NetworkManagerCreatorMock> >();
        m_networkManager = std::make_shared<NiceMock<NetworkManagerMock> >();
        m_museSamplerConfiguration = std::make_shared<NiceMock<MuseSamplerConfigurationMock> >();
        m_museSampler = std::make_shared<NiceMock<MuseSamplerInfoMock> >();

        m_service->configuration.set(m_configuration);
        m_service->networkManagerCreator.set(m_networkManagerCreator);
        m_service->museSamplerConfiguration.set(m_museSamplerConfiguration);
        m_service->museSampler.set(m_museSampler);

        ON_CALL(*m_networkManagerCreator, makeNetworkManager())
        .WillByDefault(Return(m_networkManager));

        ON_CALL(*m_configuration, checkForMuseSamplerUpdateUrl())
        .WillByDefault(Return(m_versionUrl));
    }

    //! NOTE: checkForUpdate() posts from a queued promise body, so the reply is delivered
    //! after the request is made: write the reply here and finish the progress from the test
    Progress expectVersionRequest(const QByteArray& reply)
    {
        Progress progress;

        EXPECT_CALL(*m_networkManager, post(m_versionUrl, _, _, _))
        .WillOnce(Invoke([progress, reply](const QUrl&, OutgoingDeviceVar, IncomingDevicePtr received, const RequestHeaders&) {
            received->open(QIODevice::WriteOnly);
            received->write(reply);
            received->close();
            return RetVal<Progress>::make_ok(progress);
        }));

        return progress;
    }

    std::optional<RetVal<bool> > checkForUpdate(Progress* progress = nullptr, const ProgressResult& progressResult = ProgressResult())
    {
        std::optional<RetVal<bool> > result;

        m_service->checkForUpdate().onResolve(this, [&result](const RetVal<bool>& res) {
            result = res;
        });

        muse::async::processMessages();

        if (progress) {
            progress->finish(progressResult);
        }

        return result;
    }

    std::shared_ptr<MuseSamplerCheckStatusService> m_service;
    std::shared_ptr<MuseSoundsConfigurationMock> m_configuration;
    std::shared_ptr<NetworkManagerCreatorMock> m_networkManagerCreator;
    std::shared_ptr<NetworkManagerMock> m_networkManager;
    std::shared_ptr<MuseSamplerConfigurationMock> m_museSamplerConfiguration;
    std::shared_ptr<MuseSamplerInfoMock> m_museSampler;

    const QUrl m_versionUrl = QUrl("test://musesampler/version");
};
}

// ==================================================
// libInitFailed()
// ==================================================

TEST_F(MuseSounds_MuseSamplerCheckStatusServiceTest, LibInitFailed)
{
    // [GIVEN] MuseSampler failed to init
    ON_CALL(*m_museSampler, initFailed()).WillByDefault(Return(true));

    // [THEN] The service reports it
    EXPECT_TRUE(m_service->libInitFailed());

    // [GIVEN] MuseSampler inited successfully
    ON_CALL(*m_museSampler, initFailed()).WillByDefault(Return(false));

    // [THEN] The service doesn't report a failure
    EXPECT_FALSE(m_service->libInitFailed());
}

// ==================================================
// canCheckForUpdate()
// ==================================================

TEST_F(MuseSounds_MuseSamplerCheckStatusServiceTest, CanCheckForUpdate)
{
    // [GIVEN] The local MuseSampler version is known
    ON_CALL(*m_museSampler, version()).WillByDefault(ReturnRefOfCopy(Version(0, 105, 0)));

    // [THEN] The update can be checked
    EXPECT_TRUE(m_service->canCheckForUpdate());

    // [GIVEN] MuseSampler isn't installed, so its version is unknown
    ON_CALL(*m_museSampler, version()).WillByDefault(ReturnRefOfCopy(Version()));

    // [THEN] The update can't be checked
    EXPECT_FALSE(m_service->canCheckForUpdate());
}

// ==================================================
// incompatibleLocalVersion()
// ==================================================

TEST_F(MuseSounds_MuseSamplerCheckStatusServiceTest, IncompatibleLocalVersion)
{
    // [GIVEN] The minimum supported version is 0.100.0
    ON_CALL(*m_museSamplerConfiguration, minSupportedVersion()).WillByDefault(Return(Version(0, 100, 0)));

    // [GIVEN] The local version is older than the minimum supported one
    ON_CALL(*m_museSampler, version()).WillByDefault(ReturnRefOfCopy(Version(0, 99, 0)));

    // [THEN] The local version is incompatible
    EXPECT_TRUE(m_service->incompatibleLocalVersion());

    // [GIVEN] The local version is the minimum supported one
    ON_CALL(*m_museSampler, version()).WillByDefault(ReturnRefOfCopy(Version(0, 100, 0)));

    // [THEN] The local version is compatible
    EXPECT_FALSE(m_service->incompatibleLocalVersion());

    // [GIVEN] The local version is newer than the minimum supported one
    ON_CALL(*m_museSampler, version()).WillByDefault(ReturnRefOfCopy(Version(0, 105, 0)));

    // [THEN] The local version is compatible
    EXPECT_FALSE(m_service->incompatibleLocalVersion());
}

// ==================================================
// checkForUpdate()
// ==================================================

TEST_F(MuseSounds_MuseSamplerCheckStatusServiceTest, CheckForUpdate_NewerRemoteVersion_ResolvesTrue)
{
    // [GIVEN] The local version is 0.105.0
    ON_CALL(*m_museSampler, version()).WillByDefault(ReturnRefOfCopy(Version(0, 105, 0)));

    // [GIVEN] The server reports a newer version
    Progress progress = expectVersionRequest(versionReply("0.106.0"));

    // [WHEN] Checking for an update
    std::optional<RetVal<bool> > result = checkForUpdate(&progress, ProgressResult::make_ok(Val()));

    // [THEN] An update is available
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->ret);
    EXPECT_TRUE(result->val);
}

TEST_F(MuseSounds_MuseSamplerCheckStatusServiceTest, CheckForUpdate_SameRemoteVersion_ResolvesFalse)
{
    // [GIVEN] The local version is 0.105.0
    ON_CALL(*m_museSampler, version()).WillByDefault(ReturnRefOfCopy(Version(0, 105, 0)));

    // [GIVEN] The server reports the local version
    Progress progress = expectVersionRequest(versionReply("0.105.0"));

    // [WHEN] Checking for an update
    std::optional<RetVal<bool> > result = checkForUpdate(&progress, ProgressResult::make_ok(Val()));

    // [THEN] No update is available
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->ret);
    EXPECT_FALSE(result->val);
}

TEST_F(MuseSounds_MuseSamplerCheckStatusServiceTest, CheckForUpdate_OlderRemoteVersion_ResolvesFalse)
{
    // [GIVEN] The local version is 0.105.0
    ON_CALL(*m_museSampler, version()).WillByDefault(ReturnRefOfCopy(Version(0, 105, 0)));

    // [GIVEN] The server reports an older version
    Progress progress = expectVersionRequest(versionReply("0.104.0"));

    // [WHEN] Checking for an update
    std::optional<RetVal<bool> > result = checkForUpdate(&progress, ProgressResult::make_ok(Val()));

    // [THEN] No update is available
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->ret);
    EXPECT_FALSE(result->val);
}

TEST_F(MuseSounds_MuseSamplerCheckStatusServiceTest, CheckForUpdate_ReplyWithoutAssets_ResolvesError)
{
    // [GIVEN] The local version is 0.105.0
    ON_CALL(*m_museSampler, version()).WillByDefault(ReturnRefOfCopy(Version(0, 105, 0)));

    // [GIVEN] The server replies with no assets
    Progress progress = expectVersionRequest(R"({ "data": { "product": { "assets": [] } } })");

    // [WHEN] Checking for an update
    std::optional<RetVal<bool> > result = checkForUpdate(&progress, ProgressResult::make_ok(Val()));

    // [THEN] The check fails
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->ret);
    EXPECT_FALSE(result->val);
}

TEST_F(MuseSounds_MuseSamplerCheckStatusServiceTest, CheckForUpdate_InvalidJson_ResolvesError)
{
    // [GIVEN] The local version is 0.105.0
    ON_CALL(*m_museSampler, version()).WillByDefault(ReturnRefOfCopy(Version(0, 105, 0)));

    // [GIVEN] The server replies with invalid JSON
    Progress progress = expectVersionRequest("not json");

    // [WHEN] Checking for an update
    std::optional<RetVal<bool> > result = checkForUpdate(&progress, ProgressResult::make_ok(Val()));

    // [THEN] The check fails
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->ret);
}

TEST_F(MuseSounds_MuseSamplerCheckStatusServiceTest, CheckForUpdate_RequestFinishesWithError_ResolvesError)
{
    // [GIVEN] The local version is 0.105.0
    ON_CALL(*m_museSampler, version()).WillByDefault(ReturnRefOfCopy(Version(0, 105, 0)));

    // [GIVEN] The request fails
    Progress progress = expectVersionRequest(QByteArray());

    // [WHEN] Checking for an update
    std::optional<RetVal<bool> > result = checkForUpdate(&progress,
                                                         ProgressResult::make_ret(make_ret(Ret::Code::UnknownError)));

    // [THEN] The check fails
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->ret);
    EXPECT_FALSE(result->val);
}

TEST_F(MuseSounds_MuseSamplerCheckStatusServiceTest, CheckForUpdate_RequestCannotBeSent_ResolvesError)
{
    // [GIVEN] The local version is 0.105.0
    ON_CALL(*m_museSampler, version()).WillByDefault(ReturnRefOfCopy(Version(0, 105, 0)));

    // [GIVEN] The request can't be sent
    EXPECT_CALL(*m_networkManager, post(_, _, _, _))
    .WillOnce(Return(RetVal<Progress>::make_ret(make_ret(Ret::Code::UnknownError))));

    // [WHEN] Checking for an update
    std::optional<RetVal<bool> > result = checkForUpdate();

    // [THEN] The check fails
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->ret);
}

TEST_F(MuseSounds_MuseSamplerCheckStatusServiceTest, CheckForUpdate_LocalVersionUnknown_ResolvesErrorWithoutRequest)
{
    // [GIVEN] The local MuseSampler version is unknown
    ON_CALL(*m_museSampler, version()).WillByDefault(ReturnRefOfCopy(Version()));

    // [THEN] No request is sent
    EXPECT_CALL(*m_networkManager, post(_, _, _, _)).Times(0);

    // [WHEN] Checking for an update
    std::optional<RetVal<bool> > result = checkForUpdate();

    // [THEN] The check fails
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->ret);
    EXPECT_FALSE(result->val);
}
