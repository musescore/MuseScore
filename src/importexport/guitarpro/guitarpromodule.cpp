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
#include "guitarpromodule.h"

#include "log.h"
#include "modularity/ioc.h"

#include "notation/inotationreadersregister.h"
#include "internal/guitarproreader.h"
#include "internal/guitarproconfiguration.h"

using namespace mu::iex::guitarpro;
using namespace mu::notation;

static std::shared_ptr<GuitarProConfiguration> s_configuration = std::make_shared<GuitarProConfiguration>();

std::string GuitarProModule::moduleName() const
{
    return "iex_guitarpro";
}

void GuitarProModule::registerExports()
{
    framework::ioc()->registerExport<IGuitarProConfiguration>(moduleName(), s_configuration);
}

void GuitarProModule::resolveImports()
{
    auto readers = framework::ioc()->resolve<INotationReadersRegister>(moduleName());
    if (readers) {
        readers->reg({ "gtp", "gp3", "gp4", "gp5", "gpx", "gp", "ptb" }, std::make_shared<GuitarProReader>());
    }
}

void GuitarProModule::onInit(const framework::IApplication::RunMode&)
{
    s_configuration->init();
}
