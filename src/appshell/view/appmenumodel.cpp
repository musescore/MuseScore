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
        makeAction("toggle-palette", scoreOpened(), configuration()->isPalettePanelVisible().val),
        makeAction("masterpalette", scoreOpened(), paletteController()->isMasterPaletteOpened().val),
        makeAction("inspector", scoreOpened()),
        makeAction("toggle-playpanel", scoreOpened()), // need implement
        makeAction("toggle-navigator", scoreOpened()),
        makeAction("toggle-timeline", scoreOpened()), // need implement
        makeAction("toggle-mixer", scoreOpened()), // need implement
        makeAction("synth-control", scoreOpened()), // need implement
        makeAction("toggle-selection-window", scoreOpened()), // need implement
        makeAction("toggle-piano", scoreOpened()), // need implement
        makeAction("toggle-scorecmp-tool", scoreOpened()), // need implement
        makeSeparator(),
        makeAction("zoomin", scoreOpened()),
        makeAction("zoomout", scoreOpened()),
        makeSeparator(),
        makeMenu(trc("appshell", "W&orkspaces"), workspacesItems(), true, "select-workspace"),
        makeSeparator(),
        makeAction("split-h", scoreOpened()), // need implement
        makeAction("split-v", scoreOpened()), // need implement
        makeSeparator(),
        makeAction("show-invisible", scoreOpened()),
        makeAction("show-unprintable", scoreOpened()),
        makeAction("show-frames", scoreOpened()),
        makeAction("show-pageborders", scoreOpened()),
        makeAction("mark-irregular", scoreOpened()),
        makeSeparator(),
        makeAction("fullscreen") // need implement
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
        makeAction("stretch+", scoreOpened(), selectedRangeOnScore()),
        makeAction("stretch-", scoreOpened(), selectedRangeOnScore()),
        makeAction("reset-stretch", scoreOpened(), selectedRangeOnScore())
    };

    MenuItemList formatItems {
        makeAction("edit-style", scoreOpened()),
        makeAction("page-settings", scoreOpened()), // need implement
        makeSeparator(),
        makeMenu(trc("appshell", "&Stretch"), stretchItems),
        makeSeparator(),
        makeAction("reset-text-style-overrides", scoreOpened()),
        makeAction("reset-beammode", scoreOpened(), !hasSelectionOnScore() || selectedRangeOnScore()),
        makeAction("reset", scoreOpened(), hasSelectionOnScore()),
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
        makeAction("explode"), // need implement
        makeAction("implode"), // need implement
        makeAction("realize-chord-symbols"), // need implement
        makeMenu(trc("appshell", "&Voices"), voicesItems),
        makeMenu(trc("appshell", "&Measures"), measuresItems),
        makeAction("time-delete"), // need implement
        makeSeparator(),
        makeAction("slash-fill"), // need implement
        makeAction("slash-rhythm"), // need implement
        makeSeparator(),
        makeAction("pitch-spell"), // need implement
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
        makeAction("interval1", true, selectedElementOnScore()),
        makeAction("interval2", true, selectedElementOnScore()),
        makeAction("interval3", true, selectedElementOnScore()),
        makeAction("interval4", true, selectedElementOnScore()),
        makeAction("interval5", true, selectedElementOnScore()),
        makeAction("interval6", true, selectedElementOnScore()),
        makeAction("interval7", true, selectedElementOnScore()),
        makeAction("interval8", true, selectedElementOnScore()),
        makeAction("interval9", true, selectedElementOnScore()),
        makeSeparator(),
        makeAction("interval-2", true, selectedElementOnScore()),
        makeAction("interval-3", true, selectedElementOnScore()),
        makeAction("interval-4", true, selectedElementOnScore()),
        makeAction("interval-5", true, selectedElementOnScore()),
        makeAction("interval-6", true, selectedElementOnScore()),
        makeAction("interval-7", true, selectedElementOnScore()),
        makeAction("interval-8", true, selectedElementOnScore()),
        makeAction("interval-9", true, selectedElementOnScore())
    };

    return items;
}

MenuItemList AppMenuModel::tupletsItems() const
{
    MenuItemList items {
        makeAction("duplet", true, selectedElementOnScore()),
        makeAction("triplet", true, selectedElementOnScore()),
        makeAction("quadruplet", true, selectedElementOnScore()),
        makeAction("quintuplet", true, selectedElementOnScore()),
        makeAction("sextuplet", true, selectedElementOnScore()),
        makeAction("septuplet", true, selectedElementOnScore()),
        makeAction("octuplet", true, selectedElementOnScore()),
        makeAction("nonuplet", true, selectedElementOnScore()),
        makeAction("tuplet-dialog", true, selectedElementOnScore())
    };

    return items;
}

MenuItemList AppMenuModel::measuresItems() const
{
    MenuItemList items {
        makeAction("insert-measure", true, selectedElementOnScore()),
        makeAction("insert-measures", true, selectedElementOnScore()),
        makeSeparator(),
        makeAction("append-measure", true, selectedElementOnScore()),
        makeAction("append-measures", true, selectedElementOnScore())
    };

    return items;
}

MenuItemList AppMenuModel::framesItems() const
{
    MenuItemList items {
        makeAction("insert-hbox", true, selectedElementOnScore()),
        makeAction("insert-vbox", true, selectedElementOnScore()),
        makeAction("insert-textframe", true, selectedElementOnScore()),
        makeSeparator(),
        makeAction("append-hbox", true, selectedElementOnScore()),
        makeAction("append-vbox", true, selectedElementOnScore()),
        makeAction("append-textframe", true, selectedElementOnScore())
    };

    return items;
}

MenuItemList AppMenuModel::textItems() const
{
    MenuItemList items {
        makeAction("title-text", true, selectedElementOnScore()),
        makeAction("subtitle-text", true, selectedElementOnScore()),
        makeAction("composer-text", true, selectedElementOnScore()),
        makeAction("poet-text", true, selectedElementOnScore()),
        makeAction("part-text", true, selectedElementOnScore()),
        makeSeparator(),
        makeAction("system-text", true, selectedElementOnScore()),
        makeAction("staff-text", true, selectedElementOnScore()),
        makeAction("expression-text", true, selectedElementOnScore()),
        makeAction("rehearsalmark-text", true, selectedElementOnScore()),
        makeAction("instrument-change-text", true, selectedElementOnScore()),
        makeAction("fingering-text", true, selectedElementOnScore()),
        makeSeparator(),
        makeAction("sticking-text", true, selectedElementOnScore()),
        makeAction("chord-text", true, selectedElementOnScore()),
        makeAction("roman-numeral-text", true, selectedElementOnScore()),
        makeAction("nashville-number-text", true, selectedElementOnScore()),
        makeAction("lyrics", true, selectedElementOnScore()),
        makeAction("figured-bass", true, selectedElementOnScore()),
        makeAction("tempo", true, selectedElementOnScore())
    };

    return items;
}

MenuItemList AppMenuModel::linesItems() const
{
    MenuItemList items {
        makeAction("add-slur", true, selectedElementOnScore()),
        makeAction("add-hairpin", true, selectedElementOnScore()),
        makeAction("add-hairpin-reverse", true, selectedElementOnScore()),
        makeAction("add-8va", true, selectedElementOnScore()),
        makeAction("add-8vb", true, selectedElementOnScore()),
        makeAction("add-noteline", true, selectedElementOnScore()),
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
        MenuItem item = actionsRegister()->action("select-workspace"); // need implement
        item.title = workspace->title();
        item.args = ActionData::make_arg1<std::string>(workspace->name());

        bool isCurrentWorkspace = workspace == currentWorkspace;
        item.checked = isCurrentWorkspace;
        item.checkable = true;
        item.enabled = true;

        items << item;
    }

    items << makeSeparator()
          << makeAction("new-workspace") // need implement
          << makeAction("edit-workspace") // need implement
          << makeAction("delete-workspace") // need implement
          << makeAction("reset-workspace"); // need implement

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
