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
#include "engraving/libmscore/engravingitem.h"
#include "framework/fonts/fontsmodule.h"

#include "libmscore/masterscore.h"
#include "libmscore/musescoreCore.h"

#include "mocks/engravingconfigurationmock.h"

#include "log.h"

static mu::testing::SuiteEnvironment engraving_se(
{
    new mu::fonts::FontsModule(),
    new mu::engraving::EngravingModule()
},
    []() {
    LOGI() << "engraving tests suite post init";
    mu::engraving::MScore::testMode = true;
    mu::engraving::MScore::noGui = true;

    new mu::engraving::MuseScoreCore;
    mu::engraving::MScore* mscore = new mu::engraving::MScore();
    mscore->init();

    mu::engraving::loadInstrumentTemplates(u":/data/instruments.xml");

    std::shared_ptr<testing::NiceMock<mu::engraving::EngravingConfigurationMock> > configurator
        = std::make_shared<testing::NiceMock<mu::engraving::EngravingConfigurationMock> >();
    ON_CALL(*configurator, isAccessibleEnabled()).WillByDefault(testing::Return(false));
    ON_CALL(*configurator, defaultColor()).WillByDefault(testing::Return(mu::draw::Color::black));
    mu::engraving::EngravingItem::setengravingConfiguration(configurator);
}
    );
