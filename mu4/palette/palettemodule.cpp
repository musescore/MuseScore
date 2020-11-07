//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "palettemodule.h"

#include <QQmlEngine>

#include "log.h"

#include "config.h"
#include "modularity/ioc.h"
#include "ui/iuiengine.h"
#include "ui/iinteractiveuriregister.h"

#include "internal/mu4paletteadapter.h"
#include "internal/paletteconfiguration.h"
#include "internal/palette/masterpalette.h"
#include "internal/paletteactionscontroller.h"
#include "internal/paletteactions.h"

#include "view/paletterootmodel.h"
#include "view/palettepropertiesmodel.h"
#include "view/palettecellpropertiesmodel.h"

#include "workspace/iworkspacedatastreamregister.h"
#include "internal/workspacepalettestream.h"

#include "internal/paletteworkspacesetup.h"

#include "libmscore/score.h"
#include "libmscore/sym.h"

#include "actions/iactionsregister.h"

using namespace mu::palette;
using namespace mu::framework;

static std::shared_ptr<MU4PaletteAdapter> s_adapter = std::make_shared<MU4PaletteAdapter>();
static std::shared_ptr<PaletteActionsController> s_actionsController = std::make_shared<PaletteActionsController>();

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
#ifdef BUILD_UI_MU4
    framework::ioc()->registerExport<IPaletteAdapter>(moduleName(), s_adapter);
#endif

    framework::ioc()->registerExport<IPaletteConfiguration>(moduleName(), std::make_shared<PaletteConfiguration>());

    // create a score for internal use
    using namespace Ms;
    gscore = new MasterScore();
    gscore->setPaletteMode(true);
    gscore->setMovements(new Movements());
    gscore->setStyle(MScore::baseStyle());

    gscore->style().set(Sid::MusicalTextFont, QString("Bravura Text"));
    ScoreFont* scoreFont = ScoreFont::fontFactory("Bravura");
    gscore->setScoreFont(scoreFont);
    gscore->setNoteHeadWidth(scoreFont->width(SymId::noteheadBlack, gscore->spatium()) / SPATIUM20);
}

void PaletteModule::resolveImports()
{
    auto workspaceStreams = ioc()->resolve<workspace::IWorkspaceDataStreamRegister>(moduleName());
    if (workspaceStreams) {
        workspaceStreams->regStream("PaletteBox", std::make_shared<WorkspacePaletteStream>());
    }

    auto ar = framework::ioc()->resolve<actions::IActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<PaletteActions>());
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://palette/masterpalette"),
                        ContainerMeta(ContainerType::QWidgetDialog, qRegisterMetaType<Ms::MasterPalette>("MasterPallette")));

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

    qmlRegisterUncreatableType<PaletteWorkspace>("MuseScore.Palette", 1, 0, "PaletteWorkspace", "Cannot create");
    qmlRegisterUncreatableType<AbstractPaletteController>("MuseScore.Palette", 1, 0, "PaletteController", "Cannot ...");
    qmlRegisterUncreatableType<PaletteElementEditor>("MuseScore.Palette", 1, 0, "PaletteElementEditor", "Cannot ...");
    qmlRegisterUncreatableType<PaletteTreeModel>("MuseScore.Palette", 1, 0, "PaletteTreeModel",  "Cannot create");
    qmlRegisterUncreatableType<FilterPaletteTreeModel>("MuseScore.Palette", 1, 0, "FilterPaletteTreeModel", "Cannot");

    qmlRegisterType<PaletteRootModel>("MuseScore.Palette", 1, 0, "PaletteRootModel");
    qmlRegisterType<PalettePropertiesModel>("MuseScore.Palette", 1, 0, "PalettePropertiesModel");
    qmlRegisterType<PaletteCellPropertiesModel>("MuseScore.Palette", 1, 0, "PaletteCellPropertiesModel");

    framework::ioc()->resolve<framework::IUiEngine>(moduleName())->addSourceImportPath(palette_QML_IMPORT);
}

void PaletteModule::onInit()
{
    using namespace Ms;

    // load workspace
    PaletteWorkspaceSetup w;
    w.setup();

#ifdef BUILD_UI_MU4
    s_actionsController->init();
#endif
}
