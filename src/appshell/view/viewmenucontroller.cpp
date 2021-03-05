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

ViewMenuController::ViewMenuController()
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

mu::async::Channel<std::vector<mu::actions::ActionCode> > ViewMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
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

std::string ViewMenuController::panelActionCode(PanelType panelType) const
{
    std::map<PanelType, std::string> panelTypeStrings {
        { PanelType::Palette, "toggle-palette" },
        { PanelType::Instruments, "toggle-instruments" },
        { PanelType::Inspector, "inspector" },
        { PanelType::NotationToolBar, "toggle-notationtoolbar" },
        { PanelType::NoteInputBar, "toggle-noteinput" },
        { PanelType::UndoRedoToolBar, "toggle-undoredo" },
        { PanelType::NotationNavigator, "toggle-navigator" },
        { PanelType::NotationStatusBar, "toggle-statusbar" },
        { PanelType::PlaybackToolBar, "toggle-transport" },
        { PanelType::Mixer, "toggle-mixer" },
        { PanelType::TimeLine, "toggle-timeline" },
        { PanelType::Synthesizer, "synth-control" },
        { PanelType::SelectionFilter, "toggle-selection-window" },
        { PanelType::Piano, "toggle-piano" },
        { PanelType::ComparisonTool, "toggle-scorecmp-tool" }
    };

    return panelTypeStrings[panelType];
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
