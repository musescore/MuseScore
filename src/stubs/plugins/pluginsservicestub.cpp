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
#include "pluginsservicestub.h"

using namespace mu::plugins;
using namespace mu;

RetVal<PluginInfoList> PluginsServiceStub::plugins(IPluginsService::PluginsStatus) const
{
    RetVal<PluginInfoList> result;
    result.ret = make_ret(Ret::Code::NotSupported);
    return result;
}

RetValCh<framework::Progress> PluginsServiceStub::install(const CodeKey&)
{
    RetValCh<framework::Progress> result;
    result.ret = make_ret(Ret::Code::NotSupported);
    return result;
}

RetValCh<framework::Progress> PluginsServiceStub::update(const CodeKey&)
{
    RetValCh<framework::Progress> result;
    result.ret = make_ret(Ret::Code::NotSupported);
    return result;
}

Ret PluginsServiceStub::uninstall(const CodeKey&)
{
    return make_ret(Ret::Code::NotSupported);
}

Ret PluginsServiceStub::run(const CodeKey&)
{
    return make_ret(Ret::Code::NotSupported);
}

async::Channel<PluginInfo> PluginsServiceStub::pluginChanged() const
{
    return async::Channel<PluginInfo>();
}
