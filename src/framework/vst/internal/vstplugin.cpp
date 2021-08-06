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

VstPlugin::VstPlugin(PluginModulePtr module)
    : m_module(std::move(module))
{
    ONLY_AUDIO_THREAD(threadSecurer);

    load();
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
        }
    }, threadSecurer()->mainThreadId());
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

PluginComponentPtr VstPlugin::component() const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    if (!m_pluginProvider) {
        return nullptr;
    }

    if (m_pluginComponent) {
        return m_pluginComponent;
    }

    m_pluginComponent = m_pluginProvider->getComponent();

    return m_pluginComponent;
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
