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
using namespace mu::notation;
using namespace muse;
using namespace muse::ui;
using namespace muse::actions;

static const ActionCode PLAY_FROM_SELECTION_CODE("play-from-selection");
static const ActionCode CLEAR_ONLINE_SOUNDS_CACHE_CODE("clear-online-sounds-cache");

const UiActionList PlaybackUiActions::s_mainActions = {
    UiAction("play",
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_FOCUSED,
             TranslatableString("action", "Play"),
             TranslatableString("action", "Play"),
             IconCode::Code::PLAY
             ),
    UiAction(PLAY_FROM_SELECTION_CODE,
             mu::context::UiCtxProjectOpened,
             mu::context::CTX_NOTATION_OPENED,
             TranslatableString("action", "Play from selection"),
             TranslatableString("action", "Play from selection"),
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

const UiActionList PlaybackUiActions::s_midiInputActions = {
    UiAction("midi-on",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Enable MIDI input"),
             TranslatableString("action", "Toggle MIDI input"),
             IconCode::Code::MIDI_INPUT,
             Checkable::Yes
             ),
};

const UiActionList PlaybackUiActions::s_midiInputPitchActions = {
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

const UiActionList PlaybackUiActions::s_settingsActions = {
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

const UiActionList PlaybackUiActions::s_loopBoundaryActions = {
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

const UiActionList PlaybackUiActions::s_diagnosticActions = {
    UiAction("playback-reload-cache",
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Reload playback cache")
             )
};

const UiActionList PlaybackUiActions::s_onlineSoundsActions = {
    UiAction(CLEAR_ONLINE_SOUNDS_CACHE_CODE,
             mu::context::UiCtxAny,
             mu::context::CTX_ANY,
             TranslatableString("action", "Clear online sounds cache for this score")
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

    m_controller->isPlayAllowedChanged().onNotify(this, [this]() {
        const UiActionList& actions = actionsList();

        ActionCodeList codes;
        codes.reserve(actions.size());

        for (const UiAction& action : actions) {
            codes.push_back(action.code);
        }

        m_actionEnabledChanged.send(codes);
    });

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        INotationPtr currNotation = globalContext()->currentNotation();
        if (!currNotation) {
            return;
        }

        INotationInteractionPtr interaction = currNotation->interaction();
        interaction->selectionChanged().onNotify(this, [this]() {
            m_actionEnabledChanged.send({ PLAY_FROM_SELECTION_CODE });
        });

        interaction->isEditingElementChanged().onNotify(this, [this]() {
            m_actionEnabledChanged.send({ PLAY_FROM_SELECTION_CODE });
        });
    });

    m_controller->onlineSoundsChanged().onNotify(this, [this]() {
        m_actionEnabledChanged.send({ CLEAR_ONLINE_SOUNDS_CACHE_CODE });
    });
}

const UiActionList& PlaybackUiActions::actionsList() const
{
    static UiActionList alist;
    if (alist.empty()) {
        alist.insert(alist.end(), s_mainActions.cbegin(), s_mainActions.cend());
        alist.insert(alist.end(), s_midiInputActions.cbegin(), s_midiInputActions.cend());
        alist.insert(alist.end(), s_midiInputPitchActions.cbegin(), s_midiInputPitchActions.cend());
        alist.insert(alist.end(), s_settingsActions.cbegin(), s_settingsActions.cend());
        alist.insert(alist.end(), s_loopBoundaryActions.cbegin(), s_loopBoundaryActions.cend());
        alist.insert(alist.end(), s_diagnosticActions.cbegin(), s_diagnosticActions.cend());
        alist.insert(alist.end(), s_onlineSoundsActions.cbegin(), s_onlineSoundsActions.cend());
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

    if (act.code == CLEAR_ONLINE_SOUNDS_CACHE_CODE) {
        return !m_controller->onlineSounds().empty();
    }

    if (act.code == PLAY_FROM_SELECTION_CODE) {
        const INotationPtr currNotation = globalContext()->currentNotation();
        const INotationInteractionPtr interaction = currNotation ? currNotation->interaction() : nullptr;
        return interaction && !interaction->selection()->isNone() && !interaction->isEditingElement();
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
    return s_midiInputActions;
}

const UiActionList& PlaybackUiActions::midiInputPitchActions()
{
    return s_midiInputPitchActions;
}

const UiActionList& PlaybackUiActions::settingsActions()
{
    return s_settingsActions;
}

const UiActionList& PlaybackUiActions::loopBoundaryActions()
{
    return s_loopBoundaryActions;
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
