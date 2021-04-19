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

#include "fifo.h"

namespace Ms {
//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void FifoBase::clear()
{
    ridx    = 0;
    widx    = 0;
    counter = 0;
}

//---------------------------------------------------------
//   push
//---------------------------------------------------------

void FifoBase::push()
{
    widx = (widx + 1) % maxCount;
//      q_atomic_increment(&counter);
    ++counter;
}

//---------------------------------------------------------
//   pop
//---------------------------------------------------------

void FifoBase::pop()
{
    ridx = (ridx + 1) % maxCount;
    // q_atomic_decrement(&counter);
    --counter;
}
}
