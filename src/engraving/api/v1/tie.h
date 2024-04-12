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

#ifndef MU_ENGRAVING_APIV1_TIE_H
#define MU_ENGRAVING_APIV1_TIE_H

#include "engraving/dom/tie.h"

// api
#include "elements.h"

namespace mu::engraving::apiv1 {
//---------------------------------------------------------
//   Tie
///  Provides access to internal mu::engraving::Tie objects.
///  \since MuseScore 3.3
//---------------------------------------------------------

class Tie : public EngravingItem
{
    Q_OBJECT
    /// The starting note of the tie.
    /// \since MuseScore 3.3
    Q_PROPERTY(apiv1::Note * startNote READ startNote)
    /// The ending note of the tie.
    /// \since MuseScore 3.3
    Q_PROPERTY(apiv1::Note * endNote READ endNote)

    /// \cond MS_INTERNAL

public:
    Tie(mu::engraving::Tie* tie, Ownership own = Ownership::PLUGIN)
        : EngravingItem(tie, own) {}

    Note* startNote();
    Note* endNote();

    /// \endcond
};

extern Tie* tieWrap(mu::engraving::Tie* tie);
}

#endif
