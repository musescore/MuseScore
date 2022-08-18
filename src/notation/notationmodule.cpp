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
#include "notationmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"
#include "ui/iuiactionsregister.h"
#include "project/inotationwritersregister.h"

#include "internal/notation.h"
#include "internal/notationactioncontroller.h"
#include "internal/notationconfiguration.h"
#include "internal/midiinputoutputcontroller.h"
#include "internal/notationuiactions.h"
#include "internal/positionswriter.h"
#include "internal/mscnotationwriter.h"
#include "internal/instrumentsrepository.h"

#include "view/notationpaintview.h"
#include "view/notationswitchlistmodel.h"
#include "view/partlistmodel.h"
#include "view/searchpopupmodel.h"
#include "view/noteinputbarmodel.h"
#include "view/noteinputbarcustomisemodel.h"
#include "view/noteinputbarcustomiseitem.h"
#include "view/internal/undoredomodel.h"
#include "view/notationtoolbarmodel.h"
#include "view/notationnavigator.h"
#include "view/selectionfiltermodel.h"
#include "view/editgridsizedialogmodel.h"

#include "view/pianokeyboard/pianokeyboardview.h"
#include "view/pianokeyboard/pianokeyboardpanelcontextmenumodel.h"

#include "ui/iinteractiveuriregister.h"
#include "ui/uitypes.h"
#include "view/widgets/editstyle.h"
#include "view/widgets/measureproperties.h"
#include "view/widgets/editstaff.h"
#include "view/widgets/breaksdialog.h"
#include "view/widgets/pagesettings.h"
#include "view/widgets/transposedialog.h"
#include "view/widgets/selectnotedialog.h"
#include "view/widgets/selectdialog.h"
#include "view/widgets/tupletdialog.h"
#include "view/widgets/stafftextpropertiesdialog.h"
#include "view/widgets/timelineview.h"
#include "view/widgets/realizeharmonydialog.h"
#include "view/notationcontextmenumodel.h"
#include "view/internal/undoredomodel.h"

#include "view/styledialog/styleitem.h"
#include "view/styledialog/notespagemodel.h"
#include "view/styledialog/beamspagemodel.h"

#include "diagnostics/idiagnosticspathsregister.h"

using namespace mu::notation;
using namespace mu::modularity;
using namespace mu::ui;
using namespace mu::actions;
using namespace mu::uicomponents;

static std::shared_ptr<NotationConfiguration> s_configuration = std::make_shared<NotationConfiguration>();
static std::shared_ptr<NotationActionController> s_actionController = std::make_shared<NotationActionController>();
static std::shared_ptr<NotationUiActions> s_notationUiActions = std::make_shared<NotationUiActions>(s_actionController);
static std::shared_ptr<MidiInputOutputController> s_midiInputOutputController = std::make_shared<MidiInputOutputController>();
static std::shared_ptr<InstrumentsRepository> s_instrumentsRepository = std::make_shared<InstrumentsRepository>();

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
    ioc()->registerExport<INotationConfiguration>(moduleName(), s_configuration);
    ioc()->registerExport<IInstrumentsRepository>(moduleName(), s_instrumentsRepository);
}

void NotationModule::resolveImports()
{
    auto ar = ioc()->resolve<IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(s_notationUiActions);
    }

    auto writers = modularity::ioc()->resolve<project::INotationWritersRegister>(moduleName());
    if (writers) {
        writers->reg({ "sposXML" }, std::make_shared<PositionsWriter>(PositionsWriter::ElementType::SEGMENT));
        writers->reg({ "mposXML" }, std::make_shared<PositionsWriter>(PositionsWriter::ElementType::MEASURE));
        writers->reg({ "mscz" }, std::make_shared<MscNotationWriter>(engraving::MscIoMode::Zip));
        writers->reg({ "mscx" }, std::make_shared<MscNotationWriter>(engraving::MscIoMode::Dir));
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://notation/style"),
                        ContainerMeta(ContainerType::QWidgetDialog, qRegisterMetaType<EditStyle>("EditStyle")));

        ir->registerUri(Uri("musescore://notation/pagesettings"),
                        ContainerMeta(ContainerType::QWidgetDialog, qRegisterMetaType<PageSettings>("PageSettings")));

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

        ir->registerUri(Uri("musescore://notation/stafftextproperties"),
                        ContainerMeta(ContainerType::QWidgetDialog, StaffTextPropertiesDialog::static_metaTypeId()));

        ir->registerUri(Uri("musescore://notation/parts"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/NotationScene/PartsDialog.qml"));

        ir->registerUri(Uri("musescore://notation/selectmeasurescount"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/NotationScene/SelectMeasuresCountDialog.qml"));

        ir->registerUri(Uri("musescore://notation/editgridsize"),
                        ContainerMeta(ContainerType::QmlDialog, "MuseScore/NotationScene/EditGridSizeDialog.qml"));

        ir->registerUri(Uri("musescore://notation/realizechordsymbols"),
                        ContainerMeta(ContainerType::QWidgetDialog, qRegisterMetaType<RealizeHarmonyDialog>("RealizeHarmonyDialog")));
    }
}

void NotationModule::registerResources()
{
    notationscene_init_qrc();
}

void NotationModule::registerUiTypes()
{
    qmlRegisterUncreatableType<AbstractNotationPaintView>("MuseScore.NotationScene", 1, 0, "AbstractNotationPaintView",
                                                          "Not creatable as it is an abstract type");
    qmlRegisterType<NotationPaintView>("MuseScore.NotationScene", 1, 0, "NotationPaintView");
    qmlRegisterType<NotationContextMenuModel>("MuseScore.NotationScene", 1, 0, "NotationContextMenuModel");
    qmlRegisterType<NotationSwitchListModel>("MuseScore.NotationScene", 1, 0, "NotationSwitchListModel");
    qmlRegisterType<PartListModel>("MuseScore.NotationScene", 1, 0, "PartListModel");
    qmlRegisterType<SearchPopupModel>("MuseScore.NotationScene", 1, 0, "SearchPopupModel");
    qmlRegisterType<NoteInputBarModel>("MuseScore.NotationScene", 1, 0, "NoteInputBarModel");
    qmlRegisterType<NoteInputBarCustomiseModel>("MuseScore.NotationScene", 1, 0, "NoteInputBarCustomiseModel");
    qmlRegisterType<NotationToolBarModel>("MuseScore.NotationScene", 1, 0, "NotationToolBarModel");
    qmlRegisterType<NotationNavigator>("MuseScore.NotationScene", 1, 0, "NotationNavigator");
    qmlRegisterType<UndoRedoModel>("MuseScore.NotationScene", 1, 0, "UndoRedoModel");
    qmlRegisterType<TimelineView>("MuseScore.NotationScene", 1, 0, "TimelineView");
    qmlRegisterType<SelectionFilterModel>("MuseScore.NotationScene", 1, 0, "SelectionFilterModel");
    qmlRegisterType<EditGridSizeDialogModel>("MuseScore.NotationScene", 1, 0, "EditGridSizeDialogModel");
    qmlRegisterType<PianoKeyboardView>("MuseScore.NotationScene", 1, 0, "PianoKeyboardView");
    qmlRegisterType<PianoKeyboardPanelContextMenuModel>("MuseScore.NotationScene", 1, 0, "PianoKeyboardPanelContextMenuModel");

    qmlRegisterUncreatableType<StyleItem>("MuseScore.NotationScene", 1, 0, "StyleItem", "Cannot create StyleItem from QML");
    qmlRegisterType<NotesPageModel>("MuseScore.NotationScene", 1, 0, "NotesPageModel");
    qmlRegisterType<BeamsPageModel>("MuseScore.NotationScene", 1, 0, "BeamsPageModel");

    qRegisterMetaType<EditStyle>("EditStyle");
    qRegisterMetaType<EditStaff>("EditStaff");
    qRegisterMetaType<SelectNoteDialog>("SelectNoteDialog");
    qRegisterMetaType<SelectDialog>("SelectDialog");
    qRegisterMetaType<StaffTextPropertiesDialog>("StaffTextPropertiesDialog");

    qmlRegisterUncreatableType<NoteInputBarCustomiseItem>("MuseScore.NotationScene", 1, 0, "NoteInputBarCustomiseItem", "Cannot create");

    auto ui = ioc()->resolve<IUiEngine>(moduleName());
    if (ui) {
        ui->addSourceImportPath(notation_QML_IMPORT);
    }
}

void NotationModule::onInit(const framework::IApplication::RunMode& mode)
{
    s_configuration->init();
    s_instrumentsRepository->init();
    s_actionController->init();
    s_notationUiActions->init();

    if (mode == framework::IApplication::RunMode::Editor) {
        s_midiInputOutputController->init();
    }

    Notation::init();

    auto pr = modularity::ioc()->resolve<diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        io::paths_t instrPaths = s_configuration->instrumentListPaths();
        for (const io::path_t& p : instrPaths) {
            pr->reg("instruments", p);
        }

        io::paths_t uinstrPaths = s_configuration->userInstrumentListPaths();
        for (const io::path_t& p : uinstrPaths) {
            pr->reg("user instruments", p);
        }

        io::paths_t scoreOrderPaths = s_configuration->scoreOrderListPaths();
        for (const io::path_t& p : scoreOrderPaths) {
            pr->reg("scoreOrder", p);
        }

        io::paths_t uscoreOrderPaths = s_configuration->userScoreOrderListPaths();
        for (const io::path_t& p : uscoreOrderPaths) {
            pr->reg("user scoreOrder", p);
        }
    }
}
