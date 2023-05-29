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
class NotationMidiInput : public INotationMidiInput
{
    INJECT(playback::IPlaybackController, playbackController)
    INJECT(actions::IActionsDispatcher, dispatcher)
    INJECT(INotationConfiguration, configuration)

public:
    NotationMidiInput(IGetScore* getScore, INotationInteractionPtr notationInteraction, INotationUndoStackPtr undoStack);

    void onMidiEventReceived(const midi::Event& event) override;
    async::Channel<std::vector<const Note*> > notesReceived() const override;

    void onRealtimeAdvance() override;

private:
    mu::engraving::Score* score() const;

    void doProcessEvents();
    Note* addNoteToScore(const midi::Event& e);
    Note* makeNote(const midi::Event& e);

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

    bool isSoundPreview() const;

    void playNoteOn(const std::vector<const Note*>& notes);
    void playNoteOff(const std::vector<const Note*>& notes);

    IGetScore* m_getScore = nullptr;
    INotationInteractionPtr m_notationInteraction;
    INotationUndoStackPtr m_undoStack;
    async::Channel<std::vector<const Note*> > m_notesReceivedChannel;

    QTimer m_processTimer;
    std::vector<midi::Event> m_eventsQueue;

    QTimer m_realtimeTimer;
    QTimer m_extendNoteTimer;
    bool m_allowRealtimeRests = false;

    bool m_shouldDisableMetronome = false;
};
}

#endif // MU_NOTATION_NOTATIONMIDIINPUT_H
