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
#include "playbackactions.h"

#include "ui/view/iconcodes.h"

using namespace mu::playback;
using namespace mu::actions;
using namespace mu::shortcuts;
using namespace mu::ui;

const ActionList PlaybackActions::m_mainActions = {
    ActionItem("play",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Play"),
               QT_TRANSLATE_NOOP("action", "Start or stop playback"),
               IconCode::Code::PLAY
               ),
    ActionItem("rewind",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Rewind"),
               QT_TRANSLATE_NOOP("action", "Rewind to start position"),
               IconCode::Code::REWIND
               ),
    ActionItem("loop",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Loop Playback"),
               QT_TRANSLATE_NOOP("action", "Toggle 'Loop Playback'"),
               IconCode::Code::LOOP
               ),
    ActionItem("metronome",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Metronome"),
               QT_TRANSLATE_NOOP("action", "Play metronome during playback"),
               IconCode::Code::METRONOME
               )
};

const ActionList PlaybackActions::m_settingsActions = {
    ActionItem("midi-on",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "MIDI Input"),
               QT_TRANSLATE_NOOP("action", "Enable 'MIDI Input'"),
               IconCode::Code::MIDI_INPUT
               ),
    ActionItem("repeat",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Play Repeats"),
               QT_TRANSLATE_NOOP("action", "Play repeats"),
               IconCode::Code::PLAY_REPEATS
               ),
    ActionItem("pan",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Pan Score"),
               QT_TRANSLATE_NOOP("action", "Pan score automatically"),
               IconCode::Code::PAN_SCORE
               ),
    ActionItem("countin",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Count-In"),
               QT_TRANSLATE_NOOP("action", "Enable count-in when playing"),
               IconCode::Code::COUNT_IN
               ),
    ActionItem("loop-in",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Loop In"),
               QT_TRANSLATE_NOOP("action", "Set loop marker left"),
               IconCode::Code::LOOP_IN
               ),
    ActionItem("loop-out",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Loop Out"),
               QT_TRANSLATE_NOOP("action", "Set loop marker right"),
               IconCode::Code::LOOP_OUT
               ),
};

const ActionList PlaybackActions::m_loopBoundaryActions = {
    ActionItem("loop-in",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Loop In"),
               QT_TRANSLATE_NOOP("action", "Set loop marker left"),
               IconCode::Code::LOOP_IN
               ),
    ActionItem("loop-out",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Loop Out"),
               QT_TRANSLATE_NOOP("action", "Set loop marker right"),
               IconCode::Code::LOOP_OUT
               ),
};

const ActionList& PlaybackActions::settingsActions()
{
    return m_settingsActions;
}

const ActionList& PlaybackActions::loopBoundaryActions()
{
    return m_loopBoundaryActions;
}

const ActionItem& PlaybackActions::action(const ActionCode& actionCode) const
{
    for (const ActionItem& action : m_mainActions) {
        if (action.code == actionCode) {
            return action;
        }
    }

    for (const ActionItem& action : m_settingsActions) {
        if (action.code == actionCode) {
            return action;
        }
    }

    for (const ActionItem& action : m_loopBoundaryActions) {
        if (action.code == actionCode) {
            return action;
        }
    }

    static ActionItem null;
    return null;
}
