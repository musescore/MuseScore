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
#include "tremolorw.h"

#include "../../libmscore/tremolo.h"

#include "../../types/typesconv.h"

#include "../xmlreader.h"

#include "propertyrw.h"
#include "engravingitemrw.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

void TremoloRW::read(Tremolo* t, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "subtype") {
            t->setTremoloType(TConv::fromXml(e.readAsciiText(), TremoloType::INVALID_TREMOLO));
        }
        // Style needs special handling other than readStyledProperty()
        // to avoid calling customStyleApplicable() in setProperty(),
        // which cannot be called now because durationType() isn't defined yet.
        else if (tag == "strokeStyle") {
            t->setStyle(TremoloStyle(e.readInt()));
            t->setPropertyFlags(Pid::TREMOLO_STYLE, PropertyFlags::UNSTYLED);
        } else if (tag == "Fragment") {
            BeamFragment f = BeamFragment();
            int idx = (t->direction() == DirectionV::AUTO || t->direction() == DirectionV::DOWN) ? 0 : 1;
            t->setUserModified(t->direction(), true);
            double _spatium = t->spatium();
            while (e.readNextStartElement()) {
                const AsciiStringView tag1(e.name());
                if (tag1 == "y1") {
                    f.py1[idx] = e.readDouble() * _spatium;
                } else if (tag1 == "y2") {
                    f.py2[idx] = e.readDouble() * _spatium;
                } else {
                    e.unknown();
                }
            }
            t->setBeamFragment(f);
        } else if (PropertyRW::readStyledProperty(t, tag, e, ctx)) {
        } else if (!EngravingItemRW::readProperties(t, e, ctx)) {
            e.unknown();
        }
    }
}
