/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "global/async/notification.h"

#include "vsttypes.h"
#include "audio/audiotypes.h"

namespace muse::vst {
class IVstPluginInstance
{
public:

    virtual ~IVstPluginInstance() = default;

    virtual const muse::audio::AudioResourceId& resourceId() const = 0;
    virtual const std::string& name() const = 0;
    virtual VstPluginInstanceId id() const = 0;

    virtual bool isLoaded() const = 0;
    virtual async::Notification loadingCompleted() const = 0;

    virtual PluginViewPtr createView() const = 0;
    virtual PluginControllerPtr controller() const = 0;
    virtual PluginComponentPtr component() const = 0;
    virtual PluginMidiMappingPtr midiMapping() const = 0;

    virtual void updatePluginConfig(const muse::audio::AudioUnitConfig& config) = 0;
    virtual void refreshConfig() = 0;
    virtual async::Channel<muse::audio::AudioUnitConfig> pluginSettingsChanged() const = 0;
};
}
