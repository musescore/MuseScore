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

#include "modularity/imoduleinterface.h"

#include "global/types/ret.h"
#include "global/io/path.h"

#include "audiopluginstypes.h"

namespace muse::audioplugins {
class IKnownAudioPluginsRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IKnownAudioPluginsRegister)

public:
    virtual ~IKnownAudioPluginsRegister() = default;

    virtual Ret load() = 0;

    using PluginInfoAccepted = std::function<bool (const AudioPluginInfo& info)>;

    virtual std::vector<AudioPluginInfo> pluginInfoList(PluginInfoAccepted accepted = PluginInfoAccepted()) const = 0;
    virtual const io::path_t& pluginPath(const audio::AudioResourceId& resourceId) const = 0;

    virtual bool exists(const io::path_t& pluginPath) const = 0;
    virtual bool exists(const audio::AudioResourceId& resourceId) const = 0;

    virtual Ret registerPlugin(const AudioPluginInfo& info) = 0;
    virtual Ret unregisterPlugin(const audio::AudioResourceId& resourceId) = 0;
};
}
