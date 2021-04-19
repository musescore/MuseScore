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

#ifndef __SEQUENCER_H__
#define __SEQUENCER_H__

namespace Ms {
class NPlayEvent;

enum class BeatType : char;

//---------------------------------------------------------
//   Sequencer
//---------------------------------------------------------

class Sequencer
{
public:
    Sequencer() {}
    virtual ~Sequencer() {}

    virtual void sendEvent(const NPlayEvent&) = 0;
    virtual void startNote(int channel, int, int, double nt) = 0;
    virtual void startNote(int channel, int, int, int, double nt) = 0;
    virtual void playMetronomeBeat(BeatType type) = 0;
};
}     // namespace Ms
#endif
