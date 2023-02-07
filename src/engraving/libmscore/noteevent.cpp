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

namespace mu::engraving {
//---------------------------------------------------------
//   read
//---------------------------------------------------------

void NoteEvent::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "pitch") {
            m_pitch = e.readInt();
        } else if (tag == "ontime") {
            m_ontime = e.readInt();
        } else if (tag == "len") {
            m_len = e.readInt();
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
    xml.startElement("Event");
    xml.tag("pitch", m_pitch, 0);
    xml.tag("ontime", m_ontime, 0);
    xml.tag("len", m_len, NOTE_LENGTH);
    xml.endElement();
}

//---------------------------------------------------------
//   NoteEventList
//---------------------------------------------------------

NoteEventList::NoteEventList()
    : std::vector<NoteEvent>()
{
}

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool NoteEvent::operator==(const NoteEvent& e) const
{
    return (e.m_pitch == m_pitch) && (e.m_ontime == m_ontime) && (e.m_len == m_len);
}
}
