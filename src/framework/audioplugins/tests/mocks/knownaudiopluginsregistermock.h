/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "audioplugins/iknownaudiopluginsregister.h"

namespace muse::audioplugins {
class KnownAudioPluginsRegisterMock : public IKnownAudioPluginsRegister
{
public:
    MOCK_METHOD(Ret, load, (), (override));

    MOCK_METHOD(std::vector<AudioPluginInfo>, pluginInfoList, (PluginInfoAccepted), (const, override));
    MOCK_METHOD(const io::path_t&, pluginPath, (const audio::AudioResourceId&), (const, override));

    MOCK_METHOD(bool, exists, (const io::path_t&), (const, override));
    MOCK_METHOD(bool, exists, (const audio::AudioResourceId&), (const, override));

    MOCK_METHOD(Ret, registerPlugin, (const AudioPluginInfo&), (override));
    MOCK_METHOD(Ret, unregisterPlugin, (const audio::AudioResourceId&), (override));
};
}
