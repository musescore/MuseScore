/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "types/translatablestring.h"

#include "muse_framework_config.h"

#ifdef MUSE_MODULE_WORKSPACE
#include "workspace/view/workspacesmenumodel.h"
#endif

#include "log.h"

using namespace muse;
using namespace mu::appshell;
using namespace muse::ui;
using namespace muse::uicomponents;
using namespace mu::project;
using namespace muse::workspace;
using namespace muse::actions;

static const ActionCode TOGGLE_UNDO_HISTORY_PANEL_CODE = "toggle-undo-history-panel";
static const QString VIEW_TOGGLE_UNDO_HISTORY_PANEL_ITEM_ID = "view/toggle-undo-history-panel";

AppMenuModel::AppMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
}

void AppMenuModel::load()
{
    TRACEFUNC;

    AbstractMenuModel::load();

    MenuItemList items {
        makeEditMenu(),
        makeViewMenu(),
        makeAddMenu(),
        makeFormatMenu(),
        makeToolsMenu()
    };

    if (globalConfiguration()->devModeEnabled()) {
        items << makeDiagnosticsMenu();
    }

    setItems(items);

    setupConnections();
}

bool AppMenuModel::isGlobalMenuAvailable()
{
    return uiConfiguration()->isGlobalMenuAvailable();
}

void AppMenuModel::setupConnections()
{
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        auto stack = undoStack();
        if (stack) {
            stack->stackChanged().onNotify(this, [this]() {
                updateUndoRedoItems();
            });
        }

        updateUndoRedoItems();
    });
}

void AppMenuModel::onActionsStateChanges(const muse::actions::ActionCodeList& codes)
{
    AbstractMenuModel::onActionsStateChanges(codes);

    if (muse::contains(codes, TOGGLE_UNDO_HISTORY_PANEL_CODE)) {
        // Appears both in Edit and View menus; AbstractMenuModel::onActionsStateChanges()
        // handles only one occurrence per action code
        MenuItem& item = findItem(VIEW_TOGGLE_UNDO_HISTORY_PANEL_ITEM_ID);
        if (item.isValid()) {
            item.setState(uiActionsRegister()->actionState(TOGGLE_UNDO_HISTORY_PANEL_CODE));
        }
    }
}

MenuItem* AppMenuModel::makeMenuItem(const ActionCode& actionCode, MenuItemRole menuRole)
{
    MenuItem* item = makeMenuItem(actionCode);
    item->setRole(menuRole);
    return item;
}

MenuItem* AppMenuModel::makeEditMenu()
{
    MenuItemList editItems {
        makeMenuItem("undo"),
        makeMenuItem("redo"),
        makeMenuItem(TOGGLE_UNDO_HISTORY_PANEL_CODE),
        makeSeparator(),
        makeMenuItem("notation-cut"),
        makeMenuItem("notation-copy"),
        makeMenuItem("notation-paste"),
        makeMenuItem("notation-paste-half"),
        makeMenuItem("notation-paste-double"),
        makeMenuItem("notation-swap"),
        makeMenuItem("notation-delete"),
        makeSeparator(),
        makeMenuItem("notation-select-all"),
        makeMenuItem("notation-select-section"),
        makeMenuItem("find"),
    };

    return makeMenu(TranslatableString("appshell/menu/edit", "&Edit"), editItems, "menu-edit");
}

mu::notation::INotationUndoStackPtr AppMenuModel::undoStack() const
{
    mu::notation::INotationPtr notation = globalContext()->currentNotation();
    return notation ? notation->undoStack() : nullptr;
}

void AppMenuModel::updateUndoRedoItems()
{
    auto stack = undoStack();

    MenuItem& undoItem = findItem(ActionCode("undo"));
    const TranslatableString undoActionName = stack ? stack->topMostUndoActionName() : TranslatableString();
    undoItem.setTitle(undoActionName.isEmpty()
                      ? TranslatableString("action", "Undo")
                      : TranslatableString("action", "Undo ‘%1’").arg(undoActionName));

    MenuItem& redoItem = findItem(ActionCode("redo"));
    const TranslatableString redoActionName = stack ? stack->topMostRedoActionName() : TranslatableString();
    redoItem.setTitle(redoActionName.isEmpty()
                      ? TranslatableString("action", "Redo")
                      : TranslatableString("action", "Redo ‘%1’").arg(redoActionName));
}

MenuItem* AppMenuModel::makeViewMenu()
{
    MenuItem* historyItem = makeMenuItem(TOGGLE_UNDO_HISTORY_PANEL_CODE);
    historyItem->setId(VIEW_TOGGLE_UNDO_HISTORY_PANEL_ITEM_ID);

    MenuItemList viewItems {
        makeMenuItem("toggle-palettes"),
        makeMenuItem("masterpalette"),
        makeMenuItem("toggle-instruments"),
        makeMenuItem("inspector"),
        makeMenuItem("toggle-selection-filter"),
        historyItem,
        makeMenuItem("toggle-navigator"),
        makeMenuItem("toggle-braille-panel"),
        makeMenuItem("toggle-timeline"),
        makeMenuItem("toggle-piano-keyboard"),
        makeMenuItem("toggle-percussion-panel"),
        makeSeparator(),
        makeMenu(TranslatableString("appshell/menu/view", "&Toolbars"), makeToolbarsItems(), "menu-toolbars")
    };

#ifdef MUSE_MODULE_WORKSPACE
    viewItems << makeMenu(TranslatableString("appshell/menu/view", "W&orkspaces"), m_workspacesMenuModel->items(), "menu-workspaces"),
#endif

    viewItems << makeSeparator()
              << makeMenu(TranslatableString("appshell/menu/view", "&Show"), makeShowItems(), "menu-show")
              << makeSeparator()
              << makeMenuItem("dock-restore-default-layout");

    return makeMenu(TranslatableString("appshell/menu/view", "&View"), viewItems, "menu-view");
}

MenuItem* AppMenuModel::makeAddMenu()
{
    MenuItemList addItems {
        makeMenu(TranslatableString("appshell/menu/add", "&Notes"), makeNotesItems(), "menu-notes"),
        makeMenu(TranslatableString("appshell/menu/add", "&Intervals"), makeIntervalsItems(), "menu-intervals"),
        makeMenu(TranslatableString("appshell/menu/add", "T&uplets"), makeTupletsItems(), "menu-tuplets"),
        makeSeparator(),
        makeMenu(TranslatableString("appshell/menu/add", "&Measures"), makeMeasuresItems(), "menu-measures"),
        makeMenu(TranslatableString("appshell/menu/add", "&Frames"), makeFramesItems(), "menu-frames"),
        makeMenu(TranslatableString("appshell/menu/add", "&Text"), makeTextItems(), "menu-notes"),
        makeMenu(TranslatableString("appshell/menu/add", "&Lines"), makeLinesItems(), "menu-lines"),
    };

    return makeMenu(TranslatableString("appshell/menu/add", "&Add"), addItems, "menu-add");
}

MenuItem* AppMenuModel::makeFormatMenu()
{
    MenuItemList stretchItems {
        makeMenuItem("stretch+"),
        makeMenuItem("stretch-"),
        makeMenuItem("reset-stretch")
    };

    MenuItemList formatItems {
        makeMenuItem("edit-style"),
        makeMenuItem("page-settings"),
        makeSeparator(),
        makeMenuItem("measures-per-system"),
        makeMenu(TranslatableString("appshell/menu/format", "Str&etch"), stretchItems, "menu-stretch"),
        makeSeparator(),
        makeMenuItem("reset-text-style-overrides"),
        makeMenuItem("reset-beammode"),
        makeMenuItem("reset"),
        makeMenuItem("reset-to-default-layout"),
    };

    return makeMenu(TranslatableString("appshell/menu/format", "F&ormat"), formatItems, "menu-format");
}

MenuItem* AppMenuModel::makeToolsMenu()
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
        makeMenu(TranslatableString("appshell/menu/tools", "&Voices"), voicesItems, "menu-voices"),
        makeMenu(TranslatableString("appshell/menu/tools", "&Measures"), measuresItems, "menu-tools-measures"),
        makeMenuItem("time-delete"),
        makeSeparator(),
        makeMenuItem("slash-fill"),
        makeMenuItem("slash-rhythm"),
        makeSeparator(),
        makeMenuItem("pitch-spell"),
        makeMenuItem("reset-groupings"),
        makeMenuItem("resequence-rehearsal-marks"),
        /*
         * TODO: https://github.com/musescore/MuseScore/issues/9670
        makeMenuItem("unroll-repeats"),
         */
        makeSeparator(),
        makeMenuItem("del-empty-measures"),
    };

    return makeMenu(TranslatableString("appshell/menu/tools", "&Tools"), toolsItems, "menu-tools");
}

MenuItem* AppMenuModel::makeDiagnosticsMenu()
{
    MenuItemList items {
        makeMenuItem("file-open"),
    };

    return makeMenu(TranslatableString("appshell/menu/diagnostics", "&Diagnostics"), items, "menu-diagnostic");
}

MenuItemList AppMenuModel::makeNotesItems()
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

MenuItemList AppMenuModel::makeIntervalsItems()
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

MenuItemList AppMenuModel::makeTupletsItems()
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

MenuItemList AppMenuModel::makeMeasuresItems()
{
    MenuItemList items {
        makeMenuItem("insert-measures-after-selection", TranslatableString("notation", "Insert &after selection…")),
        makeMenuItem("insert-measures", TranslatableString("notation", "Insert &before selection…")),
        makeSeparator(),
        makeMenuItem("insert-measures-at-start-of-score", TranslatableString("notation", "Insert at &start of score…")),
        makeMenuItem("append-measures", TranslatableString("notation", "Insert at &end of score…"))
    };

    return items;
}

MenuItemList AppMenuModel::makeFramesItems()
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

MenuItemList AppMenuModel::makeTextItems()
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
        makeMenuItem("add-dynamic"),
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

MenuItemList AppMenuModel::makeLinesItems()
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

MenuItemList AppMenuModel::makeToolbarsItems()
{
    MenuItemList items {
        makeMenuItem("toggle-transport"),
        makeMenuItem("toggle-noteinput"),
        makeMenuItem("toggle-statusbar")
    };

    return items;
}

MenuItemList AppMenuModel::makeShowItems()
{
    MenuItemList items {
        makeMenuItem("show-invisible"),
        makeMenuItem("show-unprintable"),
        makeMenuItem("show-frames"),
        makeMenuItem("show-pageborders"),
        makeMenuItem("show-irregular"),
        makeMenuItem("show-soundflags"),
    };

    return items;
}
