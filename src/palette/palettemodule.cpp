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

#include <qqml.h>

#include "modularity/ioc.h"
#include "ui/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"
#include "accessibility/iqaccessibleinterfaceregister.h"

#include "internal/paletteconfiguration.h"
#include "internal/paletteuiactions.h"
#include "internal/paletteactionscontroller.h"
#include "internal/paletteworkspacesetup.h"
#include "internal/paletteprovider.h"
#include "internal/palettecell.h"

#include "widgets/masterpalette.h"
#include "widgets/specialcharactersdialog.h"
#include "widgets/customizekitdialog.h"
#include "widgets/timesignaturepropertiesdialog.h"
#include "widgets/keyedit.h"
#include "widgets/timedialog.h"

using namespace mu::palette;
using namespace muse;
using namespace muse::modularity;
using namespace muse::ui;
using namespace muse::accessibility;

std::string PaletteModule::moduleName() const
{
    return "palette";
}

void PaletteModule::registerExports()
{
    m_configuration = std::make_shared<PaletteConfiguration>(iocContext());

    ioc()->registerExport<IPaletteConfiguration>(moduleName(), m_configuration);
}

void PaletteModule::resolveImports()
{
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

void PaletteModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
}

void PaletteModule::registerContextExports(const muse::modularity::ContextPtr& ctx)
{
    m_paletteProvider = std::make_shared<PaletteProvider>(ctx);
    m_actionsController = std::make_shared<PaletteActionsController>(ctx);
    m_paletteUiActions = std::make_shared<PaletteUiActions>(m_actionsController, ctx);
    m_paletteWorkspaceSetup = std::make_shared<PaletteWorkspaceSetup>(ctx);

    ioc()->registerExport<IPaletteProvider>(moduleName(), m_paletteProvider);
}

void PaletteModule::resolveContextImports(const muse::modularity::ContextPtr&)
{
    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(m_paletteUiActions);
    }
}

void PaletteModule::onContextInit(const muse::IApplication::RunMode&, const muse::modularity::ContextPtr&)
{
    m_actionsController->init();
    m_paletteUiActions->init();
    m_paletteProvider->init();
}

void PaletteModule::onContextAllInited(const muse::IApplication::RunMode&, const muse::modularity::ContextPtr&)
{
    //! NOTE We need to be sure that the workspaces are initialized.
    //! So, we loads these settings on onAllInited
    m_paletteWorkspaceSetup->setup();
}

void PaletteModule::onAllInited(const IApplication::RunMode&)
{
}

void PaletteModule::onDeinit()
{
    m_paletteWorkspaceSetup.reset();
    m_configuration.reset();
    m_paletteUiActions.reset();

    ioc()->unregisterIfRegistered<IPaletteProvider>(moduleName(), m_paletteProvider);
    m_paletteProvider.reset();
}
