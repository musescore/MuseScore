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
#ifndef MU_NOTATION_NOTATIONNOTEINPUT_H
#define MU_NOTATION_NOTATIONNOTEINPUT_H

#include "../inotationnoteinput.h"
#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "inotationconfiguration.h"
#include "igetscore.h"
#include "inotationinteraction.h"
#include "inotationundostack.h"

#include "draw/types/geometry.h"

namespace mu::engraving {
class Score;
}

namespace mu::notation {
class ScoreCallbacks;
class NotationNoteInput : public INotationNoteInput, public async::Asyncable
{
    INJECT(notation, INotationConfiguration, configuration)

public:
    NotationNoteInput(const IGetScore* getScore, INotationInteraction* interaction, INotationUndoStackPtr undoStack);
    ~NotationNoteInput() override;

    bool isNoteInputMode() const override;

    NoteInputState state() const override;

    void startNoteInput() override;
    void endNoteInput() override;
    void toggleNoteInputMethod(NoteInputMethod method) override;
    void addNote(NoteName noteName, NoteAddingMode addingMode) override;
    void padNote(const Pad& pad) override;
    Ret putNote(const PointF& pos, bool replace, bool insert) override;
    void removeNote(const PointF& pos) override;
    async::Notification noteInputStarted() const override;
    async::Notification noteInputEnded() const override;

    void addTuplet(const TupletOptions& options) override;

    void addSlur(mu::engraving::Slur* slur) override;
    void resetSlur() override;
    void addTie() override;

    void doubleNoteInputDuration() override;
    void halveNoteInputDuration() override;

    void setAccidental(AccidentalType accidentalType) override;
    void setArticulation(SymbolId articulationSymbolId) override;
    void setDrumNote(int note) override;
    void setCurrentVoice(voice_idx_t voiceIndex) override;
    void setCurrentTrack(track_idx_t trackIndex) override;

    void resetInputPosition() override;

    RectF cursorRect() const override;

    async::Notification noteAdded() const override;
    async::Notification stateChanged() const override;

    void setGetViewRectFunc(const std::function<RectF()>& func);

private:
    mu::engraving::Score* score() const;

    EngravingItem* resolveNoteInputStartPosition() const;

    void startEdit();
    void apply();

    void updateInputState();
    void notifyAboutStateChanged();
    void notifyNoteAddedChanged();
    void notifyAboutNoteInputStarted();
    void notifyAboutNoteInputEnded();

    std::set<SymbolId> articulationIds() const;

    const IGetScore* m_getScore = nullptr;
    INotationInteraction* m_interaction = nullptr;
    INotationUndoStackPtr m_undoStack;

    async::Notification m_stateChanged;
    async::Notification m_noteAdded;
    async::Notification m_noteInputStarted;
    async::Notification m_noteInputEnded;

    ScoreCallbacks* m_scoreCallbacks = nullptr;
    std::function<RectF()> m_getViewRectFunc;
};
}

#endif // MU_NOTATION_NOTATIONNOTEINPUT_H
