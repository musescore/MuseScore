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
#include "vstmodule.h"
#include "settings.h"

#include "vstscaner.h"
#include "log.h"

using namespace mu::vst;
using namespace mu::framework;

VSTConfiguration VSTModule::m_configuration = VSTConfiguration();

std::string VSTModule::moduleName() const
{
    return "vst";
}

void VSTModule::registerExports()
{
}

void VSTModule::resolveImports()
{
}

void VSTModule::onInit()
{
    m_configuration.init();
    VSTScaner scaner(m_configuration.searchPaths());
    scaner.scan();

    auto plugins = scaner.getPlugins();
    for (auto p : scaner.getPlugins()) {
        LOGI() << "Plugin: " << p.second.getName() << p.second.getId();
        auto inst = p.second.createInstance();
        LOGI() << "Plugin instance: " << inst->isValid();
    }

    //Plugin:  sforzando 5C5CA79682FC437AB6539BA204BAB349
    //Plugin:  Note Expression Synth With UI 41466D9BB0654576B641098F686371B3
    //Plugin:  Note Expression Synth 6EE65CD1B83A4AF480AA7929AEA6B8A0
    std::string testPluginId = "41466D9BB0654576B641098F686371B3";//Note Expression Synth With UI
    if (plugins.find(testPluginId) != plugins.end()) {
        auto plugin = plugins[testPluginId];
        auto pluginInstance = plugin.createInstance();
        LOGI() << "createView: " << pluginInstance->createView();

        auto ps = pluginInstance->getParameters();
        LOGI() << "count of parameters: " << ps.size();
        for (auto p : ps) {
            LOGI() << p.id()
                   << QString::fromStdU16String(p.title())
                   << QString::fromStdU16String(p.unit())
                   << pluginInstance->getParameterValue(p);
        }
    }
}
