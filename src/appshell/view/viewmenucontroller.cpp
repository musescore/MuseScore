//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "viewmenucontroller.h"

using namespace mu::appshell;
using namespace mu::notation;
using namespace mu::actions;
using namespace mu::uicomponents;

static ActionCodeList panelsActionCodes = {
    "toggle-palette",
    "toggle-instruments",
    "inspector",
    "toggle-navigator",
    "toggle-timeline",
    "toggle-mixer",
    "synth-control",
    "toggle-selection-window",
    "toggle-piano",
    "toggle-scorecmp-tool",
    "toggle-transport",
    "toggle-noteinput",
    "toggle-notationtoolbar",
    "toggle-undoredo",
    "toggle-statusbar",
};

static ActionCodeList scoreConfigActionCodes = {
    "show-invisible",
    "show-unprintable",
    "show-frames",
    "show-pageborders",
    "show-irregular"
};

static ActionCodeList zoomActionCodes = {
    "zoomin",
    "zoomout"
};

static ActionCode FULLSCREEN_ACTION_CODE("fullscreen");
static ActionCode MASTER_PALETTE_ACTION_CODE("masterpalette");

void ViewMenuController::init()
{
    notationController()->actionsAvailableChanged().onReceive(this, [this](const actions::ActionCodeList& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });

    paletteController()->actionsAvailableChanged().onReceive(this, [this](const actions::ActionCodeList& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });

    applicationController()->actionsAvailableChanged().onReceive(this, [this](const actions::ActionCodeList& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });

    paletteController()->isMasterPaletteOpened().ch.onReceive(this, [this](bool) {
        m_actionsReceiveAvailableChanged.send({ MASTER_PALETTE_ACTION_CODE });
    });

    notationPageState()->panelsVisibleChanged().onReceive(this, [this](const PanelTypeList& panels) {
        ActionCodeList changedCodes;
        for (PanelType panelType: panels) {
            changedCodes.push_back(panelTypeActionCode(panelType));
        }
        m_actionsReceiveAvailableChanged.send(changedCodes);
    });

    applicationController()->isFullScreen().ch.onReceive(this, [this](bool) {
        m_actionsReceiveAvailableChanged.send({ FULLSCREEN_ACTION_CODE });
    });
}

bool ViewMenuController::contains(const ActionCode& actionCode) const
{
    return notationControllerContains(actionCode)
           || paletteControllerContains(actionCode)
           || applicationControllerContains(actionCode);
}

ActionState ViewMenuController::actionState(const ActionCode& actionCode) const
{
    ActionState state;
    state.enabled = actionEnabled(actionCode);
    state.checkable = actionCheckable(actionCode);
    state.checked = actionChecked(actionCode);

    return state;
}

mu::async::Channel<ActionCodeList> ViewMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

IMasterNotationPtr ViewMenuController::currentMasterNotation() const
{
    return globalContext()->currentMasterNotation();
}

INotationPtr ViewMenuController::currentNotation() const
{
    return currentMasterNotation() ? currentMasterNotation()->notation() : nullptr;
}

INotationInteractionPtr ViewMenuController::notationInteraction() const
{
    return currentNotation() ? currentNotation()->interaction() : nullptr;
}

ActionCodeList ViewMenuController::notationControllerActions() const
{
    ActionCodeList actions = zoomActionCodes;
    actions.insert(actions.end(), scoreConfigActionCodes.begin(), scoreConfigActionCodes.end());

    return actions;
}

ActionCodeList ViewMenuController::paletteControllerActions() const
{
    static ActionCodeList actions = {
        MASTER_PALETTE_ACTION_CODE
    };

    return actions;
}

ActionCodeList ViewMenuController::applicationControllerActions() const
{
    ActionCodeList actions = {
        "split-h",
        "split-v",
        FULLSCREEN_ACTION_CODE
    };

    actions.insert(actions.end(), panelsActionCodes.begin(), panelsActionCodes.end());

    return actions;
}

bool ViewMenuController::notationControllerContains(const ActionCode& actionCode) const
{
    ActionCodeList notationActions = notationControllerActions();
    return std::find(notationActions.begin(), notationActions.end(), actionCode) != notationActions.end();
}

bool ViewMenuController::paletteControllerContains(const ActionCode& actionCode) const
{
    ActionCodeList paletteActions = paletteControllerActions();
    return std::find(paletteActions.begin(), paletteActions.end(), actionCode) != paletteActions.end();
}

bool ViewMenuController::applicationControllerContains(const ActionCode& actionCode) const
{
    ActionCodeList applicationActions = applicationControllerActions();
    return std::find(applicationActions.begin(), applicationActions.end(), actionCode) != applicationActions.end();
}

bool ViewMenuController::actionEnabled(const ActionCode& actionCode) const
{
    if (notationControllerContains(actionCode)) {
        return notationController()->actionAvailable(actionCode);
    }

    if (paletteControllerContains(actionCode)) {
        return paletteController()->actionAvailable(actionCode);
    }

    if (applicationControllerContains(actionCode)) {
        return applicationController()->actionAvailable(actionCode);
    }

    return false;
}

bool ViewMenuController::actionCheckable(const ActionCode& actionCode) const
{
    static ActionCodeList uncheckableActions = {
        "configure-workspaces"
    };

    uncheckableActions.insert(uncheckableActions.end(), zoomActionCodes.begin(), zoomActionCodes.end());

    return std::find(uncheckableActions.begin(), uncheckableActions.end(), actionCode) == uncheckableActions.end();
}

bool ViewMenuController::actionChecked(const ActionCode& actionCode) const
{
    auto panelActionCodeIt = std::find(panelsActionCodes.begin(), panelsActionCodes.end(), actionCode);
    if (panelActionCodeIt != panelsActionCodes.end()) {
        return isPanelVisible(panelType(actionCode));
    }

    auto scoreConfigActionCodeIt = std::find(scoreConfigActionCodes.begin(), scoreConfigActionCodes.end(), actionCode);
    if (scoreConfigActionCodeIt != scoreConfigActionCodes.end()) {
        return isScoreConfigChecked(scoreConfigType(actionCode));
    }

    if (FULLSCREEN_ACTION_CODE == actionCode) {
        return isFullScreen();
    }

    if (MASTER_PALETTE_ACTION_CODE == actionCode) {
        return isMasterPaletteOpened();
    }

    return false;
}

PanelType ViewMenuController::panelType(const ActionCode& actionCode) const
{
    std::map<std::string, PanelType> panelTypes {
        { "toggle-palette", PanelType::Palette },
        { "toggle-instruments", PanelType::Instruments },
        { "inspector", PanelType::Inspector },
        { "toggle-notationtoolbar", PanelType::NotationToolBar },
        { "toggle-noteinput", PanelType::NoteInputBar },
        { "toggle-undoredo", PanelType::UndoRedoToolBar },
        { "toggle-navigator", PanelType::NotationNavigator },
        { "toggle-statusbar", PanelType::NotationStatusBar },
        { "toggle-transport", PanelType::PlaybackToolBar },
        { "toggle-mixer", PanelType::Mixer },
        { "toggle-timeline", PanelType::TimeLine },
        { "synth-control", PanelType::Synthesizer },
        { "toggle-selection-window", PanelType::SelectionFilter },
        { "toggle-piano", PanelType::Piano },
        { "toggle-scorecmp-tool", PanelType::ComparisonTool }
    };

    return panelTypes[actionCode];
}

ActionCode ViewMenuController::panelTypeActionCode(PanelType type) const
{
    switch (type) {
    case PanelType::Palette: return "toggle-palette";
    case PanelType::Instruments: return "toggle-instruments";
    case PanelType::Inspector: return "inspector";
    case PanelType::NotationToolBar: return "toggle-notationtoolbar";
    case PanelType::NoteInputBar: return "toggle-noteinput";
    case PanelType::UndoRedoToolBar: return "toggle-undoredo";
    case PanelType::NotationNavigator: return "toggle-navigator";
    case PanelType::NotationStatusBar: return "toggle-statusbar";
    case PanelType::PlaybackToolBar: return "toggle-transport";
    case PanelType::Mixer: return "toggle-mixer";
    case PanelType::TimeLine: return "toggle-timeline";
    case PanelType::Synthesizer: return "synth-control";
    case PanelType::SelectionFilter: return "toggle-selection-window";
    case PanelType::Piano: return "toggle-piano";
    case PanelType::ComparisonTool: return "toggle-scorecmp-tool";
    }

    return "";
}

bool ViewMenuController::isPanelVisible(PanelType panelType) const
{
    return notationPageState()->isPanelVisible(panelType);
}

ScoreConfigType ViewMenuController::scoreConfigType(const ActionCode& actionCode) const
{
    std::map<std::string, ScoreConfigType> scoreConfigTypes {
        { "show-invisible", ScoreConfigType::ShowInvisibleElements },
        { "show-unprintable", ScoreConfigType::ShowUnprintableElements },
        { "show-frames", ScoreConfigType::ShowFrames },
        { "show-pageborders", ScoreConfigType::ShowPageMargins },
        { "show-irregular", ScoreConfigType::MarkIrregularMeasures },
    };

    return scoreConfigTypes[actionCode];
}

bool ViewMenuController::isScoreConfigChecked(ScoreConfigType configType) const
{
    auto interaction = notationInteraction();
    if (!interaction) {
        return false;
    }

    ScoreConfig scoreConfig = interaction->scoreConfig();
    switch (configType) {
    case ScoreConfigType::ShowInvisibleElements:
        return scoreConfig.isShowInvisibleElements;
    case ScoreConfigType::ShowUnprintableElements:
        return scoreConfig.isShowUnprintableElements;
    case ScoreConfigType::ShowFrames:
        return scoreConfig.isShowFrames;
    case ScoreConfigType::ShowPageMargins:
        return scoreConfig.isShowPageMargins;
    case ScoreConfigType::MarkIrregularMeasures:
        return scoreConfig.isMarkIrregularMeasures;
    }

    return false;
}

bool ViewMenuController::isFullScreen() const
{
    return applicationController()->isFullScreen().val;
}

bool ViewMenuController::isMasterPaletteOpened() const
{
    return paletteController()->isMasterPaletteOpened().val;
}
