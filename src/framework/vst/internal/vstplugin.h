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

#ifndef MUSE_VST_VSTPLUGIN_H
#define MUSE_VST_VSTPLUGIN_H

#include <mutex>
#include <atomic>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "async/notification.h"
#include "async/channel.h"
#include "audio/iaudiothreadsecurer.h"
#include "audio/audiotypes.h"

#include "ivstmodulesrepository.h"
#include "vsttypes.h"
#include "vstcomponenthandler.h"

namespace muse::vst {
class VstPluginProvider;
class VstPlugin : public async::Asyncable
{
    INJECT_STATIC(muse::audio::IAudioThreadSecurer, threadSecurer)
    INJECT_STATIC(IVstModulesRepository, modulesRepo)

public:
    VstPlugin(const muse::audio::AudioResourceId& resourceId);
    ~VstPlugin() override;

    const muse::audio::AudioResourceId& resourceId() const;
    const std::string& name() const;

    PluginViewPtr createView() const;

    PluginControllerPtr controller() const;
    PluginComponentPtr component() const;
    PluginMidiMappingPtr midiMapping() const;

    bool isAbleForInput() const;

    void updatePluginConfig(const muse::audio::AudioUnitConfig& config);
    void refreshConfig();

    void load();

    bool isValid() const;
    bool isLoaded() const;

    async::Notification loadingCompleted() const;

    async::Channel<muse::audio::AudioUnitConfig> pluginSettingsChanged() const;

private:
    void rescanParams();
    void stateBufferFromString(VstMemoryStream& buffer, char* strData, const size_t strSize) const;

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

#endif // MUSE_VST_VSTPLUGIN_H
