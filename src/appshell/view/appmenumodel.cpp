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
#include "appmenumodel.h"

#include "translation.h"

#include "log.h"
#include "config.h"

using namespace mu::appshell;
using namespace mu::ui;
using namespace mu::project;
using namespace mu::workspace;
using namespace mu::actions;

static QString makeId(const ActionCode& actionCode, int itemIndex)
{
    return QString::fromStdString(actionCode) + QString::number(itemIndex);
}

AppMenuModel::AppMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void AppMenuModel::load()
{
    TRACEFUNC;

    AbstractMenuModel::load();

    MenuItemList items {
        fileItem(),
        editItem(),
        viewItem(),
        addItem(),
        formatItem(),
        toolsItem(),
        helpItem(),
#ifdef BUILD_DIAGNOSTICS
        diagnosticItem()
#endif
    };

    setItems(items);

    setupConnections();
}

void AppMenuModel::onActionsStateChanges(const actions::ActionCodeList& codes)
{
    AbstractMenuModel::onActionsStateChanges(codes);
    emit itemsChanged();
}

void AppMenuModel::setupConnections()
{
    recentProjectsProvider()->recentProjectListChanged().onNotify(this, [this]() {
        MenuItem& recentScoreListItem = findMenu("menu-file-open");

        MenuItemList recentScoresList = recentScores();
        bool openRecentEnabled = !recentScoresList.empty();

        if (!recentScoresList.empty()) {
            recentScoresList = appendClearRecentSection(recentScoresList);
        }

        recentScoreListItem.state.enabled = openRecentEnabled;
        recentScoreListItem.subitems = recentScoresList;

        emit itemsChanged();
    });

    workspacesManager()->currentWorkspaceChanged().onNotify(this, [this]() {
        MenuItem& workspacesItem = findMenu("menu-select-workspace");
        workspacesItem.subitems = workspacesItems();
        emit itemsChanged();
    });

    workspacesManager()->workspacesListChanged().onNotify(this, [this]() {
        MenuItem& workspacesItem = findMenu("menu-select-workspace");
        workspacesItem.subitems = workspacesItems();
        emit itemsChanged();
    });
}

MenuItem AppMenuModel::fileItem() const
{
    MenuItemList recentScoresList = recentScores();
    bool openRecentEnabled = true;

    if (!recentScoresList.empty()) {
        recentScoresList = appendClearRecentSection(recentScoresList);
    }

    MenuItemList fileItems {
        makeMenuItem("file-new"),
        makeMenuItem("file-open"),
        makeMenu(qtrc("appshell", "Open &recent"), recentScoresList, openRecentEnabled, "menu-file-open"),
        makeSeparator(),
        makeMenuItem("file-close"),
        makeMenuItem("file-save"),
        makeMenuItem("file-save-as"),
        makeMenuItem("file-save-a-copy"),
        makeMenuItem("file-save-selection"),
        makeMenuItem("file-save-online"), // need implement
        makeSeparator(),
        makeMenuItem("file-import-pdf"),
        makeMenuItem("file-export"), // need implement
        makeSeparator(),
        makeMenuItem("edit-info"),
        makeMenuItem("parts"),
        makeSeparator(),
        makeMenuItem("print"), // need implement
        makeSeparator(),
        makeMenuItem("quit")
    };

    return makeMenu(qtrc("appshell", "&File"), fileItems);
}

MenuItem AppMenuModel::editItem() const
{
    MenuItemList editItems {
        makeMenuItem("undo"),
        makeMenuItem("redo"),
        makeSeparator(),
        makeMenuItem("cut"),
        makeMenuItem("copy"),
        makeMenuItem("paste"),
        makeMenuItem("paste-half"),
        makeMenuItem("paste-double"),
        makeMenuItem("swap"),
        makeMenuItem("delete"),
        makeSeparator(),
        makeMenuItem("select-all"),
        makeMenuItem("select-section"),
        makeMenuItem("find"),
        makeSeparator(),
        makeMenuItem("preference-dialog")
    };

    return makeMenu(qtrc("appshell", "&Edit"), editItems);
}

MenuItem AppMenuModel::viewItem() const
{
    MenuItemList viewItems {
        makeMenuItem("toggle-palettes"),
        makeMenuItem("masterpalette"),
        makeMenuItem("toggle-instruments"),
        makeMenuItem("inspector"),
        makeMenuItem("toggle-selection-filter"),
        makeMenuItem("toggle-navigator"),
        makeMenuItem("toggle-timeline"),
        makeMenuItem("toggle-mixer"),
        makeMenuItem("synth-control"), // need implement
        makeMenuItem("toggle-piano"), // need implement
        makeMenuItem("toggle-scorecmp-tool"), // need implement
        makeSeparator(),
        makeMenuItem("zoomin"),
        makeMenuItem("zoomout"),
        makeSeparator(),
        makeMenu(qtrc("appshell", "&Toolbars"), toolbarsItems()),
        makeMenu(qtrc("appshell", "W&orkspaces"), workspacesItems(), true, "menu-select-workspace"),
        makeMenuItem("toggle-statusbar"),
        makeSeparator(),
        makeMenuItem("split-h"), // need implement
        makeMenuItem("split-v"), // need implement
        makeSeparator(),
        makeMenuItem("show-invisible"),
        makeMenuItem("show-unprintable"),
        makeMenuItem("show-frames"),
        makeMenuItem("show-pageborders"),
        makeMenuItem("mark-irregular"),
        makeSeparator(),
        makeMenuItem("fullscreen")
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
        makeMenuItem("stretch+"),
        makeMenuItem("stretch-"),
        makeMenuItem("reset-stretch")
    };

    MenuItemList formatItems {
        makeMenuItem("edit-style"),
        makeMenuItem("page-settings"), // need implement
        makeSeparator(),
        makeMenuItem("add-remove-breaks"),
        makeMenu(qtrc("appshell", "&Stretch"), stretchItems),
        makeSeparator(),
        makeMenuItem("reset-text-style-overrides"),
        makeMenuItem("reset-beammode"),
        makeMenuItem("reset"),
        makeSeparator(),
        makeMenuItem("load-style"), // need implement
        makeMenuItem("save-style") // need implement
    };

    return makeMenu(qtrc("appshell", "F&ormat"), formatItems);
}

MenuItem AppMenuModel::toolsItem() const
{
    MenuItemList voicesItems {
        makeMenuItem("voice-x12"),
        makeMenuItem("voice-x13"),
        makeMenuItem("voice-x14"),
        makeMenuItem("voice-x23"),
        makeMenuItem("voice-x24"),
        makeMenuItem("voice-x34")
    };

    MenuItemList measuresItems {
        makeMenuItem("split-measure"),
        makeMenuItem("join-measures")
    };

    MenuItemList toolsItems {
        makeMenuItem("transpose"),
        makeSeparator(),
        makeMenuItem("explode"),
        makeMenuItem("implode"),
        makeMenuItem("realize-chord-symbols"),
        makeMenu(qtrc("appshell", "&Voices"), voicesItems),
        makeMenu(qtrc("appshell", "&Measures"), measuresItems),
        makeMenuItem("time-delete"),
        makeSeparator(),
        makeMenuItem("slash-fill"),
        makeMenuItem("slash-rhythm"),
        makeSeparator(),
        makeMenuItem("pitch-spell"),
        makeMenuItem("reset-groupings"),
        makeMenuItem("resequence-rehearsal-marks"),
        makeMenuItem("unroll-repeats"),
        makeSeparator(),
        makeMenuItem("copy-lyrics-to-clipboard"),
        makeMenuItem("fotomode"), // need implement
        makeMenuItem("del-empty-measures"),
    };

    return makeMenu(qtrc("appshell", "&Tools"), toolsItems);
}

MenuItem AppMenuModel::helpItem() const
{
    MenuItemList toursItems {
        makeMenuItem("show-tours"), // need implement
        makeMenuItem("reset-tours") // need implement
    };

    MenuItemList helpItems {
        makeMenuItem("online-handbook"),
        makeMenu(qtrc("appshell", "&Tours"), toursItems),
        makeSeparator(),
        makeMenuItem("about"),
        makeMenuItem("about-qt"),
        makeMenuItem("about-musicxml")
    };

    if (configuration()->isAppUpdatable()) {
        helpItems << makeMenuItem("check-update"); // need implement
    }

    helpItems << makeSeparator()
              << makeMenuItem("ask-help")
              << makeMenuItem("report-bug")
              << makeMenuItem("leave-feedback")
              << makeSeparator()
              << makeMenuItem("revert-factory");

    return makeMenu(qtrc("appshell", "&Help"), helpItems);
}

MenuItem AppMenuModel::diagnosticItem() const
{
    MenuItemList systemItems {
        makeMenuItem("diagnostic-show-paths"),
    };

    MenuItemList accessibilityItems {
        makeMenuItem("diagnostic-show-navigation-tree"),
        makeMenuItem("diagnostic-show-accessible-tree"),
        makeMenuItem("diagnostic-accessible-tree-dump"),
    };

    MenuItemList items {
        makeMenu(qtrc("appshell", "System"), systemItems),
        makeMenu(qtrc("appshell", "Accessibility"), accessibilityItems),
    };

    return makeMenu(qtrc("appshell", "Diagnostic"), items);
}

MenuItemList AppMenuModel::recentScores() const
{
    MenuItemList items;
    ProjectMetaList recentProjects = recentProjectsProvider()->recentProjectList();

    int index = 0;
    for (const ProjectMeta& meta : recentProjects) {
        MenuItem item;
        item.id = makeId(item.code, index++);
        item.code = "file-open";
        item.title = !meta.title.isEmpty() ? meta.title : meta.fileName.toQString();
        item.args = ActionData::make_arg1<io::path>(meta.filePath);
        item.state.enabled = true;
        item.selectable = true;

        items << item;
    }

    return items;
}

MenuItemList AppMenuModel::appendClearRecentSection(const ui::MenuItemList& recentScores) const
{
    MenuItemList result = recentScores;
    result << makeSeparator()
           << makeMenuItem("clear-recent");

    return result;
}

MenuItemList AppMenuModel::notesItems() const
{
    MenuItemList items {
        makeMenuItem("note-input"),
        makeSeparator(),
        makeMenuItem("note-c"),
        makeMenuItem("note-d"),
        makeMenuItem("note-e"),
        makeMenuItem("note-f"),
        makeMenuItem("note-g"),
        makeMenuItem("note-a"),
        makeMenuItem("note-b"),
        makeSeparator(),
        makeMenuItem("chord-c"),
        makeMenuItem("chord-d"),
        makeMenuItem("chord-e"),
        makeMenuItem("chord-f"),
        makeMenuItem("chord-g"),
        makeMenuItem("chord-a"),
        makeMenuItem("chord-b")
    };

    return items;
}

MenuItemList AppMenuModel::intervalsItems() const
{
    MenuItemList items {
        makeMenuItem("interval1"),
        makeMenuItem("interval2"),
        makeMenuItem("interval3"),
        makeMenuItem("interval4"),
        makeMenuItem("interval5"),
        makeMenuItem("interval6"),
        makeMenuItem("interval7"),
        makeMenuItem("interval8"),
        makeMenuItem("interval9"),
        makeSeparator(),
        makeMenuItem("interval-2"),
        makeMenuItem("interval-3"),
        makeMenuItem("interval-4"),
        makeMenuItem("interval-5"),
        makeMenuItem("interval-6"),
        makeMenuItem("interval-7"),
        makeMenuItem("interval-8"),
        makeMenuItem("interval-9")
    };

    return items;
}

MenuItemList AppMenuModel::tupletsItems() const
{
    MenuItemList items {
        makeMenuItem("duplet"),
        makeMenuItem("triplet"),
        makeMenuItem("quadruplet"),
        makeMenuItem("quintuplet"),
        makeMenuItem("sextuplet"),
        makeMenuItem("septuplet"),
        makeMenuItem("octuplet"),
        makeMenuItem("nonuplet"),
        makeMenuItem("tuplet-dialog")
    };

    return items;
}

MenuItemList AppMenuModel::measuresItems() const
{
    MenuItemList items {
        makeMenuItem("insert-measure"),
        makeMenuItem("insert-measures"),
        makeSeparator(),
        makeMenuItem("append-measure"),
        makeMenuItem("append-measures")
    };

    return items;
}

MenuItemList AppMenuModel::framesItems() const
{
    MenuItemList items {
        makeMenuItem("insert-hbox"),
        makeMenuItem("insert-vbox"),
        makeMenuItem("insert-textframe"),
        makeSeparator(),
        makeMenuItem("append-hbox"),
        makeMenuItem("append-vbox"),
        makeMenuItem("append-textframe")
    };

    return items;
}

MenuItemList AppMenuModel::textItems() const
{
    MenuItemList items {
        makeMenuItem("title-text"),
        makeMenuItem("subtitle-text"),
        makeMenuItem("composer-text"),
        makeMenuItem("poet-text"),
        makeMenuItem("part-text"),
        makeSeparator(),
        makeMenuItem("system-text"),
        makeMenuItem("staff-text"),
        makeMenuItem("expression-text"),
        makeMenuItem("rehearsalmark-text"),
        makeMenuItem("instrument-change-text"),
        makeMenuItem("fingering-text"),
        makeSeparator(),
        makeMenuItem("sticking-text"),
        makeMenuItem("chord-text"),
        makeMenuItem("roman-numeral-text"),
        makeMenuItem("nashville-number-text"),
        makeMenuItem("lyrics"),
        makeMenuItem("figured-bass"),
        makeMenuItem("tempo")
    };

    return items;
}

MenuItemList AppMenuModel::linesItems() const
{
    MenuItemList items {
        makeMenuItem("add-slur"),
        makeMenuItem("add-hairpin"),
        makeMenuItem("add-hairpin-reverse"),
        makeMenuItem("add-8va"),
        makeMenuItem("add-8vb"),
        makeMenuItem("add-noteline")
    };

    return items;
}

MenuItemList AppMenuModel::toolbarsItems() const
{
    MenuItemList items {
        makeMenuItem("toggle-transport"),
        makeMenuItem("toggle-noteinput"),
        makeMenuItem("toggle-notationtoolbar"),
        makeMenuItem("toggle-undoredo")
    };

    return items;
}

MenuItemList AppMenuModel::workspacesItems() const
{
    MenuItemList items;

    IWorkspacePtrList workspaces = workspacesManager()->workspaces();
    IWorkspacePtr currentWorkspace = workspacesManager()->currentWorkspace();

    std::sort(workspaces.begin(), workspaces.end(), [](const IWorkspacePtr& workspace1, const IWorkspacePtr& workspace2) {
        return workspace1->name() < workspace2->name();
    });

    int index = 0;
    for (const IWorkspacePtr& workspace : workspaces) {
        MenuItem item = uiactionsRegister()->action("select-workspace");
        item.id = makeId(item.code, index++);
        item.title = QString::fromStdString(workspace->title());
        item.args = ActionData::make_arg1<std::string>(workspace->name());

        item.selectable = true;
        item.selected = (workspace == currentWorkspace);
        item.state.enabled = true;

        items << item;
    }

    items << makeSeparator()
          << makeMenuItem("configure-workspaces");

    return items;
}
