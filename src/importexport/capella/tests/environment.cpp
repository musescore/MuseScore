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
#include "engraving/tests/utils/scorerw.h"

#include "engraving/libmscore/instrtemplate.h"
#include "engraving/libmscore/mscore.h"

#include "log.h"

static mu::testing::SuiteEnvironment importexport_se(
{
    new mu::draw::DrawModule(),
    new mu::fonts::FontsModule(), // needs for libmscore
    new mu::engraving::EngravingModule()
},
    nullptr,
    []() {
    LOGI() << "capella tests suite post init";

    mu::engraving::ScoreRW::setRootPath(mu::String::fromUtf8(iex_capella_tests_DATA_ROOT));

    mu::engraving::MScore::testMode = true;
    mu::engraving::MScore::noGui = true;

    mu::engraving::loadInstrumentTemplates(":/data/instruments.xml");
}
    );
