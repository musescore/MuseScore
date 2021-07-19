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
#ifndef MU_NOTATION_NOTATIONACTIONCONTROLLER_H
#define MU_NOTATION_NOTATIONACTIONCONTROLLER_H

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "actions/actiontypes.h"
#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "inotation.h"
#include "iinteractive.h"
#include "playback/iplaybackcontroller.h"
#include "playback/iplaybackconfiguration.h"
#include "inotationconfiguration.h"

namespace mu::notation {
class NotationActionController : public actions::Actionable, public async::Asyncable
{
    INJECT(notation, actions::IActionsDispatcher, dispatcher)
    INJECT(notation, context::IGlobalContext, globalContext)
    INJECT(notation, framework::IInteractive, interactive)
    INJECT(notation, playback::IPlaybackController, playbackController)
    INJECT(notation, playback::IPlaybackConfiguration, playbackConfiguration)
    INJECT(notation, INotationConfiguration, configuration)

private:
    std::map<mu::actions::ActionCode, bool (NotationActionController::*)() const> m_isEnabledMap;

public:
    void init();

    bool canReceiveAction(const actions::ActionCode& code) const override;

    async::Notification currentNotationChanged() const;

    INotationNoteInputPtr currentNotationNoteInput() const;
    async::Notification currentNotationNoteInputChanged() const;

    INotationInteractionPtr currentNotationInteraction() const;

    INotationStylePtr currentNotationStyle() const;
    async::Notification currentNotationStyleChanged() const;

private:
    INotationPtr currentNotation() const;
    INotationElementsPtr currentNotationElements() const;
    INotationSelectionPtr currentNotationSelection() const;
    INotationUndoStackPtr currentNotationUndoStack() const;

    void toggleNoteInput();
    void toggleNoteInputMethod(NoteInputMethod method);
    void addNote(NoteName note, NoteAddingMode addingMode);
    void addText(TextType type);
    void addFiguredBass();
    void padNote(const Pad& pad);
    void putNote(const actions::ActionData& data);

    void toggleVisible();

    void toggleAccidental(AccidentalType type);
    void putRestToSelection();
    void putRest(DurationType duration);
    void addArticulation(SymbolId articulationSymbolId);

    void putTuplet(int tupletCount);
    void addBeamToSelectedChordRests(BeamMode mode);
    void addBracketsToSelection(BracketsType type);

    void moveAction(const actions::ActionCode& actionCode);
    void moveChord(MoveDirection direction);
    void moveText(INotationInteractionPtr interaction, const actions::ActionCode& actionCode);

    void increaseDecreaseDuration(int steps, bool stepByDots);

    void swapVoices(int voiceIndex1, int voiceIndex2);
    void changeVoice(int voiceIndex);

    void cutSelection();
    void copySelection();
    void deleteSelection();
    void swapSelection();
    void flipSelection();
    void addTie();
    void chordTie();
    void addSlur();
    void addInterval(int interval);
    void addFret(int fretIndex);

    void undo();
    void redo();

    void addChordToSelection(MoveDirection direction);
    void selectAllSimilarElements();
    void selectAllSimilarElementsInStaff();
    void selectAllSimilarElementsInRange();
    void openSelectionMoreOptions();
    void selectAll();
    void selectSection();
    void firstElement();
    void lastElement();

    void nextLyrics();
    void previousLyrics();
    void nextLyricsVerse();
    void previousLyricsVerse();
    void nextSyllable();
    void addMelisma();
    void addLyricsVerse();

    void toggleLayoutBreak(LayoutBreakType breakType);

    void splitMeasure();
    void joinSelectedMeasures();
    void selectMeasuresCountAndInsert();
    void selectMeasuresCountAndAppend();

    void insertBox(BoxType boxType);
    void appendBox(BoxType boxType);
    void insertBoxes(BoxType boxType, int count);
    void appendBoxes(BoxType boxType, int count);
    int firstSelectedBoxIndex() const;

    void addOttava(OttavaType type);
    void addHairpin(HairpinType type);
    void addAnchoredNoteLine();

    void addStretch(qreal value);

    void explodeSelectedStaff();
    void implodeSelectedStaff();
    void realizeSelectedChordSymbols();
    void removeSelectedRange();
    void removeEmptyTrailingMeasures();
    void fillSelectionWithSlashes();
    void replaceSelectedNotesWithSlashes();
    void spellPitches();
    void regroupNotesAndRests();
    void resequenceRehearsalMarks();
    void unrollRepeats();
    void copyLyrics();
    void addGraceNotesToSelectedNotes(GraceNoteType type);

    void resetState();
    void resetStretch();
    void resetTextStyleOverrides();
    void resetBeamMode();
    void resetShapesAndPosition();

    void openEditStyleDialog();
    void openPageSettingsDialog();
    void openStaffProperties();
    void openBreaksDialog();
    void openScoreProperties();
    void openTransposeDialog();
    void openPartsDialog();
    void openTupletOtherDialog();
    void openStaffTextPropertiesDialog();

    void toggleScoreConfig(ScoreConfigType configType);
    void toggleNavigator();
    void toggleMixer();
    void toggleConcertPitch();

    void playSelectedElement(bool playChord = true);

    bool isEditingText() const;
    bool isNotEditingText() const;
    bool isEditingLyrics() const;

    void pasteSelection(PastingType type = PastingType::Default);
    Fraction resolvePastingScale(const INotationInteractionPtr& interaction, PastingType type) const;

    FilterElementsOptions elementsFilterOptions(const Element* element) const;

    void startNoteInputIfNeed();

    bool hasSelection() const;
    bool canUndo() const;
    bool canRedo() const;
    bool isNotationPage() const;
    bool isStandardStaff() const;
    bool isTablatureStaff() const;
    void registerAction(const mu::actions::ActionCode&, void (NotationActionController::*)(), bool (NotationActionController::*)() const);
    void registerAction(const mu::actions::ActionCode&, std::function<void()>, bool (NotationActionController::*)() const);
    void registerNoteInputAction(const mu::actions::ActionCode&, NoteInputMethod inputMethod);
    void registerNoteAction(const mu::actions::ActionCode&, NoteName, NoteAddingMode addingMode = NoteAddingMode::NextChord);
    void registerPadNoteAction(const mu::actions::ActionCode&, Pad padding);
    void registerLyricsAction(const mu::actions::ActionCode&, void (NotationActionController::*)());

    async::Notification m_currentNotationNoteInputChanged;
};
}

#endif // MU_NOTATION_NOTATIONACTIONCONTROLLER_H
