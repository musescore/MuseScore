//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#ifndef MU_SHORTCUTS_IMIDIREMOTE_H
#define MU_SHORTCUTS_IMIDIREMOTE_H

#include "modularity/imoduleexport.h"
#include "midi/miditypes.h"
#include "ret.h"

namespace mu {
namespace shortcuts {
class IMidiRemote : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMidiRemote)
public:
    virtual ~IMidiRemote() = default;

    enum class Action {
        RMIDI_REWIND,
        RMIDI_TOGGLE_PLAY,
        RMIDI_PLAY,
        RMIDI_STOP,
        RMIDI_NOTE1,
        RMIDI_NOTE2,
        RMIDI_NOTE4,
        RMIDI_NOTE8,
        RMIDI_NOTE16,
        RMIDI_NOTE32,
        RMIDI_NOTE64,
        RMIDI_REST,
        RMIDI_DOT,
        RMIDI_DOTDOT,
        RMIDI_TIE,
        RMIDI_UNDO,
        RMIDI_NOTE_EDIT_MODE,
        RMIDI_REALTIME_ADVANCE,
        MIDI_REMOTES
    };

    // Setting
    virtual void setIsSettingMode(bool arg) = 0;
    virtual bool isSettingMode() const = 0;

    virtual void setCurrentActionEvent(const midi::Event& ev) = 0;

    // Process
    virtual Ret process(const midi::Event& ev) = 0;
};
}
}

#endif // MU_SHORTCUTS_IMIDIREMOTE_H
