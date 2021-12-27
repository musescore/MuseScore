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
#include "context/iuicontextresolver.h"
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
    INJECT(notation, context::IUiContextResolver, uiContextResolver)
    INJECT(notation, framework::IInteractive, interactive)
    INJECT(notation, playback::IPlaybackController, playbackController)
    INJECT(notation, playback::IPlaybackConfiguration, playbackConfiguration)
    INJECT(notation, INotationConfiguration, configuration)

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
    void padNote(const Pad& pad);
    void putNote(const actions::ActionData& data);
    void doubleNoteInputDuration();
    void halveNoteInputDuration();

    void toggleAccidental(AccidentalType type);
    void addArticulation(SymbolId articulationSymbolId);

    void putTuplet(const actions::ActionData& data);
    void putTuplet(const TupletOptions& options);
    void putTuplet(int tupletCount);

    void move(MoveDirection direction, bool quickly = false);
    void moveWithinChord(MoveDirection direction);
    void selectTopOrBottomOfChord(MoveDirection direction);

    void changeVoice(int voiceIndex);

    void cutSelection();
    void addTie();
    void chordTie();
    void addSlur();

    void selectAllSimilarElements();
    void selectAllSimilarElementsInStaff();
    void selectAllSimilarElementsInRange();
    void openSelectionMoreOptions();

    void startEditSelectedElement();
    void startEditSelectedText();

    void selectMeasuresCountAndInsert();
    void selectMeasuresCountAndAppend();

    void insertBox(BoxType boxType);
    void appendBox(BoxType boxType);
    void insertBoxes(BoxType boxType, int count);
    void appendBoxes(BoxType boxType, int count);
    int firstSelectedBoxIndex() const;

    void addStretch(qreal value);

    void unrollRepeats();

    void resetState();
    void resetStretch();
    void resetBeamMode();

    void openEditStyleDialog();
    void openPageSettingsDialog();
    void openStaffProperties();
    void openBreaksDialog();
    void openScoreProperties();
    void openTransposeDialog();
    void openPartsDialog();
    void openTupletOtherDialog();
    void openStaffTextPropertiesDialog();
    void openMeasurePropertiesDialog();
    void openEditGridSizeDialog();
    mu::io::path selectStyleFile(bool forLoad);
    void loadStyle();
    void saveStyle();

    void toggleScoreConfig(ScoreConfigType configType);
    void toggleConcertPitch();

    void playSelectedElement(bool playChord = true);

    bool isEditingText() const;
    bool isEditingLyrics() const;
    bool isNoteInputMode() const;
    bool isEditingElement() const;
    bool isNotEditingElement() const;
    bool isNotNoteInputMode() const;

    void pasteSelection(PastingType type = PastingType::Default);
    Fraction resolvePastingScale(const INotationInteractionPtr& interaction, PastingType type) const;

    FilterElementsOptions elementsFilterOptions(const EngravingItem* element) const;

    bool measureNavigationAvailable() const;

    enum class TextNavigationType {
        NearNoteOrRest,
        NearBeat,
        Fraction
    };

    bool textNavigationAvailable() const;
    bool textNavigationByBeatsAvailable() const;
    bool textNavigationByFractionAvailable() const;
    bool resolveTextNavigationAvailable(TextNavigationType type = TextNavigationType::NearNoteOrRest) const;

    void nextTextElement();
    void prevTextElement();
    void nextBeatTextElement();
    void prevBeatTextElement();
    void navigateToTextElement(MoveDirection direction, bool nearNoteOrRest = false);
    void navigateToTextElementByFraction(const Fraction& fraction);
    void navigateToTextElementInNearMeasure(MoveDirection direction);

    void startNoteInputIfNeed();

    bool hasSelection() const;
    Ms::EngravingItem* selectedElement() const;

    bool canUndo() const;
    bool canRedo() const;
    bool isNotationPage() const;
    bool isStandardStaff() const;
    bool isTablatureStaff() const;
    void registerAction(const mu::actions::ActionCode&, void (NotationActionController::*)(const actions::ActionData& data),
                        bool (NotationActionController::*)() const = &NotationActionController::isNotEditingElement);
    void registerAction(const mu::actions::ActionCode&, void (NotationActionController::*)(),
                        bool (NotationActionController::*)() const = &NotationActionController::isNotEditingElement);
    void registerAction(const mu::actions::ActionCode&, std::function<void()>,
                        bool (NotationActionController::*)() const = &NotationActionController::isNotEditingElement);
    void registerAction(const mu::actions::ActionCode&, void (NotationActionController::*)(MoveDirection, bool), MoveDirection, bool,
                        bool (NotationActionController::*)() const = &NotationActionController::isNotEditingElement);
    void registerNoteInputAction(const mu::actions::ActionCode&, NoteInputMethod inputMethod);
    void registerNoteAction(const mu::actions::ActionCode&, NoteName, NoteAddingMode addingMode = NoteAddingMode::NextChord);

    void registerPadNoteAction(const mu::actions::ActionCode&, Pad padding);
    void registerTabPadNoteAction(const mu::actions::ActionCode&, Pad padding);

    enum PlayMode {
        NoPlay, PlayNote, PlayChord
    };

    void registerAction(const mu::actions::ActionCode&, void (INotationInteraction::*)(), bool (NotationActionController::*)() const);
    void registerAction(const mu::actions::ActionCode&, void (INotationInteraction::*)(), PlayMode = PlayMode::NoPlay,
                        bool (NotationActionController::*)() const = &NotationActionController::isNotEditingElement);
    template<typename P1>
    void registerAction(const mu::actions::ActionCode&, void (INotationInteraction::*)(P1), P1, PlayMode = PlayMode::NoPlay,
                        bool (NotationActionController::*)() const = &NotationActionController::isNotEditingElement);
    template<typename P1>
    void registerAction(const mu::actions::ActionCode&, void (INotationInteraction::*)(P1), P1, bool (NotationActionController::*)() const);
    template<typename P1, typename P2>
    void registerAction(const mu::actions::ActionCode&, void (INotationInteraction::*)(P1, P2), P1, P2, PlayMode = PlayMode::NoPlay,
                        bool (NotationActionController::*)() const = &NotationActionController::isNotEditingElement);

    async::Notification m_currentNotationNoteInputChanged;
    std::map<mu::actions::ActionCode, bool (NotationActionController::*)() const> m_isEnabledMap;
};
}

#endif // MU_NOTATION_NOTATIONACTIONCONTROLLER_H
