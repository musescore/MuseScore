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

#include "log.h"

using namespace muse;
using namespace mu::appshell;
using namespace muse::ui;
using namespace muse::uicomponents;
using namespace mu::project;
using namespace muse::workspace;
using namespace muse::actions;
using namespace muse::extensions;

static const ActionCode TOGGLE_UNDO_HISTORY_PANEL_CODE = "toggle-undo-history-panel";
static const QString VIEW_TOGGLE_UNDO_HISTORY_PANEL_ITEM_ID = "view/toggle-undo-history-panel";

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
        makeFileMenu(),
        makeEditMenu(),
        makeViewMenu(),
        makeAddMenu(),
        makeFormatMenu(),
        makeToolsMenu(),
        makePluginsMenu(),
    };

    if (globalConfiguration()->devModeEnabled()) {
        items << makeHelpMenu(false);
        items << makeDiagnosticsMenu();
    } else {
        items << makeHelpMenu(true);
    }

    setItems(items);

    setupConnections();

    //! NOTE: removes some undesired platform-specific items
    //! (such as "Start Dictation" and "Special Characters" on macOS)
    appMenuModelHook()->onAppMenuInited();
}

bool AppMenuModel::isGlobalMenuAvailable()
{
    return uiConfiguration()->isGlobalMenuAvailable();
}

void AppMenuModel::setupConnections()
{
    recentFilesController()->recentFilesListChanged().onNotify(this, [this]() {
        MenuItem& recentScoreListItem = findMenu("menu-file-open");

        MenuItemList recentScoresList = makeRecentScoresItems();
        bool openRecentEnabled = !recentScoresList.empty();

        if (!recentScoresList.empty()) {
            recentScoresList = appendClearRecentSection(recentScoresList);
        }

        UiActionState state = recentScoreListItem.state();
        state.enabled = openRecentEnabled;
        recentScoreListItem.setState(state);

        recentScoreListItem.setSubitems(recentScoresList);
    });

    workspacesManager()->currentWorkspaceChanged().onNotify(this, [this]() {
        MenuItem& workspacesItem = findMenu("menu-workspaces");
        workspacesItem.setSubitems(makeWorkspacesItems());
    });

    workspacesManager()->workspacesListChanged().onNotify(this, [this]() {
        MenuItem& workspacesItem = findMenu("menu-workspaces");
        workspacesItem.setSubitems(makeWorkspacesItems());
    });

    extensionsProvider()->manifestListChanged().onNotify(this, [this]() {
        MenuItem& pluginsMenu = findMenu("menu-plugins");
        pluginsMenu.setSubitems(makePluginsMenuSubitems());
    });

    extensionsProvider()->manifestChanged().onReceive(this, [this](const Manifest&) {
        MenuItem& pluginsItem = findMenu("menu-plugins");
        pluginsItem.setSubitems(makePluginsMenuSubitems());
    });

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

MenuItem* AppMenuModel::makeFileMenu()
{
    MenuItemList recentScoresList = makeRecentScoresItems();
    bool openRecentEnabled = !recentScoresList.isEmpty();

    if (!recentScoresList.empty()) {
        recentScoresList = appendClearRecentSection(recentScoresList);
    }

    MenuItemList fileItems {
        makeMenuItem("file-new"),
        makeMenuItem("file-open"),
        makeMenu(TranslatableString("appshell/menu/file", "Open &recent"), recentScoresList, "menu-file-open", openRecentEnabled),
        makeSeparator(),
        makeMenuItem("file-close"),
        makeMenuItem("file-save"),
        makeMenuItem("file-save-as"),
        makeMenuItem("file-save-a-copy"),
        makeMenuItem("file-save-selection"),
        makeMenuItem("file-save-to-cloud"),
        makeMenuItem("file-publish"),
        makeSeparator(),
        makeMenuItem("file-import-pdf"),
        makeMenuItem("file-export"),
        makeMenuItem("file-share-audio"),
        makeSeparator(),
        makeMenuItem("project-properties"),
        makeSeparator(),
        makeMenuItem("print"),
        makeSeparator(),
        makeMenuItem("quit", MenuItemRole::QuitRole)
    };

    return makeMenu(TranslatableString("appshell/menu/file", "&File"), fileItems, "menu-file");
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
        makeSeparator(),
        makeMenuItem("preference-dialog", MenuItemRole::PreferencesRole)
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
#ifndef Q_OS_MAC
        makeMenuItem("fullscreen"),
#endif
        makeMenuItem("toggle-palettes"),
        makeMenuItem("masterpalette"),
        makeMenuItem("toggle-instruments"),
        makeMenuItem("inspector"),
        makeMenuItem("toggle-selection-filter"),
        historyItem,
        makeMenuItem("toggle-navigator"),
        makeMenuItem("toggle-braille-panel"),
        makeMenuItem("toggle-timeline"),
        makeMenuItem("toggle-mixer"),
        makeMenuItem("toggle-piano-keyboard"),
        makeMenuItem("toggle-percussion-panel"),
        makeMenuItem("playback-setup"),
        //makeMenuItem("toggle-scorecmp-tool"), // not implemented
        makeSeparator(),
        makeMenu(TranslatableString("appshell/menu/view", "&Toolbars"), makeToolbarsItems(), "menu-toolbars"),
        makeMenu(TranslatableString("appshell/menu/view", "W&orkspaces"), makeWorkspacesItems(), "menu-workspaces"),
        makeSeparator(),
        makeMenu(TranslatableString("appshell/menu/view", "&Show"), makeShowItems(), "menu-show"),
        makeSeparator(),
        makeMenuItem("dock-restore-default-layout")
    };

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
        makeSeparator(),
        makeMenuItem("load-style"),
        makeMenuItem("save-style")
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
        makeMenuItem("copy-lyrics-to-clipboard"),
        makeMenuItem("del-empty-measures"),
    };

    return makeMenu(TranslatableString("appshell/menu/tools", "&Tools"), toolsItems, "menu-tools");
}

MenuItem* AppMenuModel::makePluginsMenu()
{
    return makeMenu(TranslatableString("appshell/menu/plugins", "&Plugins"), makePluginsMenuSubitems(), "menu-plugins");
}

MenuItemList AppMenuModel::makePluginsMenuSubitems()
{
    MenuItemList subitems {
        makeMenuItem("manage-plugins"),
    };

    MenuItemList enabledPlugins = makePluginsItems();

    if (!enabledPlugins.empty()) {
        subitems << makeSeparator();
    }

    subitems << enabledPlugins;

    return subitems;
}

MenuItem* AppMenuModel::makeHelpMenu(bool addDiagnosticsSubMenu)
{
    MenuItemList helpItems;

    if (updateConfiguration()->isAppUpdatable()) {
        helpItems << makeMenuItem("check-update");
        helpItems << makeSeparator();
    }

    helpItems << makeMenuItem("online-handbook");
    helpItems << makeMenuItem("ask-help");
    helpItems << makeSeparator();

    if (addDiagnosticsSubMenu) {
        helpItems << makeDiagnosticsMenu();
        helpItems << makeSeparator();
    }

    helpItems << makeMenuItem("about-musescore", MenuItemRole::AboutRole);
    helpItems << makeMenuItem("about-qt", MenuItemRole::AboutQtRole);
    helpItems << makeMenuItem("about-musicxml");

    helpItems << makeSeparator();
    helpItems << makeMenuItem("revert-factory");

    return makeMenu(TranslatableString("appshell/menu/help", "&Help"), helpItems, "menu-help");
}

MenuItem* AppMenuModel::makeDiagnosticsMenu()
{
    MenuItemList systemItems {
        makeMenuItem("diagnostic-show-paths"),
        makeMenuItem("diagnostic-show-graphicsinfo"),
        makeMenuItem("diagnostic-show-profiler"),
    };

    MenuItemList items {
        makeMenuItem("diagnostic-save-diagnostic-files"),
        makeMenuItem("playback-reload-cache"),
        makeMenu(TranslatableString("appshell/menu/diagnostics", "&System"), systemItems, "menu-system")
    };

#ifdef MUSE_MODULE_MUSESAMPLER
    bool isMuseSamplerModuleAdded = museSamplerInfo() != nullptr;
    if (isMuseSamplerModuleAdded) {
        MenuItemList museSamplerItems {
            makeMenuItem("musesampler-check"),
        };

        if (globalConfiguration()->devModeEnabled()) {
            museSamplerItems << makeMenuItem("musesampler-reload");
        }

        items << makeMenu(TranslatableString("appshell/menu/diagnostics", "&Muse Sampler"), museSamplerItems, "menu-musesampler");
    }
#endif

    if (globalConfiguration()->devModeEnabled()) {
        MenuItemList actionsItems {
            makeMenuItem("diagnostic-show-actions"),
            makeMenuItem("action://diagnostic/actions/query"),
            makeMenuItem("action://diagnostic/actions/query_params1?param1=val1"),
            makeMenuItem("action://diagnostic/actions/query_params2?param1=val1")
        };

        MenuItemList accessibilityItems {
            makeMenuItem("diagnostic-show-navigation-tree"),
            makeMenuItem("diagnostic-show-accessible-tree"),
            makeMenuItem("diagnostic-accessible-tree-dump"),
        };

        MenuItemList engravingItems {
            makeMenuItem("diagnostic-show-engraving-elements"),
            makeSeparator(),
            makeMenuItem("show-element-bounding-rects"),
            makeMenuItem("color-element-shapes"),
            makeMenuItem("show-segment-shapes"),
            makeMenuItem("color-segment-shapes"),
            makeMenuItem("show-skylines"),
            makeMenuItem("show-system-bounding-rects"),
            makeMenuItem("show-element-masks"),
            makeMenuItem("mark-corrupted-measures"),
            makeMenuItem("check-for-score-corruptions")
        };

        MenuItemList extensionsItems {
            makeMenuItem("extensions-show-apidump"),
        };

        MenuItemList autobotItems {
            makeMenuItem("autobot-show-scripts"),
        };

        MenuItemList vstItems {
            makeMenuItem("vst-use-oldview"),
            makeMenuItem("vst-use-newview"),
        };

        items << makeMenu(TranslatableString("appshell/menu/diagnostics", "A&ctions"), actionsItems, "menu-actions")
              << makeMenu(TranslatableString("appshell/menu/diagnostics", "&Accessibility"), accessibilityItems, "menu-accessibility")
              << makeMenu(TranslatableString("appshell/menu/diagnostics", "&Engraving"), engravingItems, "menu-engraving")
              << makeMenu(TranslatableString("appshell/menu/diagnostics", "E&xtensions"), extensionsItems, "menu-extensions")
              << makeMenu(TranslatableString("appshell/menu/diagnostics", "Auto&bot"), autobotItems, "menu-autobot")
              << makeMenu(TranslatableString("appshell/menu/diagnostics", "&VST"), vstItems, "menu-vst")
              << makeMenuItem("multiinstances-dev-show-info");
    }

    return makeMenu(TranslatableString("appshell/menu/diagnostics", "&Diagnostics"), items, "menu-diagnostic");
}

MenuItemList AppMenuModel::makeRecentScoresItems()
{
    MenuItemList items;
    const RecentFilesList& recentFiles = recentFilesController()->recentFilesList();

    int index = 0;
    for (const RecentFile& file : recentFiles) {
        MenuItem* item = new MenuItem(this);

        UiAction action;
        action.code = "file-open";
        action.title = TranslatableString::untranslatable(file.displayName(/*includingExtension*/ true));
        bool isCloud = projectConfiguration()->isCloudProject(file.path);

        if (isCloud) {
            action.iconCode = IconCode::Code::CLOUD;
        }

        item->setAction(action);

        item->setId(makeId(item->action().code, index++));

        UiActionState state;
        state.enabled = true;
        item->setState(state);

        item->setSelectable(true);
        item->setArgs(ActionData::make_arg2<QUrl, QString>(file.path.toQUrl(), file.displayNameOverride));

        items << item;
    }

    return items;
}

MenuItemList AppMenuModel::appendClearRecentSection(const muse::uicomponents::MenuItemList& recentScores)
{
    MenuItemList result = recentScores;
    result << makeSeparator()
           << makeMenuItem("clear-recent");

    return result;
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

MenuItemList AppMenuModel::makeWorkspacesItems()
{
    MenuItemList items;

    IWorkspacePtrList workspaces = workspacesManager()->workspaces();
    IWorkspacePtr currentWorkspace = workspacesManager()->currentWorkspace();

    std::sort(workspaces.begin(), workspaces.end(), [](const IWorkspacePtr& workspace1, const IWorkspacePtr& workspace2) {
        return workspace1->name() < workspace2->name();
    });

    int index = 0;
    for (const IWorkspacePtr& workspace : workspaces) {
        MenuItem* item = new MenuItem(uiActionsRegister()->action("select-workspace"), this);
        item->setId(makeId(item->action().code, index++));

        UiAction action = item->action();
        action.title = TranslatableString::untranslatable(String::fromStdString(workspace->title()));

        item->setAction(action);
        item->setArgs(ActionData::make_arg1<std::string>(workspace->name()));
        item->setSelectable(true);
        item->setSelected(workspace == currentWorkspace);

        UiActionState state;
        state.enabled = true;
        state.checked = item->selected();
        item->setState(state);

        items << item;
    }

    items << makeSeparator()
          << makeMenuItem("configure-workspaces");

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

MenuItemList AppMenuModel::makePluginsItems()
{
    MenuItemList result;

    KnownCategories categories = extensionsProvider()->knownCategories();
    ManifestList enabledExtensions = extensionsProvider()->manifestList(Filter::Enabled);

    auto addMenuItems = [this](MenuItemList& items, const Manifest& m) {
        if (m.actions.size() == 1) {
            const muse::extensions::Action& a = m.actions.at(0);
            if (!a.showOnAppmenu) {
                return;
            }
            items << makeMenuItem(makeActionQuery(m.uri, a.code).toString(), TranslatableString::untranslatable(a.title));
        } else {
            MenuItemList sub;
            for (const muse::extensions::Action& a : m.actions) {
                if (!a.showOnAppmenu) {
                    continue;
                }
                sub << makeMenuItem(makeActionQuery(m.uri, a.code).toString(), TranslatableString::untranslatable(a.title));
            }

            if (!sub.empty()) {
                items << makeMenu(TranslatableString::untranslatable(m.title), sub);
            }
        }
    };

    std::map<std::string, MenuItemList> categoriesMap;
    MenuItemList pluginsWithoutCategories;
    for (const Manifest& m : enabledExtensions) {
        std::string categoryStr = m.category.toStdString();
        if (!categoryStr.empty()) {
            if (!muse::contains(categories, categoryStr)) {
                categories[categoryStr] = TranslatableString("extensions", m.category);
            }
            MenuItemList& items = categoriesMap[categoryStr];
            addMenuItems(items, m);
        } else {
            addMenuItems(pluginsWithoutCategories, m);
        }
    }

    for (const auto& it : categoriesMap) {
        TranslatableString categoryTitle = muse::value(categories, it.first, {});
        result << makeMenu(categoryTitle, it.second);
    }

    std::sort(result.begin(), result.end(), [](const MenuItem& l, const MenuItem& r) {
        return l.translatedTitle() < r.translatedTitle();
    });

    std::sort(pluginsWithoutCategories.begin(), pluginsWithoutCategories.end(), [](const MenuItem& l, const MenuItem& r) {
        return l.translatedTitle() < r.translatedTitle();
    });

    for (MenuItem* plugin : pluginsWithoutCategories) {
        result << plugin;
    }

    return result;
}
