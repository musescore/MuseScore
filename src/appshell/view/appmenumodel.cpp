//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "appmenumodel.h"

#include "translation.h"

#include "log.h"

#include "notation/internal/addmenucontroller.h"
#include "filemenucontroller.h"

using namespace mu::appshell;
using namespace mu::uicomponents;
using namespace mu::notation;
using namespace mu::workspace;
using namespace mu::actions;

AppMenuModel::AppMenuModel(QObject* parent)
    : QObject(parent)
{
    m_addMenuController = new AddMenuController();
    m_fileMenuController = new FileMenuController();
}

void AppMenuModel::load()
{
    TRACEFUNC;

    m_items.clear();

    m_items << fileItem()
            << editItem()
            << viewItem()
            << addItem()
            << formatItem()
            << toolsItem()
            << helpItem();

    setupConnections();

    emit itemsChanged();
}

void AppMenuModel::handleAction(const QString& actionCodeStr, int actionIndex)
{
    ActionCode actionCode = codeFromQString(actionCodeStr);
    MenuItem menuItem = actionIndex == -1 ? item(m_items, actionCode) : itemByIndex(actionCode, actionIndex);

    actionsDispatcher()->dispatch(actionCode, menuItem.args);
}

QVariantList AppMenuModel::items()
{
    QVariantList menuItems;

    for (const MenuItem& menuItem: m_items) {
        menuItems << menuItem.toMap();
    }

    return menuItems;
}

IMasterNotationPtr AppMenuModel::currentMasterNotation() const
{
    return globalContext()->currentMasterNotation();
}

INotationPtr AppMenuModel::currentNotation() const
{
    return currentMasterNotation() ? currentMasterNotation()->notation() : nullptr;
}

INotationInteractionPtr AppMenuModel::notationInteraction() const
{
    return currentNotation() ? currentNotation()->interaction() : nullptr;
}

INotationSelectionPtr AppMenuModel::notationSelection() const
{
    return notationInteraction() ? notationInteraction()->selection() : nullptr;
}

void AppMenuModel::setupConnections()
{
    m_addMenuController->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        for (const ActionCode& actionCode: actionCodes) {
            MenuItem& actionItem = item(m_items, actionCode);
            actionItem.enabled = m_addMenuController->isActionAvailable(actionCode);
        }

        emit itemsChanged();
    });

    m_fileMenuController->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        for (const ActionCode& actionCode: actionCodes) {
            MenuItem& actionItem = item(m_items, actionCode);
            actionItem.enabled = m_fileMenuController->isActionAvailable(actionCode);
        }

        emit itemsChanged();
    });

    globalContext()->currentMasterNotationChanged().onNotify(this, [this]() {
        load();

        currentMasterNotation()->notation()->notationChanged().onNotify(this, [this]() {
            load();
        });

        currentMasterNotation()->notation()->interaction()->selectionChanged().onNotify(this, [this]() {
            load();
        });
    });

    userScoresService()->recentScoreList().ch.onReceive(this, [this](const MetaList&) {
        load();
    });

    paletteController()->isMasterPaletteOpened().ch.onReceive(this, [this](bool) {
        load();
    });

    notationPageState()->panelVisibleChanged().onReceive(this, [this](PanelType) {
        load();
    });

    controller()->isFullScreen().ch.onReceive(this, [this](bool) {
        load();
    });

    interactive()->currentUri().ch.onReceive(this, [this](const Uri&) {
        load();
    });
}

MenuItem& AppMenuModel::item(MenuItemList& items, const ActionCode& actionCode)
{
    for (MenuItem& menuItem : items) {
        if (menuItem.code == actionCode) {
            return menuItem;
        }

        if (!menuItem.subitems.empty()) {
            MenuItem& subitem = item(menuItem.subitems, actionCode);
            if (subitem.code == actionCode) {
                return subitem;
            }
        }
    }

    static MenuItem null;
    return null;
}

MenuItem& AppMenuModel::itemByIndex(const ActionCode& menuActionCode, int actionIndex)
{
    MenuItem& menuItem = menu(m_items, menuActionCode);
    MenuItemList& subitems = menuItem.subitems;
    for (int i = 0; i < subitems.size(); ++i) {
        if (i == actionIndex) {
            return subitems[i];
        }
    }

    static MenuItem null;
    return null;
}

MenuItem& AppMenuModel::menu(MenuItemList& items, const ActionCode& subitemsActionCode)
{
    for (MenuItem& item : items) {
        if (item.subitems.isEmpty()) {
            continue;
        }

        if (item.code == subitemsActionCode) {
            return item;
        }

        MenuItem& menuItem = menu(item.subitems, subitemsActionCode);
        if (menuItem.isValid()) {
            return menuItem;
        }
    }

    static MenuItem null;
    return null;
}

MenuItem AppMenuModel::fileItem()
{
    MenuItemList fileItems {
        makeAction("file-new", m_fileMenuController->isNewAvailable()),
        makeAction("file-open", m_fileMenuController->isOpenAvailable()),
        makeMenu(trc("appshell", "Open &Recent"), recentScores(), true, "file-open"),
        makeSeparator(),
        makeAction("file-close", m_fileMenuController->isCloseAvailable()),
        makeAction("file-save", m_fileMenuController->isSaveAvailable(SaveMode::Save)),
        makeAction("file-save-as", m_fileMenuController->isSaveAvailable(SaveMode::SaveAs)),
        makeAction("file-save-a-copy", m_fileMenuController->isSaveAvailable(SaveMode::SaveCopy)),
        makeAction("file-save-selection", m_fileMenuController->isSaveAvailable(SaveMode::SaveSelection)),
        makeAction("file-save-online", m_fileMenuController->isSaveAvailable(SaveMode::SaveOnline)), // need implement
        makeSeparator(),
        makeAction("file-import-pdf", m_fileMenuController->isImportAvailable()),
        makeAction("file-export", m_fileMenuController->isExportAvailable()), // need implement
        makeSeparator(),
        makeAction("edit-info", m_fileMenuController->isEditInfoAvailable()),
        makeAction("parts", m_fileMenuController->isPartsAvailable()),
        makeSeparator(),
        makeAction("print", m_fileMenuController->isPrintAvailable()), // need implement
        makeSeparator(),
        makeAction("quit", m_fileMenuController->isQuitAvailable())
    };

    return makeMenu(trc("appshell", "&File"), fileItems);
}

MenuItem AppMenuModel::editItem()
{
    MenuItemList editItems {
        makeAction("undo", canUndo()),
        makeAction("redo", canRedo()),
        makeSeparator(),
        makeAction("cut", selectedElementOnScore()),
        makeAction("copy", selectedElementOnScore()),
        makeAction("paste", selectedElementOnScore()),
        makeAction("paste-half", selectedElementOnScore()),
        makeAction("paste-double", selectedElementOnScore()),
        makeAction("paste-special", selectedElementOnScore()),
        makeAction("swap", selectedElementOnScore()),
        makeAction("delete", selectedElementOnScore()),
        makeSeparator(),
        makeAction("select-all", scoreOpened()),
        makeAction("select-similar", selectedElementOnScore()),
        makeAction("find", scoreOpened()),
        makeSeparator(),
        makeAction("preference-dialog") // need implement
    };

    return makeMenu(trc("appshell", "&Edit"), editItems);
}

MenuItem AppMenuModel::viewItem()
{
    MenuItemList viewItems {
        makeAction("toggle-palette", isNotationPage(), notationPageState()->isPanelVisible(PanelType::Palette)),
        makeAction("toggle-instruments", isNotationPage(), notationPageState()->isPanelVisible(PanelType::Instruments)),
        makeAction("masterpalette", isNotationPage(), paletteController()->isMasterPaletteOpened().val),
        makeAction("inspector", isNotationPage(), notationPageState()->isPanelVisible(PanelType::Inspector)),
        makeAction("toggle-playpanel", isNotationPage()), // need implement
        makeAction("toggle-navigator", isNotationPage(), notationPageState()->isPanelVisible(PanelType::NotationNavigator)),
        makeAction("toggle-timeline", isNotationPage()), // need implement
        makeAction("toggle-mixer", isNotationPage()), // need implement
        makeAction("synth-control", isNotationPage()), // need implement
        makeAction("toggle-selection-window", isNotationPage()), // need implement
        makeAction("toggle-piano", isNotationPage()), // need implement
        makeAction("toggle-scorecmp-tool", isNotationPage()), // need implement
        makeSeparator(),
        makeAction("zoomin", isNotationPage()),
        makeAction("zoomout", isNotationPage()),
        makeSeparator(),
        makeMenu(trc("appshell", "&Toolbars"), toolbarsItems()),
        makeMenu(trc("appshell", "W&orkspaces"), workspacesItems(), true, "select-workspace"),
        makeAction("toggle-statusbar", isNotationPage(), notationPageState()->isPanelVisible(PanelType::NotationStatusBar)),
        makeSeparator(),
        makeAction("split-h", isNotationPage()), // need implement
        makeAction("split-v", isNotationPage()), // need implement
        makeSeparator(),
        makeAction("show-invisible", isNotationPage(), scoreConfig().isShowInvisibleElements),
        makeAction("show-unprintable", isNotationPage(), scoreConfig().isShowUnprintableElements),
        makeAction("show-frames", isNotationPage(), scoreConfig().isShowFrames),
        makeAction("show-pageborders", isNotationPage(), scoreConfig().isShowPageMargins),
        makeAction("mark-irregular", isNotationPage(), scoreConfig().isMarkIrregularMeasures),
        makeSeparator(),
        makeAction("fullscreen", true, controller()->isFullScreen().val)
    };

    return makeMenu(trc("appshell", "&View"), viewItems);
}

MenuItem AppMenuModel::addItem()
{
    MenuItemList addItems {
        makeMenu(trc("appshell", "N&otes"), notesItems()),
        makeMenu(trc("appshell", "&Intervals"), intervalsItems()),
        makeMenu(trc("appshell", "T&uplets"), tupletsItems()),
        makeSeparator(),
        makeMenu(trc("appshell", "&Measures"), measuresItems()),
        makeMenu(trc("appshell", "&Frames"), framesItems()),
        makeMenu(trc("appshell", "&Text"), textItems()),
        makeMenu(trc("appshell", "&Lines"), linesItems()),
    };

    return makeMenu(trc("appshell", "&Add"), addItems, scoreOpened());
}

MenuItem AppMenuModel::formatItem()
{
    MenuItemList stretchItems {
        makeAction("stretch+", selectedRangeOnScore()),
        makeAction("stretch-", selectedRangeOnScore()),
        makeAction("reset-stretch", selectedRangeOnScore())
    };

    MenuItemList formatItems {
        makeAction("edit-style", scoreOpened()),
        makeAction("page-settings", scoreOpened()), // need implement
        makeSeparator(),
        makeMenu(trc("appshell", "&Stretch"), stretchItems),
        makeSeparator(),
        makeAction("reset-text-style-overrides", scoreOpened()),
        makeAction("reset-beammode", !hasSelectionOnScore() || selectedRangeOnScore()),
        makeAction("reset", hasSelectionOnScore()),
        makeSeparator(),
        makeAction("load-style", scoreOpened()), // need implement
        makeAction("save-style", scoreOpened()) // need implement
    };

    return makeMenu(trc("appshell", "F&ormat"), formatItems, scoreOpened());
}

MenuItem AppMenuModel::toolsItem()
{
    MenuItemList voicesItems {
        makeAction("voice-x12"),
        makeAction("voice-x13"),
        makeAction("voice-x14"),
        makeAction("voice-x23"),
        makeAction("voice-x24"),
        makeAction("voice-x34")
    };

    MenuItemList measuresItems {
        makeAction("split-measure"),
        makeAction("join-measures"),
    };

    MenuItemList toolsItems {
        makeAction("transpose"),
        makeSeparator(),
        makeAction("explode"),
        makeAction("implode"),
        makeAction("realize-chord-symbols"),
        makeMenu(trc("appshell", "&Voices"), voicesItems),
        makeMenu(trc("appshell", "&Measures"), measuresItems),
        makeAction("time-delete"),
        makeSeparator(),
        makeAction("slash-fill"),
        makeAction("slash-rhythm"),
        makeSeparator(),
        makeAction("pitch-spell"),
        makeAction("reset-groupings"),
        makeAction("resequence-rehearsal-marks"),
        makeAction("unroll-repeats"),
        makeSeparator(),
        makeAction("copy-lyrics-to-clipboard"),
        makeAction("fotomode"), // need implement
        makeAction("del-empty-measures"),
    };

    return makeMenu(trc("appshell", "&Tools"), toolsItems, scoreOpened());
}

MenuItem AppMenuModel::helpItem()
{
    MenuItemList toursItems {
        makeAction("show-tours"), // need implement
        makeAction("reset-tours") // need implement
    };

    MenuItemList helpItems {
        makeAction("online-handbook"),
        makeMenu(trc("appshell", "&Tours"), toursItems),
        makeSeparator(),
        makeAction("about"), // need implement
        makeAction("about-qt"),
        makeAction("about-musicxml"), // need implement
    };

    if (configuration()->isAppUpdatable()) {
        helpItems << makeAction("check-update"); // need implement
    }

    helpItems << makeSeparator()
              << makeAction("ask-help")
              << makeAction("report-bug")
              << makeAction("leave-feedback")
              << makeSeparator()
              << makeAction("revert-factory"); // need implement

    return makeMenu(trc("appshell", "&Help"), helpItems);
}

MenuItemList AppMenuModel::recentScores() const
{
    MenuItemList items;
    MetaList recentScores = userScoresService()->recentScoreList().val;
    if (recentScores.empty()) {
        return items;
    }

    for (const Meta& meta: recentScores) {
        MenuItem item = actionsRegister()->action("file-open");
        item.title = !meta.title.isEmpty() ? meta.title.toStdString() : meta.fileName.toStdString();
        item.args = ActionData::make_arg1<io::path>(meta.filePath);
        item.enabled = true;

        items << item;
    }

    items << makeSeparator()
          << makeAction("clear-recent");

    return items;
}

MenuItemList AppMenuModel::notesItems() const
{
    MenuItemList items {
        makeAction("note-input", m_addMenuController->isNoteInputAvailable(), isNoteInputMode()),
        makeSeparator(),
        makeAction("note-c", m_addMenuController->isNoteAvailable(NoteName::C, NoteAddingMode::NextChord)),
        makeAction("note-d", m_addMenuController->isNoteAvailable(NoteName::D, NoteAddingMode::NextChord)),
        makeAction("note-e", m_addMenuController->isNoteAvailable(NoteName::E, NoteAddingMode::NextChord)),
        makeAction("note-f", m_addMenuController->isNoteAvailable(NoteName::F, NoteAddingMode::NextChord)),
        makeAction("note-g", m_addMenuController->isNoteAvailable(NoteName::G, NoteAddingMode::NextChord)),
        makeAction("note-a", m_addMenuController->isNoteAvailable(NoteName::A, NoteAddingMode::NextChord)),
        makeAction("note-b", m_addMenuController->isNoteAvailable(NoteName::B, NoteAddingMode::NextChord)),
        makeSeparator(),
        makeAction("chord-c", m_addMenuController->isNoteAvailable(NoteName::C, NoteAddingMode::CurrentChord)),
        makeAction("chord-d", m_addMenuController->isNoteAvailable(NoteName::D, NoteAddingMode::CurrentChord)),
        makeAction("chord-e", m_addMenuController->isNoteAvailable(NoteName::E, NoteAddingMode::CurrentChord)),
        makeAction("chord-f", m_addMenuController->isNoteAvailable(NoteName::F, NoteAddingMode::CurrentChord)),
        makeAction("chord-g", m_addMenuController->isNoteAvailable(NoteName::G, NoteAddingMode::CurrentChord)),
        makeAction("chord-a", m_addMenuController->isNoteAvailable(NoteName::A, NoteAddingMode::CurrentChord)),
        makeAction("chord-b", m_addMenuController->isNoteAvailable(NoteName::G, NoteAddingMode::CurrentChord))
    };

    return items;
}

MenuItemList AppMenuModel::intervalsItems() const
{
    MenuItemList items {
        makeAction("interval1", m_addMenuController->isIntervalAvailable(1, IntervalType::Above)),
        makeAction("interval2", m_addMenuController->isIntervalAvailable(2, IntervalType::Above)),
        makeAction("interval3", m_addMenuController->isIntervalAvailable(3, IntervalType::Above)),
        makeAction("interval4", m_addMenuController->isIntervalAvailable(4, IntervalType::Above)),
        makeAction("interval5", m_addMenuController->isIntervalAvailable(5, IntervalType::Above)),
        makeAction("interval6", m_addMenuController->isIntervalAvailable(6, IntervalType::Above)),
        makeAction("interval7", m_addMenuController->isIntervalAvailable(7, IntervalType::Above)),
        makeAction("interval8", m_addMenuController->isIntervalAvailable(8, IntervalType::Above)),
        makeAction("interval9", m_addMenuController->isIntervalAvailable(9, IntervalType::Above)),
        makeSeparator(),
        makeAction("interval-2", m_addMenuController->isIntervalAvailable(2, IntervalType::Below)),
        makeAction("interval-3", m_addMenuController->isIntervalAvailable(3, IntervalType::Below)),
        makeAction("interval-4", m_addMenuController->isIntervalAvailable(4, IntervalType::Below)),
        makeAction("interval-5", m_addMenuController->isIntervalAvailable(5, IntervalType::Below)),
        makeAction("interval-6", m_addMenuController->isIntervalAvailable(6, IntervalType::Below)),
        makeAction("interval-7", m_addMenuController->isIntervalAvailable(7, IntervalType::Below)),
        makeAction("interval-8", m_addMenuController->isIntervalAvailable(8, IntervalType::Below)),
        makeAction("interval-9", m_addMenuController->isIntervalAvailable(9, IntervalType::Below))
    };

    return items;
}

MenuItemList AppMenuModel::tupletsItems() const
{
    MenuItemList items {
        makeAction("duplet", m_addMenuController->isTupletAvailable(TupletType::Duplet)),
        makeAction("triplet", m_addMenuController->isTupletAvailable(TupletType::Triplet)),
        makeAction("quadruplet", m_addMenuController->isTupletAvailable(TupletType::Quadruplet)),
        makeAction("quintuplet", m_addMenuController->isTupletAvailable(TupletType::Quintuplet)),
        makeAction("sextuplet", m_addMenuController->isTupletAvailable(TupletType::Sextuplet)),
        makeAction("septuplet", m_addMenuController->isTupletAvailable(TupletType::Septuplet)),
        makeAction("octuplet", m_addMenuController->isTupletAvailable(TupletType::Octuplet)),
        makeAction("nonuplet", m_addMenuController->isTupletAvailable(TupletType::Nonuplet)),
        makeAction("tuplet-dialog", m_addMenuController->isTupletDialogAvailable())
    };

    return items;
}

MenuItemList AppMenuModel::measuresItems() const
{
    MenuItemList items {
        makeAction("insert-measure", m_addMenuController->isMeasuresAvailable(ElementChangeOperation::Insert, 0)),
        makeAction("insert-measures", m_addMenuController->isMeasuresAvailable(ElementChangeOperation::Insert, 2)),
        makeSeparator(),
        makeAction("append-measure", m_addMenuController->isMeasuresAvailable(ElementChangeOperation::Append, 0)),
        makeAction("append-measures", m_addMenuController->isMeasuresAvailable(ElementChangeOperation::Append, 2))
    };

    return items;
}

MenuItemList AppMenuModel::framesItems() const
{
    MenuItemList items {
        makeAction("insert-hbox", m_addMenuController->isBoxAvailable(ElementChangeOperation::Insert, BoxType::Horizontal)),
        makeAction("insert-vbox", m_addMenuController->isBoxAvailable(ElementChangeOperation::Insert, BoxType::Vertical)),
        makeAction("insert-textframe", m_addMenuController->isBoxAvailable(ElementChangeOperation::Insert, BoxType::Text)),
        makeSeparator(),
        makeAction("append-hbox", m_addMenuController->isBoxAvailable(ElementChangeOperation::Append, BoxType::Horizontal)),
        makeAction("append-vbox", m_addMenuController->isBoxAvailable(ElementChangeOperation::Append, BoxType::Vertical)),
        makeAction("append-textframe", m_addMenuController->isBoxAvailable(ElementChangeOperation::Append, BoxType::Text))
    };

    return items;
}

MenuItemList AppMenuModel::textItems() const
{
    MenuItemList items {
        makeAction("title-text", m_addMenuController->isTextAvailable(TextType::TITLE)),
        makeAction("subtitle-text", m_addMenuController->isTextAvailable(TextType::SUBTITLE)),
        makeAction("composer-text", m_addMenuController->isTextAvailable(TextType::COMPOSER)),
        makeAction("poet-text", m_addMenuController->isTextAvailable(TextType::POET)),
        makeAction("part-text", m_addMenuController->isTextAvailable(TextType::INSTRUMENT_EXCERPT)),
        makeSeparator(),
        makeAction("system-text", m_addMenuController->isTextAvailable(TextType::SYSTEM)),
        makeAction("staff-text", m_addMenuController->isTextAvailable(TextType::STAFF)),
        makeAction("expression-text", m_addMenuController->isTextAvailable(TextType::EXPRESSION)),
        makeAction("rehearsalmark-text", m_addMenuController->isTextAvailable(TextType::REHEARSAL_MARK)),
        makeAction("instrument-change-text", m_addMenuController->isTextAvailable(TextType::INSTRUMENT_CHANGE)),
        makeAction("fingering-text", m_addMenuController->isTextAvailable(TextType::FINGERING)),
        makeSeparator(),
        makeAction("sticking-text", m_addMenuController->isTextAvailable(TextType::STICKING)),
        makeAction("chord-text", m_addMenuController->isTextAvailable(TextType::HARMONY_A)),
        makeAction("roman-numeral-text", m_addMenuController->isTextAvailable(TextType::HARMONY_ROMAN)),
        makeAction("nashville-number-text", m_addMenuController->isTextAvailable(TextType::HARMONY_NASHVILLE)),
        makeAction("lyrics", m_addMenuController->isTextAvailable(TextType::LYRICS_ODD)),
        makeAction("figured-bass", m_addMenuController->isFiguredBassAvailable()),
        makeAction("tempo", m_addMenuController->isTextAvailable(TextType::TEMPO))
    };

    return items;
}

MenuItemList AppMenuModel::linesItems() const
{
    MenuItemList items {
        makeAction("add-slur", m_addMenuController->isSlurAvailable()),
        makeAction("add-hairpin", m_addMenuController->isHarpinAvailable(HairpinType::CRESC_HAIRPIN)),
        makeAction("add-hairpin-reverse", m_addMenuController->isHarpinAvailable(HairpinType::DECRESC_HAIRPIN)),
        makeAction("add-8va", m_addMenuController->isOttavaAvailable(OttavaType::OTTAVA_8VA)),
        makeAction("add-8vb", m_addMenuController->isOttavaAvailable(OttavaType::OTTAVA_8VB)),
        makeAction("add-noteline", m_addMenuController->isNoteLineAvailable())
    };

    return items;
}

MenuItemList AppMenuModel::toolbarsItems() const
{
    MenuItemList items {
        makeAction("toggle-transport", isNotationPage(), notationPageState()->isPanelVisible(PanelType::PlaybackToolBar)),
        makeAction("toggle-noteinput", isNotationPage(), notationPageState()->isPanelVisible(PanelType::NoteInputBar)),
        makeAction("toggle-notationtoolbar", isNotationPage(), notationPageState()->isPanelVisible(PanelType::NotationToolBar)),
        makeAction("toggle-undoredo", isNotationPage(), notationPageState()->isPanelVisible(PanelType::UndoRedoToolBar))
    };

    return items;
}

MenuItemList AppMenuModel::workspacesItems() const
{
    MenuItemList items;

    RetVal<IWorkspacePtrList> workspaces = workspacesManager()->workspaces();
    if (!workspaces.ret) {
        return items;
    }

    IWorkspacePtr currentWorkspace = workspacesManager()->currentWorkspace().val;

    std::sort(workspaces.val.begin(), workspaces.val.end(), [](const IWorkspacePtr& workspace1, const IWorkspacePtr& workspace2) {
        return workspace1->name() < workspace2->name();
    });

    for (const IWorkspacePtr& workspace : workspaces.val) {
        MenuItem item = actionsRegister()->action("select-workspace");
        item.title = workspace->title();
        item.args = ActionData::make_arg1<std::string>(workspace->name());

        bool isCurrentWorkspace = workspace == currentWorkspace;
        item.checked = isCurrentWorkspace;
        item.checkable = true;
        item.enabled = isNotationPage();

        items << item;
    }

    items << makeSeparator()
          << makeAction("configure-workspaces", isNotationPage());

    return items;
}

MenuItem AppMenuModel::makeMenu(const std::string& title, const MenuItemList& actions, bool enabled, const ActionCode& menuActionCode)
{
    MenuItem item;
    item.code = menuActionCode;
    item.title = title;
    item.subitems = actions;
    item.enabled = enabled;
    return item;
}

MenuItem AppMenuModel::makeAction(const ActionCode& actionCode, bool enabled) const
{
    ActionItem action = actionsRegister()->action(actionCode);
    if (!action.isValid()) {
        return MenuItem();
    }

    MenuItem item = action;
    item.enabled = enabled;

    shortcuts::Shortcut shortcut = shortcutsRegister()->shortcut(action.code);
    if (shortcut.isValid()) {
        item.shortcut = shortcut.sequence;
    }

    return item;
}

MenuItem AppMenuModel::makeAction(const ActionCode& actionCode, bool enabled, bool checked) const
{
    MenuItem item = makeAction(actionCode, enabled);
    if (!item.isValid()) {
        return item;
    }

    item.checkable = true;
    item.checked = checked;

    return item;
}

MenuItem AppMenuModel::makeSeparator() const
{
    MenuItem item;
    item.title = std::string();
    return item;
}

bool AppMenuModel::needSaveScore() const
{
    return currentMasterNotation() && currentMasterNotation()->needSave().val;
}

bool AppMenuModel::scoreOpened() const
{
    return currentNotation() != nullptr;
}

bool AppMenuModel::canUndo() const
{
    return currentNotation() ? currentNotation()->undoStack()->canUndo() : false;
}

bool AppMenuModel::canRedo() const
{
    return currentNotation() ? currentNotation()->undoStack()->canRedo() : false;
}

bool AppMenuModel::selectedElementOnScore() const
{
    return notationSelection() ? notationSelection()->element() != nullptr : false;
}

bool AppMenuModel::selectedRangeOnScore() const
{
    return notationSelection() ? notationSelection()->isRange() : false;
}

bool AppMenuModel::hasSelectionOnScore() const
{
    return notationSelection() ? !notationSelection()->isNone() : false;
}

bool AppMenuModel::isNoteInputMode() const
{
    return currentNotation() ? currentNotation()->interaction()->noteInput()->isNoteInputMode() : false;
}

bool AppMenuModel::isNotationPage() const
{
    return interactive()->isOpened("musescore://notation").val;
}

ScoreConfig AppMenuModel::scoreConfig() const
{
    return currentNotation() ? currentNotation()->interaction()->scoreConfig() : ScoreConfig();
}
