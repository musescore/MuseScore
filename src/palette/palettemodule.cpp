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
#include "palettemodule.h"

#include <QQmlEngine>

#include "log.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"
#include "ui/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"
#include "accessibility/iqaccessibleinterfaceregister.h"

#include "internal/paletteconfiguration.h"
#include "internal/paletteuiactions.h"
#include "internal/paletteactionscontroller.h"
#include "internal/paletteworkspacesetup.h"
#include "internal/paletteprovider.h"
#include "internal/palettecell.h"

#include "view/paletterootmodel.h"
#include "view/palettepropertiesmodel.h"
#include "view/palettecellpropertiesmodel.h"
#include "view/palettespanelcontextmenumodel.h"
#include "view/drumsetpanelview.h"

#include "view/widgets/masterpalette.h"
#include "view/widgets/specialcharactersdialog.h"
#include "view/widgets/customizekitdialog.h"
#include "view/widgets/timesignaturepropertiesdialog.h"
#include "view/widgets/keyedit.h"
#include "view/widgets/timedialog.h"

using namespace mu::palette;
using namespace muse;
using namespace muse::modularity;
using namespace muse::ui;
using namespace muse::accessibility;

static void palette_init_qrc()
{
    Q_INIT_RESOURCE(palette);
}

std::string PaletteModule::moduleName() const
{
    return "palette";
}

void PaletteModule::registerExports()
{
    m_paletteProvider = std::make_shared<PaletteProvider>();
    m_actionsController = std::make_shared<PaletteActionsController>();
    m_paletteUiActions = std::make_shared<PaletteUiActions>(m_actionsController);
    m_configuration = std::make_shared<PaletteConfiguration>();
    m_paletteWorkspaceSetup = std::make_shared<PaletteWorkspaceSetup>();

    ioc()->registerExport<IPaletteProvider>(moduleName(), m_paletteProvider);
    ioc()->registerExport<IPaletteConfiguration>(moduleName(), m_configuration);
}

void PaletteModule::resolveImports()
{
    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(m_paletteUiActions);
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerWidgetUri<MasterPalette>(Uri("musescore://palette/masterpalette"));
        ir->registerWidgetUri<SpecialCharactersDialog>(Uri("musescore://palette/specialcharacters"));
        ir->registerWidgetUri<TimeSignaturePropertiesDialog>(Uri("musescore://palette/timesignatureproperties"));
        ir->registerWidgetUri<CustomizeKitDialog>(Uri("musescore://palette/customizekit"));
        ir->registerWidgetUri<KeyEditor>(Uri("musescore://notation/keysignatures"));
        ir->registerWidgetUri<TimeDialog>(Uri("musescore://notation/timesignatures"));

        ir->registerQmlUri(Uri("musescore://palette/properties"), "MuseScore/Palette/PalettePropertiesDialog.qml");
        ir->registerQmlUri(Uri("musescore://palette/cellproperties"), "MuseScore/Palette/PaletteCellPropertiesDialog.qml");
    }

    auto accr = ioc()->resolve<IQAccessibleInterfaceRegister>(moduleName());
    if (accr) {
        accr->registerInterfaceGetter("mu::palette::PaletteWidget", PaletteWidget::accessibleInterface);
        accr->registerInterfaceGetter("mu::palette::PaletteCell", PaletteCell::accessibleInterface);
    }
}

void PaletteModule::registerResources()
{
    palette_init_qrc();
}

void PaletteModule::registerUiTypes()
{
    using namespace mu::engraving;

    qmlRegisterUncreatableType<PaletteProvider>("MuseScore.Palette", 1, 0, "PaletteProvider", "Cannot create");
    qmlRegisterUncreatableType<AbstractPaletteController>("MuseScore.Palette", 1, 0, "PaletteController", "Cannot …");
    qmlRegisterUncreatableType<PaletteElementEditor>("MuseScore.Palette", 1, 0, "PaletteElementEditor", "Cannot …");
    qmlRegisterUncreatableType<PaletteTreeModel>("MuseScore.Palette", 1, 0, "PaletteTreeModel",  "Cannot create");
    qmlRegisterUncreatableType<FilterPaletteTreeModel>("MuseScore.Palette", 1, 0, "FilterPaletteTreeModel", "Cannot");
    qmlRegisterType<PalettesPanelContextMenuModel>("MuseScore.Palette", 1, 0, "PalettesPanelContextMenuModel");

    qmlRegisterType<PaletteRootModel>("MuseScore.Palette", 1, 0, "PaletteRootModel");
    qmlRegisterType<PalettePropertiesModel>("MuseScore.Palette", 1, 0, "PalettePropertiesModel");
    qmlRegisterType<PaletteCellPropertiesModel>("MuseScore.Palette", 1, 0, "PaletteCellPropertiesModel");
    qmlRegisterType<DrumsetPanelView>("MuseScore.Palette", 1, 0, "DrumsetPanelView");

    ioc()->resolve<muse::ui::IUiEngine>(moduleName())->addSourceImportPath(palette_QML_IMPORT);
}

void PaletteModule::onInit(const IApplication::RunMode& mode)
{
    if (IApplication::RunMode::GuiApp != mode) {
        return;
    }

    m_configuration->init();
    m_actionsController->init();
    m_paletteUiActions->init();
    m_paletteProvider->init();
}

void PaletteModule::onAllInited(const IApplication::RunMode& mode)
{
    if (IApplication::RunMode::GuiApp != mode) {
        return;
    }

    //! NOTE We need to be sure that the workspaces are initialized.
    //! So, we loads these settings on onAllInited
    m_paletteWorkspaceSetup->setup();
}

void PaletteModule::onDeinit()
{
    m_paletteWorkspaceSetup.reset();
    m_configuration.reset();
    m_paletteUiActions.reset();

    ioc()->unregisterIfRegistered<IPaletteProvider>(moduleName(), m_paletteProvider);
    m_paletteProvider.reset();
}
