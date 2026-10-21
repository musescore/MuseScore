/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

#include <map>

#include "async/asyncable.h"
#include "async/processevents.h"

#include "audio/common/rpc/rpcpacker.h"
#include "audio/main/internal/audiodrivercontroller.h"

#include "mocks/audioconfigurationmock.h"
#include "mocks/audiodriverfactorymock.h"
#include "mocks/audiodrivermock.h"
#include "mocks/rpcchannelmock.h"

using ::testing::_;
using ::testing::Assign;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::rpc;

namespace {
const std::string CURRENT_DRIVER_NAME("ASIO");
const std::string DEFAULT_DRIVER_NAME("WASAPI");

const AudioDeviceID HEADPHONES("headphones");
const AudioDeviceID SPEAKERS("speakers");

const OutputSpec DEFAULT_OUTPUT_SPEC { 44100, 1024, 2 };
const OutputSpec DESIRED_OUTPUT_SPEC { 48000, 512, 2 };

MATCHER_P2(SpecIs, deviceId, output, "")
{
    return arg.deviceId == deviceId && arg.output == output && arg.callback != nullptr;
}

MATCHER_P(SetOutputSpecRequest, output, "")
{
    OutputSpec sentOutput;
    return arg.method == Method::SetOutputSpec && RpcPacker::unpack(arg.data, sentOutput) && sentOutput == output;
}
}

namespace muse::audio {
class Audio_AudioDriverControllerTests : public ::testing::Test, public async::Asyncable
{
protected:
    struct Driver {
        std::shared_ptr<NiceMock<AudioDriverMock> > mock;
        IAudioDriver::Spec activeSpec;
        bool opened = false;
        async::Channel<IAudioDriver::Spec> activeSpecChanged;
        async::Notification availableOutputDevicesChanged;
    };

    using DriverPtr = std::shared_ptr<Driver>;

    void SetUp() override
    {
        m_configuration = std::make_shared<NiceMock<AudioConfigurationMock> >();
        m_driverFactory = std::make_shared<NiceMock<AudioDriverFactoryMock> >();
        m_rpcChannel = std::make_shared<NiceMock<RpcChannelMock> >();

        m_currentDriver = makeDriver(CURRENT_DRIVER_NAME);
        m_defaultDriver = makeDriver(DEFAULT_DRIVER_NAME);

        m_callback = [](uint8_t*, int) {};

        ON_CALL(*m_configuration, currentAudioApi())
        .WillByDefault(Return(CURRENT_DRIVER_NAME));

        ON_CALL(*m_driverFactory, createDriver(_))
        .WillByDefault(Invoke([this](const std::string& name) -> IAudioDriverPtr {
            auto it = m_drivers.find(name);
            return it != m_drivers.end() ? it->second->mock : nullptr;
        }));

        m_controller = std::make_shared<AudioDriverController>(muse::modularity::globalCtx());
        m_controller->configuration.set(m_configuration);
        m_controller->audioDriverFactory.set(m_driverFactory);
        m_controller->rpcChannel.set(m_rpcChannel);
    }

    DriverPtr makeDriver(const std::string& name)
    {
        DriverPtr driver = std::make_shared<Driver>();
        driver->mock = std::make_shared<NiceMock<AudioDriverMock> >();
        m_drivers[name] = driver;

        //! NOTE: emulates a real driver that tracks its state
        Driver* d = driver.get();

        ON_CALL(*d->mock, defaultDevice())
        .WillByDefault(Return(AudioDeviceID(DEFAULT_DEVICE_ID)));
        ON_CALL(*d->mock, isOpened())
        .WillByDefault(Invoke([d]() {
            return d->opened;
        }));
        ON_CALL(*d->mock, activeSpec())
        .WillByDefault(ReturnRef(d->activeSpec));
        ON_CALL(*d->mock, activeSpecChanged())
        .WillByDefault(Return(d->activeSpecChanged));
        ON_CALL(*d->mock, availableOutputDevicesChanged())
        .WillByDefault(Return(d->availableOutputDevicesChanged));

        ON_CALL(*d->mock, open(_, _))
        .WillByDefault(Invoke([d](const IAudioDriver::Spec& spec, IAudioDriver::Spec* activeSpec) {
            d->opened = true;
            d->activeSpec = spec;
            if (activeSpec) {
                *activeSpec = spec;
            }

            d->activeSpecChanged.send(spec);
            return true;
        }));
        ON_CALL(*d->mock, close())
        .WillByDefault(Invoke([d]() {
            d->opened = false;
        }));

        return driver;
    }

    void givenCurrentDriverIsDefault()
    {
        ON_CALL(*m_configuration, currentAudioApi())
        .WillByDefault(Return(DEFAULT_DRIVER_NAME));
        ON_CALL(*m_configuration, defaultAudioApi())
        .WillByDefault(Return(DEFAULT_DRIVER_NAME));
    }

    IAudioDriver::Spec makeSpec(const AudioDeviceID& deviceId) const
    {
        IAudioDriver::Spec spec;
        spec.deviceId = deviceId;
        spec.output = DESIRED_OUTPUT_SPEC;
        spec.callback = m_callback;
        return spec;
    }

    void givenOpened(const AudioDeviceID& deviceId)
    {
        ASSERT_TRUE(m_controller->open(makeSpec(deviceId), nullptr));
        async::processMessages();
    }

    //! NOTE: emulates a driver that closed and blanked its spec after a failed reopen
    void givenClosedWithBlankSpec(Driver& driver)
    {
        driver.opened = false;
        driver.activeSpec = IAudioDriver::Spec();
    }

    void countNotifications(async::Notification notification, int& count)
    {
        notification.onNotify(this, [&count]() {
            ++count;
        });
    }

    void notifyDevicesChanged(Driver& driver)
    {
        driver.availableOutputDevicesChanged.notify();
        async::processMessages();
    }

    std::shared_ptr<NiceMock<AudioConfigurationMock> > m_configuration;
    std::shared_ptr<NiceMock<AudioDriverFactoryMock> > m_driverFactory;
    std::shared_ptr<NiceMock<RpcChannelMock> > m_rpcChannel;

    std::map<std::string, DriverPtr> m_drivers;
    DriverPtr m_currentDriver;
    DriverPtr m_defaultDriver;

    IAudioDriver::Callback m_callback;

    std::shared_ptr<AudioDriverController> m_controller;
};

TEST_F(Audio_AudioDriverControllerTests, AvailableAudioApiList_ReturnsFactoryDrivers)
{
    //! [GIVEN] The factory provides two drivers
    const std::vector<std::string> drivers { CURRENT_DRIVER_NAME, DEFAULT_DRIVER_NAME };

    EXPECT_CALL(*m_driverFactory, availableAudioDrivers())
    .WillOnce(Return(drivers));

    //! [WHEN] Requesting the available drivers
    std::vector<std::string> result = m_controller->availableAudioApiList();

    //! [THEN] The factory drivers are returned
    EXPECT_EQ(result, drivers);
}

TEST_F(Audio_AudioDriverControllerTests, Open_OpensRequestedDeviceOnCurrentDriver)
{
    //! [THEN] The current driver is created and opened with the requested device
    EXPECT_CALL(*m_driverFactory, createDriver(CURRENT_DRIVER_NAME));
    EXPECT_CALL(*m_currentDriver->mock, init());
    EXPECT_CALL(*m_currentDriver->mock, open(SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC), _));

    //! [THEN] The device is saved and its output spec is sent to the engine
    EXPECT_CALL(*m_configuration, setAudioOutputDeviceId(HEADPHONES));
    EXPECT_CALL(*m_rpcChannel, send(SetOutputSpecRequest(DESIRED_OUTPUT_SPEC), _));

    int outputDeviceChangedCount = 0;
    countNotifications(m_controller->outputDeviceChanged(), outputDeviceChangedCount);

    //! [WHEN] Opening the headphones
    IAudioDriver::Spec activeSpec;
    bool ok = m_controller->open(makeSpec(HEADPHONES), &activeSpec);
    async::processMessages();

    //! [THEN] The headphones are the active device
    EXPECT_TRUE(ok);
    EXPECT_THAT(activeSpec, SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC));
    EXPECT_THAT(m_controller->activeSpec(), SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC));
    EXPECT_TRUE(m_controller->isOpened());
    EXPECT_EQ(m_controller->outputDevice(), HEADPHONES);
    EXPECT_EQ(outputDeviceChangedCount, 1);
}

TEST_F(Audio_AudioDriverControllerTests, Open_FallsBackToDefaultDevice_WhenRequestedDeviceIsUnavailable)
{
    //! [GIVEN] The headphones can't be opened
    ON_CALL(*m_currentDriver->mock, open(SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC), _))
    .WillByDefault(Return(false));

    //! [THEN] The current driver is kept
    EXPECT_CALL(*m_driverFactory, createDriver(CURRENT_DRIVER_NAME));
    EXPECT_CALL(*m_configuration, setCurrentAudioApi(_))
    .Times(0);

    //! [WHEN] Opening the headphones
    IAudioDriver::Spec activeSpec;
    bool ok = m_controller->open(makeSpec(HEADPHONES), &activeSpec);
    async::processMessages();

    //! [THEN] The default device is opened instead
    EXPECT_TRUE(ok);
    EXPECT_THAT(activeSpec, SpecIs(DEFAULT_DEVICE_ID, DESIRED_OUTPUT_SPEC));
    EXPECT_THAT(m_controller->activeSpec(), SpecIs(DEFAULT_DEVICE_ID, DESIRED_OUTPUT_SPEC));
    EXPECT_EQ(m_controller->outputDevice(), DEFAULT_DEVICE_ID);
    EXPECT_TRUE(m_controller->isOpened());
}

TEST_F(Audio_AudioDriverControllerTests, Open_SwitchesToDefaultDriver_WhenCurrentDriverFails)
{
    //! [GIVEN] The default driver and its output spec are configured
    ON_CALL(*m_configuration, defaultAudioApi())
    .WillByDefault(Return(DEFAULT_DRIVER_NAME));
    ON_CALL(*m_configuration, defaultOutputSpec())
    .WillByDefault(Return(DEFAULT_OUTPUT_SPEC));

    //! [GIVEN] The current driver can't open any device
    ON_CALL(*m_currentDriver->mock, open(_, _))
    .WillByDefault(Return(false));

    //! [THEN] The default driver is opened with the default device
    EXPECT_CALL(*m_defaultDriver->mock, init());
    EXPECT_CALL(*m_defaultDriver->mock, open(SpecIs(DEFAULT_DEVICE_ID, DEFAULT_OUTPUT_SPEC), _));

    //! [THEN] The default driver is saved
    EXPECT_CALL(*m_configuration, setCurrentAudioApi(DEFAULT_DRIVER_NAME));

    int currentAudioApiChangedCount = 0;
    countNotifications(m_controller->currentAudioApiChanged(), currentAudioApiChangedCount);

    //! [WHEN] Opening the headphones
    IAudioDriver::Spec activeSpec;
    bool ok = m_controller->open(makeSpec(HEADPHONES), &activeSpec);
    async::processMessages();

    //! [THEN] The default driver is active
    EXPECT_TRUE(ok);
    EXPECT_THAT(activeSpec, SpecIs(DEFAULT_DEVICE_ID, DEFAULT_OUTPUT_SPEC));
    EXPECT_THAT(m_controller->activeSpec(), SpecIs(DEFAULT_DEVICE_ID, DEFAULT_OUTPUT_SPEC));
    EXPECT_TRUE(m_controller->isOpened());
    EXPECT_EQ(m_controller->outputDevice(), DEFAULT_DEVICE_ID);
    EXPECT_EQ(currentAudioApiChangedCount, 1);
}

TEST_F(Audio_AudioDriverControllerTests, Open_Fails_WhenDefaultDriverFails)
{
    //! [GIVEN] The current driver is the default one
    givenCurrentDriverIsDefault();

    //! [GIVEN] The driver can't open any device
    ON_CALL(*m_defaultDriver->mock, open(_, _))
    .WillByDefault(Return(false));

    //! [THEN] Only the default driver is created, no other driver is tried
    EXPECT_CALL(*m_driverFactory, createDriver(DEFAULT_DRIVER_NAME))
    .Times(1);
    EXPECT_CALL(*m_configuration, setCurrentAudioApi(_))
    .Times(0);

    int currentAudioApiChangedCount = 0;
    countNotifications(m_controller->currentAudioApiChanged(), currentAudioApiChangedCount);

    //! [WHEN] Opening the headphones
    bool ok = m_controller->open(makeSpec(HEADPHONES), nullptr);
    async::processMessages();

    //! [THEN] Nothing is opened
    EXPECT_FALSE(ok);
    EXPECT_FALSE(m_controller->isOpened());
    EXPECT_FALSE(m_controller->activeSpec().isValid());
    EXPECT_EQ(currentAudioApiChangedCount, 0);
}

TEST_F(Audio_AudioDriverControllerTests, Close_ClosesDriver)
{
    //! [GIVEN] The headphones are opened
    givenOpened(HEADPHONES);

    //! [THEN] The driver is closed
    EXPECT_CALL(*m_currentDriver->mock, close());

    //! [WHEN] Closing the controller
    m_controller->close();

    //! [THEN] The controller is closed
    EXPECT_FALSE(m_controller->isOpened());
}

TEST_F(Audio_AudioDriverControllerTests, ChangeCurrentAudioApi_OpensDefaultDeviceOnNewDriver)
{
    //! [GIVEN] The default output spec is configured
    ON_CALL(*m_configuration, defaultOutputSpec())
    .WillByDefault(Return(DEFAULT_OUTPUT_SPEC));

    //! [GIVEN] The headphones are opened on the current driver
    givenOpened(HEADPHONES);

    //! [GIVEN] Another driver is available
    const std::string NEW_DRIVER_NAME("DirectSound");
    DriverPtr newDriver = makeDriver(NEW_DRIVER_NAME);

    //! [THEN] The current driver is closed
    EXPECT_CALL(*m_currentDriver->mock, close());

    //! [THEN] The new driver is created and opened with the default device
    EXPECT_CALL(*m_driverFactory, createDriver(NEW_DRIVER_NAME));
    EXPECT_CALL(*newDriver->mock, init());
    EXPECT_CALL(*newDriver->mock, open(SpecIs(DEFAULT_DEVICE_ID, DEFAULT_OUTPUT_SPEC), _));

    //! [THEN] The new driver is saved
    EXPECT_CALL(*m_configuration, setCurrentAudioApi(NEW_DRIVER_NAME));

    int currentAudioApiChangedCount = 0;
    countNotifications(m_controller->currentAudioApiChanged(), currentAudioApiChangedCount);
    int availableOutputDevicesChangedCount = 0;
    countNotifications(m_controller->availableOutputDevicesChanged(), availableOutputDevicesChangedCount);

    //! [WHEN] Switching to the new driver
    m_controller->changeCurrentAudioApi(NEW_DRIVER_NAME);
    async::processMessages();

    //! [THEN] The new driver is active
    EXPECT_FALSE(m_currentDriver->opened);
    EXPECT_TRUE(m_controller->isOpened());
    EXPECT_EQ(m_controller->outputDevice(), DEFAULT_DEVICE_ID);
    EXPECT_EQ(currentAudioApiChangedCount, 1);
    EXPECT_EQ(availableOutputDevicesChangedCount, 1);
}

TEST_F(Audio_AudioDriverControllerTests, ChangeCurrentAudioApi_DoesNotOpen_WhenNewDriverHasNoDevices)
{
    //! [GIVEN] The headphones are opened
    givenOpened(HEADPHONES);

    //! [GIVEN] Another driver is available, but it has no devices
    const std::string NEW_DRIVER_NAME("DirectSound");
    DriverPtr newDriver = makeDriver(NEW_DRIVER_NAME);
    ON_CALL(*newDriver->mock, defaultDevice())
    .WillByDefault(Return(AudioDeviceID()));

    //! [THEN] The new driver isn't opened
    EXPECT_CALL(*newDriver->mock, open(_, _))
    .Times(0);

    //! [THEN] The new driver is still saved
    EXPECT_CALL(*m_configuration, setCurrentAudioApi(NEW_DRIVER_NAME));

    int currentAudioApiChangedCount = 0;
    countNotifications(m_controller->currentAudioApiChanged(), currentAudioApiChangedCount);

    //! [WHEN] Switching to the new driver
    m_controller->changeCurrentAudioApi(NEW_DRIVER_NAME);
    async::processMessages();

    //! [THEN] Nothing is opened
    EXPECT_FALSE(m_controller->isOpened());
    EXPECT_EQ(currentAudioApiChangedCount, 1);
}

TEST_F(Audio_AudioDriverControllerTests, SelectOutputDevice_ReopensDriverWithNewDevice)
{
    //! [GIVEN] The desired output spec is configured
    ON_CALL(*m_configuration, desiredOutputSpec())
    .WillByDefault(Return(DESIRED_OUTPUT_SPEC));

    //! [GIVEN] The headphones are opened
    givenOpened(HEADPHONES);

    //! [THEN] The driver is reopened with the speakers
    EXPECT_CALL(*m_currentDriver->mock, close());
    EXPECT_CALL(*m_currentDriver->mock, open(SpecIs(SPEAKERS, DESIRED_OUTPUT_SPEC), _));

    //! [THEN] The speakers are saved
    EXPECT_CALL(*m_configuration, setAudioOutputDeviceId(SPEAKERS));

    int outputDeviceChangedCount = 0;
    countNotifications(m_controller->outputDeviceChanged(), outputDeviceChangedCount);

    //! [WHEN] Selecting the speakers
    bool ok = m_controller->selectOutputDevice(SPEAKERS);
    async::processMessages();

    //! [THEN] The speakers are the active device
    EXPECT_TRUE(ok);
    EXPECT_TRUE(m_controller->isOpened());
    EXPECT_EQ(m_controller->outputDevice(), SPEAKERS);
    EXPECT_EQ(outputDeviceChangedCount, 1);
}

TEST_F(Audio_AudioDriverControllerTests, SelectOutputDevice_RestoresPreviousDevice_WhenNewDeviceFails)
{
    //! [GIVEN] The desired output spec is configured
    ON_CALL(*m_configuration, desiredOutputSpec())
    .WillByDefault(Return(DESIRED_OUTPUT_SPEC));

    //! [GIVEN] The headphones are opened
    givenOpened(HEADPHONES);

    //! [GIVEN] The speakers can't be opened
    ON_CALL(*m_currentDriver->mock, open(SpecIs(SPEAKERS, DESIRED_OUTPUT_SPEC), _))
    .WillByDefault(Return(false));

    //! [THEN] The speakers are tried, then the headphones are reopened
    EXPECT_CALL(*m_currentDriver->mock, open(SpecIs(SPEAKERS, DESIRED_OUTPUT_SPEC), _));
    EXPECT_CALL(*m_currentDriver->mock, open(SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC), _));

    //! [THEN] The headphones are saved
    EXPECT_CALL(*m_configuration, setAudioOutputDeviceId(HEADPHONES));

    //! [WHEN] Selecting the speakers
    bool ok = m_controller->selectOutputDevice(SPEAKERS);
    async::processMessages();

    //! [THEN] The headphones are still the active device
    EXPECT_FALSE(ok);
    EXPECT_TRUE(m_controller->isOpened());
    EXPECT_EQ(m_controller->outputDevice(), HEADPHONES);
}

TEST_F(Audio_AudioDriverControllerTests, SelectOutputDevice_OpensDriver_WhenClosedAfterFailure)
{
    //! [GIVEN] The desired output spec is configured
    ON_CALL(*m_configuration, desiredOutputSpec())
    .WillByDefault(Return(DESIRED_OUTPUT_SPEC));

    //! [GIVEN] The headphones were opened, then the driver closed after a failure
    givenOpened(HEADPHONES);
    givenClosedWithBlankSpec(*m_currentDriver);

    //! [THEN] The driver is opened with the speakers
    EXPECT_CALL(*m_currentDriver->mock, close());
    EXPECT_CALL(*m_currentDriver->mock, open(SpecIs(SPEAKERS, DESIRED_OUTPUT_SPEC), _));

    //! [THEN] The speakers are saved
    EXPECT_CALL(*m_configuration, setAudioOutputDeviceId(SPEAKERS));

    int outputDeviceChangedCount = 0;
    countNotifications(m_controller->outputDeviceChanged(), outputDeviceChangedCount);

    //! [WHEN] Selecting the speakers
    bool ok = m_controller->selectOutputDevice(SPEAKERS);
    async::processMessages();

    //! [THEN] The speakers are the active device
    EXPECT_TRUE(ok);
    EXPECT_TRUE(m_controller->isOpened());
    EXPECT_THAT(m_controller->activeSpec(), SpecIs(SPEAKERS, DESIRED_OUTPUT_SPEC));
    EXPECT_EQ(outputDeviceChangedCount, 1);
}

TEST_F(Audio_AudioDriverControllerTests, SelectOutputDevice_RestoresLastValidSpec_WhenClosedAndNewDeviceFails)
{
    //! [GIVEN] The desired output spec is configured
    ON_CALL(*m_configuration, desiredOutputSpec())
    .WillByDefault(Return(DESIRED_OUTPUT_SPEC));

    //! [GIVEN] The headphones were opened, then the driver closed after a failure
    givenOpened(HEADPHONES);
    givenClosedWithBlankSpec(*m_currentDriver);

    //! [GIVEN] The speakers can't be opened
    ON_CALL(*m_currentDriver->mock, open(SpecIs(SPEAKERS, DESIRED_OUTPUT_SPEC), _))
    .WillByDefault(Return(false));

    //! [THEN] The speakers are tried, then the headphones are reopened with the last valid spec
    EXPECT_CALL(*m_currentDriver->mock, open(SpecIs(SPEAKERS, DESIRED_OUTPUT_SPEC), _));
    EXPECT_CALL(*m_currentDriver->mock, open(SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC), _));

    //! [THEN] The headphones are saved
    EXPECT_CALL(*m_configuration, setAudioOutputDeviceId(HEADPHONES));

    //! [WHEN] Selecting the speakers
    bool ok = m_controller->selectOutputDevice(SPEAKERS);
    async::processMessages();

    //! [THEN] The headphones are the active device
    EXPECT_FALSE(ok);
    EXPECT_TRUE(m_controller->isOpened());
    EXPECT_THAT(m_controller->activeSpec(), SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC));
}

TEST_F(Audio_AudioDriverControllerTests, SelectOutputDevice_OnlySavesDevice_WhenAudioNeverStarted)
{
    //! [GIVEN] The current driver is the default one, and it can't open any device
    givenCurrentDriverIsDefault();
    ON_CALL(*m_defaultDriver->mock, open(_, _))
    .WillByDefault(Return(false));

    //! [GIVEN] Opening failed, so audio has never started
    ASSERT_FALSE(m_controller->open(makeSpec(HEADPHONES), nullptr));
    async::processMessages();

    //! [THEN] The driver isn't opened
    EXPECT_CALL(*m_defaultDriver->mock, open(_, _))
    .Times(0);

    //! [THEN] The speakers are saved
    EXPECT_CALL(*m_configuration, setAudioOutputDeviceId(SPEAKERS));

    //! [WHEN] Selecting the speakers
    bool ok = m_controller->selectOutputDevice(SPEAKERS);
    async::processMessages();

    //! [THEN] The selection succeeds, and the driver stays closed
    EXPECT_TRUE(ok);
    EXPECT_FALSE(m_controller->isOpened());
    EXPECT_FALSE(m_controller->activeSpec().isValid());
}

TEST_F(Audio_AudioDriverControllerTests, DeviceListChange_ReopensActiveDevice)
{
    //! [GIVEN] The headphones are opened
    givenOpened(HEADPHONES);

    //! [THEN] The headphones are reopened
    EXPECT_CALL(*m_currentDriver->mock, close());
    EXPECT_CALL(*m_currentDriver->mock, open(SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC), _));

    int availableOutputDevicesChangedCount = 0;
    countNotifications(m_controller->availableOutputDevicesChanged(), availableOutputDevicesChangedCount);

    //! [WHEN] The device list changes
    notifyDevicesChanged(*m_currentDriver);

    //! [THEN] The headphones are still the active device, and the change is forwarded
    EXPECT_TRUE(m_controller->isOpened());
    EXPECT_THAT(m_controller->activeSpec(), SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC));
    EXPECT_EQ(m_controller->outputDevice(), HEADPHONES);
    EXPECT_EQ(availableOutputDevicesChangedCount, 1);
}

TEST_F(Audio_AudioDriverControllerTests, DeviceListChange_FallsBackToDefaultDevice_WhenActiveDeviceIsGone)
{
    //! [GIVEN] The headphones are opened
    givenOpened(HEADPHONES);

    //! [GIVEN] The headphones are disconnected
    ON_CALL(*m_currentDriver->mock, open(SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC), _))
    .WillByDefault(Return(false));

    //! [THEN] The headphones are tried, then the default device is opened
    EXPECT_CALL(*m_currentDriver->mock, open(SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC), _));
    EXPECT_CALL(*m_currentDriver->mock, open(SpecIs(DEFAULT_DEVICE_ID, DESIRED_OUTPUT_SPEC), _));

    //! [THEN] The current driver is kept
    EXPECT_CALL(*m_configuration, setCurrentAudioApi(_))
    .Times(0);

    //! [WHEN] The device list changes
    notifyDevicesChanged(*m_currentDriver);

    //! [THEN] The default device is active
    EXPECT_TRUE(m_controller->isOpened());
    EXPECT_THAT(m_controller->activeSpec(), SpecIs(DEFAULT_DEVICE_ID, DESIRED_OUTPUT_SPEC));
    EXPECT_EQ(m_controller->outputDevice(), DEFAULT_DEVICE_ID);
}

TEST_F(Audio_AudioDriverControllerTests, DeviceListChange_SwitchesToDefaultDriver_WhenCurrentDriverFails)
{
    //! [GIVEN] The default driver and its output spec are configured
    ON_CALL(*m_configuration, defaultAudioApi())
    .WillByDefault(Return(DEFAULT_DRIVER_NAME));
    ON_CALL(*m_configuration, defaultOutputSpec())
    .WillByDefault(Return(DEFAULT_OUTPUT_SPEC));

    //! [GIVEN] The headphones are opened
    givenOpened(HEADPHONES);

    //! [GIVEN] The current driver can't open any device anymore
    ON_CALL(*m_currentDriver->mock, open(_, _))
    .WillByDefault(Return(false));

    //! [THEN] The default driver is opened with the default device
    EXPECT_CALL(*m_defaultDriver->mock, init());
    EXPECT_CALL(*m_defaultDriver->mock, open(SpecIs(DEFAULT_DEVICE_ID, DEFAULT_OUTPUT_SPEC), _));

    //! [THEN] The default driver is saved
    EXPECT_CALL(*m_configuration, setCurrentAudioApi(DEFAULT_DRIVER_NAME));

    int currentAudioApiChangedCount = 0;
    countNotifications(m_controller->currentAudioApiChanged(), currentAudioApiChangedCount);

    //! [WHEN] The device list changes
    notifyDevicesChanged(*m_currentDriver);

    //! [THEN] The default driver is active
    EXPECT_TRUE(m_controller->isOpened());
    EXPECT_THAT(m_controller->activeSpec(), SpecIs(DEFAULT_DEVICE_ID, DEFAULT_OUTPUT_SPEC));
    EXPECT_EQ(m_controller->outputDevice(), DEFAULT_DEVICE_ID);
    EXPECT_EQ(currentAudioApiChangedCount, 1);
}

TEST_F(Audio_AudioDriverControllerTests, DeviceListChange_DoesNotReopen_WhenDriverWasClosed)
{
    //! [GIVEN] The headphones were opened, then the controller was closed
    givenOpened(HEADPHONES);
    m_controller->close();

    //! [THEN] Nothing is reopened
    EXPECT_CALL(*m_currentDriver->mock, open(_, _))
    .Times(0);

    int availableOutputDevicesChangedCount = 0;
    countNotifications(m_controller->availableOutputDevicesChanged(), availableOutputDevicesChangedCount);

    //! [WHEN] The device list changes
    notifyDevicesChanged(*m_currentDriver);

    //! [THEN] The controller stays closed with the same spec, but the change is forwarded
    EXPECT_FALSE(m_controller->isOpened());
    EXPECT_THAT(m_controller->activeSpec(), SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC));
    EXPECT_EQ(availableOutputDevicesChangedCount, 1);
}

TEST_F(Audio_AudioDriverControllerTests, DeviceListChange_RetriesWithLastValidSpec_AfterFailedReopen)
{
    //! [GIVEN] The headphones are opened on the default driver
    givenCurrentDriverIsDefault();
    givenOpened(HEADPHONES);

    //! [GIVEN] The driver can't open any device, and blanks its spec after a failed open
    EXPECT_CALL(*m_defaultDriver->mock, open(_, _))
    .Times(2)
    .WillRepeatedly(DoAll(Assign(&m_defaultDriver->activeSpec, IAudioDriver::Spec()), Return(false)))
    .RetiresOnSaturation();

    //! [WHEN] The device list changes
    notifyDevicesChanged(*m_defaultDriver);

    //! [THEN] The driver is closed with a blank spec
    EXPECT_FALSE(m_controller->isOpened());
    EXPECT_FALSE(m_controller->activeSpec().isValid());

    //! [THEN] The headphones are reopened with the last valid spec
    EXPECT_CALL(*m_defaultDriver->mock, open(SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC), _));

    //! [WHEN] The device list changes again
    notifyDevicesChanged(*m_defaultDriver);

    //! [THEN] The headphones are the active device
    EXPECT_TRUE(m_controller->isOpened());
    EXPECT_THAT(m_controller->activeSpec(), SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC));
    EXPECT_EQ(m_controller->outputDevice(), HEADPHONES);
}

TEST_F(Audio_AudioDriverControllerTests, ChangeBufferSize_ReopensDriverWithNewBufferSize)
{
    //! [GIVEN] The headphones are opened
    givenOpened(HEADPHONES);

    constexpr samples_t BUFFER_SIZE = 256;
    OutputSpec expectedOutput = DESIRED_OUTPUT_SPEC;
    expectedOutput.samplesPerChannel = BUFFER_SIZE;

    //! [THEN] The driver is reopened with the new buffer size
    EXPECT_CALL(*m_currentDriver->mock, close());
    EXPECT_CALL(*m_currentDriver->mock, open(SpecIs(HEADPHONES, expectedOutput), _));

    //! [THEN] The buffer size is saved and sent to the engine
    EXPECT_CALL(*m_configuration, setDriverBufferSize(BUFFER_SIZE));
    EXPECT_CALL(*m_rpcChannel, send(SetOutputSpecRequest(expectedOutput), _))
    .Times(AtLeast(1));

    //! [WHEN] Changing the buffer size
    m_controller->changeBufferSize(BUFFER_SIZE);
    async::processMessages();

    //! [THEN] The driver is still opened
    EXPECT_TRUE(m_controller->isOpened());
}

TEST_F(Audio_AudioDriverControllerTests, ChangeBufferSize_RestoresPreviousSpec_WhenReopenFails)
{
    //! [GIVEN] The headphones are opened
    givenOpened(HEADPHONES);

    constexpr samples_t BUFFER_SIZE = 256;
    OutputSpec expectedOutput = DESIRED_OUTPUT_SPEC;
    expectedOutput.samplesPerChannel = BUFFER_SIZE;

    //! [GIVEN] The headphones can't be reopened with the new buffer size
    ON_CALL(*m_currentDriver->mock, open(SpecIs(HEADPHONES, expectedOutput), _))
    .WillByDefault(Return(false));

    //! [THEN] The new buffer size is tried, then the previous spec is restored
    EXPECT_CALL(*m_currentDriver->mock, open(SpecIs(HEADPHONES, expectedOutput), _));
    EXPECT_CALL(*m_currentDriver->mock, open(SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC), _));

    //! [THEN] The new buffer size isn't saved or sent to the engine
    EXPECT_CALL(*m_configuration, setDriverBufferSize(_))
    .Times(0);
    EXPECT_CALL(*m_rpcChannel, send(SetOutputSpecRequest(expectedOutput), _))
    .Times(0);

    //! [THEN] The previous spec is sent to the engine again
    EXPECT_CALL(*m_rpcChannel, send(SetOutputSpecRequest(DESIRED_OUTPUT_SPEC), _));

    //! [WHEN] Changing the buffer size
    m_controller->changeBufferSize(BUFFER_SIZE);
    async::processMessages();

    //! [THEN] The headphones are still opened with the previous spec
    EXPECT_TRUE(m_controller->isOpened());
    EXPECT_THAT(m_controller->activeSpec(), SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC));
}

TEST_F(Audio_AudioDriverControllerTests, ChangeBufferSize_OnlySaves_WhenDriverIsClosed)
{
    //! [GIVEN] The headphones were opened, then the controller was closed
    givenOpened(HEADPHONES);
    m_controller->close();

    constexpr samples_t BUFFER_SIZE = 256;

    //! [THEN] The buffer size is saved without reopening the driver
    EXPECT_CALL(*m_currentDriver->mock, open(_, _))
    .Times(0);
    EXPECT_CALL(*m_configuration, setDriverBufferSize(BUFFER_SIZE));

    //! [WHEN] Changing the buffer size
    m_controller->changeBufferSize(BUFFER_SIZE);
    async::processMessages();

    //! [THEN] The controller stays closed
    EXPECT_FALSE(m_controller->isOpened());
}

TEST_F(Audio_AudioDriverControllerTests, ChangeSampleRate_ReopensDriverWithNewSampleRate)
{
    //! [GIVEN] The headphones are opened
    givenOpened(HEADPHONES);

    constexpr sample_rate_t SAMPLE_RATE = 96000;
    OutputSpec expectedOutput = DESIRED_OUTPUT_SPEC;
    expectedOutput.sampleRate = SAMPLE_RATE;

    //! [THEN] The driver is reopened with the new sample rate
    EXPECT_CALL(*m_currentDriver->mock, close());
    EXPECT_CALL(*m_currentDriver->mock, open(SpecIs(HEADPHONES, expectedOutput), _));

    //! [THEN] The sample rate is saved and sent to the engine
    EXPECT_CALL(*m_configuration, setSampleRate(SAMPLE_RATE));
    EXPECT_CALL(*m_rpcChannel, send(SetOutputSpecRequest(expectedOutput), _))
    .Times(AtLeast(1));

    //! [WHEN] Changing the sample rate
    m_controller->changeSampleRate(SAMPLE_RATE);
    async::processMessages();

    //! [THEN] The driver is still opened
    EXPECT_TRUE(m_controller->isOpened());
}

TEST_F(Audio_AudioDriverControllerTests, ChangeSampleRate_RestoresPreviousSpec_WhenReopenFails)
{
    //! [GIVEN] The headphones are opened
    givenOpened(HEADPHONES);

    constexpr sample_rate_t SAMPLE_RATE = 96000;
    OutputSpec expectedOutput = DESIRED_OUTPUT_SPEC;
    expectedOutput.sampleRate = SAMPLE_RATE;

    //! [GIVEN] The headphones can't be reopened with the new sample rate
    ON_CALL(*m_currentDriver->mock, open(SpecIs(HEADPHONES, expectedOutput), _))
    .WillByDefault(Return(false));

    //! [THEN] The new sample rate is tried, then the previous spec is restored
    EXPECT_CALL(*m_currentDriver->mock, open(SpecIs(HEADPHONES, expectedOutput), _));
    EXPECT_CALL(*m_currentDriver->mock, open(SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC), _));

    //! [THEN] The new sample rate isn't saved or sent to the engine
    EXPECT_CALL(*m_configuration, setSampleRate(_))
    .Times(0);
    EXPECT_CALL(*m_rpcChannel, send(SetOutputSpecRequest(expectedOutput), _))
    .Times(0);

    //! [THEN] The previous spec is sent to the engine again
    EXPECT_CALL(*m_rpcChannel, send(SetOutputSpecRequest(DESIRED_OUTPUT_SPEC), _));

    //! [WHEN] Changing the sample rate
    m_controller->changeSampleRate(SAMPLE_RATE);
    async::processMessages();

    //! [THEN] The headphones are still opened with the previous spec
    EXPECT_TRUE(m_controller->isOpened());
    EXPECT_THAT(m_controller->activeSpec(), SpecIs(HEADPHONES, DESIRED_OUTPUT_SPEC));
}

TEST_F(Audio_AudioDriverControllerTests, ChangeSampleRate_OnlySaves_WhenDriverIsClosed)
{
    //! [GIVEN] The headphones were opened, then the controller was closed
    givenOpened(HEADPHONES);
    m_controller->close();

    constexpr sample_rate_t SAMPLE_RATE = 96000;

    //! [THEN] The sample rate is saved without reopening the driver
    EXPECT_CALL(*m_currentDriver->mock, open(_, _))
    .Times(0);
    EXPECT_CALL(*m_configuration, setSampleRate(SAMPLE_RATE));

    //! [WHEN] Changing the sample rate
    m_controller->changeSampleRate(SAMPLE_RATE);
    async::processMessages();

    //! [THEN] The controller stays closed
    EXPECT_FALSE(m_controller->isOpened());
}
}
