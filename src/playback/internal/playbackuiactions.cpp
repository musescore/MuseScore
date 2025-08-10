/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "context/uicontext.h"
#include "context/shortcutcontext.h"
#include "types/translatablestring.h"

using namespace mu::playback;
using namespace muse;
using namespace muse::ui;
using namespace muse::actions;

const UiActionList PlaybackUiActions::m_mainActions = {
    UiAction("play",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Play"),
             TranslatableString("action", "Play"),
             IconCode::Code::PLAY
             ),
    UiAction("stop",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Stop"),
             TranslatableString("action", "Stop playback"),
             IconCode::Code::STOP
             ),
    UiAction("pause-and-select",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Pause and select"),
             TranslatableString("action", "Pause and select playback position"),
             IconCode::Code::PAUSE
             ),
    UiAction("rewind",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Rewind"),
             TranslatableString("action", "Rewind"),
             IconCode::Code::REWIND
             ),
    UiAction("loop",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Loop playback"),
             TranslatableString("action", "Toggle ‘Loop playback’"),
             IconCode::Code::LOOP,
             Checkable::Yes
             ),
    UiAction("metronome",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Metronome"),
             TranslatableString("action", "Toggle metronome playback"),
             IconCode::Code::METRONOME,
             Checkable::Yes
             ),
    UiAction("playback-setup",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Playback setup"),
             TranslatableString("action", "Open playback setup dialog"),
             IconCode::Code::NONE
             )
};

const UiActionList PlaybackUiActions::m_midiInputActions = {
    UiAction("midi-on",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Enable MIDI input"),
             TranslatableString("action", "Toggle MIDI input"),
             IconCode::Code::MIDI_INPUT,
             Checkable::Yes
             ),
};

const UiActionList PlaybackUiActions::m_midiInputPitchActions = {
    UiAction("midi-input-written-pitch",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Written pitch"),
             TranslatableString("action", "Input written pitch"),
             IconCode::Code::NONE,
             Checkable::Yes
             ),
    UiAction("midi-input-sounding-pitch",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Sounding pitch"),
             TranslatableString("action", "Input sounding pitch"),
             IconCode::Code::NONE,
             Checkable::Yes
             ),
};

const UiActionList PlaybackUiActions::m_settingsActions = {
    UiAction("repeat",
             mu::context::UiCtxAny,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Play repeats"),
             TranslatableString("action", "Play repeats"),
             IconCode::Code::PLAY_REPEATS,
             Checkable::Yes
             ),
    UiAction("play-chord-symbols",
             mu::context::UiCtxAny,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Play chord symbols"),
             TranslatableString("action", "Play chord symbols"),
             IconCode::Code::CHORD_SYMBOL,
             Checkable::Yes
             ),
    UiAction("toggle-hear-playback-when-editing",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Hear playback when editing"),
             TranslatableString("action", "Toggle hear playback when editing"),
             IconCode::Code::AUDIO,
             Checkable::Yes
             ),
    UiAction("pan",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Pan score automatically"),
             TranslatableString("action", "Pan score automatically during playback"),
             IconCode::Code::PAN_SCORE,
             Checkable::Yes
             ),
    UiAction("countin",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Enable count-in when playing"),
             TranslatableString("action", "Enable count-in when playing"),
             IconCode::Code::COUNT_IN,
             Checkable::Yes
             ),
};

const UiActionList PlaybackUiActions::m_loopBoundaryActions = {
    UiAction("loop-in",
             mu::context::UiCtxAny,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Set loop marker left"),
             TranslatableString("action", "Set loop marker left"),
             IconCode::Code::LOOP_IN
             ),
    UiAction("loop-out",
             mu::context::UiCtxAny,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Set loop marker right"),
             TranslatableString("action", "Set loop marker right"),
             IconCode::Code::LOOP_OUT
             ),
};

const UiActionList PlaybackUiActions::m_diagnosticActions = {
    UiAction("playback-reload-cache",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Reload playback cache")
             )
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

    m_controller->isPlayAllowedChanged().onNotify(this, [this]() {
        ActionCodeList codes;

        for (const UiAction& action : actionsList()) {
            codes.push_back(action.code);
        }

        m_actionEnabledChanged.send(codes);
    });
}

const UiActionList& PlaybackUiActions::actionsList() const
{
    static UiActionList alist;
    if (alist.empty()) {
        alist.insert(alist.end(), m_mainActions.cbegin(), m_mainActions.cend());
        alist.insert(alist.end(), m_midiInputActions.cbegin(), m_midiInputActions.cend());
        alist.insert(alist.end(), m_midiInputPitchActions.cbegin(), m_midiInputPitchActions.cend());
        alist.insert(alist.end(), m_settingsActions.cbegin(), m_settingsActions.cend());
        alist.insert(alist.end(), m_loopBoundaryActions.cbegin(), m_loopBoundaryActions.cend());
        alist.insert(alist.end(), m_diagnosticActions.cbegin(), m_diagnosticActions.cend());
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

muse::async::Channel<ActionCodeList> PlaybackUiActions::actionEnabledChanged() const
{
    return m_actionEnabledChanged;
}

muse::async::Channel<ActionCodeList> PlaybackUiActions::actionCheckedChanged() const
{
    return m_actionCheckedChanged;
}

const UiActionList& PlaybackUiActions::midiInputActions()
{
    return m_midiInputActions;
}

const UiActionList& PlaybackUiActions::midiInputPitchActions()
{
    return m_midiInputPitchActions;
}

const UiActionList& PlaybackUiActions::settingsActions()
{
    return m_settingsActions;
}

const UiActionList& PlaybackUiActions::loopBoundaryActions()
{
    return m_loopBoundaryActions;
}

const muse::ui::ToolConfig& PlaybackUiActions::defaultPlaybackToolConfig()
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
