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
    mu::notation::ScoreCreateOptions options;
    options.key = mu::notation::Key::C;
    options.timesigNumerator = 3;
    options.timesigDenominator = 4;
    options.measureTimesigDenominator = 4;
    options.measureTimesigNumerator = 3;
    options.measures = 1;

    mu::instruments::Instrument instrument;
    instrument.id = "piano";
    options.instruments.append(instrument);

    Ms::Score score;
    auto notation = notationCreator()->newMasterNotation();
    Ret ret = notation->createNew(options);

    if (!ret) {
        LOGE() << ret.toString();
    }

    context()->addMasterNotation(notation);
    context()->setCurrentMasterNotation(notation);

    dispatcher()->dispatch("note-c");
    dispatcher()->dispatch("note-d");
    dispatcher()->dispatch("note-e");
    dispatcher()->dispatch("note-f");
    dispatcher()->dispatch("note-g");
    dispatcher()->dispatch("note-a");
    dispatcher()->dispatch("note-b");
}
