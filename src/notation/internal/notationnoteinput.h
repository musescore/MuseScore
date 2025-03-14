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
class NotationNoteInput : public INotationNoteInput, public muse::Injectable, public muse::async::Asyncable
{
    muse::Inject<INotationConfiguration> configuration = { this };

public:
    NotationNoteInput(const IGetScore* getScore, INotationInteraction* interaction, INotationUndoStackPtr undoStack,
                      const muse::modularity::ContextPtr& iocCtx);
    ~NotationNoteInput() override;

    bool isNoteInputMode() const override;

    const NoteInputState& state() const override;

    void startNoteInput(NoteInputMethod method = NoteInputMethod::BY_NOTE_NAME, bool focusNotation = true) override;
    void endNoteInput(bool resetState = false) override;

    muse::async::Channel</*focusNotation*/ bool> noteInputStarted() const override;
    muse::async::Notification noteInputEnded() const override;

    bool usingNoteInputMethod(NoteInputMethod method) const override;
    void setNoteInputMethod(NoteInputMethod method) override;

    void addNote(const NoteInputParams& params, NoteAddingMode addingMode) override;
    void padNote(const Pad& pad) override;
    muse::Ret putNote(const muse::PointF& pos, bool replace, bool insert) override;
    void removeNote(const muse::PointF& pos) override;

    void addTuplet(const TupletOptions& options) override;

    void addSlur(mu::engraving::Slur* slur) override;
    void resetSlur() override;
    void addTie() override;
    void addLaissezVib() override;

    void doubleNoteInputDuration() override;
    void halveNoteInputDuration() override;

    // Used in the input-by-duration mode
    void setInputNote(const NoteInputParams& params) override;
    void setInputNotes(const NoteValList& notes) override;
    void moveInputNotes(bool up, PitchMode mode) override;

    void setRestMode(bool rest) override;
    void setAccidental(AccidentalType accidentalType) override;
    void setArticulation(SymbolId articulationSymbolId) override;
    void setDrumNote(int note) override;
    void setCurrentVoice(voice_idx_t voiceIndex) override;
    void setCurrentTrack(track_idx_t trackIndex) override;

    muse::RectF cursorRect() const override;

    muse::async::Notification noteAdded() const override;
    muse::async::Notification stateChanged() const override;

    void setGetViewRectFunc(const std::function<muse::RectF()>& func);

private:
    mu::engraving::Score* score() const;

    EngravingItem* resolveNoteInputStartPosition() const;

    bool shouldSetupInputNote() const;
    void setupInputNote();

    NoteVal noteValForLine(int line) const;

    void startEdit(const muse::TranslatableString& actionName);
    void apply();

    void updateInputState();
    void notifyAboutStateChanged();
    void notifyNoteAddedChanged();
    void notifyAboutNoteInputStarted(bool focusNotation = true);
    void notifyAboutNoteInputEnded();

    const IGetScore* m_getScore = nullptr;
    INotationInteraction* m_interaction = nullptr;
    INotationUndoStackPtr m_undoStack;

    muse::async::Notification m_stateChanged;
    muse::async::Notification m_noteAdded;
    muse::async::Channel</*focusNotation*/ bool> m_noteInputStarted;
    muse::async::Notification m_noteInputEnded;

    ScoreCallbacks* m_scoreCallbacks = nullptr;
    std::function<muse::RectF()> m_getViewRectFunc;
};
}

#endif // MU_NOTATION_NOTATIONNOTEINPUT_H
