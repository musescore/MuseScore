/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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
