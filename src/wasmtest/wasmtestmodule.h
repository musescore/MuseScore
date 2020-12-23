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
#ifndef MU_WASM_WASMMODULE_H
#define MU_WASM_WASMMODULE_H

#include "modularity/imodulesetup.h"
#include "modularity/ioc.h"
#include "notation/inotationcreator.h"
#include "actions/iactionsdispatcher.h"
#include "context/iglobalcontext.h"

namespace mu::wasmtest {
class WasmTestModule : public framework::IModuleSetup
{
    INJECT(wasmtest, notation::INotationCreator, notationCreator)
    INJECT(notation, actions::IActionsDispatcher, dispatcher)
    INJECT(wasmtest, context::IGlobalContext, context)

public:

    std::string moduleName() const override;
    void onStartApp() override;
};
}

#endif // MU_WASM_WASMMODULE_H
