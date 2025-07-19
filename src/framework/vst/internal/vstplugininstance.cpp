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

#include "vstplugininstance.h"

#include "vstpluginprovider.h"

#include "log.h"
#include "async/async.h"

using namespace muse;
using namespace muse::vst;
using namespace muse::async;

static const std::string_view COMPONENT_STATE_KEY = "componentState";
static const std::string_view CONTROLLER_STATE_KEY = "controllerState";

static VstPluginInstanceId s_lastId = 0;

VstPluginInstance::VstPluginInstance(const muse::audio::AudioResourceId& resourceId)
    : m_resourceId(resourceId), m_componentHandlerPtr(new VstComponentHandler())
{
    ONLY_AUDIO_THREAD(threadSecurer);

    m_id = ++s_lastId;

    m_componentHandlerPtr->pluginParamsChanged().onNotify(this, [this]() {
        rescanParams();
    });
}

VstPluginInstance::~VstPluginInstance()
{
    muse::audio::AudioResourceId resourceId = m_resourceId;
    std::shared_ptr<VstPluginProvider> provider = std::move(m_pluginProvider);
    PluginModulePtr module = std::move(m_module);

    auto securer = threadSecurer;
    auto repo = modulesRepo;

    Async::call(nullptr, [resourceId, provider, module, securer, repo]() mutable {
        ONLY_MAIN_THREAD(securer);

        repo()->removePluginModule(resourceId);

        //! NOTE: the order of destruction is important here
        provider.reset();
        module.reset();
    }, threadSecurer()->mainThreadId());
}

const muse::audio::AudioResourceId& VstPluginInstance::resourceId() const
{
    return m_resourceId;
}

const std::string& VstPluginInstance::name() const
{
    return m_resourceId;
}

VstPluginInstanceId VstPluginInstance::id() const
{
    return m_id;
}

/**
 * @brief VstPluginInstance::load
 * @note Some Vst plugins might not support loading in the background threads.
 *       So we have to ensure that this function being executed in the main thread
 */
void VstPluginInstance::load()
{
    Async::call(this, [this]() {
        ONLY_MAIN_THREAD(threadSecurer);

        std::lock_guard lock(m_mutex);

        m_module = modulesRepo()->pluginModule(m_resourceId);
        if (!m_module) {
            modulesRepo()->addPluginModule(m_resourceId);
            m_module = modulesRepo()->pluginModule(m_resourceId);
        }

        if (!m_module) {
            LOGE() << "Unable to find vst plugin module, resourceId: " << m_resourceId;
            return;
        }

        const auto& factory = m_module->getFactory();

        for (const ClassInfo& classInfo : factory.classInfos()) {
            if (classInfo.category() != kVstAudioEffectClass) {
                continue;
            }

            m_pluginProvider = std::make_unique<VstPluginProvider>(factory, classInfo);
            m_classInfo = classInfo;
            break;
        }

        if (!m_pluginProvider) {
            LOGE() << "Unable to load vst plugin provider";
            return;
        }

        if (!m_pluginProvider->init()) {
            LOGE() << "Unable to initialize vst plugin provider";
            return;
        }

        auto controller = m_pluginProvider->controller();

        if (!controller) {
            return;
        }

        controller->setComponentHandler(m_componentHandlerPtr);

        m_isLoaded = true;
        m_loadingCompleted.notify();
    }, threadSecurer()->mainThreadId());
}

void VstPluginInstance::rescanParams()
{
    ONLY_AUDIO_OR_MAIN_THREAD(threadSecurer);

    if (!m_pluginProvider) {
        LOGE() << "Plugin provider is not initialized";
        return;
    }

    auto component = m_pluginProvider->component();
    auto controller = m_pluginProvider->controller();

    if (!controller || !component) {
        return;
    }

    m_componentStateBuffer.seek(0, Steinberg::IBStream::kIBSeekSet, nullptr);
    m_controllerStateBuffer.seek(0, Steinberg::IBStream::kIBSeekSet, nullptr);

    m_componentStateBuffer.setSize(0);
    m_controllerStateBuffer.setSize(0);

    muse::audio::AudioUnitConfig updatedConfig;

    component->getState(&m_componentStateBuffer);
    updatedConfig.emplace(COMPONENT_STATE_KEY, std::string(m_componentStateBuffer.getData(), m_componentStateBuffer.getSize()));

    controller->getState(&m_controllerStateBuffer);
    updatedConfig.emplace(CONTROLLER_STATE_KEY, std::string(m_controllerStateBuffer.getData(), m_controllerStateBuffer.getSize()));

    m_pluginSettingsChanges.send(std::move(updatedConfig));
}

void VstPluginInstance::stateBufferFromString(VstMemoryStream& buffer, char* strData, const size_t strSize) const
{
    if (strSize == 0) {
        return;
    }

    buffer.write(strData, static_cast<Steinberg::int32>(strSize), nullptr);
    buffer.seek(0, Steinberg::IBStream::kIBSeekSet, nullptr);
}

PluginViewPtr VstPluginInstance::createView() const
{
    ONLY_MAIN_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    if (!m_pluginProvider) {
        return nullptr;
    }

    PluginControllerPtr controller = m_pluginProvider->controller();
    if (!controller) {
        return nullptr;
    }

    return owned(controller->createView(PluginEditorViewType::kEditor));
}

PluginControllerPtr VstPluginInstance::controller() const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    if (!m_pluginProvider) {
        return nullptr;
    }

    return m_pluginProvider->controller();
}

PluginComponentPtr VstPluginInstance::component() const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    if (!m_pluginProvider) {
        return nullptr;
    }

    return m_pluginProvider->component();
}

PluginMidiMappingPtr VstPluginInstance::midiMapping() const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    if (!m_pluginProvider) {
        return nullptr;
    }

    return m_pluginProvider->midiMapping();
}

bool VstPluginInstance::isAbleForInput() const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    auto search = std::find_if(m_classInfo.subCategories().begin(),
                               m_classInfo.subCategories().end(), [](const std::string& subCategoryStr) {
        return subCategoryStr == PluginSubCategory::Synth
               || subCategoryStr == PluginSubCategory::Piano
               || subCategoryStr == PluginSubCategory::Drum
               || subCategoryStr == PluginSubCategory::External;
    });

    return search != m_classInfo.subCategories().cend();
}

void VstPluginInstance::updatePluginConfig(const muse::audio::AudioUnitConfig& config)
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    if (!m_pluginProvider) {
        LOGE() << "Plugin provider is not initialized";
        return;
    }

    auto controller = m_pluginProvider->controller();
    auto component = m_pluginProvider->component();

    if (!controller || !component) {
        LOGE() << "Unable to update settings for VST plugin";
        return;
    }

    auto componentState = config.find(COMPONENT_STATE_KEY.data());
    if (componentState == config.end()) {
        return;
    }

    auto controllerState = config.find(CONTROLLER_STATE_KEY.data());
    if (controllerState == config.end()) {
        return;
    }

    try {
        stateBufferFromString(m_componentStateBuffer, const_cast<char*>(componentState->second.c_str()), componentState->second.size());
        component->setState(&m_componentStateBuffer);
        controller->setComponentState(&m_componentStateBuffer);
    } catch (...) {
        LOGW() << "Unexpected VST plugin exception";
    }

    try {
        stateBufferFromString(m_controllerStateBuffer, const_cast<char*>(controllerState->second.data()), controllerState->second.size());
        controller->setState(&m_controllerStateBuffer);
    } catch (...) {
        LOGW() << "Unexpected VST plugin exception";
    }
}

void VstPluginInstance::refreshConfig()
{
    ONLY_MAIN_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    rescanParams();
}

bool VstPluginInstance::isValid() const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    if (!m_module
        || !m_pluginProvider) {
        return false;
    }

    return true;
}

bool VstPluginInstance::isLoaded() const
{
    ONLY_AUDIO_OR_MAIN_THREAD(threadSecurer);

    return m_isLoaded;
}

Notification VstPluginInstance::loadingCompleted() const
{
    return m_loadingCompleted;
}

async::Channel<muse::audio::AudioUnitConfig> VstPluginInstance::pluginSettingsChanged() const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    return m_pluginSettingsChanges;
}
