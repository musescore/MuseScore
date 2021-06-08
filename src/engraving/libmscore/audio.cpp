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

#include "audio.h"
#include "xml.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   Audio
//---------------------------------------------------------

Audio::Audio()
{
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Audio::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        if (e.name() == "path") {
            _path = e.readElementText();
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Audio::write(XmlWriter& xml) const
{
    xml.stag("Audio");
    xml.tag("path", _path);
    xml.etag();
}
}
