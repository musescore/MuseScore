/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#include <QIODevice>

using ::testing::_;
using ::testing::Return;

#include "framework/network/networktypes.h"

#include "mocks/updateconfigurationmock.h"
#include "network/tests/mocks/networkmanagercreatormock.h"
#include "network/tests/mocks/networkmanagermock.h"
#include "global/tests/mocks/systeminfomock.h"

#include "update/internal/updateservice.h"

using namespace mu;
using namespace mu::update;

class UpdateServiceTests : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_service = new UpdateService();

        m_configuration = std::make_shared<UpdateConfigurationMock>();
        m_service->setconfiguration(m_configuration);

        m_networkManagerCreator = std::make_shared<network::NetworkManagerCreatorMock>();
        m_service->setnetworkManagerCreator(m_networkManagerCreator);

        m_networkManager = std::make_shared<network::NetworkManagerMock>();
        ON_CALL(*m_networkManagerCreator, makeNetworkManager())
        .WillByDefault(Return(m_networkManager));

        m_systemInfoMock = std::make_shared<SystemInfoMock>();
        m_service->setsystemInfo(m_systemInfoMock);
    }

    void TearDown() override
    {
        delete m_service;
    }

    void makeReleaseInfo() const
    {
        QString releaseInfo = "{"
                              "\"tag_name\": \"v5.0\","
                              "\"assets\": ["
                              "{ \"name\": \"MuseScore.dmg\", \"browser_download_url\": \"blabla\" },"
                              "{ \"name\": \"MuseScore.msi\", \"browser_download_url\": \"blabla\" },"
                              "{ \"name\": \"MuseScore.AppImage\", \"browser_download_url\": \"blabla\" }"
                              "],"
                              "\"assetsNew\": ["
                              "{ \"name\": \"MuseScore-arm.AppImage\", \"browser_download_url\": \"blabla\" },"
                              "{ \"name\": \"MuseScore-aarch64.AppImage\", \"browser_download_url\": \"blabla\" }"
                              "]"
                              "}";

        EXPECT_CALL(*m_networkManager, get(_, _, _))
        .WillOnce(testing::Invoke(
                      [releaseInfo](const QUrl&, network::IncomingDevice* buf, const network::RequestHeaders&) {
            buf->open(network::IncomingDevice::WriteOnly);
            buf->write(releaseInfo.toUtf8());
            buf->close();

            return make_ok();
        }));
    }

    UpdateService* m_service = nullptr;
    std::shared_ptr<UpdateConfigurationMock> m_configuration;

    std::shared_ptr<network::NetworkManagerCreatorMock> m_networkManagerCreator;
    std::shared_ptr<network::NetworkManagerMock> m_networkManager;
    std::shared_ptr<SystemInfoMock> m_systemInfoMock;
};

TEST_F(UpdateServiceTests, ParseRelease_Linux_x86_64)
{
    //! [GIVEN] Release info
    makeReleaseInfo();

    //! [GIVEN] System is Linux x86_64
    ON_CALL(*m_systemInfoMock, productType())
    .WillByDefault(Return(ISystemInfo::ProductType::Linux));

    ON_CALL(*m_systemInfoMock, cpuArchitecture())
    .WillByDefault(Return(ISystemInfo::CpuArchitecture::x86_64));

    //! [WHEN] Check for update
    mu::RetVal<ReleaseInfo> retVal = m_service->checkForUpdate();

    //! [THEN] Should return correct release file
    EXPECT_TRUE(retVal.ret);
    EXPECT_EQ(retVal.val.fileName, "MuseScore.AppImage");
}

TEST_F(UpdateServiceTests, ParseRelease_Linux_arm)
{
    //! [GIVEN] Release info
    makeReleaseInfo();

    //! [GIVEN] System is Linux arm
    ON_CALL(*m_systemInfoMock, productType())
    .WillByDefault(Return(ISystemInfo::ProductType::Linux));

    ON_CALL(*m_systemInfoMock, cpuArchitecture())
    .WillByDefault(Return(ISystemInfo::CpuArchitecture::Arm));

    //! [WHEN] Check for update
    mu::RetVal<ReleaseInfo> retVal = m_service->checkForUpdate();

    //! [THEN] Should return correct release file
    EXPECT_TRUE(retVal.ret);
    EXPECT_EQ(retVal.val.fileName, "MuseScore-arm.AppImage");
}

TEST_F(UpdateServiceTests, ParseRelease_Linux_aarch64)
{
    //! [GIVEN] Release info
    makeReleaseInfo();

    //! [GIVEN] System is Linux arm64
    ON_CALL(*m_systemInfoMock, productType())
    .WillByDefault(Return(ISystemInfo::ProductType::Linux));

    ON_CALL(*m_systemInfoMock, cpuArchitecture())
    .WillByDefault(Return(ISystemInfo::CpuArchitecture::Arm64));

    //! [WHEN] Check for update
    mu::RetVal<ReleaseInfo> retVal = m_service->checkForUpdate();

    //! [THEN] Should return correct release file
    EXPECT_TRUE(retVal.ret);
    EXPECT_EQ(retVal.val.fileName, "MuseScore-aarch64.AppImage");
}

TEST_F(UpdateServiceTests, ParseRelease_Linux_Unknown)
{
    //! [GIVEN] Release info
    makeReleaseInfo();

    //! [GIVEN] System is Linux Unknown
    ON_CALL(*m_systemInfoMock, productType())
    .WillByDefault(Return(ISystemInfo::ProductType::Linux));

    ON_CALL(*m_systemInfoMock, cpuArchitecture())
    .WillByDefault(Return(ISystemInfo::CpuArchitecture::Unknown));

    //! [WHEN] Check for update
    mu::RetVal<ReleaseInfo> retVal = m_service->checkForUpdate();

    //! [THEN] Should return correct release file
    EXPECT_TRUE(retVal.ret);
    EXPECT_EQ(retVal.val.fileName, "MuseScore.AppImage");
}

TEST_F(UpdateServiceTests, ParseRelease_Windows)
{
    //! [GIVEN] Release info
    makeReleaseInfo();

    //! [GIVEN] System is Windows, cpuArchitecture isn't important
    ON_CALL(*m_systemInfoMock, productType())
    .WillByDefault(Return(ISystemInfo::ProductType::Windows));

    EXPECT_CALL(*m_systemInfoMock, cpuArchitecture())
    .Times(1);

    //! [WHEN] Check for update
    mu::RetVal<ReleaseInfo> retVal = m_service->checkForUpdate();

    //! [THEN] Should return correct release file
    EXPECT_TRUE(retVal.ret);
    EXPECT_EQ(retVal.val.fileName, "MuseScore.msi");
}

TEST_F(UpdateServiceTests, ParseRelease_MacOS)
{
    //! [GIVEN] Release info
    makeReleaseInfo();

    //! [GIVEN] System is MacOS, cpuArchitecture isn't important
    ON_CALL(*m_systemInfoMock, productType())
    .WillByDefault(Return(ISystemInfo::ProductType::MacOS));

    EXPECT_CALL(*m_systemInfoMock, cpuArchitecture())
    .Times(1);

    //! [WHEN] Check for update
    mu::RetVal<ReleaseInfo> retVal = m_service->checkForUpdate();

    //! [THEN] Should return correct release file
    EXPECT_TRUE(retVal.ret);
    EXPECT_EQ(retVal.val.fileName, "MuseScore.dmg");
}
