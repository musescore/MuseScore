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
#include "wasmtestmodule.h"
#include "log.h"

using namespace mu::wasmtest;

std::string WasmTestModule::moduleName() const
{
    return "wasmTest";
}

void WasmTestModule::onStartApp()
{
    Ms::Score score;
    auto notation = notationCreator()->newMasterNotation();
    Ret ret = notation->load("/files/test3.mscz");

    if (!ret) {
        LOGE() << ret.toString();
    }

    context()->addMasterNotation(notation);
    context()->setCurrentMasterNotation(notation);
}
