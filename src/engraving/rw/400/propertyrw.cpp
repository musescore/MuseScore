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
#include "propertyrw.h"

#include "../xmlreader.h"
#include "readcontext.h"

#include "../../libmscore/engravingitem.h"
#include "../../libmscore/mscore.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

bool PropertyRW::readProperty(EngravingItem* item, const AsciiStringView& tag, XmlReader& xml, ReadContext& ctx, Pid pid)
{
    if (tag == propertyName(pid)) {
        readProperty(item, xml, ctx, pid);
        return true;
    }
    return false;
}

void PropertyRW::readProperty(EngravingItem* item, XmlReader& xml, ReadContext& ctx, Pid pid)
{
    PropertyValue v = mu::engraving::readProperty(pid, xml);
    switch (propertyType(pid)) {
    case P_TYPE::MILLIMETRE: //! NOTE type mm, but stored in xml as spatium
        v = v.value<Spatium>().toMM(ctx.spatium());
        break;
    case P_TYPE::POINT:
        if (item->offsetIsSpatiumDependent()) {
            v = v.value<PointF>() * ctx.spatium();
        } else {
            v = v.value<PointF>() * DPMM;
        }
        break;
    default:
        break;
    }

    item->setProperty(pid, v);
    if (item->isStyled(pid)) {
        item->setPropertyFlags(pid, PropertyFlags::UNSTYLED);
    }
}
