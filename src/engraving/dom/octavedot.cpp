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

#include "octavedot.h"

#include "chord.h"
#include "measure.h"
#include "note.h"
#include "system.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   OctaveDot
//---------------------------------------------------------

OctaveDot::OctaveDot(DummyParentOr<Note> parent)
    : EngravingItem(ElementType::OCTAVE_DOT, parent)
{
    setSelectable(false);
}

//---------------------------------------------------------
//   note
//---------------------------------------------------------

Note* OctaveDot::note() const
{
    auto* parent = ownershipParent();
    if (parent && parent->isNote()) {
        return toNote(parent);
    } else {
        return nullptr;
    }
}
}
