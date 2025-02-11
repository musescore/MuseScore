/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#pragma once

#include <gmock/gmock.h>

#include "notation/inotationinteraction.h"

namespace mu::notation {
class NotationInteractionMock : public INotationInteraction
{
public:
    MOCK_METHOD(INotationNoteInputPtr, noteInput, (), (const, override));

    MOCK_METHOD(bool, showShadowNote, (const muse::PointF&), (override));
    MOCK_METHOD(void, hideShadowNote, (), (override));
    MOCK_METHOD(muse::RectF, shadowNoteRect, (), (const, override));

    MOCK_METHOD(void, toggleVisible, (), (override));

    MOCK_METHOD(EngravingItem*, hitElement, (const muse::PointF&, float), (const, override));
    MOCK_METHOD(std::vector<EngravingItem*>, hitElements, (const muse::PointF&, float), (const, override));
    MOCK_METHOD(Staff*, hitStaff, (const muse::PointF&), (const, override));

    MOCK_METHOD(const HitElementContext&, hitElementContext, (), (const, override));
    MOCK_METHOD(void, setHitElementContext, (const HitElementContext&), (override));

    MOCK_METHOD(void, moveChordNoteSelection, (MoveDirection), (override));
    MOCK_METHOD(void, select, (const std::vector<EngravingItem*>&, SelectType, engraving::staff_idx_t), (override));
    MOCK_METHOD(void, selectAll, (), (override));
    MOCK_METHOD(void, selectSection, (), (override));
    MOCK_METHOD(void, selectFirstElement, (bool), (override));
    MOCK_METHOD(void, selectLastElement, (), (override));
    MOCK_METHOD(INotationSelectionPtr, selection, (), (const, override));
    MOCK_METHOD(void, clearSelection, (), (override));
    MOCK_METHOD(muse::async::Notification, selectionChanged, (), (const, override));
    MOCK_METHOD(void, selectTopOrBottomOfChord, (MoveDirection), (override));

    MOCK_METHOD(bool, isSelectionTypeFiltered, (SelectionFilterType), (const, override));
    MOCK_METHOD(void, setSelectionTypeFiltered, (SelectionFilterType, bool), (override));

    MOCK_METHOD(bool, isDragStarted, (), (const, override));
    MOCK_METHOD(void, startDrag, (const std::vector<EngravingItem*>&, const muse::PointF&, const IsDraggable&), (override));
    MOCK_METHOD(void, drag, (const muse::PointF&, const muse::PointF&, DragMode), (override));
    MOCK_METHOD(void, endDrag, (), (override));
    MOCK_METHOD(muse::async::Notification, dragChanged, (), (const, override));

    MOCK_METHOD(bool, isOutgoingDragElementAllowed, (const EngravingItem*), (const, override));
    MOCK_METHOD(void, startOutgoingDragElement, (const EngravingItem*, QObject*), (override));
    MOCK_METHOD(void, startOutgoingDragRange, (QObject*), (override));
    MOCK_METHOD(bool, isOutgoingDragStarted, (), (const, override));
    MOCK_METHOD(void, endOutgoingDrag, (), (override));

    MOCK_METHOD(bool, startDropSingle, (const QByteArray&), (override));
    MOCK_METHOD(bool, startDropRange, (const QByteArray&), (override));
    MOCK_METHOD(bool, startDropImage, (const QUrl&), (override));
    MOCK_METHOD(bool, isDropSingleAccepted, (const muse::PointF&, Qt::KeyboardModifiers), (override));
    MOCK_METHOD(bool, isDropRangeAccepted, (const muse::PointF&), (override));
    MOCK_METHOD(bool, dropSingle, (const muse::PointF&, Qt::KeyboardModifiers), (override));
    MOCK_METHOD(bool, dropRange, (const QByteArray&, const muse::PointF&, bool), (override));
    MOCK_METHOD(void, setDropTarget, (EngravingItem*, bool), (override));
    MOCK_METHOD(void, setDropRect, (const muse::RectF&), (override));
    MOCK_METHOD(void, endDrop, (), (override));
    MOCK_METHOD(muse::async::Notification, dropChanged, (), (const, override));

    MOCK_METHOD(bool, applyPaletteElement, (mu::engraving::EngravingItem*, Qt::KeyboardModifiers), (override));
    MOCK_METHOD(void, undo, (), (override));
    MOCK_METHOD(void, redo, (), (override));
    MOCK_METHOD(void, undoRedoToIndex, (size_t idx), (override));

    MOCK_METHOD(bool, moveSelectionAvailable, (MoveSelectionType), (const, override));
    MOCK_METHOD(void, moveSelection, (MoveDirection, MoveSelectionType), (override));

    MOCK_METHOD(void, moveLyrics, (MoveDirection), (override));
    MOCK_METHOD(void, expandSelection, (ExpandSelectionMode), (override));
    MOCK_METHOD(void, addToSelection, (MoveDirection, MoveSelectionType), (override));
    MOCK_METHOD(void, selectTopStaff, (), (override));
    MOCK_METHOD(void, selectEmptyTrailingMeasure, (), (override));
    MOCK_METHOD(void, moveSegmentSelection, (MoveDirection), (override));

    MOCK_METHOD(void, movePitch, (MoveDirection, PitchMode), (override));
    MOCK_METHOD(void, nudge, (MoveDirection, bool), (override));
    MOCK_METHOD(void, nudgeAnchors, (MoveDirection), (override));
    MOCK_METHOD(void, moveChordRestToStaff, (MoveDirection), (override));
    MOCK_METHOD(void, swapChordRest, (MoveDirection), (override));
    MOCK_METHOD(void, toggleSnapToPrevious, (), (override));
    MOCK_METHOD(void, toggleSnapToNext, (), (override));

    MOCK_METHOD(bool, isTextSelected, (), (const, override));
    MOCK_METHOD(bool, isTextEditingStarted, (), (const, override));
    MOCK_METHOD(bool, textEditingAllowed, (const EngravingItem*), (const, override));
    MOCK_METHOD(void, startEditText, (EngravingItem*, const muse::PointF&), (override));
    MOCK_METHOD(void, editText, (QInputMethodEvent*), (override));
    MOCK_METHOD(void, endEditText, (), (override));
    MOCK_METHOD(void, changeTextCursorPosition, (const muse::PointF&), (override));
    MOCK_METHOD(void, selectText, (mu::engraving::SelectTextType), (override));
    MOCK_METHOD(const TextBase*, editedText, (), (const, override));
    MOCK_METHOD(muse::async::Notification, textEditingStarted, (), (const, override));
    MOCK_METHOD(muse::async::Notification, textEditingChanged, (), (const, override));
    MOCK_METHOD(muse::async::Channel<TextBase*>, textEditingEnded, (), (const, override));

    MOCK_METHOD(muse::async::Channel<ScoreConfigType>, scoreConfigChanged, (), (const, override));
    MOCK_METHOD(bool, isGripEditStarted, (), (const, override));
    MOCK_METHOD(bool, isHitGrip, (const muse::PointF&), (const, override));
    MOCK_METHOD(void, startEditGrip, (const muse::PointF&), (override));
    MOCK_METHOD(void, startEditGrip, (EngravingItem*, mu::engraving::Grip), (override));
    MOCK_METHOD(void, endEditGrip, (), (override));

    MOCK_METHOD(bool, isElementEditStarted, (), (const, override));
    MOCK_METHOD(void, startEditElement, (EngravingItem*, bool), (override));
    MOCK_METHOD(void, changeEditElement, (EngravingItem*), (override));
    MOCK_METHOD(bool, isEditAllowed, (QKeyEvent*), (override));
    MOCK_METHOD(void, editElement, (QKeyEvent*), (override));
    MOCK_METHOD(void, endEditElement, (), (override));

    MOCK_METHOD(void, splitSelectedMeasure, (), (override));
    MOCK_METHOD(void, joinSelectedMeasures, (), (override));

    MOCK_METHOD(muse::Ret, canAddBoxes, (), (const, override));
    MOCK_METHOD(void, addBoxes, (BoxType, int, AddBoxesTarget), (override));
    MOCK_METHOD(void, addBoxes, (BoxType, int, int, bool), (override));

    MOCK_METHOD(void, copySelection, (), (override));
    MOCK_METHOD(muse::Ret, repeatSelection, (), (override));
    MOCK_METHOD(void, copyLyrics, (), (override));
    MOCK_METHOD(void, pasteSelection, (const Fraction&), (override));
    MOCK_METHOD(void, swapSelection, (), (override));
    MOCK_METHOD(void, deleteSelection, (), (override));
    MOCK_METHOD(void, flipSelection, (), (override));
    MOCK_METHOD(void, addTieToSelection, (), (override));
    MOCK_METHOD(void, addLaissezVibToSelection, (), (override));
    MOCK_METHOD(void, addTiedNoteToChord, (), (override));
    MOCK_METHOD(void, addSlurToSelection, (), (override));
    MOCK_METHOD(void, addOttavaToSelection, (OttavaType), (override));
    MOCK_METHOD(void, addHairpinOnGripDrag, (engraving::Dynamic*, bool), (override));
    MOCK_METHOD(void, addHairpinsToSelection, (HairpinType), (override));
    MOCK_METHOD(void, putRestToSelection, (), (override));
    MOCK_METHOD(void, putRest, (Duration), (override));
    MOCK_METHOD(void, addBracketsToSelection, (BracketsType), (override));
    MOCK_METHOD(void, toggleAccidentalForSelection, (AccidentalType), (override));
    MOCK_METHOD(void, toggleArticulationForSelection, (SymbolId), (override));
    MOCK_METHOD(void, toggleDotsForSelection, (Pad), (override));
    MOCK_METHOD(void, addGraceNotesToSelectedNotes, (GraceNoteType), (override));
    MOCK_METHOD(bool, canAddTupletToSelectedChordRests, (), (const, override));
    MOCK_METHOD(void, addTupletToSelectedChordRests, (const TupletOptions&), (override));
    MOCK_METHOD(void, addBeamToSelectedChordRests, (BeamMode), (override));

    MOCK_METHOD(void, increaseDecreaseDuration, (int, bool), (override));

    MOCK_METHOD(bool, toggleLayoutBreakAvailable, (), (const, override));
    MOCK_METHOD(void, toggleLayoutBreak, (LayoutBreakType), (override));
    MOCK_METHOD(void, moveMeasureToPrevSystem, (), (override));
    MOCK_METHOD(void, moveMeasureToNextSystem, (), (override));
    MOCK_METHOD(void, toggleSystemLock, (), (override));
    MOCK_METHOD(void, toggleScoreLock, (), (override));
    MOCK_METHOD(void, makeIntoSystem, (), (override));
    MOCK_METHOD(void, applySystemLock, (), (override));

    MOCK_METHOD(void, addRemoveSystemLocks, (AddRemoveSystemLockType, int), (override));
    MOCK_METHOD(bool, transpose, (const TransposeOptions&), (override));
    MOCK_METHOD(void, swapVoices, (voice_idx_t, voice_idx_t), (override));
    MOCK_METHOD(void, addIntervalToSelectedNotes, (int), (override));
    MOCK_METHOD(void, addFret, (int), (override));
    MOCK_METHOD(void, changeSelectedElementsVoice, (voice_idx_t), (override));
    MOCK_METHOD(void, changeSelectedElementsVoiceAssignment, (VoiceAssignment), (override));
    MOCK_METHOD(void, addAnchoredLineToSelectedNotes, (), (override));

    MOCK_METHOD(void, addTextToTopFrame, (TextStyleType), (override));

    MOCK_METHOD(muse::Ret, canAddTextToItem, (TextStyleType, const EngravingItem*), (const, override));
    MOCK_METHOD(void, addTextToItem, (TextStyleType, EngravingItem*), (override));

    MOCK_METHOD(muse::Ret, canAddImageToItem, (const EngravingItem*), (const, override));
    MOCK_METHOD(void, addImageToItem, (const muse::io::path_t&, EngravingItem*), (override));

    MOCK_METHOD(muse::Ret, canAddFiguredBass, (), (const, override));
    MOCK_METHOD(void, addFiguredBass, (), (override));

    MOCK_METHOD(void, addStretch, (qreal), (override));

    MOCK_METHOD(Measure*, selectedMeasure, (), (const, override));
    MOCK_METHOD(void, addTimeSignature, (Measure*, engraving::staff_idx_t, TimeSignature*), (override));

    MOCK_METHOD(void, explodeSelectedStaff, (), (override));
    MOCK_METHOD(void, implodeSelectedStaff, (), (override));

    MOCK_METHOD(void, realizeSelectedChordSymbols, (bool, Voicing, HarmonyDurationType), (override));
    MOCK_METHOD(void, removeSelectedMeasures, (), (override));
    MOCK_METHOD(void, removeSelectedRange, (), (override));
    MOCK_METHOD(void, removeEmptyTrailingMeasures, (), (override));

    MOCK_METHOD(void, fillSelectionWithSlashes, (), (override));
    MOCK_METHOD(void, replaceSelectedNotesWithSlashes, (), (override));

    MOCK_METHOD(void, changeEnharmonicSpelling, (bool), (override));
    MOCK_METHOD(void, spellPitches, (), (override));
    MOCK_METHOD(void, regroupNotesAndRests, (), (override));
    MOCK_METHOD(void, resequenceRehearsalMarks, (), (override));

    MOCK_METHOD(void, resetStretch, (), (override));
    MOCK_METHOD(void, resetTextStyleOverrides, (), (override));
    MOCK_METHOD(void, resetBeamMode, (), (override));
    MOCK_METHOD(void, resetShapesAndPosition, (), (override));
    MOCK_METHOD(void, resetToDefaultLayout, (), (override));

    MOCK_METHOD(ScoreConfig, scoreConfig, (), (const, override));
    MOCK_METHOD(void, setScoreConfig, (const ScoreConfig&), (override));

    MOCK_METHOD(void, addMelisma, (), (override));
    MOCK_METHOD(void, addLyricsVerse, (), (override));

    MOCK_METHOD(muse::Ret, canAddGuitarBend, (), (const, override));
    MOCK_METHOD(void, addGuitarBend, (GuitarBendType), (override));

    MOCK_METHOD(void, navigateToLyrics, (MoveDirection, bool), (override));
    MOCK_METHOD(void, navigateToLyricsVerse, (MoveDirection), (override));

    MOCK_METHOD(void, navigateToNextSyllable, (), (override));

    MOCK_METHOD(void, navigateToNearHarmony, (MoveDirection, bool), (override));
    MOCK_METHOD(void, navigateToHarmonyInNearMeasure, (MoveDirection), (override));
    MOCK_METHOD(void, navigateToHarmony, (const Fraction&), (override));

    MOCK_METHOD(void, navigateToNearFiguredBass, (MoveDirection), (override));
    MOCK_METHOD(void, navigateToFiguredBassInNearMeasure, (MoveDirection), (override));
    MOCK_METHOD(void, navigateToFiguredBass, (const Fraction&), (override));

    MOCK_METHOD(void, navigateToNearText, (MoveDirection), (override));

    MOCK_METHOD(void, toggleBold, (), (override));
    MOCK_METHOD(void, toggleItalic, (), (override));
    MOCK_METHOD(void, toggleUnderline, (), (override));
    MOCK_METHOD(void, toggleStrike, (), (override));
    MOCK_METHOD(void, toggleSubScript, (), (override));
    MOCK_METHOD(void, toggleSuperScript, (), (override));

    MOCK_METHOD(bool, canInsertClef, (mu::engraving::ClefType), (const, override));
    MOCK_METHOD(void, insertClef, (mu::engraving::ClefType), (override));

    MOCK_METHOD(void, toggleArticulation, (mu::engraving::SymId), (override));
    MOCK_METHOD(void, toggleOrnament, (mu::engraving::SymId), (override));
    MOCK_METHOD(void, changeAccidental, (mu::engraving::AccidentalType), (override));
    MOCK_METHOD(void, transposeSemitone, (int), (override));
    MOCK_METHOD(void, transposeDiatonicAlterations, (mu::engraving::TransposeDirection), (override));
    MOCK_METHOD(void, toggleAutoplace, (bool), (override));
    MOCK_METHOD(void, getLocation, (), (override));
    MOCK_METHOD(void, execute, (void (mu::engraving::Score::*)(), const muse::TranslatableString&), (override));

    MOCK_METHOD(void, showItem, (const mu::engraving::EngravingItem*, int), (override));
    MOCK_METHOD(muse::async::Channel<ShowItemRequest>, showItemRequested, (), (const, override));

    MOCK_METHOD(void, setGetViewRectFunc, (const std::function<muse::RectF()>&), (override));
};
}
