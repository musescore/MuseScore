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

#include "testing/environment.h"

#include "log.h"
#include "framework/fonts/fontsmodule.h"
#include "instrumentsscene/instrumentsscenemodule.h"
#include "framework/system/systemmodule.h"
#include "importexport/guitarpro/guitarpromodule.h"
#include "engraving/engravingmodule.h"

#include "libmscore/masterscore.h"
#include "libmscore/musescoreCore.h"

static mu::testing::SuiteEnvironment importexport_se(
{
    new mu::engraving::EngravingModule(),
    new mu::fonts::FontsModule(), // needs for libmscore
    new mu::instrumentsscene::InstrumentsSceneModule(),
    new mu::iex::guitarpro::GuitarProModule()
},
    []() {
    LOGI() << "guitarpro tests suite post init";
    Ms::MScore::noGui = true;

    new Ms::MuseScoreCore();
    Ms::MScore::init(); // initialize libmscore
}
    );
