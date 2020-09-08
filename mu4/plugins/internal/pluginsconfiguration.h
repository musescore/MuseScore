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
#ifndef MU_PLUGINS_PLUGINSCONFIGURATION_H
#define MU_PLUGINS_PLUGINSCONFIGURATION_H

#include "ipluginsconfiguration.h"
#include "modularity/ioc.h"
#include "iglobalconfiguration.h"

namespace mu::plugins {
class PluginsConfiguration : public IPluginsConfiguration
{
    INJECT(extensions, framework::IGlobalConfiguration, globalConfiguration)

public:
    io::paths pluginsDirPaths() const override;
    io::path pluginsRepositoryPath() const override;

    QUrl pluginsServerUrl() const override;
    QUrl pluginDetailsUrl(const std::string& codeKey) const override;
};
}

#endif // MU_PLUGINS_PLUGINSCONFIGURATION_H
