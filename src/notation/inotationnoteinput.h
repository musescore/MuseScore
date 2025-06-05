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
#ifndef MU_NOTATION_INOTATIONNOTEINPUT_H
#define MU_NOTATION_INOTATIONNOTEINPUT_H

#include "async/notification.h"
#include "types/ret.h"

#include "notationtypes.h"

namespace mu::notation {
class INotationNoteInput
{
public:
    virtual ~INotationNoteInput() = default;

    virtual bool isNoteInputMode() const = 0;

    virtual const NoteInputState& state() const = 0;

    virtual void startNoteInput(NoteInputMethod method = NoteInputMethod::BY_NOTE_NAME, bool focusNotation = true) = 0;
    virtual void endNoteInput(bool resetState = false) = 0;

    virtual muse::async::Channel</*focusNotation*/ bool> noteInputStarted() const = 0;
    virtual muse::async::Notification noteInputEnded() const = 0;

    virtual bool usingNoteInputMethod(NoteInputMethod method) const = 0;
    virtual void setNoteInputMethod(NoteInputMethod method) = 0;

    virtual void addNote(const NoteInputParams& params, NoteAddingMode addingMode) = 0;
    virtual void padNote(const Pad& pad)  = 0;
    virtual muse::Ret putNote(const muse::PointF& pos, bool replace, bool insert) = 0;
    virtual void removeNote(const muse::PointF& pos) = 0;

    virtual void addTuplet(const TupletOptions& options) = 0;

    virtual void doubleNoteInputDuration() = 0;
    virtual void halveNoteInputDuration() = 0;

    virtual void addSlur(mu::engraving::Slur* slur) = 0;
    virtual void resetSlur() = 0;

    virtual void addTie() = 0;

    virtual void addLaissezVib() = 0;

    // Used in the input-by-duration mode
    virtual void setInputNote(const NoteInputParams& params) = 0;
    virtual void setInputNotes(const NoteValList& notes) = 0;
    virtual void moveInputNotes(bool up, PitchMode mode) = 0;

    virtual void setRestMode(bool rest) = 0;
    virtual void setAccidental(AccidentalType accidentalType) = 0;
    virtual void setArticulation(SymbolId articulationSymbolId) = 0;
    virtual void setDrumNote(int note) = 0;
    virtual void setCurrentVoice(voice_idx_t voiceIndex) = 0;
    virtual void setCurrentTrack(track_idx_t trackIndex) = 0;

    virtual muse::RectF cursorRect() const = 0;

    virtual muse::async::Notification noteAdded() const = 0;
    virtual muse::async::Notification stateChanged() const = 0;
};

using INotationNoteInputPtr = std::shared_ptr<INotationNoteInput>;
}

#endif // MU_NOTATION_INOTATIONNOTEINPUT_H
