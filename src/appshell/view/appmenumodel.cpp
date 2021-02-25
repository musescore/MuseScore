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

using namespace mu::appshell;
using namespace mu::uicomponents;
using namespace mu::notation;
using namespace mu::workspace;
using namespace mu::actions;

AppMenuModel::AppMenuModel(QObject* parent)
    : QObject(parent)
{
}

void AppMenuModel::load()
{
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
    MenuItem menuItem = actionIndex == -1 ? item(actionCode) : itemByIndex(actionCode, actionIndex);

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
    globalContext()->currentMasterNotationChanged().onNotify(this, [this]() {
        load();

        currentMasterNotation()->notation()->notationChanged().onNotify(this, [this]() {
            load();
        });

        currentMasterNotation()->notation()->interaction()->selectionChanged().onNotify(this, [this]() {
            load();
        });
    });

    userScoresService()->recentScoreList().ch.onReceive(this, [this](const std::vector<Meta>&) {
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

MenuItem& AppMenuModel::item(const ActionCode& actionCode)
{
    for (MenuItem& item : m_items) {
        if (item.code == actionCode) {
            return item;
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
        makeAction("file-new"),
        makeAction("file-open"),
        makeMenu(trc("appshell", "Open &Recent"), recentScores(), true, "file-open"),
        makeSeparator(),
        makeAction("file-close", scoreOpened()),
        makeAction("file-save", needSaveScore()),
        makeAction("file-save-as", scoreOpened()),
        makeAction("file-save-a-copy", scoreOpened()),
        makeAction("file-save-selection", scoreOpened()),
        makeAction("file-save-online", scoreOpened()), // need implement
        makeSeparator(),
        makeAction("file-import-pdf", scoreOpened()),
        makeAction("file-export", scoreOpened()), // need implement
        makeSeparator(),
        makeAction("edit-info", scoreOpened()),
        makeAction("parts", scoreOpened()),
        makeSeparator(),
        makeAction("print", scoreOpened()), // need implement
        makeSeparator(),
        makeAction("quit")
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
        makeAction("reset-groupings"), // need implement
        makeAction("resequence-rehearsal-marks"), // need implement
        makeAction("unroll-repeats"), // need implement
        makeSeparator(),
        makeAction("copy-lyrics-to-clipboard"), // need implement
        makeAction("fotomode"), // need implement
        makeAction("del-empty-measures"), // need implement
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
    std::vector<Meta> recentScores = userScoresService()->recentScoreList().val;
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
        makeAction("note-input", true, isNoteInputMode()),
        makeSeparator(),
        makeAction("note-c", selectedElementOnScore()),
        makeAction("note-d", selectedElementOnScore()),
        makeAction("note-e", selectedElementOnScore()),
        makeAction("note-f", selectedElementOnScore()),
        makeAction("note-g", selectedElementOnScore()),
        makeAction("note-a", selectedElementOnScore()),
        makeAction("note-b", selectedElementOnScore()),
        makeSeparator(),
        makeAction("chord-c", selectedElementOnScore()),
        makeAction("chord-d", selectedElementOnScore()),
        makeAction("chord-e", selectedElementOnScore()),
        makeAction("chord-f", selectedElementOnScore()),
        makeAction("chord-g", selectedElementOnScore()),
        makeAction("chord-a", selectedElementOnScore()),
        makeAction("chord-b", selectedElementOnScore())
    };

    return items;
}

MenuItemList AppMenuModel::intervalsItems() const
{
    MenuItemList items {
        makeAction("interval1", selectedElementOnScore()),
        makeAction("interval2", selectedElementOnScore()),
        makeAction("interval3", selectedElementOnScore()),
        makeAction("interval4", selectedElementOnScore()),
        makeAction("interval5", selectedElementOnScore()),
        makeAction("interval6", selectedElementOnScore()),
        makeAction("interval7", selectedElementOnScore()),
        makeAction("interval8", selectedElementOnScore()),
        makeAction("interval9", selectedElementOnScore()),
        makeSeparator(),
        makeAction("interval-2", selectedElementOnScore()),
        makeAction("interval-3", selectedElementOnScore()),
        makeAction("interval-4", selectedElementOnScore()),
        makeAction("interval-5", selectedElementOnScore()),
        makeAction("interval-6", selectedElementOnScore()),
        makeAction("interval-7", selectedElementOnScore()),
        makeAction("interval-8", selectedElementOnScore()),
        makeAction("interval-9", selectedElementOnScore())
    };

    return items;
}

MenuItemList AppMenuModel::tupletsItems() const
{
    MenuItemList items {
        makeAction("duplet", selectedElementOnScore()),
        makeAction("triplet", selectedElementOnScore()),
        makeAction("quadruplet", selectedElementOnScore()),
        makeAction("quintuplet", selectedElementOnScore()),
        makeAction("sextuplet", selectedElementOnScore()),
        makeAction("septuplet", selectedElementOnScore()),
        makeAction("octuplet", selectedElementOnScore()),
        makeAction("nonuplet", selectedElementOnScore()),
        makeAction("tuplet-dialog", selectedElementOnScore())
    };

    return items;
}

MenuItemList AppMenuModel::measuresItems() const
{
    MenuItemList items {
        makeAction("insert-measure", selectedElementOnScore()),
        makeAction("insert-measures", selectedElementOnScore()),
        makeSeparator(),
        makeAction("append-measure", selectedElementOnScore()),
        makeAction("append-measures", selectedElementOnScore())
    };

    return items;
}

MenuItemList AppMenuModel::framesItems() const
{
    MenuItemList items {
        makeAction("insert-hbox", selectedElementOnScore()),
        makeAction("insert-vbox", selectedElementOnScore()),
        makeAction("insert-textframe", selectedElementOnScore()),
        makeSeparator(),
        makeAction("append-hbox", selectedElementOnScore()),
        makeAction("append-vbox", selectedElementOnScore()),
        makeAction("append-textframe", selectedElementOnScore())
    };

    return items;
}

MenuItemList AppMenuModel::textItems() const
{
    MenuItemList items {
        makeAction("title-text", selectedElementOnScore()),
        makeAction("subtitle-text", selectedElementOnScore()),
        makeAction("composer-text", selectedElementOnScore()),
        makeAction("poet-text", selectedElementOnScore()),
        makeAction("part-text", selectedElementOnScore()),
        makeSeparator(),
        makeAction("system-text", selectedElementOnScore()),
        makeAction("staff-text", selectedElementOnScore()),
        makeAction("expression-text", selectedElementOnScore()),
        makeAction("rehearsalmark-text", selectedElementOnScore()),
        makeAction("instrument-change-text", selectedElementOnScore()),
        makeAction("fingering-text", selectedElementOnScore()),
        makeSeparator(),
        makeAction("sticking-text", selectedElementOnScore()),
        makeAction("chord-text", selectedElementOnScore()),
        makeAction("roman-numeral-text", selectedElementOnScore()),
        makeAction("nashville-number-text", selectedElementOnScore()),
        makeAction("lyrics", selectedElementOnScore()),
        makeAction("figured-bass", selectedElementOnScore()),
        makeAction("tempo", selectedElementOnScore())
    };

    return items;
}

MenuItemList AppMenuModel::linesItems() const
{
    MenuItemList items {
        makeAction("add-slur", selectedElementOnScore()),
        makeAction("add-hairpin", selectedElementOnScore()),
        makeAction("add-hairpin-reverse", selectedElementOnScore()),
        makeAction("add-8va", selectedElementOnScore()),
        makeAction("add-8vb", selectedElementOnScore()),
        makeAction("add-noteline", selectedElementOnScore()),
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
