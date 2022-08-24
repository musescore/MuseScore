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

#include "playback/iplaybackcontroller.h"

#include <QTimer>

#include "../inotationmidiinput.h"
#include "igetscore.h"
#include "inotationundostack.h"
#include "inotationinteraction.h"

namespace mu::engraving {
class Score;
}

namespace mu::notation {
class NotationMidiInput : public INotationMidiInput
{
    INJECT(notation, playback::IPlaybackController, playbackController)

public:
    NotationMidiInput(IGetScore* getScore, INotationInteractionPtr notationInteraction, INotationUndoStackPtr undoStack);

    void onMidiEventsReceived(const std::vector<midi::Event>& events) override;
    async::Notification noteChanged() const override;

private:
    mu::engraving::Score* score() const;

    void doPlayNotes();
    Note* onAddNote(const midi::Event& e);

    IGetScore* m_getScore = nullptr;
    INotationInteractionPtr m_notationInteraction;
    INotationUndoStackPtr m_undoStack;
    async::Notification m_noteChanged;

    QTimer m_playTimer;
    std::vector<const EngravingItem*> m_playNotesQueue;
};
}

#endif // MU_NOTATION_NOTATIONMIDIINPUT_H
