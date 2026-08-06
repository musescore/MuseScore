/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "engraving/dom/input.h"
#include "engraving/dom/noteval.h"
#include "engraving/dom/tuplet.h"
#include "engraving/editing/editnote.h"
#include "engraving/editing/noteinput.h"

namespace mu::notation {
using NoteInputState = mu::engraving::InputState;
using NoteInputMethod = mu::engraving::NoteEntryMethod;
using NoteInputParams = mu::engraving::NoteInputParams;
using PitchMode = mu::engraving::UpDownMode;

using NoteVal = mu::engraving::NoteVal;
using NoteValList = mu::engraving::NoteValList;

enum class NoteAddingMode : unsigned char
{
    CurrentChord,
    NextChord,
    InsertChord
};

static const std::map<std::string, NoteAddingMode> STR_NOTE_ADDING_MODE = {
    { "current", NoteAddingMode::CurrentChord },
    { "next", NoteAddingMode::NextChord },
    { "insert", NoteAddingMode::InsertChord },
};

inline NoteAddingMode str_conv(const std::string& mode, NoteAddingMode def)
{
    return muse::value(STR_NOTE_ADDING_MODE, mode, def);
}

inline std::string str_conv(NoteAddingMode mode)
{
    return muse::key(STR_NOTE_ADDING_MODE, mode);
}

struct TupletOptions
{
    engraving::Fraction ratio = { -1, -1 };
    engraving::TupletNumberType numberType = engraving::TupletNumberType::SHOW_NUMBER;
    engraving::TupletBracketType bracketType = engraving::TupletBracketType::AUTO_BRACKET;
    bool autoBaseLen = false;
};
}
