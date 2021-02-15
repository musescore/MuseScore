//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#include "pluginloader.h"

#include <map>
//Qt
#include <QDir>
#include <QString>

//mu
#include "log.h"

//Steinberg
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"

using namespace mu::vst;
using namespace Steinberg;
using namespace Steinberg::Vst;

PluginLoader::PluginLoader(std::string folder, std::string filename)
    : m_folder(folder),
    m_filename(filename)
{
}

PluginLoader::~PluginLoader()
{
    unload();
}

bool PluginLoader::load()
{
    QDir directory(QString::fromStdString(m_folder));
    auto pluginPath = directory.filePath(QString::fromStdString(m_filename));

    m_library.setFileName(pluginPath);

    IF_ASSERT_FAILED(m_library.load()) {
        return false;
    }

    auto factory = (GetFactoryProc)m_library.resolve("GetPluginFactory");
    IF_ASSERT_FAILED(factory) {
        unload();
        return false;
    }
    m_factory = static_cast<Steinberg::IPluginFactory3*>(factory());

    //scan all classes and create Plugins
    initPlugins();

    return true;
}

void PluginLoader::unload()
{
    m_factory = nullptr;

    //library will unload automatically with app exits
    //don't call unload, it will destroy all pointers to plugin's functions (factory, audioEffectClass etc)
    //m_library.unload();
}

const std::vector<Plugin>& PluginLoader::getPlugins() const
{
    return m_plugins;
}

void PluginLoader::initPlugins()
{
    IF_ASSERT_FAILED(m_factory) {
        return;
    }

    for (int32 i = 0; i < m_factory->countClasses(); i++) {
        PClassInfo2 classInfo;
        m_factory->getClassInfo2(i, &classInfo);

        if (std::string(classInfo.category).compare(kVstAudioEffectClass) == 0) {
            m_plugins.push_back(Plugin(classInfo, m_factory));
        }
    }
}
