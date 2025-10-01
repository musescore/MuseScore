/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "editbrackets.h"

#include "../dom/staff.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   AddBracket
//---------------------------------------------------------

void AddBracket::redo(EditData*)
{
    staff->setBracketType(level, bracketType);
    staff->setBracketSpan(level, span);
    staff->triggerLayout();
}

void AddBracket::undo(EditData*)
{
    staff->setBracketType(level, BracketType::NO_BRACKET);
    staff->triggerLayout();
}

//---------------------------------------------------------
//   RemoveBracket
//---------------------------------------------------------

void RemoveBracket::redo(EditData*)
{
    staff->setBracketType(level, BracketType::NO_BRACKET);
    staff->triggerLayout();
}

void RemoveBracket::undo(EditData*)
{
    staff->setBracketType(level, bracketType);
    staff->setBracketSpan(level, span);
    staff->triggerLayout();
}
