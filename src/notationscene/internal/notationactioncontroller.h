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
#include "actions/actionable.h"
#include "actions/actiontypes.h"
#include "rcommand/commandable.h"

#include "modularity/ioc.h"
#include "interactive/iinteractive.h"
#include "actions/iactionsdispatcher.h"
#include "rcommand/commandtypes.h"
#include "rcommand/icommandsstate.h"
#include "rcommand/icommanddispatcher.h"
#include "context/iglobalcontext.h"
#include "context/iuicontextresolver.h"
#include "playback/iplaybackcontroller.h"
#include "engraving/iengravingconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "notationscene/inotationsceneconfiguration.h"

#include "notation/inotation_fwd.h"
#include "notation/notationtypes.h"

#include "inotationviewcontroller.h"

namespace mu::notation {
class NotationActionController : public INotationCommandsController, public muse::actions::Actionable, public muse::rcommand::Commandable,
    public muse::async::Asyncable, public muse::Contextable
{
    muse::GlobalInject<INotationConfiguration> configuration;
    muse::GlobalInject<INotationSceneConfiguration> sceneConfiguration;
    muse::GlobalInject<engraving::IEngravingConfiguration> engravingConfiguration;
    muse::ContextInject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::ContextInject<muse::rcommand::ICommandDispatcher> commandDispatcher = { this };
    muse::ContextInject<muse::rcommand::ICommandsState> commandsState = { this };
    muse::ContextInject<context::IGlobalContext> globalContext = { this };
    muse::ContextInject<context::IUiContextResolver> uiContextResolver = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<playback::IPlaybackController> playbackController = { this };

public:

    NotationActionController(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    void init();

    bool canReceiveAction(const muse::actions::ActionCode& code) const override;

    void setViewController(INotationViewController* controller) override;
    INotationViewController* viewController() const override;

    bool hasSelection() const override;
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
    bool isMoveSelectionAvailable(MoveSelectionType type) const override;

    bool isToggleLayoutBreakAvailable() const override;

    ScoreConfig scoreConfig() const override;
    muse::async::Channel<ScoreConfigType> scoreConfigChanged() const override;

    INotationStylePtr notationStyle() const override;
    muse::async::Notification notationStyleChanged() const override;

    bool isTablatureStaff() const override;

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

    void toggleNoteInput(NoteInputMethod method);
    void toggleNoteInputInsert();

    muse::Ret addNote(const muse::rcommand::CommandQuery& query);
    void addNote(NoteName noteName, NoteAddingMode addingMode);
    muse::Ret addDrumNote(const muse::rcommand::CommandQuery& query);
    void doAddNote(const NoteInputParams& params, const NoteAddingMode& addingMode);
    void setDuration(engraving::DurationType duration);
    void toggleRest();
    void toggleDots(int dots);
    muse::Ret putNote(const muse::rcommand::CommandQuery& query);
    muse::Ret removeNote(const muse::rcommand::CommandQuery& query);
    void increaseDecreaseDuration(int steps, bool stepByDots);
    void realtimeAdvance();

    void toggleAccidental(engraving::AccidentalType type);
    void toggleArticulation(SymbolId articulationSymbolId);

    muse::Ret putTuplet(const muse::rcommand::CommandQuery& query);
    void putTuplet(const TupletOptions& options);
    void putTuplet(int tupletCount);

    muse::Ret moveWithRet(MoveDirection direction, bool quickly = false);
    void move(MoveDirection direction, bool quickly = false);
    void moveInputNotes(bool up, PitchMode mode);
    void movePitchDiatonic(MoveDirection direction, bool);

    void changeVoice(voice_idx_t voiceIndex);
    void swapVoices(voice_idx_t voiceIndex1, voice_idx_t voiceIndex2);

    void cutSelection();
    void repeatSelection();
    void addTie();
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

    void openSelectionMoreOptions();

    muse::Ret startEditSelectedElement(const muse::rcommand::CommandQuery& query);
    muse::Ret startEditSelectedText(const muse::rcommand::CommandQuery& query);

    muse::Ret addMeasures(const muse::rcommand::CommandQuery& query, AddBoxesTarget target);
    void addBoxes(BoxType boxType, int count, AddBoxesTarget target);

    void addStretch(qreal value);

    void unrollRepeats();

    void toggleMmrest();
    void toggleHideEmpty();
    void addFullMeasureRest();

    void resetState();
    void resetStretch();
    void resetBeamMode();

    muse::Ret openEditStyleDialog(const muse::rcommand::CommandQuery& query);
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

    bool isNotationPage() const;

    void checkForScoreCorruptions();

    void toggleAutomation();
    muse::Ret selectAutomationType(const muse::rcommand::CommandQuery& query);

    // commands
    void registerCommand(const muse::rcommand::Command&, std::function<void()>);
    void registerCommand(const muse::rcommand::Command&, std::function<void()>, bool (NotationActionController::*)() const);
    void registerQueryCommand(const muse::rcommand::Command&, std::function<muse::Ret(const muse::rcommand::CommandQuery&)>);
    void registerCommand(const muse::rcommand::Command&, void (NotationActionController::*)());
    void registerCommand(const muse::rcommand::Command&, void (NotationActionController::*)(), bool (NotationActionController::*)() const);
    void registerQueryCommand(const muse::rcommand::Command&, muse::Ret (NotationActionController::*)(const muse::rcommand::CommandQuery&));

    void registerCommand(const muse::rcommand::Command&, void (INotationInteraction::*)(), PlayMode = PlayMode::NoPlay,
                         bool (NotationActionController::*)() const = nullptr);
    template<typename P1>
    void registerCommand(const muse::rcommand::Command&, void (INotationInteraction::*)(P1), P1, PlayMode = PlayMode::NoPlay,
                         bool (NotationActionController::*)() const = nullptr);
    void registerNoteInputCommand(const muse::rcommand::Command& command, NoteInputMethod method);
    void registerNoteCommand(const muse::rcommand::Command&, NoteName, NoteAddingMode addingMode = NoteAddingMode::NextChord);
    void registerNavigationByFractionCommand(const muse::rcommand::Command&, const Fraction&);

    muse::Ret select(const muse::rcommand::CommandQuery& query);
    void select(SelectionTarget target, PlayMode playMode = PlayMode::NoPlay);

    void registerViewCommand(const muse::rcommand::Command&, void (INotationViewController::*)());
    template<typename P1>
    void registerViewCommand(const muse::rcommand::Command&, void (INotationViewController::*)(P1), P1);

    muse::Ret zoomToPercent(const muse::rcommand::CommandQuery& query);

    muse::Ret setPianoKeyboardNumberOfKeys(const muse::rcommand::CommandQuery& query);

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

    using IsActionEnabledFunc = std::function<bool ()>;
    std::map<muse::actions::ActionCode, IsActionEnabledFunc> m_isEnabledMap;
    std::unordered_set<muse::actions::ActionCode> m_isAllowedDuringPlayback;
};
}
