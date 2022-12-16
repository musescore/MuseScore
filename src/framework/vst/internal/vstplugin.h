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

#ifndef MU_VST_VSTPLUGIN_H
#define MU_VST_VSTPLUGIN_H

#include <mutex>
#include <atomic>

#include "modularity/ioc.h"
#include "io/path.h"
#include "async/asyncable.h"
#include "async/notification.h"
#include "async/channel.h"
#include "audio/iaudiothreadsecurer.h"
#include "audio/audiotypes.h"

#include "ivstmodulesrepository.h"
#include "vsttypes.h"
#include "vstcomponenthandler.h"
#include "vsterrors.h"

namespace mu::vst {
class VstPlugin : public async::Asyncable
{
    INJECT_STATIC(vst, audio::IAudioThreadSecurer, threadSecurer)
    INJECT_STATIC(vst, IVstModulesRepository, modulesRepo)

public:
    VstPlugin(const audio::AudioResourceId& resourceId);

    const audio::AudioResourceId& resourceId() const;
    const std::string& name() const;

    PluginViewPtr createView() const;
    PluginProviderPtr provider() const;
    bool isAbleForInput() const;

    void updatePluginConfig(const audio::AudioUnitConfig& config);
    void refreshConfig();

    void load();
    void unload();

    bool isValid() const;
    bool isLoaded() const;

    async::Notification loadingCompleted() const;
    async::Notification unloadingCompleted() const;

    async::Channel<audio::AudioUnitConfig> pluginSettingsChanged() const;

private:
    void rescanParams();
    void stateBufferFromString(VstMemoryStream& buffer, char* strData, const size_t strSize) const;

    audio::AudioResourceId m_resourceId;

    PluginModulePtr m_module = nullptr;
    PluginProviderPtr m_pluginProvider = nullptr;
    ClassInfo m_classInfo;

    Steinberg::FUnknownPtr<VstComponentHandler> m_componentHandlerPtr = nullptr;

    VstMemoryStream m_componentStateBuffer;
    VstMemoryStream m_controllerStateBuffer;
    mutable async::Channel<audio::AudioUnitConfig> m_pluginSettingsChanges;

    std::atomic_bool m_isLoaded = false;
    async::Notification m_loadingCompleted;
    async::Notification m_unloadingCompleted;

    mutable std::mutex m_mutex;
};
}

#endif // MU_VST_VSTPLUGIN_H
