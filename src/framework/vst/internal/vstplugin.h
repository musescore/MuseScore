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
    PluginViewPtr view() const;
    PluginComponentPtr component() const;

    bool isValid() const;

private:

    PluginModulePtr m_module = nullptr;
    PluginFactory m_factory;
    PluginProviderPtr m_pluginProvider = nullptr;
    PluginControllerPtr m_pluginController = nullptr;
    mutable PluginComponentPtr m_pluginComponent = nullptr;
    mutable PluginViewPtr m_pluginView = nullptr;

    PluginContext m_pluginContext;
};
}

#endif // MU_VST_VSTPLUGIN_H
