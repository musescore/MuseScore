/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "modularity/imoduleinterface.h"

#include "global/async/channel.h"
#include "global/async/notification.h"

#include "rcommand/commandtypes.h"

#include "notation/types/noteinputtypes.h"
#include "notation/notationtypes.h"
#include "notation/inotationstyle.h"

namespace mu::notation {
class INotationViewController;
class INotationCommandsController : MODULE_CONTEXT_INTERFACE
{
    INTERFACE_ID(INotationCommandsController)
public:
    virtual ~INotationCommandsController() = default;

    //! NOTE We need to set a "current" view controller that will "receive" commands, such as zoom.
    //! Don't forget to reset the current view controller when deleting, but only if it is the current one.
    virtual void setViewController(INotationViewController* controller) = 0;
    virtual INotationViewController* viewController() const = 0;

    virtual bool hasSelection() const = 0;
    virtual muse::async::Notification selectionChanged() const = 0;
    virtual bool selectionHasTie() const = 0;
    virtual bool selectionHasLaissezVib() const = 0;
    virtual bool selectionHasSlur() const = 0;

    virtual bool canUndo() const = 0;
    virtual bool canRedo() const = 0;
    virtual muse::async::Notification stackChanged() const = 0;

    virtual bool isTextEditing() const = 0;
    virtual bool isLyricsEditing() const = 0;
    virtual muse::async::Channel<bool> textEditingChanged() const = 0;

    virtual bool isNoteInputAllowed() const = 0;
    virtual muse::async::Channel<bool> isNoteInputAllowedChanged() const = 0;

    virtual muse::async::Notification noteInputStateChanged() const = 0;
    virtual bool isNoteInputMode() const = 0;
    virtual NoteInputMethod noteInputMethod() const = 0;
    virtual engraving::DurationType currentDurationType() const = 0;
    virtual int currentDotCount() const = 0;
    virtual bool currentIsRest() const = 0;
    virtual engraving::AccidentalType currentAccidentalType() const = 0;
    virtual std::set<engraving::SymId> currentArticulations() const = 0;
    virtual engraving::voice_idx_t currentVoice() const = 0;

    virtual bool isNoteInputActionAllowed() const = 0;
    virtual bool isNoteOrRestSelected() const = 0;
    virtual bool isMoveSelectionAvailable(MoveSelectionType type) const = 0;

    virtual bool isToggleLayoutBreakAvailable() const = 0;

    virtual ScoreConfig scoreConfig() const = 0;
    virtual muse::async::Channel<ScoreConfigType> scoreConfigChanged() const = 0;

    virtual INotationStylePtr notationStyle() const = 0;
    virtual muse::async::Notification notationStyleChanged() const = 0;

    virtual bool isTablatureStaff() const = 0;

    virtual bool isAutomationModeEnabled() const = 0;
    virtual muse::async::Notification automationModeEnabledChanged() const = 0;

    virtual bool isDebuggingCommandEnabled(const muse::rcommand::Command& command) const = 0;
    virtual muse::async::Notification debuggingOptionsChanged() const = 0;
};
}
