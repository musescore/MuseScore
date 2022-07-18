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

#include "fonts/fontsmodule.h"
#include "draw/drawmodule.h"
#include "engraving/engravingmodule.h"
#include "engraving/utests/utils/scorerw.h"

#include "importexport/guitarpro/guitarpromodule.h"

#include "libmscore/masterscore.h"
#include "libmscore/musescoreCore.h"

#include "log.h"

using namespace mu::iex::guitarpro;

static mu::testing::SuiteEnvironment importexport_se(
{
    new mu::draw::DrawModule(),         // needs for engraving
    new mu::fonts::FontsModule(),       // needs for engraving
    new mu::engraving::EngravingModule(),
    new mu::iex::guitarpro::GuitarProModule()
},
    []() {
    LOGI() << "guitarpro tests suite post init";

    mu::engraving::ScoreRW::setRootPath(mu::String::fromUtf8(iex_guitarpro_tests_DATA_ROOT));

    mu::engraving::MScore::testMode = true;
    mu::engraving::MScore::testWriteStyleToScore = false;
    mu::engraving::MScore::noGui = true;

    new mu::engraving::MuseScoreCore();
    mu::engraving::MScore::init(); // initialize libmscore
}
    );
