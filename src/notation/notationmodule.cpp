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
#include "notationmodule.h"

#include "modularity/ioc.h"
#include "project/inotationwritersregister.h"

#include "internal/notationconfiguration.h"
#include "internal/positionswriter.h"
#include "internal/mscnotationwriter.h"
#include "internal/instrumentsrepository.h"
#include "internal/engravingfontscontroller.h"

#include "diagnostics/idiagnosticspathsregister.h"

using namespace mu::notation;
using namespace muse;
using namespace muse::modularity;

std::string NotationModule::moduleName() const
{
    return "notation";
}

void NotationModule::registerExports()
{
    m_configuration = std::make_shared<NotationConfiguration>(iocContext());
    m_instrumentsRepository = std::make_shared<InstrumentsRepository>(iocContext());

#ifdef MUE_BUILD_ENGRAVING_FONTSCONTROLLER
    m_engravingFontsController = std::make_shared<EngravingFontsController>();
#endif

    ioc()->registerExport<INotationConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IInstrumentsRepository>(moduleName(), m_instrumentsRepository);
}

void NotationModule::resolveImports()
{
    auto writers = ioc()->resolve<project::INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "spos" }, std::make_shared<PositionsWriter>(PositionsWriter::ElementType::SEGMENT, iocContext()));
        writers->reg({ "mpos" }, std::make_shared<PositionsWriter>(PositionsWriter::ElementType::MEASURE, iocContext()));
        writers->reg({ "mscz" }, std::make_shared<MscNotationWriter>(engraving::MscIoMode::Zip));
        writers->reg({ "mscx" }, std::make_shared<MscNotationWriter>(engraving::MscIoMode::Dir));
    }
}

void NotationModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
    m_instrumentsRepository->init();

#ifdef MUE_BUILD_ENGRAVING_FONTSCONTROLLER
    m_engravingFontsController->init();
#endif

    bool isVertical = m_configuration->canvasOrientation().val == muse::Orientation::Vertical;
    mu::engraving::MScore::setVerticalOrientation(isVertical);

    auto pr = ioc()->resolve<diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        pr->reg("instruments", m_configuration->instrumentsXmlPath());
        pr->reg("score orders", m_configuration->scoreOrdersXmlPath());

        muse::io::path_t userInstrumentsPath = m_configuration->userInstrumentsFolder();
        if (!userInstrumentsPath.empty()) {
            pr->reg("user instruments folder", userInstrumentsPath);
        }
    }
}
