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

#include "tempomap.h"

//---------------------------------------------------------
//   tempo
//---------------------------------------------------------

double TempoMap::tempo(int tick) const
{
    if (empty()) {
        return 2.0;
    }
    auto i = lower_bound(tick);
    if (i == end()) {
        --i;
        return i->second;
    }
    if (i->first == tick) {
        return i->second;
    }
    if (i == begin()) {
        return 2.0;
    }
    return i->second;
}

//---------------------------------------------------------
//   time2tick
//---------------------------------------------------------

int TempoMap::time2tick(double val, double relTempo, int division) const
{
    double time  = 0;
    int tick    = 0;
    double tempoDiv = division * relTempo;

    double tempo = 2.0;
    for (auto e = begin(); e != end(); ++e) {
        int delta    = e->first - tick;
        double time2 = time + double(delta) / (tempoDiv * tempo);
        if (val > time2) {
            break;
        }
        tick  = e->first;
        tempo = e->second;
        time  = time2;
    }
    return tick + (val - time) * tempoDiv * tempo;
}
