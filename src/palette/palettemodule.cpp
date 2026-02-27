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
#include "interactive/iinteractiveuriregister.h"
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

static const std::string mname("palette");

std::string PaletteModule::moduleName() const
{
    return mname;
}

void PaletteModule::registerExports()
{
    m_configuration = std::make_shared<PaletteConfiguration>(globalCtx());

    globalIoc()->registerExport<IPaletteConfiguration>(mname, m_configuration);
}

void PaletteModule::resolveImports()
{
    auto ir = globalIoc()->resolve<muse::interactive::IInteractiveUriRegister>(mname);
    if (ir) {
        ir->registerWidgetUri<MasterPalette>(Uri("musescore://palette/masterpalette"));
        ir->registerWidgetUri<SpecialCharactersDialog>(Uri("musescore://palette/specialcharacters"));
        ir->registerWidgetUri<TimeSignaturePropertiesDialog>(Uri("musescore://palette/timesignatureproperties"));
        ir->registerWidgetUri<CustomizeKitDialog>(Uri("musescore://palette/customizekit"));
        ir->registerWidgetUri<KeyEditor>(Uri("musescore://notation/keysignatures"));
        ir->registerWidgetUri<TimeDialog>(Uri("musescore://notation/timesignatures"));

        ir->registerQmlUri(Uri("musescore://palette/properties"), "MuseScore.Palette", "PalettePropertiesDialog");
        ir->registerQmlUri(Uri("musescore://palette/cellproperties"), "MuseScore.Palette", "PaletteCellPropertiesDialog");
    }

    auto accr = globalIoc()->resolve<muse::accessibility::IQAccessibleInterfaceRegister>(mname);
    if (accr) {
        accr->registerInterfaceGetter("mu::palette::PaletteWidget", PaletteWidget::accessibleInterface);
        accr->registerInterfaceGetter("mu::palette::PaletteCell", PaletteCell::accessibleInterface);
    }
}

void PaletteModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
}

IContextSetup* PaletteModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new PaletteContext(ctx);
}

void PaletteContext::registerExports()
{
    m_paletteProvider = std::make_shared<PaletteProvider>(iocContext());
    m_actionsController = std::make_shared<PaletteActionsController>(iocContext());
    m_paletteUiActions = std::make_shared<PaletteUiActions>(m_actionsController, iocContext());
    m_paletteWorkspaceSetup = std::make_shared<PaletteWorkspaceSetup>(iocContext());

    ioc()->registerExport<IPaletteProvider>(mname, m_paletteProvider);
}

void PaletteContext::resolveImports()
{
    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(mname);
    if (ar) {
        ar->reg(m_paletteUiActions);
    }
}

void PaletteContext::onInit(const IApplication::RunMode&)
{
    m_actionsController->init();
    m_paletteUiActions->init();
    m_paletteProvider->init();
}

void PaletteContext::onAllInited(const IApplication::RunMode&)
{
    //! NOTE We need to be sure that the workspaces are initialized.
    //! So, we loads these settings on onAllInited
    m_paletteWorkspaceSetup->setup();
}

void PaletteContext::onDeinit()
{
    m_paletteWorkspaceSetup.reset();
    m_paletteUiActions.reset();

    ioc()->unregisterIfRegistered<IPaletteProvider>(mname, m_paletteProvider);
    m_paletteProvider.reset();
}
