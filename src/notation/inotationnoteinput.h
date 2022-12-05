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
#ifndef MU_NOTATION_INOTATIONNOTEINPUT_H
#define MU_NOTATION_INOTATIONNOTEINPUT_H

#include "async/notification.h"
#include "notationtypes.h"
#include "types/retval.h"

namespace mu::notation {
class INotationNoteInput
{
public:
    virtual ~INotationNoteInput() = default;

    virtual bool isNoteInputMode() const = 0;

    virtual NoteInputState state() const = 0;

    virtual void startNoteInput() = 0;
    virtual void endNoteInput() = 0;
    virtual void toggleNoteInputMethod(NoteInputMethod method) = 0;
    virtual void addNote(NoteName noteName, NoteAddingMode addingMode) = 0;
    virtual void padNote(const Pad& pad)  = 0;
    virtual Ret putNote(const PointF& pos, bool replace, bool insert) = 0;
    virtual void removeNote(const PointF& pos) = 0;
    virtual async::Notification noteInputStarted() const = 0;
    virtual async::Notification noteInputEnded() const = 0;

    virtual void addTuplet(const TupletOptions& options) = 0;

    virtual void doubleNoteInputDuration() = 0;
    virtual void halveNoteInputDuration() = 0;

    virtual void addSlur(mu::engraving::Slur* slur) = 0;
    virtual void resetSlur() = 0;

    virtual void addTie() = 0;

    virtual void setAccidental(AccidentalType accidentalType) = 0;
    virtual void setArticulation(SymbolId articulationSymbolId) = 0;
    virtual void setDrumNote(int note) = 0;
    virtual void setCurrentVoice(voice_idx_t voiceIndex) = 0;
    virtual void setCurrentTrack(track_idx_t trackIndex) = 0;

    virtual void resetInputPosition() = 0;

    virtual RectF cursorRect() const = 0;

    virtual async::Notification noteAdded() const = 0;
    virtual async::Notification stateChanged() const = 0;
};

using INotationNoteInputPtr = std::shared_ptr<INotationNoteInput>;
}

#endif // MU_NOTATION_INOTATIONNOTEINPUT_H
