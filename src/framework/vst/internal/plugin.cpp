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
#include "plugin.h"
#include "plugininstance.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"

using namespace mu::vst;
using namespace Steinberg;
using namespace Steinberg::Vst;

Plugin::Plugin()
    : m_effectClass()
{
}

Plugin::Plugin(Steinberg::PClassInfo2 effectClass, Steinberg::IPluginFactory3* factory)
    : m_effectClass(effectClass), m_factory(factory)
{
}

Plugin::Plugin(const Plugin& second)
    : m_effectClass(second.m_effectClass), m_factory(second.m_factory)
{
}

std::string Plugin::getId() const
{
    FUID uid(m_effectClass.cid);
    char8 strUID[33] = { 0 };
    uid.toString(strUID);
    return std::string(strUID);
}

std::string Plugin::getName() const
{
    return m_effectClass.name;
}

Plugin::Type Plugin::getType() const
{
    if (subCategoriesMap.find(m_effectClass.subCategories) == subCategoriesMap.end()) {
        return UNKNOWN;
    }
    return subCategoriesMap.at(m_effectClass.subCategories);
}

std::shared_ptr<PluginInstance> Plugin::createInstance()
{
    return PluginInstance::create(this);
}

const std::map<std::string, Plugin::Type> Plugin::subCategoriesMap =
{
    { PlugType::kFxInstrument, Instrument },
    { PlugType::kFxInstrumentExternal, Instrument },
    { PlugType::kInstrument, Instrument },
    { PlugType::kInstrumentDrum, Instrument },
    { PlugType::kInstrumentExternal, Instrument },
    { PlugType::kInstrumentPiano, Instrument },
    { PlugType::kInstrumentSampler, Instrument },
    { PlugType::kInstrumentSynth, Instrument },
    { PlugType::kInstrumentSynthSampler, Instrument }
};
