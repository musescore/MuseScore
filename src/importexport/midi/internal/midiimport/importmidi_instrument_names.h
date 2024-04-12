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

#ifndef IMPORTMIDI_INSTRUMENT_NAMES_H
#define IMPORTMIDI_INSTRUMENT_NAMES_H

#include <QString>

namespace mu::iex::midi {
struct MidiInstrument {
    int type;
    int hbank, lbank, patch;
    int split;
    const char* name;

    static QString instrName(int type, int hbank, int lbank, int program);
};
}

#endif // IMPORTMIDI_INSTRUMENT_NAMES_H
