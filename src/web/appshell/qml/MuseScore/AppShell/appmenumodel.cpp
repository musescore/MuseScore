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

#include "types/translatablestring.h"

#include "notation/inotation.h"

#include "project/projectcommands.h"
#include "notationscene/notationcommands.h"
#include "palette/palettecommands.h"
#include "diagnostics/diagnosticscommands.h"

#include "log.h"

using namespace muse;
using namespace mu::appshell;
using namespace muse::ui;
using namespace muse::uicomponents;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::palette;
using namespace muse::diagnostics;

AppMenuModel::AppMenuModel(QObject* parent)
    : AbstractMenuModel(parent)
{
    setObjectName("AppMenuModel");
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

MenuItem* AppMenuModel::makeFileMenu()
{
    MenuItemList fileItems {
        makeMenuItem(PROJECT_SAVE_COMMAND)
    };

    return makeMenu(TranslatableString("appshell/menu/file", "&File"), fileItems, "menu-file");
}

MenuItem* AppMenuModel::makeEditMenu()
{
    MenuItemList editItems {
        makeMenuItem(UNDO_COMMAND),
        makeMenuItem(REDO_COMMAND),
        makeSeparator(),
        makeMenuItem(CUT_COMMAND),
        makeMenuItem(COPY_COMMAND),
        makeMenuItem(PASTE_COMMAND),
        makeMenuItem(PASTE_HALF_COMMAND),
        makeMenuItem(PASTE_DOUBLE_COMMAND),
        makeMenuItem(COPY_PASTE_SWAP_COMMAND),
        makeMenuItem(DELETE_COMMAND),
        makeSeparator(),
        makeMenuItem(SelectAllCommand::id),
        makeMenuItem(SelectSectionCommand::id),
        makeMenuItem(SHOW_SEARCH_COMMAND)
    };

    return makeMenu(TranslatableString("appshell/menu/edit", "&Edit"), editItems, "menu-edit");
}

MenuItem* AppMenuModel::makeViewMenu()
{
    MenuItemList viewItems {
        makeMenuItem(TOGGLE_MASTER_PALETTE_COMMAND),
        makeSeparator(),
        makeMenu(TranslatableString("appshell/menu/view", "&Show"), {
            makeMenuItem(SHOW_INVISIBLE_COMMAND),
            makeMenuItem(SHOW_UNPRINTABLE_COMMAND),
            makeMenuItem(SHOW_FRAMES_COMMAND),
            makeMenuItem(SHOW_PAGEBORDERS_COMMAND),
            makeMenuItem(SHOW_IRREGULAR_COMMAND),
            makeMenuItem(SHOW_SOUNDFLAGS_COMMAND),
        }, "menu-show")
    };

    return makeMenu(TranslatableString("appshell/menu/view", "&View"), viewItems, "menu-view");
}

MenuItem* AppMenuModel::makeDiagnosticsMenu()
{
    MenuItemList items {
        makeMenuItem(DIAGNOSTICS_SHOW_RCOMMANDS_COMMAND),
        makeMenu(TranslatableString("appshell/menu/diagnostics", "&Engraving"), {
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
        }, "menu-diagnostics-engraving")
    };

    return makeMenu(TranslatableString("appshell/menu/diagnostics", "&Diagnostics"), items, "menu-diagnostic");
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

mu::notation::INotationUndoStackPtr AppMenuModel::undoStack() const
{
    mu::notation::INotationPtr notation = projectSession()->currentNotation();
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
