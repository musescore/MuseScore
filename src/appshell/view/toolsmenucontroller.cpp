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
#include "toolsmenucontroller.h"

#include "log.h"

using namespace mu::appshell;
using namespace mu::notation;

ToolsMenuController::ToolsMenuController()
{
    controller()->actionsAvailableChanged().onReceive(this, [this](const std::vector<actions::ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

mu::async::Channel<std::vector<mu::actions::ActionCode> > ToolsMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

bool ToolsMenuController::isVoiceAvailable(int voiceIndex1, int voiceIndex2) const
{
    return controller()->actionAvailable("voice-x" + std::to_string(voiceIndex1 - 1) + std::to_string(voiceIndex2 - 1));
}

bool ToolsMenuController::isSplitMeasureAvailable() const
{
    return controller()->actionAvailable("split-measure");
}

bool ToolsMenuController::isJoinMeasuresAvailable() const
{
    return controller()->actionAvailable("join-measures");
}

bool ToolsMenuController::isTransposeAvailable() const
{
    return controller()->actionAvailable("transpose");
}

bool ToolsMenuController::isExplodeAvailable() const
{
    return controller()->actionAvailable("explode");
}

bool ToolsMenuController::isImplodeAvailable() const
{
    return controller()->actionAvailable("implode");
}

bool ToolsMenuController::isRealizeSelectedChordSymbolsAvailable() const
{
    return controller()->actionAvailable("realize-chord-symbols");
}

bool ToolsMenuController::isRemoveSelectedRangeAvailable() const
{
    return controller()->actionAvailable("time-delete");
}

bool ToolsMenuController::isRemoveEmptyTrailingMeasuresAvailable() const
{
    return controller()->actionAvailable("del-empty-measures");
}

bool ToolsMenuController::isFillSelectionWithSlashesAvailable() const
{
    return controller()->actionAvailable("slash-fill");
}

bool ToolsMenuController::isReplaceSelectedNotesWithSlashesAvailable() const
{
    return controller()->actionAvailable("slash-rhythm");
}

bool ToolsMenuController::isSpellPitchesAvailable() const
{
    return controller()->actionAvailable("pitch-spell");
}

bool ToolsMenuController::isRegroupNotesAndRestsAvailable() const
{
    return controller()->actionAvailable("reset-groupings");
}

bool ToolsMenuController::isResequenceRehearsalMarksAvailable() const
{
    return controller()->actionAvailable("resequence-rehearsal-marks");
}

bool ToolsMenuController::isUnrollRepeatsAvailable() const
{
    return controller()->actionAvailable("unroll-repeats");
}

bool ToolsMenuController::isCopyLyricsAvailable() const
{
    return controller()->actionAvailable("copy-lyrics-to-clipboard");
}

bool ToolsMenuController::isFotomodeAvailable() const
{
    return controller()->actionAvailable("fotomode");
}
