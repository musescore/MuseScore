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

#include "noteevent.h"
#include "rw/xml.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   read
//---------------------------------------------------------

void NoteEvent::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "pitch") {
            _pitch = e.readInt();
        } else if (tag == "ontime") {
            _ontime = e.readInt();
        } else if (tag == "len") {
            _len = e.readInt();
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void NoteEvent::write(XmlWriter& xml) const
{
    xml.startObject("Event");
    xml.tag("pitch", _pitch, 0);
    xml.tag("ontime", _ontime, 0);
    xml.tag("len", _len, NOTE_LENGTH);
    xml.endObject();
}

//---------------------------------------------------------
//   NoteEventList
//---------------------------------------------------------

NoteEventList::NoteEventList()
    : QList<NoteEvent>()
{
}

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool NoteEvent::operator==(const NoteEvent& e) const
{
    return (e._pitch == _pitch) && (e._ontime == _ontime) && (e._len == _len);
}
}
