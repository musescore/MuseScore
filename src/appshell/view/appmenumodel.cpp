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

using namespace mu::appshell;
using namespace mu::ui;
using namespace mu::notation;
using namespace mu::workspace;
using namespace mu::actions;

AppMenuModel::AppMenuModel(QObject* parent)
    : QObject(parent)
{
}

void AppMenuModel::load()
{
    TRACEFUNC;

    clear();

    appendItem(fileItem());
    appendItem(editItem());
    appendItem(viewItem());
    appendItem(addItem());
    appendItem(formatItem());
    appendItem(toolsItem());
    appendItem(helpItem());

    listenActionsStateChanges();
    setupConnections();

    emit itemsChanged();
}

void AppMenuModel::onActionsStateChanges(const actions::ActionCodeList& codes)
{
    AbstractMenuModel::onActionsStateChanges(codes);
    emit itemsChanged();
}

void AppMenuModel::handleAction(const QString& actionCodeStr, int actionIndex)
{
    ActionCode actionCode = codeFromQString(actionCodeStr);
    MenuItem menuItem = actionIndex == -1 ? findItem(actionCode) : findItemByIndex(actionCode, actionIndex);

    actionsDispatcher()->dispatch(actionCode, menuItem.args);
}

void AppMenuModel::setupConnections()
{
    userScoresService()->recentScoreList().ch.onReceive(this, [this](const MetaList&) {
        MenuItem& recentScoreListItem = findMenu("file-open");
        recentScoreListItem.subitems = recentScores();
        emit itemsChanged();
    });

    workspacesManager()->currentWorkspace().ch.onReceive(this, [this](const IWorkspacePtr&) {
        MenuItem& workspacesItem = findMenu("select-workspace");
        workspacesItem.subitems = workspacesItems();
        emit itemsChanged();
    });
}

MenuItem AppMenuModel::fileItem() const
{
    MenuItemList recentScoresList = recentScores();

    MenuItemList fileItems {
        makeAction("file-new"),
        makeAction("file-open"),
        makeMenu(qtrc("appshell", "Open &Recent"), recentScoresList, !recentScoresList.empty(), "file-open"),
        makeSeparator(),
        makeAction("file-close"),
        makeAction("file-save"),
        makeAction("file-save-as"),
        makeAction("file-save-a-copy"),
        makeAction("file-save-selection"),
        makeAction("file-save-online"), // need implement
        makeSeparator(),
        makeAction("file-import-pdf"),
        makeAction("file-export"), // need implement
        makeSeparator(),
        makeAction("edit-info"),
        makeAction("parts"),
        makeSeparator(),
        makeAction("print"), // need implement
        makeSeparator(),
        makeAction("quit")
    };

    return makeMenu(qtrc("appshell", "&File"), fileItems);
}

MenuItem AppMenuModel::editItem() const
{
    MenuItemList editItems {
        makeAction("undo"),
        makeAction("redo"),
        makeSeparator(),
        makeAction("cut"),
        makeAction("copy"),
        makeAction("paste"),
        makeAction("paste-half"),
        makeAction("paste-double"),
        makeAction("swap"),
        makeAction("delete"),
        makeSeparator(),
        makeAction("select-all"),
        makeAction("select-section"),
        makeAction("find"),
        makeSeparator(),
        makeAction("preference-dialog")
    };

    return makeMenu(qtrc("appshell", "&Edit"), editItems);
}

MenuItem AppMenuModel::viewItem() const
{
    MenuItemList viewItems {
        makeAction("toggle-palette"),
        makeAction("toggle-instruments"),
        makeAction("masterpalette"),
        makeAction("inspector"),
        makeAction("toggle-navigator"),
        makeAction("toggle-timeline"), // need implement
        makeAction("toggle-mixer"), // need implement
        makeAction("synth-control"), // need implement
        makeAction("toggle-piano"), // need implement
        makeAction("toggle-scorecmp-tool"), // need implement
        makeSeparator(),
        makeAction("zoomin"),
        makeAction("zoomout"),
        makeSeparator(),
        makeMenu(qtrc("appshell", "&Toolbars"), toolbarsItems()),
        makeMenu(qtrc("appshell", "W&orkspaces"), workspacesItems(), true, "select-workspace"),
        makeAction("toggle-statusbar"),
        makeSeparator(),
        makeAction("split-h"), // need implement
        makeAction("split-v"), // need implement
        makeSeparator(),
        makeAction("show-invisible"),
        makeAction("show-unprintable"),
        makeAction("show-frames"),
        makeAction("show-pageborders"),
        makeAction("show-irregular"),
        makeSeparator(),
        makeAction("fullscreen")
    };

    return makeMenu(qtrc("appshell", "&View"), viewItems);
}

MenuItem AppMenuModel::addItem() const
{
    MenuItemList addItems {
        makeMenu(qtrc("appshell", "N&otes"), notesItems()),
        makeMenu(qtrc("appshell", "&Intervals"), intervalsItems()),
        makeMenu(qtrc("appshell", "T&uplets"), tupletsItems()),
        makeSeparator(),
        makeMenu(qtrc("appshell", "&Measures"), measuresItems()),
        makeMenu(qtrc("appshell", "&Frames"), framesItems()),
        makeMenu(qtrc("appshell", "&Text"), textItems()),
        makeMenu(qtrc("appshell", "&Lines"), linesItems()),
    };

    return makeMenu(qtrc("appshell", "&Add"), addItems);
}

MenuItem AppMenuModel::formatItem() const
{
    MenuItemList stretchItems {
        makeAction("stretch+"),
        makeAction("stretch-"),
        makeAction("reset-stretch")
    };

    MenuItemList formatItems {
        makeAction("edit-style"),
        makeAction("page-settings"), // need implement
        makeSeparator(),
        makeAction("add-remove-breaks"),
        makeMenu(qtrc("appshell", "&Stretch"), stretchItems),
        makeSeparator(),
        makeAction("reset-text-style-overrides"),
        makeAction("reset-beammode"),
        makeAction("reset"),
        makeSeparator(),
        makeAction("load-style"), // need implement
        makeAction("save-style") // need implement
    };

    return makeMenu(qtrc("appshell", "F&ormat"), formatItems);
}

MenuItem AppMenuModel::toolsItem() const
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
        makeAction("join-measures")
    };

    MenuItemList toolsItems {
        makeAction("transpose"),
        makeSeparator(),
        makeAction("explode"),
        makeAction("implode"),
        makeAction("realize-chord-symbols"),
        makeMenu(qtrc("appshell", "&Voices"), voicesItems),
        makeMenu(qtrc("appshell", "&Measures"), measuresItems),
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

    return makeMenu(qtrc("appshell", "&Tools"), toolsItems);
}

MenuItem AppMenuModel::helpItem() const
{
    MenuItemList toursItems {
        makeAction("show-tours"), // need implement
        makeAction("reset-tours") // need implement
    };

    MenuItemList helpItems {
        makeAction("online-handbook"),
        makeMenu(qtrc("appshell", "&Tours"), toursItems),
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

    return makeMenu(qtrc("appshell", "&Help"), helpItems);
}

MenuItemList AppMenuModel::recentScores() const
{
    MenuItemList items;
    MetaList recentScores = userScoresService()->recentScoreList().val;

    for (const Meta& meta : recentScores) {
        MenuItem item = uiactionsRegister()->action("file-open");
        item.title = !meta.title.isEmpty() ? meta.title : meta.fileName.toQString();
        item.args = ActionData::make_arg1<io::path>(meta.filePath);
        item.state.enabled = true;

        items << item;
    }

    items << makeSeparator()
          << makeAction("clear-recent");

    return items;
}

MenuItemList AppMenuModel::notesItems() const
{
    MenuItemList items {
        makeAction("note-input"),
        makeSeparator(),
        makeAction("note-c"),
        makeAction("note-d"),
        makeAction("note-e"),
        makeAction("note-f"),
        makeAction("note-g"),
        makeAction("note-a"),
        makeAction("note-b"),
        makeSeparator(),
        makeAction("chord-c"),
        makeAction("chord-d"),
        makeAction("chord-e"),
        makeAction("chord-f"),
        makeAction("chord-g"),
        makeAction("chord-a"),
        makeAction("chord-b")
    };

    return items;
}

MenuItemList AppMenuModel::intervalsItems() const
{
    MenuItemList items {
        makeAction("interval1"),
        makeAction("interval2"),
        makeAction("interval3"),
        makeAction("interval4"),
        makeAction("interval5"),
        makeAction("interval6"),
        makeAction("interval7"),
        makeAction("interval8"),
        makeAction("interval9"),
        makeSeparator(),
        makeAction("interval-2"),
        makeAction("interval-3"),
        makeAction("interval-4"),
        makeAction("interval-5"),
        makeAction("interval-6"),
        makeAction("interval-7"),
        makeAction("interval-8"),
        makeAction("interval-9")
    };

    return items;
}

MenuItemList AppMenuModel::tupletsItems() const
{
    MenuItemList items {
        makeAction("duplet"),
        makeAction("triplet"),
        makeAction("quadruplet"),
        makeAction("quintuplet"),
        makeAction("sextuplet"),
        makeAction("septuplet"),
        makeAction("octuplet"),
        makeAction("nonuplet"),
        makeAction("tuplet-dialog")
    };

    return items;
}

MenuItemList AppMenuModel::measuresItems() const
{
    MenuItemList items {
        makeAction("insert-measure"),
        makeAction("insert-measures"),
        makeSeparator(),
        makeAction("append-measure"),
        makeAction("append-measures")
    };

    return items;
}

MenuItemList AppMenuModel::framesItems() const
{
    MenuItemList items {
        makeAction("insert-hbox"),
        makeAction("insert-vbox"),
        makeAction("insert-textframe"),
        makeSeparator(),
        makeAction("append-hbox"),
        makeAction("append-vbox"),
        makeAction("append-textframe")
    };

    return items;
}

MenuItemList AppMenuModel::textItems() const
{
    MenuItemList items {
        makeAction("title-text"),
        makeAction("subtitle-text"),
        makeAction("composer-text"),
        makeAction("poet-text"),
        makeAction("part-text"),
        makeSeparator(),
        makeAction("system-text"),
        makeAction("staff-text"),
        makeAction("expression-text"),
        makeAction("rehearsalmark-text"),
        makeAction("instrument-change-text"),
        makeAction("fingering-text"),
        makeSeparator(),
        makeAction("sticking-text"),
        makeAction("chord-text"),
        makeAction("roman-numeral-text"),
        makeAction("nashville-number-text"),
        makeAction("lyrics"),
        makeAction("figured-bass"),
        makeAction("tempo")
    };

    return items;
}

MenuItemList AppMenuModel::linesItems() const
{
    MenuItemList items {
        makeAction("add-slur"),
        makeAction("add-hairpin"),
        makeAction("add-hairpin-reverse"),
        makeAction("add-8va"),
        makeAction("add-8vb"),
        makeAction("add-noteline")
    };

    return items;
}

MenuItemList AppMenuModel::toolbarsItems() const
{
    MenuItemList items {
        makeAction("toggle-transport"),
        makeAction("toggle-noteinput"),
        makeAction("toggle-notationtoolbar"),
        makeAction("toggle-undoredo")
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
        MenuItem item = uiactionsRegister()->action("select-workspace");
        item.title = QString::fromStdString(workspace->title());
        item.args = ActionData::make_arg1<std::string>(workspace->name());

        bool isCurrentWorkspace = workspace == currentWorkspace;
        item.selectable = true;
        item.selected = isCurrentWorkspace;
        item.state.enabled = true;

        items << item;
    }

    items << makeSeparator()
          << makeAction("configure-workspaces");

    return items;
}
