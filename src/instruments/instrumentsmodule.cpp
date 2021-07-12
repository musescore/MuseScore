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
#include "instrumentsmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "internal/instrumentsrepository.h"
#include "internal/instrumentsconfiguration.h"
#include "internal/selectinstrumentscenario.h"
#include "internal/instrumentsuiactions.h"

#include "view/instrumentspaneltreemodel.h"
#include "view/instrumentlistmodel.h"
#include "view/instrumentsettingsmodel.h"
#include "view/staffsettingsmodel.h"
#include "ui/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"
#include "instrumentstypes.h"

#include "diagnostics/idiagnosticspathsregister.h"

using namespace mu::instruments;
using namespace mu::modularity;
using namespace mu::ui;

static std::shared_ptr<InstrumentsRepository> s_instrumentsRepository = std::make_shared<InstrumentsRepository>();
static std::shared_ptr<InstrumentsConfiguration> s_configuration = std::make_shared<InstrumentsConfiguration>();

static void instruments_init_qrc()
{
    Q_INIT_RESOURCE(instruments);
}

std::string InstrumentsModule::moduleName() const
{
    return "instruments";
}

void InstrumentsModule::registerExports()
{
    ioc()->registerExport<IInstrumentsConfiguration>(moduleName(), s_configuration);
    ioc()->registerExport<IInstrumentsRepository>(moduleName(), s_instrumentsRepository);
    ioc()->registerExport<ISelectInstrumentsScenario>(moduleName(), new SelectInstrumentsScenario());
}

void InstrumentsModule::resolveImports()
{
    auto ar = modularity::ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<InstrumentsUiActions>());
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://instruments/select"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/Instruments/InstrumentsDialog.qml"));
    }
}

void InstrumentsModule::registerResources()
{
    instruments_init_qrc();
}

void InstrumentsModule::registerUiTypes()
{
    qmlRegisterType<InstrumentListModel>("MuseScore.Instruments", 1, 0, "InstrumentListModel");
    qmlRegisterType<InstrumentSettingsModel>("MuseScore.Instruments", 1, 0, "InstrumentSettingsModel");
    qmlRegisterType<StaffSettingsModel>("MuseScore.Instruments", 1, 0, "StaffSettingsModel");
    qmlRegisterType<InstrumentsPanelTreeModel>("MuseScore.Instruments", 1, 0, "InstrumentsPanelTreeModel");
    qmlRegisterUncreatableType<InstrumentsTreeItemType>("MuseScore.Instruments", 1, 0, "InstrumentsTreeItemType",
                                                        "Cannot create a ContainerType");

    auto uiengine = modularity::ioc()->resolve<ui::IUiEngine>(moduleName());
    if (uiengine) {
        uiengine->addSourceImportPath(instruments_QML_IMPORT);
    }
}

void InstrumentsModule::onInit(const framework::IApplication::RunMode&)
{
    s_configuration->init();
    s_instrumentsRepository->init();

    auto pr = modularity::ioc()->resolve<diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        io::paths instrPaths = s_configuration->instrumentListPaths();
        for (const io::path& p : instrPaths) {
            pr->reg("instruments", p);
        }

        io::paths uinstrPaths = s_configuration->userInstrumentListPaths();
        for (const io::path& p : uinstrPaths) {
            pr->reg("user instruments", p);
        }

        io::paths scoreOrderPaths = s_configuration->scoreOrderListPaths();
        for (const io::path& p : scoreOrderPaths) {
            pr->reg("scoreOrder", p);
        }

        io::paths uscoreOrderPaths = s_configuration->userScoreOrderListPaths();
        for (const io::path& p : uscoreOrderPaths) {
            pr->reg("user scoreOrder", p);
        }
    }
}
