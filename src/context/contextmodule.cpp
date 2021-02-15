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
#include "contextmodule.h"

#include "modularity/ioc.h"
#include "internal/globalcontext.h"

using namespace mu::context;

static std::shared_ptr<GlobalContext> s_globalContext = std::make_shared<GlobalContext>();

std::string ContextModule::moduleName() const
{
    return "context";
}

void ContextModule::registerExports()
{
    framework::ioc()->registerExport<IGlobalContext>(moduleName(), s_globalContext);
    framework::ioc()->registerExport<shortcuts::IShortcutContextResolver>(moduleName(), s_globalContext);
}

void ContextModule::onDeinit()
{
    s_globalContext->setCurrentMasterNotation(nullptr);
}
