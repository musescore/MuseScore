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
#include "notation/notationcommands.h"

#include "appshell/appshellcommands.h"
#include "project/projectcommands.h"
#include "notation/notationcommands.h"
#include "playback/playbackcommands.h"
#include "palette/palettecommands.h"
#include "dockwindow/dockcommands.h"
#include "update/updatecommands.h"
#include "diagnostics/diagnosticscommands.h"
#include "musesampler/musesamplercommands.h"
#include "vst/vstcommands.h"
#include "audio/main/audiocommands.h"
#include "multiwindows/multiwindowscommands.h"
#include "extensions/extensionscommands.h"
#include "testflow/testflowcommands.h"

using namespace muse;
using namespace mu::appshell;
using namespace muse::ui;
using namespace muse::uicomponents;
using namespace mu::project;
using namespace muse::workspace;
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
using namespace muse::testflow;

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
        makeRecordingMenu(),
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
    recentFilesController()->recentFilesListChanged().onReceive(this, [this](const RecentFilesChange&) {
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

    projectSession()->currentNotationChanged().onNotify(this, [this]() {
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
        makeMenuItem(AddChordTextCommand::id),
        makeMenuItem(ADD_FRETBOARD_DIAGRAM_COMMAND),
        makeSeparator(),
        makeMenuItem(InsertFretFrameCommand::id, TranslatableString("appshell/menu/add/chordandfret", "Fretboard diagram legend"))
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
            makeMenuItem(PROJECT_SHARE_AUDIO_COMMAND),
        }),
        makeSeparator(),
        makeMenuItem(PROJECT_CONVERT_TO_SCORE_COMMAND),
        makeMenuItem(PROJECT_EXPORT_COMMAND),
        makeSeparator(),
        makeMenuItem(PROJECT_PROPERTIES_COMMAND),
        makeMenuItem(OpenPartsCommand::id),
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
        makeMenuItem(UndoCommand::id),
        makeMenuItem(RedoCommand::id),
        makeMenuItem(DOCK_TOGGLE_UNDO_HISTORY_COMMAND),
        makeSeparator(),
        makeMenuItem(CutCommand::id),
        makeMenuItem(CopyCommand::id),
        makeMenuItem(PasteDefaultCommand::id),
        makeMenuItem(PasteHalfCommand::id),
        makeMenuItem(PasteDoubleCommand::id),
        makeMenuItem(CopyPasteSwapCommand::id),
        makeMenuItem(DeleteCommand::id),
        makeSeparator(),
        makeMenuItem(SelectAllCommand::id),
        makeMenuItem(SelectSectionCommand::id),
        makeMenuItem(SHOW_SEARCH_COMMAND),
        makeSeparator(),
        makeMenuItem(APP_PREFERENCES_COMMAND, MenuItemRole::PreferencesRole)
    };

    return makeMenu(TranslatableString("appshell/menu/edit", "&Edit"), editItems, "menu-edit");
}

mu::notation::INotationUndoStackPtr AppMenuModel::undoStack() const
{
    mu::notation::INotationPtr notation = projectSession()->currentNotation();
    return notation ? notation->undoStack() : nullptr;
}

void AppMenuModel::updateUndoRedoItems()
{
    auto stack = undoStack();

    MenuItem& undoItem = findItem(UndoCommand::id);
    const TranslatableString undoActionName = stack ? stack->topMostUndoActionName() : TranslatableString();
    undoItem.setTitle(undoActionName.isEmpty()
                      ? TranslatableString("action", "Undo")
                      : TranslatableString("action", "Undo ‘%1’").arg(undoActionName));

    MenuItem& redoItem = findItem(RedoCommand::id);
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
        makeMenuItem(OpenPageSettingsCommand::id),
        makeSeparator(),
        makeMenuItem(OpenBreaksCommand::id),
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
        makeMenuItem(OpenTransposeCommand::id),
        makeSeparator(),
        makeMenuItem(STAFF_EXPLODE_COMMAND),
        makeMenuItem(STAFF_IMPLODE_COMMAND),
        makeMenuItem(OpenRealizeChordSymbolsCommand::id),
        makeMenu(TranslatableString("appshell/menu/tools", "&Voices"), {
            makeMenuItem(SwapVoice12Command::id),
            makeMenuItem(SwapVoice13Command::id),
            makeMenuItem(SwapVoice14Command::id),
            makeMenuItem(SwapVoice23Command::id),
            makeMenuItem(SwapVoice24Command::id),
            makeMenuItem(SwapVoice34Command::id)
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
        makeMenuItem(CopyLyricsCommand::id),
        makeMenuItem(REMOVE_EMPTY_TRAILING_MEASURES_COMMAND),
    };

    return makeMenu(TranslatableString("appshell/menu/tools", "&Tools"), toolsItems, "menu-tools");
}

MenuItem* AppMenuModel::makeRecordingMenu()
{
    MenuItemList quantizationItems {
        makeMenuItem(MIDI_RECORD_QUANTIZATION_4_COMMAND),
        makeMenuItem(MIDI_RECORD_QUANTIZATION_8_COMMAND),
        makeMenuItem(MIDI_RECORD_QUANTIZATION_16_COMMAND),
        makeMenuItem(MIDI_RECORD_QUANTIZATION_32_COMMAND),
    };

    MenuItemList countInItems {
        makeMenuItem(MIDI_RECORD_COUNTIN_0_COMMAND),
        makeMenuItem(MIDI_RECORD_COUNTIN_1_COMMAND),
        makeMenuItem(MIDI_RECORD_COUNTIN_2_COMMAND),
    };

    MenuItemList stavesItems {
        makeMenuItem(MIDI_RECORD_ONE_STAFF_COMMAND),
        makeMenuItem(MIDI_RECORD_TWO_STAVES_COMMAND),
    };

    MenuItemList recordingItems {
        makeMenuItem(MIDI_RECORD_TOGGLE_COMMAND),
        makeMenuItem(MIDI_RECORD_REQUANTIZE_COMMAND),
        makeSeparator(),
        makeMenu(TranslatableString("appshell/menu/recording", "Minimum &quantization"), quantizationItems, "menu-recording-quantization"),
        makeMenuItem(MIDI_RECORD_TUPLETS_TOGGLE_COMMAND),
        makeMenuItem(MIDI_RECORD_LEGATO_TOGGLE_COMMAND),
        makeMenu(TranslatableString("appshell/menu/recording", "&Count-in"), countInItems, "menu-recording-count-in"),
        makeSeparator(),
        makeMenu(TranslatableString("appshell/menu/recording", "&Staves"), stavesItems, "menu-recording-staves"),
        makeMenuItem(MIDI_RECORD_SPLIT_POINT_COMMAND),
        makeSeparator(),
        makeMenuItem(MIDI_RECORD_LATENCY_COMMAND),
    };

    return makeMenu(TranslatableString("appshell/menu/recording", "&Recording"), recordingItems, "menu-recording");
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
            makeMenuItem(OPEN_APIDUMP_COMMAND),
        };

        MenuItemList testflowItems {
            makeMenuItem(TESTFLOW_OPEN_SCRIPTS_COMMAND),
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
        makeMenuItem(ToggleNoteInputDefaultCommand {}),
        makeSeparator(),
        makeMenuItem(AddNoteC_ToNextChordCommand::id),
        makeMenuItem(AddNoteD_ToNextChordCommand::id),
        makeMenuItem(AddNoteE_ToNextChordCommand::id),
        makeMenuItem(AddNoteF_ToNextChordCommand::id),
        makeMenuItem(AddNoteG_ToNextChordCommand::id),
        makeMenuItem(AddNoteA_ToNextChordCommand::id),
        makeMenuItem(AddNoteB_ToNextChordCommand::id),
        makeSeparator(),
        makeMenuItem(AddNoteC_ToCurrentChordCommand::id),
        makeMenuItem(AddNoteD_ToCurrentChordCommand::id),
        makeMenuItem(AddNoteE_ToCurrentChordCommand::id),
        makeMenuItem(AddNoteF_ToCurrentChordCommand::id),
        makeMenuItem(AddNoteG_ToCurrentChordCommand::id),
        makeMenuItem(AddNoteA_ToCurrentChordCommand::id),
        makeMenuItem(AddNoteB_ToCurrentChordCommand::id),
    };

    return items;
}

MenuItemList AppMenuModel::makeIntervalsItems()
{
    MenuItemList items {
        makeMenuItem(AddInterval1Command::id),
        makeMenuItem(AddInterval2Command::id),
        makeMenuItem(AddInterval3Command::id),
        makeMenuItem(AddInterval4Command::id),
        makeMenuItem(AddInterval5Command::id),
        makeMenuItem(AddInterval6Command::id),
        makeMenuItem(AddInterval7Command::id),
        makeMenuItem(AddInterval8Command::id),
        makeMenuItem(AddInterval9Command::id),
        makeMenuItem(AddInterval10Command::id),
        makeSeparator(),
        makeMenuItem(AddIntervalMinus2Command::id),
        makeMenuItem(AddIntervalMinus3Command::id),
        makeMenuItem(AddIntervalMinus4Command::id),
        makeMenuItem(AddIntervalMinus5Command::id),
        makeMenuItem(AddIntervalMinus6Command::id),
        makeMenuItem(AddIntervalMinus7Command::id),
        makeMenuItem(AddIntervalMinus8Command::id),
        makeMenuItem(AddIntervalMinus9Command::id),
        makeMenuItem(AddIntervalMinus10Command::id)
    };

    return items;
}

MenuItemList AppMenuModel::makeTupletsItems()
{
    MenuItemList items {
        makeMenuItem(AddDupletCommand::id),
        makeMenuItem(AddTripletCommand::id),
        makeMenuItem(AddQuadrupletCommand::id),
        makeMenuItem(AddQuintupletCommand::id),
        makeMenuItem(AddSextupletCommand::id),
        makeMenuItem(AddSeptupletCommand::id),
        makeMenuItem(AddOctupletCommand::id),
        makeMenuItem(AddNonupletCommand::id),
        makeMenuItem(OpenTupletConfigureCommand::id)
    };

    return items;
}

MenuItemList AppMenuModel::makeMeasuresItems()
{
    MenuItemList items {
        makeMenuItem(InsertMeasureCommand::id),
        makeMenuItem(AppendMeasureCommand::id),
        makeSeparator(),
        makeMenuItem(InsertMeasuresCommand::id),
        makeMenuItem(InsertMeasuresAfterSelectionCommand::id),
        makeSeparator(),
        makeMenuItem(InsertMeasuresAtStartOfScoreCommand::id),
        makeMenuItem(AppendMeasuresCommand::id)
    };

    return items;
}

MenuItemList AppMenuModel::makeFramesItems()
{
    MenuItemList items {
        makeMenuItem(InsertHBoxCommand::id),
        makeMenuItem(InsertVBoxCommand::id),
        makeMenuItem(InsertTextFrameCommand::id),
        makeMenuItem(InsertFretFrameCommand::id),
        makeSeparator(),
        makeMenu(TranslatableString("notation", "Insert at end of score"), {
            makeMenuItem(AppendHBoxCommand::id),
            makeMenuItem(AppendVBoxCommand::id),
            makeMenuItem(AppendTextFrameCommand::id),
            makeMenuItem(AppendFretFrameCommand::id)
        })
    };

    return items;
}

MenuItemList AppMenuModel::makeTextItems()
{
    MenuItemList items {
        makeMenuItem(AddTitleTextCommand::id),
        makeMenuItem(AddSubtitleTextCommand::id),
        makeMenuItem(AddComposerTextCommand::id),
        makeMenuItem(AddLyricistTextCommand::id),
        makeMenuItem(AddPartTextCommand::id),
        makeSeparator(),
        makeMenuItem(AddSystemTextCommand::id),
        makeMenuItem(AddStaffTextCommand::id),
        makeMenuItem(ADD_DYNAMIC_COMMAND),
        makeMenuItem(AddExpressionTextCommand::id),
        makeMenuItem(AddRehearsalMarkTextCommand::id),
        makeMenuItem(AddInstrumentChangeTextCommand::id),
        makeMenuItem(AddFingeringTextCommand::id),
        makeSeparator(),
        makeMenuItem(AddStickingTextCommand::id),
        makeMenuItem(AddChordTextCommand::id),
        makeMenuItem(AddRomanNumeralTextCommand::id),
        makeMenuItem(AddNashvilleNumberTextCommand::id),
        makeMenuItem(AddLyricsCommand::id),
        makeMenuItem(ADD_FIGURED_BASS_COMMAND),
        makeMenuItem(AddTempoCommand::id)
    };

    return items;
}

MenuItemList AppMenuModel::makeLinesItems()
{
    MenuItemList items {
        makeMenuItem(AddSlurCommand::id),
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
