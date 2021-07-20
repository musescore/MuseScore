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
#include "palettemodule.h"

#include <QQmlEngine>

#include "log.h"

#include "config.h"
#include "modularity/ioc.h"
#include "ui/iuiengine.h"
#include "ui/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

#include "internal/paletteconfiguration.h"
#include "internal/widgets/masterpalette.h"
#include "internal/widgets/specialcharactersdialog.h"
#include "internal/paletteactionscontroller.h"
#include "internal/paletteuiactions.h"
#include "internal/palette/paletteprovider.h"

#include "view/paletterootmodel.h"
#include "view/palettepropertiesmodel.h"
#include "view/palettecellpropertiesmodel.h"

#include "internal/paletteworkspacesetup.h"
#include "internal/widgets/timesignaturepropertiesdialog.h"

using namespace mu::palette;
using namespace mu::modularity;
using namespace mu::ui;

static std::shared_ptr<Ms::PaletteProvider> s_paletteProvider = std::make_shared<Ms::PaletteProvider>();
static std::shared_ptr<PaletteActionsController> s_actionsController = std::make_shared<PaletteActionsController>();
static std::shared_ptr<PaletteUiActions> s_paletteUiActions = std::make_shared<PaletteUiActions>(s_actionsController);
static std::shared_ptr<PaletteConfiguration> s_configuration = std::make_shared<PaletteConfiguration>();
static std::shared_ptr<PaletteWorkspaceSetup> s_paletteWorkspaceSetup = std::make_shared<PaletteWorkspaceSetup>();

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
    ioc()->registerExport<IPaletteProvider>(moduleName(), s_paletteProvider);
    ioc()->registerExport<IPaletteConfiguration>(moduleName(), s_configuration);
}

void PaletteModule::resolveImports()
{
    auto ar = ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(s_paletteUiActions);
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://palette/masterpalette"),
                        ContainerMeta(ContainerType::QWidgetDialog, Ms::MasterPalette::static_metaTypeId()));

        ir->registerUri(Uri("musescore://palette/specialcharacters"),
                        ContainerMeta(ContainerType::QWidgetDialog, Ms::SpecialCharactersDialog::static_metaTypeId()));

        ir->registerUri(Uri("musescore://palette/timesignatureproperties"),
                        ContainerMeta(ContainerType::QWidgetDialog, Ms::TimeSignaturePropertiesDialog::static_metaTypeId()));

        ir->registerUri(Uri("musescore://palette/properties"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/Palette/PalettePropertiesDialog.qml"));

        ir->registerUri(Uri("musescore://palette/cellproperties"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/Palette/PaletteCellPropertiesDialog.qml"));
    }
}

void PaletteModule::registerResources()
{
    palette_init_qrc();
}

void PaletteModule::registerUiTypes()
{
    using namespace Ms;

    qmlRegisterUncreatableType<PaletteProvider>("MuseScore.Palette", 1, 0, "PaletteProvider", "Cannot create");
    qmlRegisterUncreatableType<AbstractPaletteController>("MuseScore.Palette", 1, 0, "PaletteController", "Cannot ...");
    qmlRegisterUncreatableType<PaletteElementEditor>("MuseScore.Palette", 1, 0, "PaletteElementEditor", "Cannot ...");
    qmlRegisterUncreatableType<PaletteTreeModel>("MuseScore.Palette", 1, 0, "PaletteTreeModel",  "Cannot create");
    qmlRegisterUncreatableType<FilterPaletteTreeModel>("MuseScore.Palette", 1, 0, "FilterPaletteTreeModel", "Cannot");

    qmlRegisterType<PaletteRootModel>("MuseScore.Palette", 1, 0, "PaletteRootModel");
    qmlRegisterType<PalettePropertiesModel>("MuseScore.Palette", 1, 0, "PalettePropertiesModel");
    qmlRegisterType<PaletteCellPropertiesModel>("MuseScore.Palette", 1, 0, "PaletteCellPropertiesModel");

    qRegisterMetaType<SpecialCharactersDialog>("SpecialCharactersDialog");
    qRegisterMetaType<TimeSignaturePropertiesDialog>("TimeSignaturePropertiesDialog");

    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(palette_QML_IMPORT);
}

void PaletteModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (framework::IApplication::RunMode::Editor != mode) {
        return;
    }

    s_configuration->init();
    s_actionsController->init();
    s_paletteUiActions->init();
    s_paletteProvider->init();
}

void PaletteModule::onAllInited(const framework::IApplication::RunMode& mode)
{
    if (framework::IApplication::RunMode::Editor != mode) {
        return;
    }

    //! NOTE We need to be sure that the workspaces are initialized.
    //! So, we loads these settings on onAllInited
    s_paletteWorkspaceSetup->setup();
}
