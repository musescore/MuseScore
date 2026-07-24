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

OctaveDot::OctaveDot(EngravingItem* s)
    : EngravingItem(ElementType::OCTAVE_DOT, s)
{
    setSelectable(false);
    m_len = 0.;
    m_above = false;
}

OctaveDot::~OctaveDot()
{
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void OctaveDot::spatiumChanged(double oldValue, double newValue)
{
    m_len = (m_len / oldValue) * newValue;
}

//---------------------------------------------------------
//   note
//---------------------------------------------------------

Note* OctaveDot::note() const
{
    auto* parent = explicitParent();
    if (parent && parent->isNote()) {
        return toNote(parent);
    } else {
        return nullptr;
    }
}
}
