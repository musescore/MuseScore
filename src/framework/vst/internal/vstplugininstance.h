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

#include <mutex>
#include <atomic>

#include "../ivstplugininstance.h"

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "async/notification.h"
#include "async/channel.h"
#include "audio/iaudiothreadsecurer.h"
#include "audio/audiotypes.h"

#include "../ivstmodulesrepository.h"
#include "../vsttypes.h"
#include "vstcomponenthandler.h"

namespace muse::vst {
class VstPluginProvider;
class VstPluginInstance : public IVstPluginInstance, public async::Asyncable
{
    muse::GlobalInject<muse::audio::IAudioThreadSecurer> threadSecurer;
    muse::GlobalInject<IVstModulesRepository> modulesRepo;

public:
    VstPluginInstance(const muse::audio::AudioResourceId& resourceId);
    ~VstPluginInstance() override;

    const muse::audio::AudioResourceId& resourceId() const override;
    const std::string& name() const override;
    VstPluginInstanceId id() const override;

    PluginViewPtr createView() const override;

    PluginControllerPtr controller() const override;
    PluginComponentPtr component() const override;
    PluginMidiMappingPtr midiMapping() const override;

    bool isAbleForInput() const;

    void updatePluginConfig(const muse::audio::AudioUnitConfig& config) override;
    void refreshConfig() override;

    void load();

    bool isValid() const;
    bool isLoaded() const override;

    async::Notification loadingCompleted() const override;

    async::Channel<muse::audio::AudioUnitConfig> pluginSettingsChanged() const override;

private:
    void rescanParams();
    void stateBufferFromString(VstMemoryStream& buffer, char* strData, const size_t strSize) const;

    VstPluginInstanceId m_id = 0;
    muse::audio::AudioResourceId m_resourceId;

    PluginModulePtr m_module = nullptr;
    std::unique_ptr<VstPluginProvider> m_pluginProvider;
    ClassInfo m_classInfo;

    Steinberg::FUnknownPtr<VstComponentHandler> m_componentHandlerPtr = nullptr;

    VstMemoryStream m_componentStateBuffer;
    VstMemoryStream m_controllerStateBuffer;
    mutable async::Channel<muse::audio::AudioUnitConfig> m_pluginSettingsChanges;

    std::atomic_bool m_isLoaded = false;
    async::Notification m_loadingCompleted;

    mutable std::mutex m_mutex;
};
}
