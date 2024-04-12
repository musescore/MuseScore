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
#ifndef MU_NOTATION_NOTATIONERRORS_H
#define MU_NOTATION_NOTATIONERRORS_H

#include "types/ret.h"
#include "translation.h"

namespace mu::notation {
enum class Err {
    Undefined       = int(muse::Ret::Code::Undefined),
    NoError         = int(muse::Ret::Code::Ok),
    UnknownError    = int(muse::Ret::Code::NotationFirst),

    // selection
    NoteIsNotSelected = 1050,
    NoteOrRestIsNotSelected,
    NoteOrFiguredBassIsNotSelected,
    MeasureIsNotSelected,
    SelectCompleteTupletOrTremolo,
    EmptySelection,
};

inline muse::Ret make_ret(Err err)
{
    std::string text;

    switch (err) {
    case Err::UnknownError:
        text = muse::trc("notation", "Unknown error");
        break;
    case Err::NoteIsNotSelected:
        text = muse::trc("notation", "No note selected")
               + "\n" + muse::trc("notation", "Please select a note and retry");
        break;
    case Err::NoteOrRestIsNotSelected:
        text = muse::trc("notation", "No note or rest selected")
               + "\n" + muse::trc("notation", "Please select a note or rest and retry");
        break;
    case Err::NoteOrFiguredBassIsNotSelected:
        text = muse::trc("notation", "No note or figured bass selected")
               + "\n" + muse::trc("notation", "Please select a note or figured bass and retry");
        break;
    case Err::MeasureIsNotSelected:
        text = muse::trc("notation", "No measure selected")
               + "\n" + muse::trc("notation", "Please select a measure and retry");
        break;
    case Err::SelectCompleteTupletOrTremolo:
        text = muse::trc("notation", "Please select the complete tuplet or tremolo and retry");
        break;
    case Err::EmptySelection:
        text = muse::trc("notation", "The selection is empty");
        break;
    case Err::Undefined:
    case Err::NoError:
        break;
    }

    return muse::Ret(static_cast<int>(err), text);
}
}

#endif // MU_NOTATION_NOTATIONERRORS_H
