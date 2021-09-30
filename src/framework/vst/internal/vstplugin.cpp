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

#include "vstplugin.h"

#include "log.h"
#include "async/async.h"

using namespace mu;
using namespace mu::vst;
using namespace mu::async;

static const std::string_view COMPONENT_STATE_KEY = "componentState";
static const std::string_view CONTROLLER_STATE_KEY = "controllerState";

VstPlugin::VstPlugin(PluginModulePtr module)
    : m_module(std::move(module))
{
    ONLY_AUDIO_THREAD(threadSecurer);

    m_componentHandler.pluginParamsChanged().onNotify(this, [this]() {
        rescanParams();
    });
}

const std::string& VstPlugin::name() const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    return m_module->getName();
}

/**
 * @brief VstPlugin::load
 * @note Some Vst plugins might not support loading in the background threads.
 *       So we have to ensure that this function being executed in the main thread
 */
void VstPlugin::load()
{
    Async::call(this, [this]() {
        ONLY_MAIN_THREAD(threadSecurer);

        std::lock_guard lock(m_mutex);

        const auto& factory = m_module->getFactory();

        for (const ClassInfo& classInfo : factory.classInfos()) {
            if (classInfo.category() != kVstAudioEffectClass) {
                LOGI() << "Non-audio plugins are not supported";
                continue;
            }

            m_pluginProvider = owned(new PluginProvider(factory, classInfo));
            break;
        }

        if (!m_pluginProvider) {
            LOGE() << "Unable to load vst plugin provider";
            return;
        }

        auto controller = m_pluginProvider->getController();

        if (!controller) {
            return;
        }

        controller->setComponentHandler(&m_componentHandler);

        m_isLoaded = true;
        m_loadingCompleted.notify();
    }, threadSecurer()->mainThreadId());
}

void VstPlugin::rescanParams()
{
    ONLY_AUDIO_THREAD(threadSecurer);

    auto component = m_pluginProvider->getComponent();
    auto controller = m_pluginProvider->getController();

    if (!controller || !component) {
        return;
    }

    m_componentStateBuffer.seek(0, Steinberg::IBStream::kIBSeekSet, nullptr);
    m_controllerStateBuffer.seek(0, Steinberg::IBStream::kIBSeekSet, nullptr);

    audio::AudioUnitConfig updatedConfig;

    component->getState(&m_componentStateBuffer);
    updatedConfig.emplace(COMPONENT_STATE_KEY, std::string(m_componentStateBuffer.getData(), m_componentStateBuffer.getSize()));

    controller->getState(&m_controllerStateBuffer);
    updatedConfig.emplace(CONTROLLER_STATE_KEY, std::string(m_controllerStateBuffer.getData(), m_controllerStateBuffer.getSize()));

    for (int32_t i = 0; i < controller->getParameterCount(); ++i) {
        PluginParamInfo info;
        controller->getParameterInfo(i, info);

        updatedConfig.insert_or_assign(std::to_string(info.id), std::to_string(controller->getParamNormalized(info.id)));
    }

    m_pluginSettingsChanges.send(std::move(updatedConfig));
}

void VstPlugin::stateBufferFromString(VstMemoryStream& buffer, char* strData, const size_t strSize) const
{
    if (strSize == 0) {
        return;
    }

    static Steinberg::int32 numBytesRead = 0;

    buffer.write(strData, strSize, &numBytesRead);
    buffer.seek(0, Steinberg::IBStream::kIBSeekSet, nullptr);
}

PluginViewPtr VstPlugin::view() const
{
    ONLY_MAIN_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    if (m_pluginView) {
        return m_pluginView;
    }

    auto controller = m_pluginProvider->getController();

    if (!controller) {
        return nullptr;
    }

    m_pluginView = owned(controller->createView(PluginEditorViewType::kEditor));

    return m_pluginView;
}

PluginProviderPtr VstPlugin::provider() const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    return m_pluginProvider;
}

void VstPlugin::updatePluginConfig(const audio::AudioUnitConfig& config)
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    auto controller = m_pluginProvider->getController();
    auto component = m_pluginProvider->getComponent();

    if (!controller || !component) {
        LOGE() << "Unable to update settings for VST plugin";
        return;
    }

    for (auto& pair : config) {
        if (pair.first == COMPONENT_STATE_KEY) {
            stateBufferFromString(m_componentStateBuffer, const_cast<char*>(pair.second.data()), pair.second.size());
            component->setState(&m_componentStateBuffer);
            controller->setComponentState(&m_componentStateBuffer);
            continue;
        }

        if (pair.first == CONTROLLER_STATE_KEY) {
            stateBufferFromString(m_controllerStateBuffer, const_cast<char*>(pair.second.data()), pair.second.size());
            controller->setState(&m_controllerStateBuffer);

            continue;
        }

        PluginParamId id = std::stoi(pair.first);
        PluginParamValue val = std::stod(pair.second);

        controller->setParamNormalized(id, val);
    }
}

bool VstPlugin::isValid() const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    if (!m_module
        || !m_pluginProvider) {
        return false;
    }

    return true;
}

bool VstPlugin::isLoaded() const
{
    ONLY_AUDIO_OR_MAIN_THREAD(threadSecurer);

    return m_isLoaded;
}

Notification VstPlugin::loadingCompleted() const
{
    return m_loadingCompleted;
}

async::Channel<audio::AudioUnitConfig> VstPlugin::pluginSettingsChanged() const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    return m_pluginSettingsChanges;
}
