//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "vstplugin.h"

#include "log.h"

using namespace mu;
using namespace mu::vst;

VstPlugin::VstPlugin()
    : m_factory(nullptr)
{
}

Ret VstPlugin::load(const io::path& pluginPath)
{
    PluginContextFactory::instance().setPluginContext(&m_pluginContext);

    std::string errorString;

    m_module = PluginModule::create(pluginPath.toStdString(), errorString);

    if (!m_module) {
        LOGE() << errorString;
        return make_ret(Err::NoPluginModule);
    }

    m_factory = m_module->getFactory();

    if (!m_factory.get()) {
        return make_ret(Err::NoPluginFactory);
    }

    for (const ClassInfo& classInfo : m_factory.classInfos()) {
        if (classInfo.category() != kVstAudioEffectClass) {
            LOGI() << "Non-audio plugins are not supported, plugin path: "
                   << pluginPath;
            continue;
        }

        m_pluginProvider = owned(new PluginProvider(m_factory, classInfo));
        break;
    }

    if (!m_pluginProvider) {
        return make_ret(Err::NoPluginProvider);
    }

    m_pluginController = owned(m_pluginProvider->getController());

    if (!m_pluginController) {
        return make_ret(Err::NoPluginController);
    }

    return Ret(true);
}

PluginId VstPlugin::id() const
{
    if (!m_pluginProvider) {
        return "";
    }

    Steinberg::FUID id;
    m_pluginProvider->getComponentUID(id);

    VST3::UID uid = VST3::UID::fromTUID(id.toTUID());

    return uid.toString(false);
}

VstPluginMeta VstPlugin::meta() const
{
    VstPluginMeta result;

    if (!isValid()) {
        return result;
    }

    result.id = id();
    result.name = m_module->getName();
    result.path = m_module->getPath();

    return result;
}

PluginView VstPlugin::view()
{
    if (m_pluginView) {
        return m_pluginView;
    }

    m_pluginView = owned(m_pluginController->createView(PluginEditorViewType::kEditor));

    return m_pluginView;
}

bool VstPlugin::isValid() const
{
    if (!m_module
        || !m_factory.get()
        || !m_pluginProvider
        || !m_pluginController) {
        return false;
    }

    return true;
}
