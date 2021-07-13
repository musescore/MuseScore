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
#include "instrumentsscenestubmodule.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "selectinstrumentscenariostub.h"

#include "ui/iinteractiveuriregister.h"

using namespace mu::instrumentsscene;
using namespace mu::modularity;
using namespace mu::ui;

static void instrumentsscene_init_qrc()
{
    Q_INIT_RESOURCE(instrumentsscene);
}

std::string InstrumentsSceneStubModule::moduleName() const
{
    return "instrumentsscene";
}

void InstrumentsSceneStubModule::registerExports()
{
    ioc()->registerExport<notation::ISelectInstrumentsScenario>(moduleName(), new SelectInstrumentsScenarioStub());
}

void InstrumentsSceneStubModule::resolveImports()
{
    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://instruments/select"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/Instruments/InstrumentsDialog.qml"));
    }
}

void InstrumentsSceneStubModule::registerResources()
{
    instrumentsscene_init_qrc();
}

void InstrumentsSceneStubModule::registerUiTypes()
{
    ioc()->resolve<IUiEngine>(moduleName())->addSourceImportPath(instrumentsscene_QML_IMPORT);
}
