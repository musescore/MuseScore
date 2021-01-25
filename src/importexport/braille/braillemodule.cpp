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
#include "braillemodule.h"

#include "log.h"
#include "modularity/ioc.h"

#include "notation/inotationwritersregister.h"

using namespace mu::iex::braille;
using namespace mu::notation;

std::string BrailleModule::moduleName() const
{
    return "iex_braille";
}

void BrailleModule::resolveImports()
{
    auto writers = framework::ioc()->resolve<INotationWritersRegister>(moduleName());
    if (writers) {
    }
}
