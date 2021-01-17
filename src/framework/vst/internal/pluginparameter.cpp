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

#include "pluginparameter.h"

using namespace mu::vst;
using namespace Steinberg;
using namespace Steinberg::Vst;

PluginParameter::PluginParameter(ParameterInfo info)
    : m_info(info)
{
}

PluginParameter::PluginParameter(ParameterInfo&& info)
    : m_info(std::move(info))
{
}

unsigned int PluginParameter::id() const
{
    return m_info.id;
}

std::u16string PluginParameter::title() const
{
    return m_info.title;
}

std::u16string PluginParameter::shortTitle() const
{
    return m_info.shortTitle;
}

std::u16string PluginParameter::unit() const
{
    return m_info.units;
}

unsigned int PluginParameter::stepCount() const
{
    return m_info.stepCount;
}

double PluginParameter::defaultValue() const
{
    return m_info.defaultNormalizedValue;
}

bool PluginParameter::isEditable() const
{
    return !(m_info.flags & ParameterInfo::kIsReadOnly) & isVisible();
}

bool PluginParameter::isVisible() const
{
    return !(m_info.flags & ParameterInfo::kIsHidden);
}
