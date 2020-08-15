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
#include "notationscenemodule.h"

#include "modularity/ioc.h"
#include "internal/scenenotationconfiguration.h"
#include "view/notationpaintview.h"
#include "view/notationaccessibilitymodel.h"
#include "view/zoomcontrolmodel.h"
#include "view/notationtoolbarmodel.h"
#include "view/notationtoolbarmodel.h"
#include "ui/iinteractiveuriregister.h"
#include "ui/uitypes.h"
#include "internal/widgets/editstyle.h"
#include "view/notationswitchlistmodel.h"

using namespace mu::scene::notation;
using namespace mu::framework;

static SceneNotationConfiguration* m_configuration = new SceneNotationConfiguration();

static void notationscene_init_qrc()
{
    Q_INIT_RESOURCE(notationscene);
}

std::string NotationSceneModule::moduleName() const
{
    return "notation_scene";
}

void NotationSceneModule::registerExports()
{
    framework::ioc()->registerExport<ISceneNotationConfiguration>(moduleName(), m_configuration);
}

void NotationSceneModule::resolveImports()
{
    auto ir = framework::ioc()->resolve<framework::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://notation/style"),
                        ContainerMeta(ContainerType::QWidgetDialog, EditStyle::metaTypeId()));
    }
}

void NotationSceneModule::registerResources()
{
    notationscene_init_qrc();
}

void NotationSceneModule::registerUiTypes()
{
    qmlRegisterType<NotationPaintView>("MuseScore.NotationScene", 1, 0, "NotationPaintView");
    qmlRegisterType<NotationToolBarModel>("MuseScore.NotationScene", 1, 0, "NotationToolBarModel");
    qmlRegisterType<NotationAccessibilityModel>("MuseScore.NotationScene", 1, 0, "NotationAccessibilityModel");
    qmlRegisterType<ZoomControlModel>("MuseScore.NotationScene", 1, 0, "ZoomControlModel");
    qmlRegisterType<NotationSwitchListModel>("MuseScore.NotationScene", 1, 0, "NotationSwitchListModel");

    qRegisterMetaType<EditStyle>("EditStyle");
}

void NotationSceneModule::onInit()
{
    m_configuration->init();
}
