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

#ifndef MIDISHARED_MIDIEVENT_H
#define MIDISHARED_MIDIEVENT_H

#include "engraving/compat/midi/midicoreevent.h"

namespace mu::iex::midi {
class MidiEvent : public engraving::MidiCoreEvent
{
protected:
    std::vector<uchar> _edata;
    int _len { 0 };
    int _metaType { 0 };

public:
    MidiEvent() {}
    MidiEvent(uchar t, uchar c, uchar a, uchar b)
        : MidiCoreEvent(t, c, a, b), _edata(0), _len(0) {}

    const uchar* edata() const { return _edata.data(); }
    void setEData(std::vector<uchar>& d) { _edata = d; }
    int len() const { return _len; }
    void setLen(int l) { _len = l; }
    int metaType() const { return _metaType; }
    void setMetaType(int v) { _metaType = v; }
};
}

#endif
