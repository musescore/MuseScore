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

#ifndef MU_VST_VSTPLUGIN_H
#define MU_VST_VSTPLUGIN_H

#include "io/path.h"

#include "vsttypes.h"
#include "vsterrors.h"

namespace mu::vst {
class VstPlugin
{
public:

    VstPlugin();
    Ret load(const io::path& pluginPath);

    PluginId id() const;
    VstPluginMeta meta() const;
    PluginView view();

    bool isValid() const;

private:

    PluginModulePtr m_module = nullptr;
    PluginFactory m_factory;
    PluginProviderPtr m_pluginProvider = nullptr;
    PluginController m_pluginController = nullptr;
    PluginView m_pluginView = nullptr;

    PluginContext m_pluginContext;
};
}

#endif // MU_VST_VSTPLUGIN_H
