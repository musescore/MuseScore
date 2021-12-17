/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "playbackuiactions.h"

#include "ui/view/iconcodes.h"

using namespace mu::playback;
using namespace mu::ui;
using namespace mu::actions;

const UiActionList PlaybackUiActions::m_mainActions = {
    UiAction("play",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Play"),
             QT_TRANSLATE_NOOP("action", "Start or stop playback"),
             IconCode::Code::PLAY
             ),
    UiAction("stop",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Stop"),
             QT_TRANSLATE_NOOP("action", "Stop playback"),
             IconCode::Code::STOP
             ),
    UiAction("rewind",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Rewind"),
             QT_TRANSLATE_NOOP("action", "Rewind to start position"),
             IconCode::Code::REWIND
             ),
    UiAction("loop",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Loop playback"),
             QT_TRANSLATE_NOOP("action", "Toggle 'Loop playback'"),
             IconCode::Code::LOOP,
             Checkable::Yes
             ),
    UiAction("metronome",
             mu::context::UiCtxNotationOpened,
             QT_TRANSLATE_NOOP("action", "Metronome"),
             QT_TRANSLATE_NOOP("action", "Play metronome during playback"),
             IconCode::Code::METRONOME,
             Checkable::Yes
             )
};

const UiActionList PlaybackUiActions::m_settingsActions = {
    UiAction("midi-on",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "MIDI input"),
             QT_TRANSLATE_NOOP("action", "Enable 'MIDI input'"),
             IconCode::Code::MIDI_INPUT,
             Checkable::Yes
             ),
    UiAction("repeat",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Play repeats"),
             QT_TRANSLATE_NOOP("action", "Play repeats"),
             IconCode::Code::PLAY_REPEATS,
             Checkable::Yes
             ),
    UiAction("pan",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Pan score"),
             QT_TRANSLATE_NOOP("action", "Pan score automatically"),
             IconCode::Code::PAN_SCORE,
             Checkable::Yes
             ),
    UiAction("countin",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Count-in"),
             QT_TRANSLATE_NOOP("action", "Enable count-in when playing"),
             IconCode::Code::COUNT_IN,
             Checkable::Yes
             ),
};

const UiActionList PlaybackUiActions::m_loopBoundaryActions = {
    UiAction("loop-in",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Loop in"),
             QT_TRANSLATE_NOOP("action", "Set loop marker left"),
             IconCode::Code::LOOP_IN
             ),
    UiAction("loop-out",
             mu::context::UiCtxAny,
             QT_TRANSLATE_NOOP("action", "Loop out"),
             QT_TRANSLATE_NOOP("action", "Set loop marker right"),
             IconCode::Code::LOOP_OUT
             ),
};

PlaybackUiActions::PlaybackUiActions(std::shared_ptr<PlaybackController> controller)
    : m_controller(controller)
{
}

void PlaybackUiActions::init()
{
    m_controller->actionCheckedChanged().onReceive(this, [this](const ActionCode& code) {
        m_actionCheckedChanged.send({ code });
    });
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
    if (!m_controller->isPlayAllowed()) {
        return false;
    }

    if (!m_controller->canReceiveAction(act.code)) {
        return false;
    }

    return true;
}

bool PlaybackUiActions::actionChecked(const UiAction& act) const
{
    return m_controller->actionChecked(act.code);
}

mu::async::Channel<mu::actions::ActionCodeList> PlaybackUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

mu::async::Channel<mu::actions::ActionCodeList> PlaybackUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}

const UiActionList& PlaybackUiActions::settingsActions()
{
    return m_settingsActions;
}

const UiActionList& PlaybackUiActions::loopBoundaryActions()
{
    return m_loopBoundaryActions;
}

const mu::ui::ToolConfig& PlaybackUiActions::defaultPlaybackToolConfig()
{
    static ToolConfig config;
    if (!config.isValid()) {
        config.items = {
            { "rewind", true },
            { "play", true },
            { "loop", true },
            { "loop-in", true },
            { "loop-out", true },
            { "metronome", true },
        };
    }
    return config;
}
