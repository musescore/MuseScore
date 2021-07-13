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

#include "internal/selectinstrumentscenario.h"
#include "internal/instrumentsuiactions.h"

#include "view/instrumentspaneltreemodel.h"
#include "view/instrumentlistmodel.h"
#include "view/instrumentsettingsmodel.h"
#include "view/staffsettingsmodel.h"
#include "ui/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

#include "instrumentstypes.h"

using namespace mu::instruments;
using namespace mu::modularity;
using namespace mu::ui;

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
    ioc()->registerExport<notation::ISelectInstrumentsScenario>(moduleName(), new SelectInstrumentsScenario());
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
