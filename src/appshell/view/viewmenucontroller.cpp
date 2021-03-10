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

#include "log.h"

using namespace mu::appshell;
using namespace mu::notation;
using namespace mu::framework;
using namespace mu::actions;

void ViewMenuController::init()
{
    notationController()->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });

    paletteController()->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });

    applicationController()->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

bool ViewMenuController::contains(const ActionCode& actionCode) const
{
    std::map<ActionCode, AvailableCallback> actions = this->actions();
    return actions.find(actionCode) != actions.end();
}

bool ViewMenuController::actionAvailable(const ActionCode& actionCode) const
{
    std::map<ActionCode, AvailableCallback> actions = this->actions();

    if (actions.find(actionCode) == actions.end()) {
        return false;
    }

    return actions[actionCode]();
}

mu::async::Channel<ActionCodeList> ViewMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

std::map<ActionCode, ViewMenuController::AvailableCallback> ViewMenuController::actions() const
{
    static std::map<ActionCode, AvailableCallback> _actions = {
        { "toggle-palette", std::bind(&ViewMenuController::isPanelAvailable, this, PanelType::Palette) },
        { "toggle-instruments", std::bind(&ViewMenuController::isPanelAvailable, this, PanelType::Instruments) },
        { "masterpalette", std::bind(&ViewMenuController::isMasterPaletteAvailable, this) },
        { "inspector", std::bind(&ViewMenuController::isPanelAvailable, this, PanelType::Inspector) },
        { "toggle-navigator", std::bind(&ViewMenuController::isPanelAvailable, this, PanelType::NotationNavigator) },
        { "toggle-timeline", std::bind(&ViewMenuController::isPanelAvailable, this, PanelType::TimeLine) },
        { "toggle-mixer", std::bind(&ViewMenuController::isPanelAvailable, this, PanelType::Mixer) },
        { "synth-control", std::bind(&ViewMenuController::isPanelAvailable, this, PanelType::Synthesizer) },
        { "toggle-selection-window", std::bind(&ViewMenuController::isPanelAvailable, this, PanelType::SelectionFilter) },
        { "toggle-piano", std::bind(&ViewMenuController::isPanelAvailable, this, PanelType::Piano) },
        { "toggle-scorecmp-tool", std::bind(&ViewMenuController::isPanelAvailable, this, PanelType::ComparisonTool) },
        { "zoomin", std::bind(&ViewMenuController::isZoomInAvailable, this) },
        { "zoomout", std::bind(&ViewMenuController::isZoomInAvailable, this) },
        { "toggle-transport", std::bind(&ViewMenuController::isPanelAvailable, this, PanelType::PlaybackToolBar) },
        { "toggle-noteinput", std::bind(&ViewMenuController::isPanelAvailable, this, PanelType::NoteInputBar) },
        { "toggle-notationtoolbar", std::bind(&ViewMenuController::isPanelAvailable, this, PanelType::NotationToolBar) },
        { "toggle-undoredo", std::bind(&ViewMenuController::isPanelAvailable, this, PanelType::UndoRedoToolBar) },
        { "toggle-statusbar", std::bind(&ViewMenuController::isPanelAvailable, this, PanelType::NotationStatusBar) },
        { "split-h", std::bind(&ViewMenuController::isSplitAvailable, this, framework::Orientation::Horizontal) },
        { "split-v", std::bind(&ViewMenuController::isSplitAvailable, this, framework::Orientation::Vertical) },
        { "show-invisible", std::bind(&ViewMenuController::isScoreConfigAvailable, this,ScoreConfigType::ShowInvisibleElements) },
        { "show-unprintable", std::bind(&ViewMenuController::isScoreConfigAvailable, this, ScoreConfigType::ShowUnprintableElements) },
        { "show-frames", std::bind(&ViewMenuController::isScoreConfigAvailable, this,ScoreConfigType::ShowFrames) },
        { "show-pageborders", std::bind(&ViewMenuController::isScoreConfigAvailable, this,ScoreConfigType::ShowPageMargins) },
        { "mark-irregular", std::bind(&ViewMenuController::isScoreConfigAvailable, this,ScoreConfigType::MarkIrregularMeasures) },
        { "fullscreen", std::bind(&ViewMenuController::isFullScreenAvailable, this) }
    };

    return _actions;
}

bool ViewMenuController::isPanelAvailable(PanelType panelType) const
{
    return applicationController()->actionAvailable(panelActionCode(panelType));
}

bool ViewMenuController::isMasterPaletteAvailable() const
{
    return paletteController()->actionAvailable("masterpalette");
}

bool ViewMenuController::isZoomInAvailable() const
{
    return notationController()->actionAvailable("zoomin");
}

bool ViewMenuController::isZoomOutAvailable() const
{
    return notationController()->actionAvailable("zoomout");
}

bool ViewMenuController::isSplitAvailable(Orientation) const
{
    NOT_IMPLEMENTED;
    return false;
}

bool ViewMenuController::isScoreConfigAvailable(ScoreConfigType configType) const
{
    return notationController()->actionAvailable(scoreConfigActionCode(configType));
}

bool ViewMenuController::isFullScreenAvailable() const
{
    return applicationController()->actionAvailable("fullscreen");
}

std::string ViewMenuController::scoreConfigActionCode(ScoreConfigType configType) const
{
    std::map<ScoreConfigType, std::string> scoreConfigStrings {
        { ScoreConfigType::ShowInvisibleElements, "show-invisible" },
        { ScoreConfigType::ShowUnprintableElements,"show-unprintable" },
        { ScoreConfigType::ShowFrames,"show-frames" },
        { ScoreConfigType::ShowPageMargins,"show-pageborders" },
        { ScoreConfigType::MarkIrregularMeasures, "show-irregular" }
    };

    return scoreConfigStrings[configType];
}
