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
#ifndef MU_PLUGINS_PLUGINSSERVICESTUB_H
#define MU_PLUGINS_PLUGINSSERVICESTUB_H

#include "plugins/ipluginsservice.h"

namespace mu::plugins {
class PluginsServiceStub : public IPluginsService
{
public:
    RetVal<PluginInfoList> plugins(PluginsStatus status = All) const override;

    RetValCh<framework::Progress> install(const CodeKey& codeKey) override;
    RetValCh<framework::Progress> update(const CodeKey& codeKey) override;
    Ret uninstall(const CodeKey& codeKey) override;

    Ret run(const CodeKey& codeKey) override;

    async::Channel<PluginInfo> pluginChanged() const override;
};
}

#endif // MU_PLUGINS_IPLUGINSSERVICE_H
