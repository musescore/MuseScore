/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "log.h"
#include "rcommand/commandtypes.h"
#include "types/translatablestring.h"

#include "muse_framework_config.h"

#ifdef MUSE_MODULE_WORKSPACE
#include "workspace/qml/Muse/Workspace/workspacesmenumodel.h"
#endif

#include "notation/inotation.h"
#include "notation/inotationundostack.h"
#include "notationscene/notationcommands.h"

#include "appshell/appshellcommands.h"
#include "project/projectcommands.h"
#include "notationscene/notationcommands.h"
#include "playback/playbackcommands.h"
#include "palette/palettecommands.h"
#include "dockwindow_v2/dockcommands.h"
#include "update/updatecommands.h"
#include "diagnostics/diagnosticscommands.h"
#include "musesampler/musesamplercommands.h"
#include "vst/vstcommands.h"
#include "audio/main/audiocommands.h"
#include "multiwindows/multiwindowscommands.h"
#include "extensions/extensionscommands.h"

using namespace muse;
using namespace mu::appshell;
using namespace muse::ui;
using namespace muse::uicomponents;
using namespace mu::project;
using namespace muse::workspace;
using namespace muse::actions;
using namespace muse::extensions;
using namespace mu::notation;
using namespace mu::playback;
using namespace mu::palette;
using namespace muse::dock;
using namespace muse::update;
using namespace muse::diagnostics;
using namespace muse::musesampler;
using namespace muse::vst;
using namespace muse::audio;
using namespace muse::mi;

AppMenuModel::AppMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
    setObjectName("AppMenuModel");

#ifdef MUSE_MODULE_WORKSPACE
    m_workspacesMenuModel = std::make_shared<WorkspacesMenuModel>(this);
#endif
}

void AppMenuModel::load()
{
    TRACEFUNC;

    AbstractMenuModel::load();

#ifdef MUSE_MODULE_WORKSPACE
    m_workspacesMenuModel->load();
#endif

    MenuItemList items {
        makeFileMenu(),
        makeEditMenu(),
        makeViewMenu(),
        makeAddMenu(),
        makeFormatMenu(),
        makeToolsMenu(),
        makeExtensionsMenu()
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
        MenuItem& recentMenuItem = findMenu("menu-file-open");
        MenuItemList recentSubMenuItems = makeRecentSubMenuItems();
        recentMenuItem.setSubitems(recentSubMenuItems);
        recentMenuItem.setEnabled(!recentSubMenuItems.empty());
    });

#ifdef MUSE_MODULE_WORKSPACE
    connect(m_workspacesMenuModel.get(), &WorkspacesMenuModel::itemsChanged, this, [this]() {
        MenuItem& workspacesItem = findMenu("menu-workspaces");
        workspacesItem.setSubitems(m_workspacesMenuModel->items());
    });
#endif

    extensionsRegister()->manifestListChanged().onNotify(this, [this]() {
        MenuItem& pluginsMenu = findMenu("menu-extensions");
        pluginsMenu.setSubitems(makeExtensionsSubitems());
    });

    extensionsRegister()->enabledChanged().onReceive(this, [this](const ExtensionUri&) {
        MenuItem& pluginsItem = findMenu("menu-extensions");
        pluginsItem.setSubitems(makeExtensionsSubitems());
    });

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        auto stack = undoStack();
        if (stack) {
            stack->stackChanged().onNotify(this, [this]() {
                updateUndoRedoItems();
            }, Asyncable::Mode::SetReplace /* FIXME */);
        }

        updateUndoRedoItems();
    });
}

bool AppMenuModel::isMuseSamplerModuleAdded() const
{
#ifdef MUSE_MODULE_MUSESAMPLER
    return museSamplerInfo() != nullptr;
#else
    return false;
#endif
}

MenuItemList AppMenuModel::makeChordAndFretboardDiagramsItems()
{
    MenuItemList items {
        makeMenuItem(ADD_CHORD_TEXT_COMMAND),
        makeMenuItem(ADD_FRETBOARD_DIAGRAM_COMMAND),
        makeSeparator(),
        makeMenuItem(INSERT_FRETFRAME_COMMAND, TranslatableString("appshell/menu/add/chordandfret", "Fretboard diagram legend"))
    };

    return items;
}

MenuItem* AppMenuModel::makeMenuItem(const muse::rcommand::Command& command, MenuItemRole role)
{
    MenuItem* item = makeMenuItem(command);
    item->setRole(role);
    return item;
}

MenuItem* AppMenuModel::makeFileMenu()
{
    MenuItemList recentSubMenuItems = makeRecentSubMenuItems();
    bool openRecentEnabled = !recentSubMenuItems.isEmpty();

    MenuItemList fileItems {
        makeMenuItem(PROJECT_NEW_COMMAND),
        makeMenuItem(PROJECT_OPEN_COMMAND),
        makeMenu(TranslatableString("appshell/menu/file", "Open &recent"), recentSubMenuItems, "menu-file-open", openRecentEnabled),
        makeMenuItem(PROJECT_CLOSE_COMMAND),
        makeSeparator(),
        makeMenuItem(PROJECT_SAVE_COMMAND),
        makeMenuItem(PROJECT_SAVE_AS_COMMAND),
        makeMenuItem(PROJECT_SAVE_TO_CLOUD_COMMAND),
        makeMenu(TranslatableString("appshell/menu/file", "Save o&ther"), {
            makeMenuItem(PROJECT_SAVE_A_COPY_COMMAND),
            makeMenuItem(PROJECT_SAVE_SELECTION_COMMAND),
        }),
        makeMenu(TranslatableString("appshell/menu/file", "Pu&blish online"), {
            makeMenuItem(PROJECT_PUBLISH_COMMAND),
            makeMenuItem(PROJECT_SHARED_AUDIO_COMMAND),
        }),
        makeSeparator(),
        makeMenuItem(PROJECT_IMPORT_PDF_COMMAND),
        makeMenuItem(PROJECT_IMPORT_AUDIO_TO_SCORE_COMMAND),
        makeMenuItem(PROJECT_EXPORT_COMMAND),
        makeSeparator(),
        makeMenuItem(PROJECT_PROPERTIES_COMMAND),
        makeMenuItem(OPEN_PARTS_COMMAND),
        makeSeparator(),
        makeMenuItem(PROJECT_PRINT_COMMAND),
        makeSeparator(),
        makeMenuItem(APP_QUIT_COMMAND, MenuItemRole::QuitRole)
    };

    return makeMenu(TranslatableString("appshell/menu/file", "&File"), fileItems, "menu-file");
}

MenuItem* AppMenuModel::makeEditMenu()
{
    MenuItemList editItems {
        makeMenuItem(UNDO_COMMAND),
        makeMenuItem(REDO_COMMAND),
        makeMenuItem(DOCK_TOGGLE_UNDO_HISTORY_COMMAND),
        makeSeparator(),
        makeMenuItem(CUT_COMMAND),
        makeMenuItem(COPY_COMMAND),
        makeMenuItem(PASTE_COMMAND),
        makeMenuItem(PASTE_HALF_COMMAND),
        makeMenuItem(PASTE_DOUBLE_COMMAND),
        makeMenuItem(COPY_PASTE_SWAP_COMMAND),
        makeMenuItem(DELETE_COMMAND),
        makeSeparator(),
        makeMenuItem(SELECT_ALL_COMMAND),
        makeMenuItem(SELECT_SECTION_COMMAND),
        makeMenuItem(SHOW_SEARCH_COMMAND),
        makeSeparator(),
        makeMenuItem(APP_PREFERENCES_COMMAND, MenuItemRole::PreferencesRole)
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

    MenuItem& undoItem = findItem(UNDO_COMMAND);
    const TranslatableString undoActionName = stack ? stack->topMostUndoActionName() : TranslatableString();
    undoItem.setTitle(undoActionName.isEmpty()
                      ? TranslatableString("action", "Undo")
                      : TranslatableString("action", "Undo ‘%1’").arg(undoActionName));

    MenuItem& redoItem = findItem(REDO_COMMAND);
    const TranslatableString redoActionName = stack ? stack->topMostRedoActionName() : TranslatableString();
    redoItem.setTitle(redoActionName.isEmpty()
                      ? TranslatableString("action", "Redo")
                      : TranslatableString("action", "Redo ‘%1’").arg(redoActionName));
}

MenuItem* AppMenuModel::makeViewMenu()
{
    MenuItemList viewItems {
#ifndef Q_OS_MAC
        makeMenuItem(APP_FULLSCREEN_COMMAND),
#endif
        makeMenuItem(DOCK_TOGGLE_PALETTES_COMMAND),
        makeMenuItem(TOGGLE_MASTER_PALETTE_COMMAND),
        makeMenuItem(DOCK_TOGGLE_INSTRUMENTS_COMMAND),
        makeMenuItem(DOCK_TOGGLE_PROPERTIES_COMMAND),
        makeMenuItem(DOCK_TOGGLE_SELECTION_FILTER_COMMAND),
        makeMenuItem(DOCK_TOGGLE_UNDO_HISTORY_COMMAND),
        makeMenuItem(DOCK_TOGGLE_NAVIGATOR_COMMAND),
        makeMenuItem(DOCK_TOGGLE_BRAILLE_COMMAND),
        makeMenuItem(DOCK_TOGGLE_TIMELINE_COMMAND),
        makeMenuItem(DOCK_TOGGLE_MIXER_COMMAND),
        makeMenuItem(DOCK_TOGGLE_PIANO_KEYBOARD_COMMAND),
        makeMenuItem(DOCK_TOGGLE_PERCUSSION_COMMAND),
        makeMenuItem(OPEN_PLAYBACK_SETUP_COMMAND),
        makeSeparator(),
        makeMenu(TranslatableString("appshell/menu/view", "&Toolbars"), {
            makeMenuItem(DOCK_TOGGLE_PLAYBACK_COMMAND),
            makeMenuItem(DOCK_TOGGLE_NOTEINPUT_COMMAND),
            makeMenuItem(DOCK_TOGGLE_STATUSBAR_COMMAND)
        }, "menu-toolbars"),
#ifdef MUSE_MODULE_WORKSPACE
        makeMenu(TranslatableString("appshell/menu/view", "W&orkspaces"), m_workspacesMenuModel->items(), "menu-workspaces"),
#endif
        makeSeparator(),
        makeMenu(TranslatableString("appshell/menu/view", "&Show"), {
            makeMenuItem(SHOW_INVISIBLE_COMMAND),
            makeMenuItem(SHOW_UNPRINTABLE_COMMAND),
            makeMenuItem(SHOW_FRAMES_COMMAND),
            makeMenuItem(SHOW_PAGEBORDERS_COMMAND),
            makeMenuItem(SHOW_IRREGULAR_COMMAND),
            makeMenuItem(SHOW_SOUNDFLAGS_COMMAND),
        }, "menu-show"),
        makeSeparator(),
        makeMenuItem(DOCK_RESTORE_DEFAULT_LAYOUT_COMMAND),
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
        makeMenu(TranslatableString("appshell/menu/add", "&Text"), makeTextItems(), "menu-text"),
        makeMenu(TranslatableString("appshell/menu/add", "&Lines"), makeLinesItems(), "menu-lines"),
        makeMenu(TranslatableString("appshell/menu/add", "&Chords and fretboard diagrams"),
                 makeChordAndFretboardDiagramsItems(), "menu-chord-and-frets"),
    };

    return makeMenu(TranslatableString("appshell/menu/add", "&Add"), addItems, "menu-add");
}

MenuItem* AppMenuModel::makeFormatMenu()
{
    MenuItemList formatItems {
        makeMenuItem(OPEN_EDIT_STYLE_COMMAND),
        makeMenuItem(OPEN_PAGE_SETTINGS_COMMAND),
        makeSeparator(),
        makeMenuItem(OPEN_BREAKS_COMMAND),
        makeMenu(TranslatableString("appshell/menu/format", "Str&etch"), {
            makeMenuItem(STRETCH_INCREASE_COMMAND),
            makeMenuItem(STRETCH_DECREASE_COMMAND),
            makeMenuItem(STRETCH_RESET_COMMAND)
        }, "menu-stretch"),
        makeSeparator(),
        makeMenuItem(RESET_TEXT_STYLE_OVERRIDES_COMMAND),
        makeMenuItem(RESET_BEAMS_COMMAND),
        makeMenuItem(RESET_SHAPES_AND_POSITIONS_COMMAND),
        makeMenuItem(RESET_TO_DEFAULT_LAYOUT_COMMAND),
        makeSeparator(),
        makeMenuItem(LOAD_STYLE_COMMAND),
        makeMenuItem(SAVE_STYLE_COMMAND)
    };

    return makeMenu(TranslatableString("appshell/menu/format", "F&ormat"), formatItems, "menu-format");
}

MenuItem* AppMenuModel::makeToolsMenu()
{
    MenuItemList toolsItems {
        makeMenuItem(OPEN_TRANSPOSE_COMMAND),
        makeSeparator(),
        makeMenuItem(STAFF_EXPLODE_COMMAND),
        makeMenuItem(STAFF_IMPLODE_COMMAND),
        makeMenuItem(OPEN_REALIZECHORDSYMBOLS_COMMAND),
        makeMenu(TranslatableString("appshell/menu/tools", "&Voices"), {
            makeMenuItem(SWAP_VOICE_X12_COMMAND),
            makeMenuItem(SWAP_VOICE_X13_COMMAND),
            makeMenuItem(SWAP_VOICE_X14_COMMAND),
            makeMenuItem(SWAP_VOICE_X23_COMMAND),
            makeMenuItem(SWAP_VOICE_X24_COMMAND),
            makeMenuItem(SWAP_VOICE_X34_COMMAND)
        }, "menu-voices"),
        makeMenu(TranslatableString("appshell/menu/tools", "&Measures"), {
            makeMenuItem(SPLIT_MEASURE_COMMAND),
            makeMenuItem(JOIN_MEASURES_COMMAND)
        }, "menu-tools-measures"),
        makeMenuItem(REMOVE_SELECTED_RANGE_COMMAND),
        makeSeparator(),
        makeMenuItem(SLASH_FILL_COMMAND),
        makeMenuItem(SLASH_RHYTHM_COMMAND),
        makeSeparator(),
        makeMenuItem(PITCH_SPELL_SHARPS_COMMAND),
        makeMenuItem(PITCH_SPELL_FLATS_COMMAND),
        makeMenu(TranslatableString("appshell/menu/tools", "Enharmonic spelling"), {
            makeMenuItem(ENHARMONIC_SPELL_BOTH_COMMAND),
            makeMenuItem(ENHARMONIC_SPELL_CURRENT_COMMAND),
            makeMenuItem(PITCH_SPELL_COMMAND),
        }, "menu-enharmonic-spelling"),
        makeSeparator(),
        makeMenuItem(REGROUP_RHYTHMS_COMMAND),
        makeMenuItem(RESEQUENCE_REHEARSAL_MARKS_COMMAND),
        makeSeparator(),
        makeMenuItem(COPY_LYRICS_COMMAND),
        makeMenuItem(REMOVE_EMPTY_TRAILING_MEASURES_COMMAND),
    };

    return makeMenu(TranslatableString("appshell/menu/tools", "&Tools"), toolsItems, "menu-tools");
}

muse::uicomponents::MenuItemList AppMenuModel::makeExtensionsSubitems()
{
    MenuItemList subitems {
        makeMenuItem(APP_EXTENSIONS_COMMAND),
    };

    MenuItemList extensions = makeExtensionsItems();

    if (!extensions.empty()) {
        subitems << makeSeparator();
    }

    subitems << extensions;
    return subitems;
}

MenuItem* AppMenuModel::makeExtensionsMenu()
{
    return makeMenu(TranslatableString("appshell/menu/plugins", "E&xtensions"), makeExtensionsSubitems(), "menu-extensions");
}

MenuItem* AppMenuModel::makeHelpMenu(bool addDiagnosticsSubMenu)
{
    MenuItemList helpItems;

    if (updateConfiguration()->isAppUpdatable()) {
        helpItems << makeMenuItem(UPDATE_CHECK_COMMAND);
        helpItems << makeSeparator();
    }

    helpItems << makeMenuItem(APP_ONLINE_HANDBOOK_COMMAND);
    helpItems << makeMenuItem(APP_ASK_HELP_COMMAND);
    helpItems << makeSeparator();

    if (addDiagnosticsSubMenu) {
        helpItems << makeDiagnosticsMenu();
        helpItems << makeSeparator();
    }

    helpItems << makeMenuItem(APP_ABOUT_MUSESCORE_COMMAND, MenuItemRole::AboutRole);
    helpItems << makeMenuItem(APP_ABOUT_QT_COMMAND, MenuItemRole::AboutQtRole);
    helpItems << makeMenuItem(APP_ABOUT_MUSICXML_COMMAND);
    helpItems << makeMenuItem(APP_ACCESSIBILITY_STATEMENT_COMMAND);
    helpItems << makeSeparator();

#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    if (isMuseSamplerModuleAdded()) {
        helpItems << makeMenuItem(CLEAR_ONLINESOUNDS_CACHE_COMMAND);
        helpItems << makeSeparator();
    }
#endif

    helpItems << makeMenuItem(APP_REVERT_TO_FACTORY_COMMAND);

    return makeMenu(TranslatableString("appshell/menu/help", "&Help"), helpItems, "menu-help");
}

MenuItem* AppMenuModel::makeDiagnosticsMenu()
{
    MenuItemList items {
        makeMenuItem(DIAGNOSTICS_SAVE_FILES_COMMAND),
        makeMenuItem(RELOAD_PLAYBACK_CACHE_COMMAND),
        makeMenu(TranslatableString("appshell/menu/diagnostics", "&System"), {
            makeMenuItem(DIAGNOSTICS_SHOW_PATHS_COMMAND),
            makeMenuItem(DIAGNOSTICS_SHOW_GRAPHICSINFO_COMMAND),
            makeMenuItem(DIAGNOSTICS_SHOW_PROFILER_COMMAND),
        }, "menu-system")
    };

    if (isMuseSamplerModuleAdded()) {
        MenuItemList museSamplerItems {
            makeMenuItem(MUSESAMPLER_CHECK_COMMAND),
        };

        if (globalConfiguration()->devModeEnabled()) {
            museSamplerItems << makeMenuItem(MUSESAMPLER_RELOAD_COMMAND);
        }

        items << makeMenu(TranslatableString("appshell/menu/diagnostics", "&MuseSampler"), museSamplerItems, "menu-musesampler");
    }

    if (globalConfiguration()->devModeEnabled()) {
        MenuItemList actionsItems {
            makeMenuItem(DIAGNOSTICS_SHOW_ACTIONS_COMMAND),
            makeMenuItem(DIAGNOSTICS_SHOW_RCOMMANDS_COMMAND)
        };

        MenuItemList accessibilityItems {
            makeMenuItem(DIAGNOSTICS_SHOW_NAVIGATION_TREE_COMMAND),
            makeMenuItem(DIAGNOSTICS_SHOW_ACCESSIBLE_TREE_COMMAND),
            makeMenuItem(DIAGNOSTICS_DUMP_ACCESSIBLE_TREE_COMMAND),
        };

        MenuItemList engravingItems {
            makeMenuItem(DIAGNOSTICS_SHOW_ENGRAVING_ELEMENTS_COMMAND),
            makeMenuItem(DIAGNOSTICS_SHOW_ENGRAVING_UNDOSTACK_COMMAND),
            makeMenuItem(DIAGNOSTICS_SHOW_ENGRAVING_STYLE_COMMAND),
            makeSeparator(),
            makeMenuItem(SHOW_ELEMENT_BOUNDING_RECTS_COMMAND),
            makeMenuItem(COLOR_ELEMENT_SHAPES_COMMAND),
            makeMenuItem(SHOW_SEGMENT_SHAPES_COMMAND),
            makeMenuItem(COLOR_SEGMENT_SHAPES_COMMAND),
            makeMenuItem(SHOW_SKYLINES_COMMAND),
            makeMenuItem(SHOW_SYSTEM_BOUNDING_RECTS_COMMAND),
            makeMenuItem(SHOW_ELEMENT_MASKS_COMMAND),
            makeMenuItem(SHOW_GAP_RESTS_COMMAND),
            makeMenuItem(SHOW_LINE_ATTACH_POINTS_COMMAND),
            makeMenuItem(MARK_EMPTY_STAFF_COMMAND),
            makeMenuItem(SHOW_ORIGIN_AND_COMBINED_COMMAND),
            makeMenuItem(MARK_CORRUPTED_MEASURES_COMMAND),
            makeMenuItem(CHECK_FOR_SCORE_CORRUPTIONS_COMMAND)
        };

        MenuItemList extensionsItems {
            makeMenuItem("command://extensions/open-apidump"),
        };

        MenuItemList testflowItems {
            makeMenuItem("testflow-show-scripts"),
        };

#ifdef MUSE_MODULE_VST
        MenuItemList vstItems {
            makeMenuItem(VST_USE_OLDVIEW_COMMAND),
            makeMenuItem(VST_USE_NEWVIEW_COMMAND),
        };
#endif

        MenuItemList audioItems {
            makeMenuItem(AUDIO_DEV_USE_DRIVER_MODE_COMMAND),
            makeMenuItem(AUDIO_DEV_USE_HYBRID_MODE_COMMAND),
        };

        items << makeMenu(TranslatableString("appshell/menu/diagnostics", "A&ctions"), actionsItems, "menu-diagnostics-actions")
              << makeMenu(TranslatableString("appshell/menu/diagnostics",
                                       "&Accessibility"), accessibilityItems, "menu-diagnostics-accessibility")
              << makeMenu(TranslatableString("appshell/menu/diagnostics", "&Engraving"), engravingItems, "menu-diagnostics-engraving")
              << makeMenu(TranslatableString("appshell/menu/diagnostics", "E&xtensions"), extensionsItems, "menu-diagnostics-extensions")
              << makeMenu(TranslatableString("appshell/menu/diagnostics", "&Testflow"), testflowItems, "menu-diagnostics-testflow");

#ifdef MUSE_MODULE_VST
        items << makeMenu(TranslatableString("appshell/menu/diagnostics", "&VST"), vstItems, "menu-diagnostics-vst");
#endif

        items << makeMenu(TranslatableString("appshell/menu/diagnostics", "&Audio"), audioItems, "menu-diagnostics-audio")
              << makeMenuItem(MULTIWINDOWS_DEV_SHOW_INFO_COMMAND);
    }

    return makeMenu(TranslatableString("appshell/menu/diagnostics", "&Diagnostics"), items, "menu-diagnostic");
}

MenuItemList AppMenuModel::makeRecentSubMenuItems()
{
    MenuItemList items;
    const RecentFilesList& recentFiles = recentFilesController()->recentFilesList();

    for (const RecentFile& file : recentFiles) {
        rcommand::CommandQuery query(PROJECT_OPEN_COMMAND);
        query.set("url", Val(file.path.toQUrl().toString().toStdString()));
        query.set("display_name", Val(file.displayNameOverride.toStdString()));

        MenuItem* item = new MenuItem(this);
        item->setCommandQuery(query);
        item->setTitle(TranslatableString::untranslatable(file.displayName(/*includingExtension*/ true)));
        item->setSelectable(true);

        bool isCloud = projectConfiguration()->isCloudProject(file.path);
        if (isCloud) {
            item->setIcon(IconCode::Code::CLOUD);
        }

        items << item;
    }

    if (!items.empty()) {
        items << makeSeparator()
              << makeMenuItem(PROJECT_CLEAR_RECENT_COMMAND);
    }

    return items;
}

MenuItemList AppMenuModel::makeNotesItems()
{
    MenuItemList items {
        makeMenuItem(TOGGLE_NOTE_INPUT_COMMAND),
        makeSeparator(),
        makeMenuItem(ENTER_NOTE_C_COMMAND),
        makeMenuItem(ENTER_NOTE_D_COMMAND),
        makeMenuItem(ENTER_NOTE_E_COMMAND),
        makeMenuItem(ENTER_NOTE_F_COMMAND),
        makeMenuItem(ENTER_NOTE_G_COMMAND),
        makeMenuItem(ENTER_NOTE_A_COMMAND),
        makeMenuItem(ENTER_NOTE_B_COMMAND),
        makeSeparator(),
        makeMenuItem(ADD_NOTE_C_COMMAND),
        makeMenuItem(ADD_NOTE_D_COMMAND),
        makeMenuItem(ADD_NOTE_E_COMMAND),
        makeMenuItem(ADD_NOTE_F_COMMAND),
        makeMenuItem(ADD_NOTE_G_COMMAND),
        makeMenuItem(ADD_NOTE_A_COMMAND),
        makeMenuItem(ADD_NOTE_B_COMMAND),
    };

    return items;
}

MenuItemList AppMenuModel::makeIntervalsItems()
{
    MenuItemList items {
        makeMenuItem(ADD_INTERVAL_PLUS_1_COMMAND),
        makeMenuItem(ADD_INTERVAL_PLUS_2_COMMAND),
        makeMenuItem(ADD_INTERVAL_PLUS_3_COMMAND),
        makeMenuItem(ADD_INTERVAL_PLUS_4_COMMAND),
        makeMenuItem(ADD_INTERVAL_PLUS_5_COMMAND),
        makeMenuItem(ADD_INTERVAL_PLUS_6_COMMAND),
        makeMenuItem(ADD_INTERVAL_PLUS_7_COMMAND),
        makeMenuItem(ADD_INTERVAL_PLUS_8_COMMAND),
        makeMenuItem(ADD_INTERVAL_PLUS_9_COMMAND),
        makeMenuItem(ADD_INTERVAL_PLUS_10_COMMAND),
        makeSeparator(),
        makeMenuItem(ADD_INTERVAL_MINUS_2_COMMAND),
        makeMenuItem(ADD_INTERVAL_MINUS_3_COMMAND),
        makeMenuItem(ADD_INTERVAL_MINUS_4_COMMAND),
        makeMenuItem(ADD_INTERVAL_MINUS_5_COMMAND),
        makeMenuItem(ADD_INTERVAL_MINUS_6_COMMAND),
        makeMenuItem(ADD_INTERVAL_MINUS_7_COMMAND),
        makeMenuItem(ADD_INTERVAL_MINUS_8_COMMAND),
        makeMenuItem(ADD_INTERVAL_MINUS_9_COMMAND),
        makeMenuItem(ADD_INTERVAL_MINUS_10_COMMAND)
    };

    return items;
}

MenuItemList AppMenuModel::makeTupletsItems()
{
    MenuItemList items {
        makeMenuItem(ADD_DUPLET_COMMAND),
        makeMenuItem(ADD_TRIPLET_COMMAND),
        makeMenuItem(ADD_QUADRUPLET_COMMAND),
        makeMenuItem(ADD_QUINTUPLET_COMMAND),
        makeMenuItem(ADD_SEXTUPLET_COMMAND),
        makeMenuItem(ADD_SEPTUPLET_COMMAND),
        makeMenuItem(ADD_OCTUPLET_COMMAND),
        makeMenuItem(ADD_NONUPLET_COMMAND),
        makeMenuItem(OPEN_TUPLET_CONFIGURE_COMMAND)
    };

    return items;
}

MenuItemList AppMenuModel::makeMeasuresItems()
{
    MenuItemList items {
        makeMenuItem(INSERT_MEASURE_COMMAND),
        makeMenuItem(APPEND_MEASURE_COMMAND),
        makeSeparator(),
        makeMenuItem(INSERT_MEASURES_COMMAND),
        makeMenuItem(INSERT_MEASURES_AFTER_SELECTION_COMMAND),
        makeSeparator(),
        makeMenuItem(INSERT_MEASURES_AT_START_OF_SCORE_COMMAND),
        makeMenuItem(APPEND_MEASURES_COMMAND)
    };

    return items;
}

MenuItemList AppMenuModel::makeFramesItems()
{
    MenuItemList items {
        makeMenuItem(INSERT_HBOX_COMMAND),
        makeMenuItem(INSERT_VBOX_COMMAND),
        makeMenuItem(INSERT_TEXTFRAME_COMMAND),
        makeMenuItem(INSERT_FRETFRAME_COMMAND),
        makeSeparator(),
        makeMenu(TranslatableString("notation", "Insert at end of score"), {
            makeMenuItem(APPEND_HBOX_COMMAND),
            makeMenuItem(APPEND_VBOX_COMMAND),
            makeMenuItem(APPEND_TEXTFRAME_COMMAND),
            makeMenuItem(APPEND_FRETFRAME_COMMAND)
        })
    };

    return items;
}

MenuItemList AppMenuModel::makeTextItems()
{
    MenuItemList items {
        makeMenuItem(ADD_TITLE_TEXT_COMMAND),
        makeMenuItem(ADD_SUBTITLE_TEXT_COMMAND),
        makeMenuItem(ADD_COMPOSER_TEXT_COMMAND),
        makeMenuItem(ADD_LYRICIST_TEXT_COMMAND),
        makeMenuItem(ADD_PART_TEXT_COMMAND),
        makeSeparator(),
        makeMenuItem(ADD_SYSTEM_TEXT_COMMAND),
        makeMenuItem(ADD_STAFF_TEXT_COMMAND),
        makeMenuItem(ADD_DYNAMIC_COMMAND),
        makeMenuItem(ADD_EXPRESSION_TEXT_COMMAND),
        makeMenuItem(ADD_REHEARSALMARK_TEXT_COMMAND),
        makeMenuItem(ADD_INSTRUMENT_CHANGE_TEXT_COMMAND),
        makeMenuItem(ADD_FINGERING_TEXT_COMMAND),
        makeSeparator(),
        makeMenuItem(ADD_STICKING_TEXT_COMMAND),
        makeMenuItem(ADD_CHORD_TEXT_COMMAND),
        makeMenuItem(ADD_ROMAN_NUMERAL_TEXT_COMMAND),
        makeMenuItem(ADD_NASHVILLE_NUMBER_TEXT_COMMAND),
        makeMenuItem(ADD_LYRICS_COMMAND),
        makeMenuItem(ADD_FIGURED_BASS_COMMAND),
        makeMenuItem(ADD_TEMPO_COMMAND)
    };

    return items;
}

MenuItemList AppMenuModel::makeLinesItems()
{
    MenuItemList items {
        makeMenuItem(ADD_SLUR_COMMAND),
        makeMenuItem(ADD_HAIRPIN_COMMAND),
        makeMenuItem(ADD_HAIRPIN_REVERSE_COMMAND),
        makeMenuItem(ADD_OTTAVA_8VA_COMMAND),
        makeMenuItem(ADD_OTTAVA_8VB_COMMAND),
        makeMenuItem(ADD_NOTELINE_COMMAND)
    };

    return items;
}

MenuItemList AppMenuModel::makeExtensionsItems()
{
    MenuItemList result;

    KnownCategories categories = extensionsRegister()->knownCategories();
    ManifestList manifests = extensionsRegister()->manifestList(Filter::Enabled);

    auto makeMenuItem = [this](const Manifest& m, const Action& a) {
        rcommand::Command command = makeCommand(m.uri, a.code);
        return this->makeMenuItem(command);
    };

    auto addMenuItems = [this, makeMenuItem](MenuItemList& items, const Manifest& m) {
        MenuItemList sub;
        for (const muse::extensions::Action& a : m.actions) {
            if (!a.showOnAppmenu) {
                continue;
            }
            sub << makeMenuItem(m, a);
        }

        if (sub.empty()) {
            return;
        }

        if (sub.size() == 1) {
            items << sub.at(0);
            return;
        }

        items << makeMenu(TranslatableString::untranslatable(m.title), sub);
    };

    std::map<std::string, MenuItemList> categoriesMap;
    MenuItemList pluginsWithoutCategories;
    for (const Manifest& m : manifests) {
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
