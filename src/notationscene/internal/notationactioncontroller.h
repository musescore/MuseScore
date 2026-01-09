/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "async/asyncable.h"
#include "actions/actionable.h"
#include "actions/actiontypes.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "actions/iactionsdispatcher.h"
#include "ui/inavigationcontroller.h"
#include "ui/iuiactionsregister.h"
#include "context/iglobalcontext.h"
#include "context/iuicontextresolver.h"
#include "playback/iplaybackcontroller.h"
#include "engraving/iengravingconfiguration.h"
#include "notation/inotationconfiguration.h"

#include "notation/inotation.h"

namespace mu::notation {
class NotationActionController : public muse::actions::Actionable, public muse::async::Asyncable, public muse::Injectable
{
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::Inject<muse::ui::INavigationController> navigationController = { this };
    muse::Inject<muse::ui::IUiActionsRegister> actionRegister = { this };
    muse::Inject<context::IGlobalContext> globalContext = { this };
    muse::Inject<context::IUiContextResolver> uiContextResolver = { this };
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<playback::IPlaybackController> playbackController = { this };
    muse::Inject<INotationConfiguration> configuration = { this };
    muse::Inject<engraving::IEngravingConfiguration> engravingConfiguration = { this };

public:

    NotationActionController(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

    void init();

    bool canReceiveAction(const muse::actions::ActionCode& code) const override;

    muse::async::Notification currentNotationChanged() const;

    INotationNoteInputPtr currentNotationNoteInput() const;
    muse::async::Notification currentNotationNoteInputChanged() const;

    INotationInteractionPtr currentNotationInteraction() const;
    INotationUndoStackPtr currentNotationUndoStack() const;

    INotationStylePtr currentNotationStyle() const;
    muse::async::Notification currentNotationStyleChanged() const;

    using EngravingDebuggingOptions = engraving::IEngravingConfiguration::DebuggingOptions;
    static const std::unordered_map<muse::actions::ActionCode, bool EngravingDebuggingOptions::*> engravingDebuggingActions;

private:
    INotationPtr currentNotation() const;
    IMasterNotationPtr currentMasterNotation() const;
    INotationElementsPtr currentNotationElements() const;
    INotationSelectionPtr currentNotationSelection() const;
    INotationMidiInputPtr currentNotationMidiInput() const;

    mu::engraving::Score* currentNotationScore() const;

    void toggleNoteInput();
    void toggleNoteInputMethod(NoteInputMethod method);
    void toggleNoteInputInsert();
    void handleNoteAction(NoteName note, NoteAddingMode addingMode);
    void handleNoteAction(const muse::actions::ActionData& args);
    void padNote(const Pad& pad);
    void putNote(const muse::actions::ActionData& args);
    void removeNote(const muse::actions::ActionData& args);
    void doubleNoteInputDuration();
    void halveNoteInputDuration();
    void realtimeAdvance();

    void toggleAccidental(AccidentalType type);
    void toggleArticulation(SymbolId articulationSymbolId);

    void putTuplet(const muse::actions::ActionData& data);
    void putTuplet(const TupletOptions& options);
    void putTuplet(int tupletCount);

    bool moveSelectionAvailable(MoveSelectionType type) const;
    void moveSelection(MoveSelectionType type, MoveDirection direction);
    void move(MoveDirection direction, bool quickly = false);
    void moveInputNotes(bool up, PitchMode mode);
    void movePitchDiatonic(MoveDirection direction, bool);
    void moveWithinChord(MoveDirection direction);
    void selectTopOrBottomOfChord(MoveDirection direction);

    void changeVoice(voice_idx_t voiceIndex);

    void cutSelection();
    void repeatSelection();
    void addTie();
    void chordTie();
    void addLaissezVib();
    void addSlur();
    void addHammerOnPullOff();
    void addFret(int num);

    void insertClef(mu::engraving::ClefType type);

    void addText(TextStyleType type);
    void addImage();
    void addFiguredBass();
    void addGuitarBend(GuitarBendType bendType);
    void addFretboardDiagram();

    void selectAllSimilarElements();
    void selectAllSimilarElementsInStaff();
    void selectAllSimilarElementsInRange();
    void openSelectionMoreOptions();

    void startEditSelectedElement(const muse::actions::ActionData& args);
    void startEditSelectedText(const muse::actions::ActionData& args);

    void addMeasures(const muse::actions::ActionData& actionData, AddBoxesTarget target);
    void addBoxes(BoxType boxType, int count, AddBoxesTarget target);

    void addStretch(qreal value);

    void unrollRepeats();

    void resetState();
    void resetStretch();
    void resetBeamMode();

    void openEditStyleDialog(const muse::actions::ActionData& args);
    void openPageSettingsDialog();
    void openStaffProperties();
    void openEditStringsDialog();
    void openBreaksDialog();
    void openTransposeDialog();
    void openPartsDialog();
    void openTupletOtherDialog();
    void openStaffTextPropertiesDialog();
    void openMeasurePropertiesDialog();
    void openEditGridSizeDialog();
    void openRealizeChordSymbolsDialog();
    muse::io::path_t selectStyleFile(bool forLoad);
    void loadStyle();
    void saveStyle();

    void toggleScoreConfig(ScoreConfigType configType);
    void toggleConcertPitch();

    void seekAndPlaySelectedElement(bool playChord = true);
    void seekSelectedElement();
    void playSelectedElement(bool playChord = true);

    bool isEditingText() const;
    bool isEditingLyrics() const;
    bool isNoteInputMode() const;
    bool isEditingElement() const;
    bool isNotEditingElement() const;
    bool isNotEditingOrHasPopup() const;
    bool isNotNoteInputMode() const;

    bool isToggleVisibleAllowed() const;

    void pasteSelection(PastingType type = PastingType::Default);
    Fraction resolvePastingScale(const INotationInteractionPtr& interaction, PastingType type) const;

    FilterElementsOptions elementsFilterOptions(const EngravingItem* element) const;

    bool measureNavigationAvailable() const;
    bool toggleLayoutBreakAvailable() const;

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
    void nextWord();
    void navigateToTextElement(MoveDirection direction, bool nearNoteOrRest = false, bool moveOnly = true);
    void navigateToTextElementByFraction(const Fraction& fraction);
    void navigateToTextElementInNearMeasure(MoveDirection direction);

    bool startNoteInputAllowed() const;
    void startNoteInput();

    bool hasSelection() const;
    mu::engraving::EngravingItem* selectedElement() const;
    bool noteOrRestSelected() const;

    const mu::engraving::Harmony* editedChordSymbol() const;

    bool elementHasPopup(const EngravingItem* e) const;

    bool canUndo() const;
    bool canRedo() const;

    bool isNotationPage() const;
    bool isTablatureStaff() const;

    void checkForScoreCorruptions();

    void registerAction(const muse::actions::ActionCode&, void (NotationActionController::*)(const muse::actions::ActionData& data),
                        bool (NotationActionController::*)() const = &NotationActionController::isNotationPage);
    void registerAction(const muse::actions::ActionCode&, void (NotationActionController::*)(),
                        bool (NotationActionController::*)() const = &NotationActionController::isNotationPage);
    void registerAction(const muse::actions::ActionCode&, std::function<void()>,
                        bool (NotationActionController::*)() const = &NotationActionController::isNotationPage);
    void registerAction(const muse::actions::ActionCode&, std::function<void(const muse::actions::ActionData& data)>,
                        bool (NotationActionController::*)() const = &NotationActionController::isNotationPage);
    void registerAction(const muse::actions::ActionCode&, void (NotationActionController::*)(MoveDirection, bool), MoveDirection, bool,
                        bool (NotationActionController::*)() const = &NotationActionController::isNotEditingElement);
    void registerAction(const muse::actions::ActionCode&, void (NotationActionController::*)(),
                        muse::Ret (INotationInteraction::*)() const);
    void registerAction(const muse::actions::ActionCode&, std::function<void()>,
                        muse::Ret (INotationInteraction::*)() const);

    void registerNoteInputAction(const muse::actions::ActionCode&, NoteInputMethod inputMethod);

    bool noteInputActionAllowed() const;
    void registerNoteAction(const muse::actions::ActionCode&, NoteName, NoteAddingMode addingMode = NoteAddingMode::NextChord);

    void registerPadNoteAction(const muse::actions::ActionCode&, Pad padding);
    void registerTabPadNoteAction(const muse::actions::ActionCode&, Pad padding);

    enum PlayMode {
        NoPlay, PlayNote, PlayChord
    };

    void registerMoveSelectionAction(const muse::actions::ActionCode& code, MoveSelectionType type, MoveDirection direction,
                                     PlayMode playMode = PlayMode::NoPlay);

    void registerAction(const muse::actions::ActionCode&, void (INotationInteraction::*)(), bool (NotationActionController::*)() const);
    void registerAction(const muse::actions::ActionCode&, void (INotationInteraction::*)(), PlayMode = PlayMode::NoPlay,
                        bool (NotationActionController::*)() const = &NotationActionController::isNotationPage);
    template<typename P1>
    void registerAction(const muse::actions::ActionCode&, void (INotationInteraction::*)(P1), P1, PlayMode = PlayMode::NoPlay,
                        bool (NotationActionController::*)() const = &NotationActionController::isNotationPage);
    template<typename P1>
    void registerAction(const muse::actions::ActionCode&, void (INotationInteraction::*)(P1), P1,
                        bool (NotationActionController::*)() const);
    template<typename P1, typename P2, typename Q1, typename Q2>
    void registerAction(const muse::actions::ActionCode&, void (INotationInteraction::*)(P1, P2), Q1, Q2, PlayMode = PlayMode::NoPlay,
                        bool (NotationActionController::*)() const = &NotationActionController::isNotationPage);

    muse::async::Notification m_currentNotationNoteInputChanged;

    using IsActionEnabledFunc = std::function<bool ()>;
    std::map<muse::actions::ActionCode, IsActionEnabledFunc> m_isEnabledMap;
};
}
