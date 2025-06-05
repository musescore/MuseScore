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
#include "instrumentsscenemodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "internal/selectinstrumentscenario.h"
#include "internal/instrumentsuiactions.h"
#include "internal/instrumentsactionscontroller.h"

#include "view/instrumentlistmodel.h"
#include "view/instrumentsonscorelistmodel.h"
#include "view/instrumentsettingsmodel.h"
#include "view/staffsettingsmodel.h"
#include "view/systemobjectslayersettingsmodel.h"
#include "view/layoutpaneltreemodel.h"
#include "view/layoutpanelcontextmenumodel.h"
#include "ui/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

#include "instrumentsscenetypes.h"

using namespace mu::instrumentsscene;
using namespace muse;
using namespace muse::modularity;
using namespace muse::ui;

static void instrumentsscene_init_qrc()
{
    Q_INIT_RESOURCE(instrumentsscene);
}

std::string InstrumentsSceneModule::moduleName() const
{
    return "instrumentsscene";
}

void InstrumentsSceneModule::registerExports()
{
    m_actionsController = std::make_shared<InstrumentsActionsController>();

    ioc()->registerExport<notation::ISelectInstrumentsScenario>(moduleName(), new SelectInstrumentsScenario());
}

void InstrumentsSceneModule::resolveImports()
{
    auto ar = ioc()->resolve<IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<InstrumentsUiActions>());
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://instruments/select"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/InstrumentsScene/InstrumentsDialog.qml"));
    }
}

void InstrumentsSceneModule::registerResources()
{
    instrumentsscene_init_qrc();
}

void InstrumentsSceneModule::registerUiTypes()
{
    qmlRegisterType<InstrumentListModel>("MuseScore.InstrumentsScene", 1, 0, "InstrumentListModel");
    qmlRegisterType<InstrumentSettingsModel>("MuseScore.InstrumentsScene", 1, 0, "InstrumentSettingsModel");
    qmlRegisterType<StaffSettingsModel>("MuseScore.InstrumentsScene", 1, 0, "StaffSettingsModel");
    qmlRegisterType<SystemObjectsLayerSettingsModel>("MuseScore.InstrumentsScene", 1, 0, "SystemObjectsLayerSettingsModel");
    qmlRegisterType<LayoutPanelTreeModel>("MuseScore.InstrumentsScene", 1, 0, "LayoutPanelTreeModel");
    qmlRegisterType<LayoutPanelContextMenuModel>("MuseScore.InstrumentsScene", 1, 0, "LayoutPanelContextMenuModel");
    qmlRegisterType<InstrumentsOnScoreListModel>("MuseScore.InstrumentsScene", 1, 0, "InstrumentsOnScoreListModel");

    qmlRegisterUncreatableType<LayoutPanelItemType>("MuseScore.InstrumentsScene", 1, 0, "LayoutPanelItemType",
                                                    "Cannot create a ContainerType");

    auto uiengine = ioc()->resolve<IUiEngine>(moduleName());
    if (uiengine) {
        uiengine->addSourceImportPath(instrumentsscene_QML_IMPORT);
    }
}

void InstrumentsSceneModule::onInit(const IApplication::RunMode& mode)
{
    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }

    m_actionsController->init();
}
