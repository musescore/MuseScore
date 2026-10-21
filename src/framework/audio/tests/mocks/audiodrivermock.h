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
#pragma once

#include <gmock/gmock.h>

#include "audio/iaudiodriver.h"

namespace muse::audio {
class AudioDriverMock : public IAudioDriver
{
public:
    MOCK_METHOD(void, init, (), (override));

    MOCK_METHOD(std::string, name, (), (const, override));

    MOCK_METHOD(AudioDeviceID, defaultDevice, (), (const, override));

    MOCK_METHOD(bool, open, (const Spec&, Spec*), (override));
    MOCK_METHOD(void, close, (), (override));
    MOCK_METHOD(bool, isOpened, (), (const, override));

    MOCK_METHOD(const Spec&, activeSpec, (), (const, override));
    MOCK_METHOD(async::Channel<Spec>, activeSpecChanged, (), (const, override));

    MOCK_METHOD(std::vector<samples_t>, availableOutputDeviceBufferSizes, (), (const, override));
    MOCK_METHOD(std::vector<sample_rate_t>, availableOutputDeviceSampleRates, (), (const, override));
    MOCK_METHOD(AudioDeviceList, availableOutputDevices, (), (const, override));
    MOCK_METHOD(async::Notification, availableOutputDevicesChanged, (), (const, override));
};
}
