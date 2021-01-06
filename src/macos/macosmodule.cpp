//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "macosmodule.h"

#include "modularity/ioc.h"
#include "internal/macos.h"

#include "log.h"

using namespace mu::macos;
using namespace mu::framework;

static std::shared_ptr<MacOS> s_macos = std::make_shared<MacOS>();

std::string MacosModule::moduleName() const
{
    return "macos";
}

void MacosModule::registerExports()
{
    framework::ioc()->registerExport<IMacOS>(moduleName(), s_macos);
}

void MacosModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (IApplication::RunMode::Converter == mode) {
        return;
    }

    s_macos->init();
}

void MacosModule::onDeinit()
{
    s_macos->stopObservingDistributedAllNotifications();
}
