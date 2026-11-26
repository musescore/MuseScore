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
#include "notation/notationcommands.h"
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
        makeMenuItem(PROJECT_OPEN_COMMAND),
        makeSeparator(),
        makeMenuItem(PROJECT_SAVE_COMMAND),
        makeSeparator(),
        makeMenuItem(PROJECT_EXPORT_COMMAND)
    };

    return makeMenu(TranslatableString("appshell/menu/file", "&File"), fileItems, "menu-file");
}

MenuItem* AppMenuModel::makeEditMenu()
{
    MenuItemList editItems {
        makeMenuItem(UndoCommand::id),
        makeMenuItem(RedoCommand::id),
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
            makeMenuItem(ShowInvisibleCommand::id),
            makeMenuItem(ShowUnprintableCommand::id),
            makeMenuItem(ShowFramesCommand::id),
            makeMenuItem(ShowPageBordersCommand::id),
            makeMenuItem(ShowIrregularCommand::id),
            makeMenuItem(ShowSoundFlagsCommand::id),
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
        makeMenuItem(AddDynamicCommand::id),
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
        makeMenuItem(AddFiguredBassCommand::id),
        makeMenuItem(AddTempoCommand::id)
    };

    return items;
}

MenuItemList AppMenuModel::makeLinesItems()
{
    MenuItemList items {
        makeMenuItem(AddSlurCommand::id),
        makeMenuItem(AddHairpinCrescendoCommand::id),
        makeMenuItem(AddHairpinDiminuendoCommand::id),
        makeMenuItem(AddOttava8vaCommand::id),
        makeMenuItem(AddOttava8vbCommand::id),
        makeMenuItem(AddNoteLineCommand::id)
    };

    return items;
}

MenuItemList AppMenuModel::makeChordAndFretboardDiagramsItems()
{
    MenuItemList items {
        makeMenuItem(AddChordTextCommand::id),
        makeMenuItem(AddFretboardDiagramCommand::id),
        makeSeparator(),
        makeMenuItem(InsertFretFrameCommand::id, TranslatableString("appshell/menu/add/chordandfret", "Fretboard diagram legend"))
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
        makeMenuItem(OpenEditStyleCommand::id),
        makeMenuItem(OpenPageSettingsCommand::id),
        makeSeparator(),
        makeMenuItem(OpenBreaksCommand::id),
        makeMenu(TranslatableString("appshell/menu/format", "Str&etch"), {
            makeMenuItem(IncreaseStretchCommand::id),
            makeMenuItem(DecreaseStretchCommand::id),
            makeMenuItem(ResetStretchCommand::id)
        }, "menu-stretch"),
        makeSeparator(),
        makeMenuItem(ResetTextStyleOverridesCommand::id),
        makeMenuItem(ResetBeamsCommand::id),
        makeMenuItem(ResetShapesAndPositionsCommand::id),
        makeMenuItem(ResetToDefaultLayoutCommand::id),
        makeSeparator(),
        makeMenuItem(LoadStyleCommand::id),
        makeMenuItem(SaveStyleCommand::id)
    };

    return makeMenu(TranslatableString("appshell/menu/format", "F&ormat"), formatItems, "menu-format");
}

MenuItem* AppMenuModel::makeToolsMenu()
{
    MenuItemList toolsItems {
        makeMenuItem(OpenTransposeCommand::id),
        makeSeparator(),
        makeMenuItem(StaffExplodeCommand::id),
        makeMenuItem(StaffImplodeCommand::id),
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
            makeMenuItem(SplitMeasureCommand::id),
            makeMenuItem(JoinMeasuresCommand::id)
        }, "menu-tools-measures"),
        makeMenuItem(REMOVE_SELECTED_RANGE_COMMAND),
        makeSeparator(),
        makeMenuItem(SlashFillCommand::id),
        makeMenuItem(SlashRhythmCommand::id),
        makeSeparator(),
        makeMenuItem(RespellPitchesSharpsCommand::id),
        makeMenuItem(RespellPitchesFlatsCommand::id),
        makeMenu(TranslatableString("appshell/menu/tools", "Enharmonic spelling"), {
            makeMenuItem(ChangeEnharmonicSpellingBothCommand::id),
            makeMenuItem(ChangeEnharmonicSpellingCurrentCommand::id),
            makeMenuItem(OptimizeEnharmonicSpellingCommand::id),
        }, "menu-enharmonic-spelling"),
        makeSeparator(),
        makeMenuItem(RegroupRhythmsCommand::id),
        makeMenuItem(ResequenceRehearsalMarksCommand::id),
        makeSeparator(),
        makeMenuItem(CopyLyricsCommand::id),
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
