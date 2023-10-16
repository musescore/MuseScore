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

#include "engraving/engravingmodule.h"
#include "engraving/dom/engravingitem.h"
#include "fonts/fontsmodule.h"
#include "draw/drawmodule.h"

#include "dom/instrtemplate.h"
#include "dom/mscore.h"

#include "mocks/engravingconfigurationmock.h"

#include "utils/scorerw.h"

#include "log.h"

static mu::testing::SuiteEnvironment engraving_se(
{
    new mu::draw::DrawModule(),
    new mu::fonts::FontsModule(),
    new mu::engraving::EngravingModule()
},
    nullptr,
    []() {
    LOGI() << "engraving tests suite post init";

    mu::engraving::ScoreRW::setRootPath(mu::String::fromUtf8(engraving_tests_DATA_ROOT));

    mu::engraving::MScore::testMode = true;
    mu::engraving::MScore::noGui = true;

    mu::engraving::loadInstrumentTemplates(":/data/instruments.xml");

    std::shared_ptr<::testing::NiceMock<mu::engraving::EngravingConfigurationMock> > configurator
        = std::make_shared<::testing::NiceMock<mu::engraving::EngravingConfigurationMock> >();
    ON_CALL(*configurator, isAccessibleEnabled()).WillByDefault(::testing::Return(false));
    ON_CALL(*configurator, defaultColor()).WillByDefault(::testing::Return(mu::draw::Color::BLACK));
    mu::engraving::EngravingItem::setengravingConfiguration(configurator);
}
    );
