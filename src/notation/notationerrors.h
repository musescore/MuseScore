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
#ifndef MU_NOTATION_NOTATIONERRORS_H
#define MU_NOTATION_NOTATIONERRORS_H

#include "types/ret.h"
#include "translation.h"

namespace mu::notation {
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::NotationFirst),

    // selection
    NoteIsNotSelected = 1050,
    NoteOrRestIsNotSelected,
    NoteOrFiguredBassIsNotSelected,
    MeasureIsNotSelected,
    SelectCompleteTupletOrTremolo,
    EmptySelection,
};

inline Ret make_ret(Err err)
{
    std::string text;

    switch (err) {
    case Err::UnknownError:
        text = trc("notation", "Unknown error");
        break;
    case Err::NoteIsNotSelected:
        text = trc("notation", "No note selected")
               + "\n" + trc("notation", "Please select a note and retry");
        break;
    case Err::NoteOrRestIsNotSelected:
        text = trc("notation", "No note or rest selected")
               + "\n" + trc("notation", "Please select a note or rest and retry");
        break;
    case Err::NoteOrFiguredBassIsNotSelected:
        text = trc("notation", "No note or figured bass selected")
               + "\n" + trc("notation", "Please select a note or figured bass and retry");
        break;
    case Err::MeasureIsNotSelected:
        text = trc("notation", "No measure selected")
               + "\n" + trc("notation", "Please select a measure and retry");
        break;
    case Err::SelectCompleteTupletOrTremolo:
        text = trc("notation", "Please select the complete tuplet or tremolo and retry");
        break;
    case Err::EmptySelection:
        text = trc("notation", "The selection is empty");
        break;
    case Err::Undefined:
    case Err::NoError:
        break;
    }

    return mu::Ret(static_cast<int>(err), text);
}
}

#endif // MU_NOTATION_NOTATIONERRORS_H
