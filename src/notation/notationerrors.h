/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
    SelectCompleteTupletOrTremolo = 1050,
    EmptySelection,

    // note input
    NoteInputRemovesMeasureRepeat = 1060,

    // measure repeats
    MeasureRepeatReplacesContents = 1070,
};

inline muse::Ret make_ret(Err err)
{
    std::string text;

    switch (err) {
    case Err::UnknownError:
        text = muse::trc("notation", "Unknown error");
        break;
    case Err::SelectCompleteTupletOrTremolo:
        text = muse::trc("notation", "Please select the complete tuplet or tremolo and retry");
        break;
    case Err::EmptySelection:
        text = muse::trc("notation", "The selection is empty");
        break;
    case Err::NoteInputRemovesMeasureRepeat:
        text = muse::trc("engraving", "Note input will remove measure repeat");
        text += "\n\n";
        text += muse::trc("engraving", "This measure contains a measure repeat."
                                       " If you enter notes here, it will be deleted."
                                       " Do you want to continue?");
        break;
    case Err::MeasureRepeatReplacesContents:
        text = muse::trc("engraving", "Current contents of measures will be replaced");
        text += "\n\n";
        text += muse::trc("engraving", "Continue with inserting measure repeat?");
        break;
    case Err::Undefined:
    case Err::NoError:
        break;
    }

    return muse::Ret(static_cast<int>(err), text);
}
}

#endif // MU_NOTATION_NOTATIONERRORS_H
