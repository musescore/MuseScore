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
#ifndef MU_NOTATION_NOTATIONMIDIINPUT_H
#define MU_NOTATION_NOTATIONMIDIINPUT_H

#include <QTimer>

#include "modularity/ioc.h"
#include "playback/iplaybackcontroller.h"
#include "inotationconfiguration.h"
#include "actions/iactionsdispatcher.h"

#include "../inotationmidiinput.h"
#include "igetscore.h"
#include "inotationinteraction.h"
#include "inotationundostack.h"

namespace mu::engraving {
class Score;
}

namespace mu::notation {
class NotationMidiInput : public INotationMidiInput, public muse::Injectable
{
    muse::Inject<playback::IPlaybackController> playbackController = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::Inject<INotationConfiguration> configuration = { this };

public:
    NotationMidiInput(IGetScore* getScore, INotationInteractionPtr notationInteraction, INotationUndoStackPtr undoStack,
                      const muse::modularity::ContextPtr& iocCtx);

    void onMidiEventReceived(const muse::midi::Event& event) override;
    muse::async::Channel<std::vector<const Note*> > notesReceived() const override;

    void onRealtimeAdvance() override;

private:
    mu::engraving::Score* score() const;

    void doProcessEvents();
    Note* addNoteToScore(const muse::midi::Event& e);
    Note* makeNote(const muse::midi::Event& e);

    void enableMetronome();
    void disableMetronome();

    void runRealtime();
    void stopRealtime();

    void doRealtimeAdvance();
    void doExtendCurrentNote();

    NoteInputMethod noteInputMethod() const;
    bool isRealtime() const;
    bool isRealtimeAuto() const;
    bool isRealtimeManual() const;

    bool isNoteInputMode() const;

    IGetScore* m_getScore = nullptr;
    INotationInteractionPtr m_notationInteraction;
    INotationUndoStackPtr m_undoStack;
    muse::async::Channel<std::vector<const Note*> > m_notesReceivedChannel;

    QTimer m_processTimer;
    std::vector<muse::midi::Event> m_eventsQueue;

    QTimer m_realtimeTimer;
    QTimer m_extendNoteTimer;
    bool m_allowRealtimeRests = false;

    bool m_shouldDisableMetronome = false;
};
}

#endif // MU_NOTATION_NOTATIONMIDIINPUT_H
