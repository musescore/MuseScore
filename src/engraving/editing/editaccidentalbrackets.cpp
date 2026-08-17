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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "editaccidentalbrackets.h"

#include "../dom/accidental.h"
#include "../dom/score.h"
#include "../dom/select.h"

using namespace mu::engraving;

void EditAccidentalBrackets::addBracket(Transaction&, Score* score)
{
    for (EngravingItem* el : score->selection().elements()) {
        if (el->isAccidental()) {
            Accidental* acc = toAccidental(el);
            acc->undoChangeProperty(Pid::ACCIDENTAL_BRACKET, int(AccidentalBracket::BRACKET));
        }
    }
}

void EditAccidentalBrackets::addBraces(Transaction&, Score* score)
{
    for (EngravingItem* el : score->selection().elements()) {
        if (el->isAccidental()) {
            Accidental* acc = toAccidental(el);
            acc->undoChangeProperty(Pid::ACCIDENTAL_BRACKET, int(AccidentalBracket::BRACE));
        }
    }
}
