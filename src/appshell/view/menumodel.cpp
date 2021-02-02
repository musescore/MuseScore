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
#include "menumodel.h"

#include "translation.h"

using namespace mu::appshell;
using namespace mu::uicomponents;
using namespace mu::notation;
using namespace mu::workspace;
using namespace mu::actions;

MenuModel::MenuModel(QObject* parent)
    : QObject(parent)
{
}

void MenuModel::load()
{
    m_items.clear();
    m_items << fileItem()
            << editItem()
            << viewItem()
            << addItem()
            << formatItem()
            << toolsItem()
            << helpItem();

    globalContext()->currentMasterNotationChanged().onNotify(this, [this]() {
        load();

        currentMasterNotation()->notation()->notationChanged().onNotify(this, [this]() {
            load();
        });
    });

    emit itemsChanged();
}

void MenuModel::handleAction(const QString& actionCode)
{
    actionsDispatcher()->dispatch(actionCode.toStdString());
}

QVariantList MenuModel::items()
{
    QVariantList menuItems;

    for (const MenuItem& menuItem: m_items) {
        menuItems << menuItem.toVariantMap();
    }

    return menuItems;
}

IMasterNotationPtr MenuModel::currentMasterNotation() const
{
    return globalContext()->currentMasterNotation();
}

INotationPtr MenuModel::currentNotation() const
{
    return currentMasterNotation() ? currentMasterNotation()->notation() : nullptr;
}

MenuItem MenuModel::fileItem()
{
    MenuItemList fileItems {
        makeAction(actionsRegister()->action("file-new")),
        makeAction(actionsRegister()->action("file-open")),
        makeAction(actionsRegister()->action("file-import")),
        makeSeparator(),
        makeAction(actionsRegister()->action("file-close"), scoreOpened()),
        makeAction(actionsRegister()->action("file-save"), needSaveScore()),
        makeAction(actionsRegister()->action("file-save-as"), scoreOpened()),
        makeAction(actionsRegister()->action("file-save-a-copy"), scoreOpened()), // need implement
        makeAction(actionsRegister()->action("file-save-selection"), scoreOpened()), // need implement
        makeAction(actionsRegister()->action("file-save-online"), scoreOpened()), // need implement
        makeSeparator(),
        makeAction(actionsRegister()->action("file-import-pdf"), scoreOpened()), // need implement
        makeAction(actionsRegister()->action("file-export"), scoreOpened()), // need implement
        makeSeparator(),
        makeAction(actionsRegister()->action("edit-info"), scoreOpened()),
        makeAction(actionsRegister()->action("parts"), scoreOpened()),
        makeSeparator(),
        makeAction(actionsRegister()->action("print"), scoreOpened()), // need implement
        makeSeparator(),
        makeAction(actionsRegister()->action("quit"))
    };

    return makeMenu(trc("appshell", "&File"), fileItems);
}

MenuItem MenuModel::editItem()
{
    MenuItemList editItems {
        makeAction(actionsRegister()->action("undo"), canUndo()),
        makeAction(actionsRegister()->action("redo"), canRedo()),
        makeSeparator(),
        makeAction(actionsRegister()->action("cut"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("copy"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("paste"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("paste-half"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("paste-double"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("paste-special"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("swap"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("delete"), selectedElementOnScore()),
        makeSeparator(),
        makeAction(actionsRegister()->action("select-all"), scoreOpened()),
        makeAction(actionsRegister()->action("select-similar"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("find"), scoreOpened()),
        makeSeparator(),
        makeAction(actionsRegister()->action("preference-dialog")) // need implement
    };

    return makeMenu(trc("appshell", "&Edit"), editItems);
}

MenuItem MenuModel::viewItem()
{
    MenuItemList viewItems {
        makeAction(actionsRegister()->action("toggle-palette"), canUndo()), // need implement
        makeAction(actionsRegister()->action("masterpalette"), canRedo()), // need implement
        makeAction(actionsRegister()->action("inspector"), selectedElementOnScore()), // need implement
        makeAction(actionsRegister()->action("toggle-playpanel"), selectedElementOnScore()), // need implement
        makeAction(actionsRegister()->action("toggle-navigator"), selectedElementOnScore()), // need implement
        makeAction(actionsRegister()->action("toggle-timeline"), selectedElementOnScore()), // need implement
        makeAction(actionsRegister()->action("toggle-mixer"), selectedElementOnScore()), // need implement
        makeAction(actionsRegister()->action("synth-control"), selectedElementOnScore()), // need implement
        makeAction(actionsRegister()->action("toggle-selection-window"), selectedElementOnScore()), // need implement
        makeAction(actionsRegister()->action("toggle-piano"), selectedElementOnScore()), // need implement
        makeAction(actionsRegister()->action("toggle-scorecmp-tool"), scoreOpened()), // need implement
        makeSeparator(),
        makeAction(actionsRegister()->action("zoomin"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("zoomout"), scoreOpened()),
        makeSeparator(),
        makeMenu(trc("appshell", "W&orkspaces"), workspacesItems()),
        makeSeparator(),
        makeAction(actionsRegister()->action("split-h")), // need implement
        makeAction(actionsRegister()->action("split-v")), // need implement
        makeSeparator(),
        makeAction(actionsRegister()->action("show-invisible")), // need implement
        makeAction(actionsRegister()->action("show-unprintable")), // need implement
        makeAction(actionsRegister()->action("show-frames")), // need implement
        makeAction(actionsRegister()->action("show-pageborders")), // need implement
        makeAction(actionsRegister()->action("mark-irregular")), // need implement
        makeSeparator(),
        makeAction(actionsRegister()->action("fullscreen")) // need implement
    };

    return makeMenu(trc("appshell", "&View"), viewItems);
}

MenuItem MenuModel::addItem()
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

MenuItem MenuModel::formatItem()
{
    MenuItemList stretchItems {
        makeAction(actionsRegister()->action("stretch+")), // need implement
        makeAction(actionsRegister()->action("stretch-")), // need implement
        makeAction(actionsRegister()->action("reset-stretch")) // need implement
    };

    MenuItemList formatItems {
        makeAction(actionsRegister()->action("edit-style")),
        makeAction(actionsRegister()->action("page-settings")), // need implement
        makeSeparator(),
        makeMenu(trc("appshell", "&Stretch"), stretchItems),
        makeSeparator(),
        makeAction(actionsRegister()->action("reset-text-style-overrides")), // need implement
        makeAction(actionsRegister()->action("reset-beammode")), // need implement
        makeAction(actionsRegister()->action("reset")), // need implement
        makeSeparator(),
        makeAction(actionsRegister()->action("load-style")), // need implement
        makeAction(actionsRegister()->action("save-style")) // need implement
    };

    return makeMenu(trc("appshell", "F&ormat"), formatItems, scoreOpened());
}

MenuItem MenuModel::toolsItem()
{
    MenuItemList voicesItems {
        makeAction(actionsRegister()->action("voice-x12")),
        makeAction(actionsRegister()->action("voice-x13")),
        makeAction(actionsRegister()->action("voice-x14")),
        makeAction(actionsRegister()->action("voice-x23")),
        makeAction(actionsRegister()->action("voice-x24")),
        makeAction(actionsRegister()->action("voice-x34"))
    };

    MenuItemList measuresItems {
        makeAction(actionsRegister()->action("split-measure")),
        makeAction(actionsRegister()->action("join-measures")),
    };

    MenuItemList toolsItems {
        makeAction(actionsRegister()->action("transpose")),
        makeSeparator(),
        makeAction(actionsRegister()->action("explode")), // need implement
        makeAction(actionsRegister()->action("implode")), // need implement
        makeAction(actionsRegister()->action("realize-chord-symbols")), // need implement
        makeMenu(trc("appshell", "&Voices"), voicesItems),
        makeMenu(trc("appshell", "&Measures"), measuresItems),
        makeAction(actionsRegister()->action("time-delete")), // need implement
        makeSeparator(),
        makeAction(actionsRegister()->action("slash-fill")), // need implement
        makeAction(actionsRegister()->action("slash-rhythm")), // need implement
        makeSeparator(),
        makeAction(actionsRegister()->action("pitch-spell")), // need implement
        makeAction(actionsRegister()->action("reset-groupings")), // need implement
        makeAction(actionsRegister()->action("resequence-rehearsal-marks")), // need implement
        makeAction(actionsRegister()->action("unroll-repeats")), // need implement
        makeSeparator(),
        makeAction(actionsRegister()->action("copy-lyrics-to-clipboard")), // need implement
        makeAction(actionsRegister()->action("fotomode")), // need implement
        makeAction(actionsRegister()->action("del-empty-measures")), // need implement
    };

    return makeMenu(trc("appshell", "&Tools"), toolsItems, scoreOpened());
}

MenuItem MenuModel::helpItem()
{
    MenuItemList toursItems {
        makeAction(actionsRegister()->action("show-tours")), // need implement
        makeAction(actionsRegister()->action("reset-tours")) // need implement
    };

    MenuItemList helpItems {
        makeAction(actionsRegister()->action("online-handbook")), // need implement
        makeMenu(trc("appshell", "&Tours"), toursItems),
        makeSeparator(),
        makeAction(actionsRegister()->action("about")), // need implement
        makeAction(actionsRegister()->action("about-qt")), // need implement
        makeAction(actionsRegister()->action("about-musicxml")), // need implement
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
#if (!defined(FOR_WINSTORE)) && (!defined(WIN_PORTABLE))
        makeAction(actionsRegister()->action("check-update")), // need implement
#endif
#endif
        makeSeparator(),
        makeAction(actionsRegister()->action("ask-help")), // need implement
        makeAction(actionsRegister()->action("report-bug")), // need implement
        makeAction(actionsRegister()->action("leave-feedback")), // need implement
        makeSeparator(),
        makeAction(actionsRegister()->action("revert-factory")), // need implement
    };

    return makeMenu(trc("appshell", "&Help"), helpItems, scoreOpened());
}

MenuItemList MenuModel::notesItems() const
{
    MenuItemList items {
        makeAction(actionsRegister()->action("note-input"), true, isNoteInputMode()),
        makeSeparator(),
        makeAction(actionsRegister()->action("note-c"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("note-d"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("note-e"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("note-f"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("note-g"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("note-a"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("note-b"), selectedElementOnScore()),
        makeSeparator(),
        makeAction(actionsRegister()->action("chord-c"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("chord-d"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("chord-e"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("chord-f"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("chord-g"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("chord-a"), selectedElementOnScore()),
        makeAction(actionsRegister()->action("chord-b"), selectedElementOnScore())
    };

    return items;
}

MenuItemList MenuModel::intervalsItems() const
{
    MenuItemList items {
        makeAction(actionsRegister()->action("interval1"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("interval2"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("interval3"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("interval4"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("interval5"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("interval6"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("interval7"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("interval8"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("interval9"), true, selectedElementOnScore()),
        makeSeparator(),
        makeAction(actionsRegister()->action("interval-2"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("interval-3"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("interval-4"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("interval-5"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("interval-6"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("interval-7"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("interval-8"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("interval-9"), true, selectedElementOnScore())
    };

    return items;
}

MenuItemList MenuModel::tupletsItems() const
{
    MenuItemList items {
        makeAction(actionsRegister()->action("duplet"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("triplet"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("quadruplet"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("quintuplet"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("sextuplet"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("septuplet"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("octuplet"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("nonuplet"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("tuplet-dialog"), true, selectedElementOnScore())
    };

    return items;
}

MenuItemList MenuModel::measuresItems() const
{
    MenuItemList items {
        makeAction(actionsRegister()->action("insert-measure"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("insert-measures"), true, selectedElementOnScore()),
        makeSeparator(),
        makeAction(actionsRegister()->action("append-measure"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("append-measures"), true, selectedElementOnScore())
    };

    return items;
}

MenuItemList MenuModel::framesItems() const
{
    MenuItemList items {
        makeAction(actionsRegister()->action("insert-hbox"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("insert-vbox"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("insert-textframe"), true, selectedElementOnScore()),
        makeSeparator(),
        makeAction(actionsRegister()->action("append-hbox"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("append-vbox"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("append-textframe"), true, selectedElementOnScore())
    };

    return items;
}

MenuItemList MenuModel::textItems() const
{
    MenuItemList items {
        makeAction(actionsRegister()->action("title-text"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("subtitle-text"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("composer-text"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("poet-text"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("part-text"), true, selectedElementOnScore()),
        makeSeparator(),
        makeAction(actionsRegister()->action("system-text"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("staff-text"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("expression-text"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("rehearsalmark-text"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("instrument-change-text"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("fingering-text"), true, selectedElementOnScore()),
        makeSeparator(),
        makeAction(actionsRegister()->action("sticking-text"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("chord-text"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("roman-numeral-text"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("nashville-number-text"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("lyrics"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("figured-bass"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("tempo"), true, selectedElementOnScore())
    };

    return items;
}

MenuItemList MenuModel::linesItems() const
{
    MenuItemList items {
        makeAction(actionsRegister()->action("add-slur"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("add-hairpin"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("add-hairpin-reverse"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("add-8va"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("add-8vb"), true, selectedElementOnScore()),
        makeAction(actionsRegister()->action("add-noteline"), true, selectedElementOnScore()),
    };

    return items;
}

MenuItemList MenuModel::workspacesItems() const
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
        item.data = ActionData::make_arg1<std::string>(workspace->name());

        bool isCurrentWorkspace = workspace == currentWorkspace;
        items << makeAction(item, true, isCurrentWorkspace);
    }

    items << makeSeparator()
          << makeAction(actionsRegister()->action("new-workspace")) // need implement
          << makeAction(actionsRegister()->action("edit-workspace")) // need implement
          << makeAction(actionsRegister()->action("delete-workspace")) // need implement
          << makeAction(actionsRegister()->action("reset-workspace")); // need implement

    return items;
}

MenuItem MenuModel::makeMenu(const std::string& title, const MenuItemList& actions, bool enabled)
{
    MenuItem item;
    item.title = title;
    item.subitems = actions;
    item.enabled = enabled;
    return item;
}

MenuItem MenuModel::makeAction(const mu::actions::ActionItem& action, bool enabled, bool checked, const std::string& section) const
{
    MenuItem item = action;
    item.enabled = enabled;
    item.checked = checked;
    item.section = section;

    shortcuts::Shortcut shortcut = shortcutsRegister()->shortcut(action.code);
    if (shortcut.isValid()) {
        item.shortcut = shortcut.sequence;
    }

    return item;
}

MenuItem MenuModel::makeSeparator() const
{
    MenuItem item;
    item.title = std::string();
    return item;
}

bool MenuModel::needSaveScore() const
{
    return currentMasterNotation() && currentMasterNotation()->needSave().val;
}

bool MenuModel::scoreOpened() const
{
    return currentNotation() != nullptr;
}

bool MenuModel::canUndo() const
{
    return currentNotation() ? currentNotation()->undoStack()->canUndo() : false;
}

bool MenuModel::canRedo() const
{
    return currentNotation() ? currentNotation()->undoStack()->canRedo() : false;
}

bool MenuModel::selectedElementOnScore() const
{
    return currentNotation() ? !currentNotation()->interaction()->selection()->isNone() : false;
}

bool MenuModel::isNoteInputMode() const
{
    return currentNotation() ? currentNotation()->interaction()->noteInput()->isNoteInputMode() : false;
}
