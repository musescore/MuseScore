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
#include "lyricsrw.h"

#include "../../types/typesconv.h"

#include "../../libmscore/lyrics.h"

#include "../xmlreader.h"

#include "readcontext.h"
#include "propertyrw.h"
#include "textbaserw.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

void LyricsRW::read(Lyrics* l, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        if (!readProperties(l, e, ctx)) {
            e.unknown();
        }
    }
    if (!l->isStyled(Pid::OFFSET) && !ctx.pasteMode()) {
        // fix offset for pre-3.1 scores
        // 3.0: y offset was meaningless if autoplace is set
        String version = ctx.mscoreVersion();
        if (l->autoplace() && !version.isEmpty() && version < u"3.1") {
            PointF off = l->propertyDefault(Pid::OFFSET).value<PointF>();
            l->ryoffset() = off.y();
        }
    }
}

bool LyricsRW::readProperties(Lyrics* l, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "no") {
        l->setNo(e.readInt());
    } else if (tag == "syllabic") {
        l->setSyllabic(TConv::fromXml(e.readAsciiText(), LyricsSyllabic::SINGLE));
    } else if (tag == "ticks") {          // obsolete
        l->setTicks(e.readFraction());     // will fall back to reading integer ticks on older scores
    } else if (tag == "ticks_f") {
        l->setTicks(e.readFraction());
    } else if (PropertyRW::readProperty(l, tag, e, ctx, Pid::PLACEMENT)) {
    } else if (!TextBaseRW::readProperties(l, e, ctx)) {
        return false;
    }
    return true;
}
