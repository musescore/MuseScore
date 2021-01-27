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

using namespace mu::instruments;

mu::io::paths InstrumentsConfiguration::instrumentPaths() const
{
    io::paths paths;
    io::path sharePath = globalConfiguration()->sharePath() + "/instruments";
    paths.push_back(sharePath);

    io::path dataPath = globalConfiguration()->dataPath() + "/instruments";
    paths.push_back(dataPath);

    io::paths extensionsPath = this->extensionsPaths();
    paths.insert(paths.end(), extensionsPath.begin(), extensionsPath.end());

    return paths;
}

mu::io::paths InstrumentsConfiguration::extensionsPaths() const
{
    if (extensionsConfigurator()) {
        return extensionsConfigurator()->instrumentsPaths();
    }
    return mu::io::paths();
}
