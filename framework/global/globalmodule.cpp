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
#include "globalmodule.h"

#include "modularity/ioc.h"
#include "internal/globalconfiguration.h"

#include "internal/interactive.h"
#include "internal/launcher.h"
#include "invoker.h"

using namespace mu::framework;

std::string GlobalModule::moduleName() const
{
    return "global";
}

void GlobalModule::registerExports()
{
    ioc()->registerExport<IGlobalConfiguration>(moduleName(), new GlobalConfiguration());
    ioc()->registerExport<IInteractive>(moduleName(), new Interactive());
    ioc()->registerExport<ILauncher>(moduleName(), new Launcher());
}

void GlobalModule::onInit()
{
    Invoker::setup();
}
