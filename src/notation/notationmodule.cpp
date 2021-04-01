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
#include "notationmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"
#include "ui/iuiactionsregister.h"

#include "internal/notationcreator.h"
#include "internal/notation.h"
#include "internal/notationactioncontroller.h"
#include "internal/notationconfiguration.h"
#include "internal/midiinputcontroller.h"
#include "internal/notationuiactions.h"
#include "internal/notationreadersregister.h"
#include "internal/notationwritersregister.h"
#include "internal/mscznotationreader.h"
#include "internal/msczmetareader.h"

#include "view/notationpaintview.h"
#include "view/notationaccessibilitymodel.h"
#include "view/zoomcontrolmodel.h"
#include "view/concertpitchcontrolmodel.h"
#include "view/viewmodecontrolmodel.h"
#include "view/notationswitchlistmodel.h"
#include "view/partlistmodel.h"
#include "view/searchpopupmodel.h"
#include "view/noteinputbarmodel.h"
#include "view/noteinputbarcustomisemodel.h"
#include "view/internal/abstractnoteinputbaritem.h"
#include "view/internal/undoredomodel.h"
#include "view/notationtoolbarmodel.h"
#include "view/notationnavigator.h"

#include "ui/iinteractiveuriregister.h"
#include "ui/uitypes.h"
#include "view/widgets/editstyle.h"
#include "view/widgets/measureproperties.h"
#include "view/widgets/editstaff.h"
#include "view/widgets/breaksdialog.h"
#include "view/widgets/scoreproperties.h"
#include "view/widgets/transposedialog.h"
#include "view/widgets/selectnotedialog.h"
#include "view/widgets/selectdialog.h"
#include "view/widgets/tupletdialog.h"
#include "view/notationcontextmenu.h"
#include "view/internal/undoredomodel.h"

using namespace mu::notation;
using namespace mu::framework;
using namespace mu::ui;
using namespace mu::actions;
using namespace mu::uicomponents;

static std::shared_ptr<NotationConfiguration> s_configuration = std::make_shared<NotationConfiguration>();
static std::shared_ptr<NotationActionController> s_actionController = std::make_shared<NotationActionController>();
static std::shared_ptr<NotationUiActions> s_notationUiActions = std::make_shared<NotationUiActions>(s_actionController);
static std::shared_ptr<MidiInputController> s_midiInputController = std::make_shared<MidiInputController>();

static void notationscene_init_qrc()
{
    Q_INIT_RESOURCE(notationscene);
}

std::string NotationModule::moduleName() const
{
    return "notation";
}

void NotationModule::registerExports()
{
    ioc()->registerExport<INotationCreator>(moduleName(), new NotationCreator());
    ioc()->registerExport<INotationConfiguration>(moduleName(), s_configuration);
    ioc()->registerExport<IMsczMetaReader>(moduleName(), new MsczMetaReader());
    ioc()->registerExport<INotationContextMenu>(moduleName(), new NotationContextMenu());

    std::shared_ptr<INotationReadersRegister> readers = std::make_shared<NotationReadersRegister>();
    readers->reg({ "mscz", "mscx" }, std::make_shared<MsczNotationReader>());
    ioc()->registerExport<INotationReadersRegister>(moduleName(), readers);
    ioc()->registerExport<INotationWritersRegister>(moduleName(), std::make_shared<NotationWritersRegister>());
}

void NotationModule::resolveImports()
{
    auto ar = ioc()->resolve<IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(s_notationUiActions);
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://notation/style"),
                        ContainerMeta(ContainerType::QWidgetDialog, qRegisterMetaType<EditStyle>("EditStyle")));

        ir->registerUri(Uri("musescore://notation/properties"),
                        ContainerMeta(ContainerType::QWidgetDialog, qRegisterMetaType<ScorePropertiesDialog>("ScorePropertiesDialog")));

        ir->registerUri(Uri("musescore://notation/measureproperties"),
                        ContainerMeta(ContainerType::QWidgetDialog, qRegisterMetaType<MeasurePropertiesDialog>("MeasurePropertiesDialog")));

        ir->registerUri(Uri("musescore://notation/breaks"),
                        ContainerMeta(ContainerType::QWidgetDialog, qRegisterMetaType<BreaksDialog>("BreaksDialog")));

        ir->registerUri(Uri("musescore://notation/staffproperties"),
                        ContainerMeta(ContainerType::QWidgetDialog, EditStaff::metaTypeId()));

        ir->registerUri(Uri("musescore://notation/transpose"),
                        ContainerMeta(ContainerType::QWidgetDialog, qRegisterMetaType<TransposeDialog>("TransposeDialog")));

        ir->registerUri(Uri("musescore://notation/selectnote"),
                        ContainerMeta(ContainerType::QWidgetDialog, SelectNoteDialog::metaTypeId()));

        ir->registerUri(Uri("musescore://notation/selectelement"),
                        ContainerMeta(ContainerType::QWidgetDialog, SelectDialog::metaTypeId()));

        ir->registerUri(Uri("musescore://notation/othertupletdialog"),
                        ContainerMeta(ContainerType::QWidgetDialog, qRegisterMetaType<TupletDialog>("TupletDialog")));

        ir->registerUri(Uri("musescore://notation/parts"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/NotationScene/PartsDialog.qml"));

        ir->registerUri(Uri("musescore://notation/noteinputbar/customise"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/NotationScene/NoteInputBarCustomiseDialog.qml"));

        ir->registerUri(Uri("musescore://notation/selectmeasurescount"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/NotationScene/SelectMeasuresCountDialog.qml"));
    }
}

void NotationModule::registerResources()
{
    notationscene_init_qrc();
}

void NotationModule::registerUiTypes()
{
    qmlRegisterType<NotationPaintView>("MuseScore.NotationScene", 1, 0, "NotationPaintView");
    qmlRegisterType<NotationAccessibilityModel>("MuseScore.NotationScene", 1, 0, "NotationAccessibilityModel");
    qmlRegisterType<ZoomControlModel>("MuseScore.NotationScene", 1, 0, "ZoomControlModel");
    qmlRegisterType<ConcertPitchControlModel>("MuseScore.NotationScene", 1, 0, "ConcertPitchControlModel");
    qmlRegisterType<ViewModeControlModel>("MuseScore.NotationScene", 1, 0, "ViewModeControlModel");
    qmlRegisterType<NotationSwitchListModel>("MuseScore.NotationScene", 1, 0, "NotationSwitchListModel");
    qmlRegisterType<PartListModel>("MuseScore.NotationScene", 1, 0, "PartListModel");
    qmlRegisterType<SearchPopupModel>("MuseScore.NotationScene", 1, 0, "SearchPopupModel");
    qmlRegisterType<NoteInputBarModel>("MuseScore.NotationScene", 1, 0, "NoteInputBarModel");
    qmlRegisterType<NoteInputBarCustomiseModel>("MuseScore.NotationScene", 1, 0, "NoteInputBarCustomiseModel");
    qmlRegisterType<AbstractNoteInputBarItem>("MuseScore.NotationScene", 1, 0, "NoteInputBarItem");
    qmlRegisterType<NotationToolBarModel>("MuseScore.NotationScene", 1, 0, "NotationToolBarModel");
    qmlRegisterType<NotationNavigator>("MuseScore.NotationScene", 1, 0, "NotationNavigator");
    qmlRegisterType<UndoRedoModel>("MuseScore.NotationScene", 1, 0, "UndoRedoModel");

    qRegisterMetaType<EditStyle>("EditStyle");
    qRegisterMetaType<EditStaff>("EditStaff");
    qRegisterMetaType<SelectNoteDialog>("SelectNoteDialog");
    qRegisterMetaType<SelectDialog>("SelectDialog");

    auto ui = ioc()->resolve<IUiEngine>(moduleName());
    if (ui) {
        ui->addSourceImportPath(notation_QML_IMPORT);
    }

    Ms::MScore::registerUiTypes();
}

void NotationModule::onInit(const IApplication::RunMode&)
{
    s_configuration->init();
    s_actionController->init();
    s_notationUiActions->init();
    s_midiInputController->init();

    Notation::init();
}
