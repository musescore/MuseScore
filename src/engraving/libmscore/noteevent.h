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

#ifndef __NOTEEVENT_H__
#define __NOTEEVENT_H__

#include <vector>
#include <algorithm>

namespace mu::engraving {
class XmlWriter;
class XmlReader;

//---------------------------------------------------------
//    NoteEvent
//---------------------------------------------------------

class NoteEvent
{
    int _pitch;     // relative pitch to note pitch
    int _ontime;    // one unit is 1/1000 of nominal note len
    int _len;       // one unit is 1/1000 of nominal note len

public:
    constexpr static int NOTE_LENGTH = 1000;

    NoteEvent()
        : _pitch(0), _ontime(0), _len(NOTE_LENGTH) {}
    NoteEvent(int a, int b, int c)
        : _pitch(a), _ontime(b), _len(c) {}

    void read(XmlReader&);
    void write(XmlWriter&) const;

    int  pitch() const { return _pitch; }
    int ontime() const { return _ontime; }
    int offtime() const { return _ontime + _len; }
    int len() const { return _len; }
    void setPitch(int v) { _pitch = v; }
    void setOntime(int v) { _ontime = v; }
    void setLen(int v) { _len = v; }
    bool operator==(const NoteEvent&) const;
};

//---------------------------------------------------------
//   NoteEventList
//---------------------------------------------------------

class NoteEventList : public std::vector<NoteEvent>
{
public:
    NoteEventList();

    int offtime()
    {
        return empty() ? 0 : std::max_element(cbegin(), cend(), [](const NoteEvent& n1, const NoteEvent& n2) {
            return n1.offtime() < n2.offtime();
        })->offtime();
    }
};
} // namespace mu::engraving
#endif
