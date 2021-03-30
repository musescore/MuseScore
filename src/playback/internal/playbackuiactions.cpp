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
#include "playbackuiactions.h"

#include "ui/view/iconcodes.h"

using namespace mu::playback;
using namespace mu::ui;

const UiActionList PlaybackUiActions::m_mainActions = {
    UiAction("play",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Play"),
             QT_TRANSLATE_NOOP("action", "Start or stop playback"),
             IconCode::Code::PLAY
             ),
    UiAction("rewind",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Rewind"),
             QT_TRANSLATE_NOOP("action", "Rewind to start position"),
             IconCode::Code::REWIND
             ),
    UiAction("loop",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Loop Playback"),
             QT_TRANSLATE_NOOP("action", "Toggle 'Loop Playback'"),
             IconCode::Code::LOOP
             ),
    UiAction("metronome",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Metronome"),
             QT_TRANSLATE_NOOP("action", "Play metronome during playback"),
             IconCode::Code::METRONOME
             )
};

const UiActionList PlaybackUiActions::m_settingsActions = {
    UiAction("midi-on",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "MIDI Input"),
             QT_TRANSLATE_NOOP("action", "Enable 'MIDI Input'"),
             IconCode::Code::MIDI_INPUT
             ),
    UiAction("repeat",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Play Repeats"),
             QT_TRANSLATE_NOOP("action", "Play repeats"),
             IconCode::Code::PLAY_REPEATS
             ),
    UiAction("pan",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Pan Score"),
             QT_TRANSLATE_NOOP("action", "Pan score automatically"),
             IconCode::Code::PAN_SCORE
             ),
    UiAction("countin",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Count-In"),
             QT_TRANSLATE_NOOP("action", "Enable count-in when playing"),
             IconCode::Code::COUNT_IN
             ),
    UiAction("loop-in",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Loop In"),
             QT_TRANSLATE_NOOP("action", "Set loop marker left"),
             IconCode::Code::LOOP_IN
             ),
    UiAction("loop-out",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Loop Out"),
             QT_TRANSLATE_NOOP("action", "Set loop marker right"),
             IconCode::Code::LOOP_OUT
             ),
};

const UiActionList PlaybackUiActions::m_loopBoundaryActions = {
    UiAction("loop-in",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Loop In"),
             QT_TRANSLATE_NOOP("action", "Set loop marker left"),
             IconCode::Code::LOOP_IN
             ),
    UiAction("loop-out",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Loop Out"),
             QT_TRANSLATE_NOOP("action", "Set loop marker right"),
             IconCode::Code::LOOP_OUT
             ),
};

const UiActionList& PlaybackUiActions::settingsActions()
{
    return m_settingsActions;
}

const UiActionList& PlaybackUiActions::loopBoundaryActions()
{
    return m_loopBoundaryActions;
}

PlaybackUiActions::PlaybackUiActions(std::shared_ptr<PlaybackController> controller)
    : m_controller(controller)
{
}

const UiActionList& PlaybackUiActions::actionsList() const
{
    static UiActionList alist;
    if (alist.empty()) {
        alist.insert(alist.end(), m_mainActions.cbegin(), m_mainActions.cend());
        alist.insert(alist.end(), m_settingsActions.cbegin(), m_settingsActions.cend());
        alist.insert(alist.end(), m_loopBoundaryActions.cbegin(), m_loopBoundaryActions.cend());
    }
    return alist;
}

bool PlaybackUiActions::actionEnabled(const UiAction& act) const
{
    if (!m_controller->canReceiveAction(act.code)) {
        return false;
    }

    return true;
}

bool PlaybackUiActions::actionChecked(const UiAction&) const
{
    return false;
}

mu::async::Channel<mu::actions::ActionCodeList> PlaybackUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

mu::async::Channel<mu::actions::ActionCodeList> PlaybackUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}
