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

#include "vstpluginprovider.h"

using namespace mu::vst;

class VstPluginProvider::Impl : public Steinberg::Vst::PlugProvider
{
    friend class VstPluginProvider;

public:
    Impl(const PluginFactory& factory, const ClassInfo& info)
        : Steinberg::Vst::PlugProvider(factory, info)
    {
        controller->queryInterface(Steinberg::Vst::IMidiMapping_iid, (void**)&midiMapping);
    }

    PluginMidiMappingPtr midiMapping;
};

VstPluginProvider::VstPluginProvider(const PluginFactory& factory, const ClassInfo& info)
{
    m_impl = std::make_unique<Impl>(factory, info);
}

VstPluginProvider::~VstPluginProvider()
{
}

PluginComponentPtr VstPluginProvider::component() const
{
    return m_impl->component;
}

PluginControllerPtr VstPluginProvider::controller() const
{
    return m_impl->controller;
}

PluginMidiMappingPtr VstPluginProvider::midiMapping() const
{
    return m_impl->midiMapping;
}
