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
#include "instrumentsstubmodule.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "instrumentsreaderstub.h"
#include "instrumentsrepositorystub.h"
#include "instrumentsconfigurationstub.h"
#include "selectinstrumentscenariostub.h"

#include "ui/iinteractiveuriregister.h"

using namespace mu::instruments;
using namespace mu::framework;
using namespace mu::ui;

static void instruments_init_qrc()
{
    Q_INIT_RESOURCE(instruments);
}

std::string InstrumentsStubModule::moduleName() const
{
    return "instruments";
}

void InstrumentsStubModule::registerExports()
{
    ioc()->registerExport<IInstrumentsConfiguration>(moduleName(), new InstrumentsConfigurationStub());
    ioc()->registerExport<IInstrumentsRepository>(moduleName(), new InstrumentsRepositoryStub());
    ioc()->registerExport<IInstrumentsReader>(moduleName(), new InstrumentsReaderStub());
    ioc()->registerExport<ISelectInstrumentsScenario>(moduleName(), new SelectInstrumentsScenario());
}

void InstrumentsStubModule::resolveImports()
{
    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://instruments/select"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/Instruments/InstrumentsDialog.qml"));
    }
}

void InstrumentsStubModule::registerResources()
{
    instruments_init_qrc();
}

void InstrumentsStubModule::registerUiTypes()
{
    modularity::ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(instruments_QML_IMPORT);
}
