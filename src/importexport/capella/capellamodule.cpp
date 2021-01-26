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
#include "capellamodule.h"

#include "log.h"
#include "config.h"
#include "modularity/ioc.h"

#include "notation/inotationreadersregister.h"
#include "internal/capellareader.h"

using namespace mu::iex::capella;
using namespace mu::notation;

std::string CapellaModule::moduleName() const
{
    return "iex_capella";
}

void CapellaModule::resolveImports()
{
    auto readers = framework::ioc()->resolve<INotationReadersRegister>(moduleName());
    if (readers) {
        readers->reg({ "cap", "capx" }, std::make_shared<CapellaReader>());
    }
}
