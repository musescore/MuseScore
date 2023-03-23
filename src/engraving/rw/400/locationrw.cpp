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
#include "locationrw.h"

#include "../../libmscore/location.h"

#include "../xmlreader.h"

using namespace mu::engraving::rw400;

void LocationRW::read(Location* l, XmlReader& e, ReadContext&)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());

        if (tag == "staves") {
            l->setStaff(e.readInt());
        } else if (tag == "voices") {
            l->setVoice(e.readInt());
        } else if (tag == "measures") {
            l->setMeasure(e.readInt());
        } else if (tag == "fractions") {
            l->setFrac(e.readFraction());
        } else if (tag == "grace") {
            l->setGraceIndex(e.readInt());
        } else if (tag == "notes") {
            l->setNote(e.readInt());
        } else {
            e.unknown();
        }
    }
}
