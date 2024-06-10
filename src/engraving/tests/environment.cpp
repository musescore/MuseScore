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

#include "engraving/engravingmodule.h"
#include "engraving/dom/engravingitem.h"
#include "draw/drawmodule.h"

#include "dom/instrtemplate.h"
#include "dom/mscore.h"

#include "mocks/engravingconfigurationmock.h"

#include "utils/scorerw.h"

#include "log.h"

static muse::testing::SuiteEnvironment engraving_se(
{
    new muse::draw::DrawModule(),
    new mu::engraving::EngravingModule()
},
    nullptr,
    []() {
    LOGI() << "engraving tests suite post init";

    mu::engraving::ScoreRW::setRootPath(muse::String::fromUtf8(engraving_tests_DATA_ROOT));

    mu::engraving::MScore::testMode = true;
    mu::engraving::MScore::noGui = true;

    mu::engraving::loadInstrumentTemplates(":/data/instruments.xml");

    using ECMock = ::testing::NiceMock<mu::engraving::EngravingConfigurationMock>;

    std::shared_ptr<ECMock> configurator(new ECMock(), [](ECMock*) {}); // no delete
    ON_CALL(*configurator, isAccessibleEnabled()).WillByDefault(::testing::Return(false));
    ON_CALL(*configurator, defaultColor()).WillByDefault(::testing::Return(muse::draw::Color::BLACK));

    muse::modularity::globalIoc()->unregister<mu::engraving::IEngravingConfiguration>("utests");
    muse::modularity::globalIoc()->registerExport<mu::engraving::IEngravingConfiguration>("utests", configurator);
},

    []() {
    std::shared_ptr<mu::engraving::IEngravingConfiguration> mock
        = muse::modularity::globalIoc()->resolve<mu::engraving::IEngravingConfiguration>("utests");
    muse::modularity::globalIoc()->unregister<mu::engraving::IEngravingConfiguration>("utests");

    //! HACK
    //! There are still live pointers to the mock
    //! because of this, the mock generates an error stating that it has not been deleted
    //! This is a hack to remove it manually to get rid of this error
    //! The problem itself is deeper, it seems that some score objects are not deleted
    mu::engraving::IEngravingConfiguration* ecptr = mock.get();
    delete ecptr;
}
    );
