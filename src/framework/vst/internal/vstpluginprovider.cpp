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

#include "log.h"

using namespace muse::vst;

class VstPluginProvider::Impl : public Steinberg::Vst::PlugProvider
{
    friend class VstPluginProvider;

public:
    Impl(const PluginFactory& factory, const ClassInfo& info)
        : Steinberg::Vst::PlugProvider(factory, info)
    {
    }

    bool init()
    {
        if (!initialize()) {
            return false;
        }

        IF_ASSERT_FAILED(controller) {
            return false;
        }

        controller->queryInterface(Steinberg::Vst::IMidiMapping_iid, (void**)&midiMapping);

        return true;
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

bool VstPluginProvider::init()
{
    return m_impl->init();
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
