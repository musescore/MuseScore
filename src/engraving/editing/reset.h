/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "../style/styledef.h"

namespace mu::engraving {
class Score;
class Transaction;

class Reset
{
public:
    static void resetAllPositions(Transaction& tx, Score* score);
    static void resetToDefaultLayout(Transaction& tx, Score* score);
    static void resetTextStyleOverrides(Score* score); //! Not undoable
    static void resetAllStyles(Transaction& tx, Score* score, const StyleIdSet& exceptTheseOnes = {});

    // Removes system/page breaks and spacers
    static void resetMeasuresLayout(Transaction& tx, Score* score);
};
}
