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
#include "bbmodule.h"

#include "log.h"
#include "modularity/ioc.h"

#include "notation/inotationreadersregister.h"
#include "internal/notationbbreader.h"

using namespace mu::iex::bb;
using namespace mu::notation;

std::string BBModule::moduleName() const
{
    return "iex_bb";
}

void BBModule::resolveImports()
{
    auto readers = framework::ioc()->resolve<INotationReadersRegister>(moduleName());
    if (readers) {
        readers->reg({ "mgu", "sgu" }, std::make_shared<NotationBBReader>());
    }
}
