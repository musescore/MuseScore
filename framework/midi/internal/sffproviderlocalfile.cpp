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
#include "networkmodule.h"

#include "modularity/ioc.h"
#include "internal/networkmanagercreator.h"

using namespace mu::framework;

<<<<<<< HEAD
std::string NetworkModule::moduleName() const
{
    return "network";
}

void NetworkModule::registerExports()
{
    framework::ioc()->registerExport<INetworkManagerCreator>(moduleName(), new NetworkManagerCreator());
}
=======
using namespace mu::midi;

void SFFProviderLocalFile::loadSF(const midi::Programs& programs, const OnLoading& onloading, const OnLoaded& onloaded)
{
    //! NOTE For tests
    io::path sffilePath = globalConfiguration()->dataPath() + "/sound/GeneralUser GS v1.471.sf2";

    onloading(100);
    onloaded(true, sffilePath, programs);
}
>>>>>>> 5191d9776... renamed midi and audio namespaces
