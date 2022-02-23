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

#include "engraving/infrastructure/draw/geometry.h"

namespace Ms {
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
    void putNote(const PointF& pos, bool replace, bool insert) override;
    void removeNote(const PointF& pos) override;
    void setAccidental(AccidentalType accidentalType) override;
    void setArticulation(SymbolId articulationSymbolId) override;
    void setDrumNote(int note) override;
    void addTuplet(const TupletOptions& options) override;

    void addSlur(Ms::Slur* slur) override;
    void resetSlur() override;
    void addTie() override;

    void doubleNoteInputDuration() override;
    void halveNoteInputDuration() override;

    void setCurrentVoiceIndex(int voiceIndex) override;

    void resetInputPosition() override;

    RectF cursorRect() const override;

    async::Notification noteAdded() const override;
    async::Notification stateChanged() const override;

private:
    Ms::Score* score() const;

    Ms::EngravingItem* resolveNoteInputStartPosition() const;

    void startEdit();
    void apply();

    void updateInputState();
    void notifyAboutStateChanged();
    void notifyNoteAddedChanged();

    std::set<SymbolId> articulationIds() const;

    const IGetScore* m_getScore = nullptr;
    INotationInteraction* m_interaction = nullptr;
    INotationUndoStackPtr m_undoStack;

    async::Notification m_stateChanged;
    async::Notification m_noteAdded;

    ScoreCallbacks* m_scoreCallbacks = nullptr;
};
}

#endif // MU_NOTATION_NOTATIONNOTEINPUT_H
