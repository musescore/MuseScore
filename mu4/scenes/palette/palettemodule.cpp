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

#include "internal/mu4paletteadapter.h"
#include "internal/paletteconfiguration.h"

#include "view/paletterootmodel.h"
#include "internal/palette/paletteworkspace.h"
#include "internal/palette/palettecreator.h"

#include "workspace/iworkspacedatastreamregister.h"
#include "workspace/iworkspacemanager.h"
#include "internal/workspacepalettestream.h"

#include "libmscore/score.h"
#include "libmscore/sym.h"

using namespace mu::scene::palette;

static std::shared_ptr<MU4PaletteAdapter> m_adapter = std::make_shared<MU4PaletteAdapter>();
static std::shared_ptr<PaletteConfiguration> m_configuration = std::make_shared<PaletteConfiguration>();

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
    framework::ioc()->registerExport<IPaletteAdapter>(moduleName(), m_adapter);
#endif

    framework::ioc()->registerExport<IPaletteConfiguration>(moduleName(), m_configuration);

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
    auto workspaceStreams = framework::ioc()->resolve<workspace::IWorkspaceDataStreamRegister>(moduleName());
    if (workspaceStreams) {
        workspaceStreams->regStream("PaletteBox", std::make_shared<WorkspacePaletteStream>());
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
}

void PaletteModule::onInit()
{
    using namespace Ms;

    // init configuration
    m_configuration->init();

    // load workspace

    auto workspaceManager = framework::ioc()->resolve<workspace::IWorkspaceManager>(moduleName());
    if (!workspaceManager) {
        return;
    }

    Ms::PaletteWorkspace* paletteWorkspace = m_adapter->paletteWorkspace();
    auto applyWorkspaceData = [paletteWorkspace](std::shared_ptr<workspace::IWorkspace> w) {
                                  std::shared_ptr<workspace::AbstractData> data = w->data("PaletteBox");
                                  if (!data) {
                                      LOGE() << "no palette data in workspace: " << w->name();
                                      return false;
                                  }

                                  PaletteWorkspaceData* pdata = dynamic_cast<PaletteWorkspaceData*>(data.get());
                                  IF_ASSERT_FAILED(pdata) {
                                      return false;
                                  }

                                  paletteWorkspace->setDefaultPaletteTree(std::move(pdata->tree));
                                  return true;
                              };

    RetValCh<std::shared_ptr<workspace::IWorkspace> > workspace = workspaceManager->currentWorkspace();
    if (workspace.val) {
        bool ok = applyWorkspaceData(workspace.val);
        if (!ok) {
            std::unique_ptr<PaletteTree> tree(PaletteCreator::newDefaultPaletteTree());
            paletteWorkspace->setUserPaletteTree(std::move(tree));
        }
    }

    workspace.ch.onReceive(nullptr, [paletteWorkspace, applyWorkspaceData](std::shared_ptr<workspace::IWorkspace> w) {
        bool ok = applyWorkspaceData(w);
        if (!ok) {
            std::unique_ptr<PaletteTree> tree(PaletteCreator::newDefaultPaletteTree());
            paletteWorkspace->setUserPaletteTree(std::move(tree));
        }
    });
}
