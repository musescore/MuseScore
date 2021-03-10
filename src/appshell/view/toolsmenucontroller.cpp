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
using namespace mu::actions;

void ToolsMenuController::init()
{
    controller()->actionsAvailableChanged().onReceive(this, [this](const std::vector<ActionCode>& actionCodes) {
        m_actionsReceiveAvailableChanged.send(actionCodes);
    });
}

bool ToolsMenuController::contains(const ActionCode& actionCode) const
{
    std::map<ActionCode, AvailableCallback> actions = this->actions();
    return actions.find(actionCode) != actions.end();
}

bool ToolsMenuController::actionAvailable(const ActionCode& actionCode) const
{
    std::map<ActionCode, AvailableCallback> actions = this->actions();

    if (actions.find(actionCode) == actions.end()) {
        return false;
    }

    return actions[actionCode]();
}

mu::async::Channel<ActionCodeList> ToolsMenuController::actionsAvailableChanged() const
{
    return m_actionsReceiveAvailableChanged;
}

std::map<ActionCode, ToolsMenuController::AvailableCallback> ToolsMenuController::actions() const
{
    static std::map<ActionCode, AvailableCallback> _actions = {
        { "voice-x12", std::bind(&ToolsMenuController::isVoiceAvailable, this, 1, 2) },
        { "voice-x13", std::bind(&ToolsMenuController::isVoiceAvailable, this, 1, 3) },
        { "voice-x14", std::bind(&ToolsMenuController::isVoiceAvailable, this, 1, 4) },
        { "voice-x23", std::bind(&ToolsMenuController::isVoiceAvailable, this, 2, 3) },
        { "voice-x24", std::bind(&ToolsMenuController::isVoiceAvailable, this, 2, 4) },
        { "voice-x34", std::bind(&ToolsMenuController::isVoiceAvailable, this, 3, 4) },
        { "split-measure", std::bind(&ToolsMenuController::isSplitMeasureAvailable, this) },
        { "join-measures", std::bind(&ToolsMenuController::isJoinMeasuresAvailable, this) },
        { "transpose", std::bind(&ToolsMenuController::isTransposeAvailable, this) },
        { "explode", std::bind(&ToolsMenuController::isExplodeAvailable, this) },
        { "implode", std::bind(&ToolsMenuController::isImplodeAvailable, this) },
        { "realize-chord-symbols", std::bind(&ToolsMenuController::isRealizeSelectedChordSymbolsAvailable, this) },
        { "time-delete", std::bind(&ToolsMenuController::isRemoveSelectedRangeAvailable, this) },
        { "slash-fill", std::bind(&ToolsMenuController::isFillSelectionWithSlashesAvailable, this) },
        { "slash-rhythm", std::bind(&ToolsMenuController::isReplaceSelectedNotesWithSlashesAvailable, this) },
        { "pitch-spell", std::bind(&ToolsMenuController::isSpellPitchesAvailable, this) },
        { "reset-groupings", std::bind(&ToolsMenuController::isRegroupNotesAndRestsAvailable, this) },
        { "resequence-rehearsal-marks", std::bind(&ToolsMenuController::isResequenceRehearsalMarksAvailable, this) },
        { "unroll-repeats", std::bind(&ToolsMenuController::isUnrollRepeatsAvailable, this) },
        { "copy-lyrics-to-clipboard", std::bind(&ToolsMenuController::isCopyLyricsAvailable, this) },
        { "fotomode", std::bind(&ToolsMenuController::isFotomodeAvailable, this) },
        { "del-empty-measures", std::bind(&ToolsMenuController::isRemoveEmptyTrailingMeasuresAvailable, this) },
    };

    return _actions;
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
