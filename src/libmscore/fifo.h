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

#ifndef __FIFO_H__
#define __FIFO_H__

#include <atomic>

namespace Ms {
//---------------------------------------------------------
//   FifoBase
//    - works only for one reader/writer
//    - reader writes ridx
//    - writer writes widx
//    - reader decrements counter
//    - writer increments counter
//    - counter increment/decrement must be atomic
//---------------------------------------------------------

class FifoBase
{
protected:
    int ridx;                   // read index
    int widx;                   // write index
    std::atomic<int> counter;   // objects in fifo
    int maxCount;

    void push();
    void pop();

public:
    FifoBase() { clear(); }
    virtual ~FifoBase() {}
    void clear();
    int count() const { return counter; }
    bool empty() const { return counter == 0; }
    bool isFull() const { return maxCount == counter; }
};
}     // namespace Ms
#endif
