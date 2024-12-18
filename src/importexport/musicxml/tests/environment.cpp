/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "draw/drawmodule.h"
#include "engraving/engravingmodule.h"
#include "importexport/musicxml/musicxmlmodule.h"

#include "engraving/tests/utils/scorerw.h"

#include "engraving/dom/instrtemplate.h"
#include "engraving/dom/mscore.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

static muse::testing::SuiteEnvironment musicxml_se(
{
    new muse::draw::DrawModule(),
    new mu::engraving::EngravingModule(),
    new mu::iex::musicxml::MusicXmlModule() // necessary for initialising resources
},
    nullptr,
    []() {
    LOGI() << "musicxml tests suite post init";

    mu::engraving::ScoreRW::setRootPath(muse::String::fromUtf8(iex_musicxml_tests_DATA_ROOT));

    mu::engraving::MScore::testMode = true;
    mu::engraving::MScore::noGui = true;

    loadInstrumentTemplates(":/data/instruments.xml");
}
    );
