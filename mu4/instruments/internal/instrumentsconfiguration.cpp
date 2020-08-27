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
#include "instrumentsconfiguration.h"

#include "log.h"
#include "settings.h"

using namespace mu::scene::instruments;

std::vector<mu::io::path> InstrumentsConfiguration::instrumentPaths() const
{
    std::vector<io::path> paths;
    io::path sharePath = globalConfiguration()->sharePath() + "/instruments";
    paths.push_back(sharePath);

    io::path dataPath = globalConfiguration()->dataPath() + "/instruments";
    paths.push_back(dataPath);

    std::vector<io::path> extensionsPath = this->extensionsPaths();
    paths.insert(paths.end(), extensionsPath.begin(), extensionsPath.end());

    return paths;
}

std::vector<mu::io::path> InstrumentsConfiguration::extensionsPaths() const
{
    std::vector<io::path> result;

    QStringList workspacesPaths = extensionsConfigurator()->instrumentsPaths();
    for (const QString& path: workspacesPaths) {
        result.push_back(io::pathFromQString(path));
    }

    return result;
}
