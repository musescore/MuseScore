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

#ifndef MU_VST_PLUGINPARAMETER_H
#define MU_VST_PLUGINPARAMETER_H

#include "pluginterfaces/vst/ivsteditcontroller.h"

namespace mu {
namespace vst {
class PluginParameter
{
public:
    PluginParameter() = default;
    PluginParameter(Steinberg::Vst::ParameterInfo info);
    PluginParameter(Steinberg::Vst::ParameterInfo&& info);

    //! unique id of the parameter in the plugin
    uint id() const { return m_info.id; }

    //! return title of the parameter, ex: Volume
    std::u16string title() const { return m_info.title; }

    //! return short title, ex: Vol
    std::u16string shortTitle() const { return m_info.shortTitle; }

    //! parameter units, ex: dB
    std::u16string unit() const { return m_info.units; }

    //! how many steps the parameter has
    uint stepCount() const { return m_info.stepCount; }

    //! parameter's default value
    double defaultValue() const { return m_info.defaultNormalizedValue; }

    //! return true if visible and not readonly
    bool isEditable() const;

    //! return true if not hidden
    bool isVisible() const;

private:
    Steinberg::Vst::ParameterInfo m_info;
};
}
}
#endif // MU_VST_PLUGINPARAMETER_H
