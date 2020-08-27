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
#include "notationdomainmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "internal/notationcreator.h"
#include "internal/notation.h"
#include "internal/notationactioncontroller.h"
#include "internal/notationconfiguration.h"

#include "actions/iactionsregister.h"
#include "internal/notationactions.h"
#include "internal/notationreadersregister.h"
#include "internal/mscznotationreader.h"
#include "internal/msczmetareader.h"

#include "view/notationpaintview.h"
#include "view/notationaccessibilitymodel.h"
#include "view/zoomcontrolmodel.h"
#include "view/notationtoolbarmodel.h"
#include "view/notationswitchlistmodel.h"

#include "ui/iinteractiveuriregister.h"
#include "ui/uitypes.h"
#include "view/widgets/editstyle.h"
#include "view/widgets/measureproperties.h"

using namespace mu::domain::notation;
using namespace mu::scene::notation;
using namespace mu::framework;

static NotationConfiguration* m_configuration = new NotationConfiguration();

static void notationscene_init_qrc()
{
    Q_INIT_RESOURCE(notationscene);
}

std::string NotationDomainModule::moduleName() const
{
    return "notation";
}

void NotationDomainModule::registerExports()
{
    framework::ioc()->registerExport<INotationCreator>(moduleName(), new NotationCreator());
    framework::ioc()->registerExport<INotationConfiguration>(moduleName(), m_configuration);
    framework::ioc()->registerExport<IMsczMetaReader>(moduleName(), new MsczMetaReader());

    std::shared_ptr<INotationReadersRegister> readers = std::make_shared<NotationReadersRegister>();
    readers->reg({ "mscz", "mscx" }, std::make_shared<MsczNotationReader>());
    framework::ioc()->registerExport<INotationReadersRegister>(moduleName(), readers);

    Notation::init();
}

void NotationDomainModule::resolveImports()
{
    auto ar = framework::ioc()->resolve<actions::IActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<NotationActions>());
    }

    auto ir = framework::ioc()->resolve<framework::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://notation/style"), ContainerMeta(ContainerType::QWidgetDialog, EditStyle::metaTypeId()));

        ir->registerUri(Uri("musescore://notation/measureproperties"),
                        ContainerMeta(ContainerType::QWidgetDialog, qRegisterMetaType<MeasurePropertiesDialog>("MeasurePropertiesDialog")));
    }
}

void NotationDomainModule::registerResources()
{
    notationscene_init_qrc();
}

void NotationDomainModule::registerUiTypes()
{
    qmlRegisterType<NotationPaintView>("MuseScore.NotationScene", 1, 0, "NotationPaintView");
    qmlRegisterType<NotationToolBarModel>("MuseScore.NotationScene", 1, 0, "NotationToolBarModel");
    qmlRegisterType<NotationAccessibilityModel>("MuseScore.NotationScene", 1, 0, "NotationAccessibilityModel");
    qmlRegisterType<ZoomControlModel>("MuseScore.NotationScene", 1, 0, "ZoomControlModel");
    qmlRegisterType<NotationSwitchListModel>("MuseScore.NotationScene", 1, 0, "NotationSwitchListModel");

    qRegisterMetaType<EditStyle>("EditStyle");
}

void NotationDomainModule::onInit()
{
    NotationActionController::instance(); //! NOTE Only need to create
    m_configuration->init();
}
