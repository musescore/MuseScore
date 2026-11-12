/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "../inotationcommandscontroller.h"

#include "types/ret.h"
#include "async/asyncable.h"
#include "rcommand/commandable.h"

#include "modularity/ioc.h"
#include "interactive/iinteractive.h"
#include "rcommand/commandtypes.h"
#include "rcommand/icommandsstate.h"
#include "rcommand/icommanddispatcher.h"
#include "project/iprojectsession.h"
#include "playback/iplaybackcontroller.h"
#include "engraving/iengravingconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "notationscene/inotationsceneconfiguration.h"

#include "notation/inotation_fwd.h"
#include "notation/notationtypes.h"

#include "inotationviewcontroller.h"
#include "notationcommands.h"

namespace mu::notation {
class NotationActionController : public INotationCommandsController, public muse::rcommand::Commandable, public muse::async::Asyncable,
    public muse::Contextable
{
    muse::GlobalInject<INotationConfiguration> configuration;
    muse::GlobalInject<INotationSceneConfiguration> sceneConfiguration;
    muse::GlobalInject<engraving::IEngravingConfiguration> engravingConfiguration;
    muse::ContextInject<muse::rcommand::ICommandDispatcher> commandDispatcher = { this };
    muse::ContextInject<muse::rcommand::ICommandsState> commandsState = { this };
    muse::ContextInject<project::IProjectSession> projectSession = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<playback::IPlaybackController> playbackController = { this };

public:

    NotationActionController(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    void init();

    void setViewController(INotationViewController* controller) override;
    INotationViewController* viewController() const override;

    bool hasSelection() const override;
    bool isSingleSelection() const override;
    bool isRangeSelection() const override;
    muse::async::Notification selectionChanged() const override;
    bool selectionHasTie() const override;
    bool selectionHasLaissezVib() const override;
    bool selectionHasSlur() const override;

    bool canUndo() const override;
    bool canRedo() const override;
    muse::async::Notification stackChanged() const override;

    bool isTextEditing() const override;
    bool isLyricsEditing() const override;
    muse::async::Channel<bool> textEditingChanged() const override;

    bool isNoteInputAllowed() const override;
    muse::async::Channel<bool> isNoteInputAllowedChanged() const override;

    muse::async::Notification noteInputStateChanged() const override;
    bool isNoteInputMode() const override;
    NoteInputMethod noteInputMethod() const override;
    engraving::DurationType currentDurationType() const override;
    int currentDotCount() const override;
    bool currentIsRest() const override;
    engraving::AccidentalType currentAccidentalType() const override;
    std::set<engraving::SymId> currentArticulations() const override;
    engraving::voice_idx_t currentVoice() const override;

    bool isNoteInputActionAllowed() const override;
    bool isNoteOrRestSelected() const override;
    bool isHairpinSelected() const override;
    bool isMoveSelectionAvailable(MoveSelectionType type) const override;

    bool isToggleLayoutBreakAvailable() const override;

    ScoreConfig scoreConfig() const override;
    muse::async::Channel<ScoreConfigType> scoreConfigChanged() const override;

    INotationStylePtr notationStyle() const override;
    muse::async::Notification notationStyleChanged() const override;

    bool isTablatureStaff() const override;

    INotationAutomationPtr automation() const;

    bool isAutomationModeEnabled() const override;
    muse::async::Notification automationModeEnabledChanged() const override;

    bool isDebuggingCommandEnabled(const muse::rcommand::Command& command) const override;
    muse::async::Notification debuggingOptionsChanged() const override;

    muse::async::Notification currentNotationChanged() const;

    INotationNoteInputPtr currentNotationNoteInput() const;
    INotationInteractionPtr currentNotationInteraction() const;
    INotationUndoStackPtr currentNotationUndoStack() const;

    IMasterNotationPtr currentMasterNotation() const;
    muse::async::Notification currentMasterNotationChanged() const;

private:
    INotationPtr currentNotation() const;
    INotationElementsPtr currentNotationElements() const;
    INotationSelectionPtr currentNotationSelection() const;
    INotationMidiInputPtr currentNotationMidiInput() const;

    mu::engraving::Score* currentNotationScore() const;

    muse::Ret select(const SelectCommand& command);
    muse::Ret select(SelectionTarget target, PlayMode playMode = PlayMode::NoPlay);
    muse::Ret addToSelection(const AddToSelectionCommand& command);
    muse::Ret addToSelection(SelectionTarget target);
    muse::Ret openSelectionMoreOptions();
    muse::Ret showLocation();

    muse::Ret toggleNoteInput(const ToggleNoteInputCommand& command);
    muse::Ret toggleNoteInput(NoteInputMethod method);
    muse::Ret toggleNoteInputInsert();

    muse::Ret setDuration(const SetDurationCommand& command);
    muse::Ret setDuration(engraving::DurationType duration);
    muse::Ret changeDuration(const ChangeDurationCommand& command);
    muse::Ret changeDuration(ChangeDurationMode mode);
    muse::Ret toggleDots(const ToggleDotsCommand& command);
    muse::Ret toggleDots(uint dots);

    muse::Ret toggleRest();

    muse::RetVal<RequestNoteInputStateCommand::Output> requestNoteInputState() const;

    muse::Ret addNote(const AddNoteCommand& command);
    muse::Ret addNote(NoteName noteName, NoteAddingMode addingMode);
    muse::Ret addDrumNote(const muse::rcommand::Params& params);
    void doAddNote(const NoteInputParams& params, const NoteAddingMode& addingMode);

    muse::Ret toggleAccidental(const ToggleAccidentalCommand& command);
    muse::Ret changeVoice(const UseVoiceCommand& command);
    muse::Ret swapVoices(const SwapVoicesCommand& command);

    muse::Ret flipSelection();

    muse::Ret putNote(const muse::rcommand::Params& params);
    muse::Ret removeNote(const muse::rcommand::Params& params);

    void realtimeAdvance();

    void toggleArticulation(SymbolId articulationSymbolId);

    muse::Ret putTuplet(const muse::rcommand::Params& params);
    void putTuplet(const TupletOptions& options);
    void putTuplet(int tupletCount);

    muse::Ret moveWithRet(MoveDirection direction, bool quickly = false);
    void move(MoveDirection direction, bool quickly = false);
    void moveInputNotes(bool up, PitchMode mode);
    void movePitchDiatonic(MoveDirection direction, bool);

    void cutSelection();
    void repeatSelection();
    void addTie();
    void addLaissezVib();
    void addSlur();
    void addHammerOnPullOff();
    void addFret(int num);

    void insertClef(mu::engraving::ClefType type);

    void addText(TextStyleType type);
    void addDynamic();
    void addImage();
    void addFiguredBass();
    void addGuitarBend(GuitarBendType bendType);
    void addFretboardDiagram();

    muse::Ret startEditSelectedElement(const muse::rcommand::Params& params);
    muse::Ret startEditSelectedText(const muse::rcommand::Params& params);

    muse::Ret addMeasures(const muse::rcommand::Params& params, AddBoxesTarget target);
    void addBoxes(BoxType boxType, int count, AddBoxesTarget target);

    void addStretch(qreal value);

    void unrollRepeats();

    void toggleMmrest();
    void toggleHideEmpty();
    void addFullMeasureRest();

    void resetState();
    void resetStretch();
    void resetBeamMode();

    muse::Ret openEditStyleDialog(const muse::rcommand::Params& params);
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
    bool isEditingElement() const;
    bool isNotEditingElement() const;

    void pasteSelection(PastingType type = PastingType::Default);
    Fraction resolvePastingScale(const INotationInteractionPtr& interaction, PastingType type) const;

    enum class TextNavigationType {
        NearNoteOrRest,
        NearBeat,
        Fraction
    };

    bool textNavigationAvailable() const;
    bool textNavigationByBeatsAvailable() const;
    bool textNavigationByFractionAvailable() const;
    bool resolveTextNavigationAvailable(TextNavigationType type = TextNavigationType::NearNoteOrRest) const;

    muse::Ret nextTextElement();
    muse::Ret prevTextElement();
    muse::Ret nextWord();
    void nextBeatTextElement();
    void prevBeatTextElement();
    void navigateToTextElement(MoveDirection direction, bool nearNoteOrRest = false, bool moveOnly = true);
    void navigateToTextItemByFraction(const Fraction& fraction);
    void navigateToTextElementInNearMeasure(MoveDirection direction);

    bool toggleNoteInputAllowed() const;
    void startNoteInput();

    mu::engraving::EngravingItem* selectedElement() const;

    const mu::engraving::Harmony* editedChordSymbol() const;

    bool elementHasPopup(const EngravingItem* e) const;

    void checkForScoreCorruptions();

    void toggleAutomation();
    muse::Ret selectAutomationType(const muse::rcommand::Params& params);

    // commands
    template<muse::rcommand::TypedCommand C>
    void registerCommand(std::function<muse::Ret()>);
    template<muse::rcommand::TypedCommand C>
    void registerCommand(muse::Ret (NotationActionController::*)());
    template<muse::rcommand::TypedCommand C>
    void registerCommand(muse::Ret (NotationActionController::*)(const C&));
    template<muse::rcommand::HasOutput C>
    void registerRequestCommand(muse::RetVal<typename C::Output>(NotationActionController::*)() const);
    template<muse::rcommand::HasOutput C>
    void registerRequestCommand(muse::RetVal<typename C::Output>(NotationActionController::*)(const C&) const);

    void registerCommand(const muse::rcommand::Command&, std::function<void()>);
    void registerCommand(const muse::rcommand::Command&, std::function<void()>, bool (NotationActionController::*)() const);
    void registerCommandWithParams(const muse::rcommand::Command&, std::function<muse::Ret(const muse::rcommand::Params&)>);
    void registerCommand(const muse::rcommand::Command&, void (NotationActionController::*)());
    void registerCommand(const muse::rcommand::Command&, void (NotationActionController::*)(), bool (NotationActionController::*)() const);
    void registerCommandWithParams(const muse::rcommand::Command&, muse::Ret (NotationActionController::*)(const muse::rcommand::Params&));

    void registerCommand(const muse::rcommand::Command&, void (INotationInteraction::*)(), PlayMode = PlayMode::NoPlay,
                         bool (NotationActionController::*)() const = nullptr);
    template<typename P1>
    void registerCommand(const muse::rcommand::Command&, void (INotationInteraction::*)(P1), P1, PlayMode = PlayMode::NoPlay,
                         bool (NotationActionController::*)() const = nullptr);
    void registerNavigationByFractionCommand(const muse::rcommand::Command&, const Fraction&);

    void registerGetCommand(const muse::rcommand::Command&, std::function<muse::RetVal<muse::rcommand::Params>()>);
    void registerGetCommand(const muse::rcommand::Command&, muse::RetVal<muse::rcommand::Params>(NotationActionController::*)() const);

    void registerViewCommand(const muse::rcommand::Command&, void (INotationViewController::*)());
    template<typename P1>
    void registerViewCommand(const muse::rcommand::Command&, void (INotationViewController::*)(P1), P1);

    muse::Ret zoomToPercent(const muse::rcommand::Params& params);

    muse::Ret setPianoKeyboardNumberOfKeys(const muse::rcommand::Params& params);

    INotationViewController* m_viewController = nullptr;

    muse::async::Channel<bool> m_hasSelectionChanged;
    muse::async::Channel<bool> m_textEditingChanged;
    muse::async::Notification m_stackChanged;
    muse::async::Notification m_selectionChanged;

    muse::async::Channel<bool> m_isNoteInputAllowedChanged;
    muse::async::Notification m_noteInputStateChanged;

    muse::async::Channel<ScoreConfigType> m_scoreConfigChanged;
    muse::async::Notification m_currentNotationStyleChanged;
    muse::async::Notification m_automationModeEnabledChanged;
};
}
