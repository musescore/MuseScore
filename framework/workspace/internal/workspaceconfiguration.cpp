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
#include "workspaceconfiguration.h"

#include "settings.h"

using namespace mu;
using namespace mu::workspace;
using namespace mu::framework;

static const Settings::Key CURRENT_WORKSPACE("workspace", "application/workspacer");

std::vector<io::path> WorkspaceConfiguration::workspacePaths() const
{
    std::vector<io::path> paths;
    io::path sharePath = globalConfiguration()->sharePath() + "/workspaces";
    paths.push_back(sharePath);

    io::path dataPath = globalConfiguration()->dataPath() + "/workspaces";
    paths.push_back(dataPath);

    std::vector<io::path> extensionsPath = this->extensionsPaths();
    paths.insert(paths.end(), extensionsPath.begin(), extensionsPath.end());

    return paths;
}

std::string WorkspaceConfiguration::currentWorkspaceName() const
{
    return settings()->value(CURRENT_WORKSPACE).toString();
}

io::paths WorkspaceConfiguration::extensionsPaths() const
{
    return extensionsConfiguration()->workspacesPaths();
}
