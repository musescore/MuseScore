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
#include "engravingitemrw.h"

#include "../../libmscore/engravingitem.h"
#include "../../libmscore/mscore.h"
#include "../../libmscore/score.h"
#include "../../libmscore/staff.h"
#include "../../libmscore/linkedobjects.h"

#include "../xmlreader.h"

#include "readcontext.h"
#include "propertyrw.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

bool EngravingItemRW::readProperties(EngravingItem* item, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (PropertyRW::readProperty(item, tag, e, ctx, Pid::SIZE_SPATIUM_DEPENDENT)) {
    } else if (PropertyRW::readProperty(item, tag, e, ctx, Pid::OFFSET)) {
    } else if (PropertyRW::readProperty(item, tag, e, ctx, Pid::MIN_DISTANCE)) {
    } else if (PropertyRW::readProperty(item, tag, e, ctx, Pid::AUTOPLACE)) {
    } else if (tag == "track") {
        item->setTrack(e.readInt() + ctx.trackOffset());
    } else if (tag == "color") {
        item->setColor(e.readColor());
    } else if (tag == "visible") {
        item->setVisible(e.readInt());
    } else if (tag == "selected") { // obsolete
        e.readInt();
    } else if ((tag == "linked") || (tag == "linkedMain")) {
        Staff* s = item->staff();
        if (!s) {
            s = ctx.score()->staff(ctx.track() / VOICES);
            if (!s) {
                LOGW("EngravingItem::readProperties: linked element's staff not found (%s)", item->typeName());
                e.skipCurrentElement();
                return true;
            }
        }
        if (tag == "linkedMain") {
            item->setLinks(new LinkedObjects(item->score()));
            item->links()->push_back(item);

            ctx.addLink(s, item->links(), ctx.location(true));

            e.readNext();
        } else {
            Staff* ls = s->links() ? toStaff(s->links()->mainElement()) : nullptr;
            bool linkedIsMaster = ls ? ls->score()->isMaster() : false;
            Location loc = ctx.location(true);
            if (ls) {
                loc.setStaff(static_cast<int>(ls->idx()));
            }
            Location mainLoc = Location::relative();
            bool locationRead = false;
            int localIndexDiff = 0;
            while (e.readNextStartElement()) {
                const AsciiStringView ntag(e.name());

                if (ntag == "score") {
                    String val(e.readText());
                    if (val == "same") {
                        linkedIsMaster = item->score()->isMaster();
                    }
                } else if (ntag == "location") {
                    mainLoc.read(e);
                    mainLoc.toAbsolute(loc);
                    locationRead = true;
                } else if (ntag == "indexDiff") {
                    localIndexDiff = e.readInt();
                } else {
                    e.unknown();
                }
            }
            if (!locationRead) {
                mainLoc = loc;
            }
            LinkedObjects* link = ctx.getLink(linkedIsMaster, mainLoc, localIndexDiff);
            if (link) {
                EngravingObject* linked = link->mainElement();
                if (linked->type() == item->type()) {
                    item->linkTo(linked);
                } else {
                    LOGW("EngravingItem::readProperties: linked elements have different types: %s, %s. Input file corrupted?",
                         item->typeName(), linked->typeName());
                }
            }
            if (!item->links()) {
                LOGW("EngravingItem::readProperties: could not link %s at staff %d", item->typeName(), mainLoc.staff() + 1);
            }
        }
    } else if (tag == "lid") {
        if (ctx.mscVersion() >= 301) {
            e.skipCurrentElement();
            return true;
        }
        int id = e.readInt();
        item->setLinks(mu::value(ctx.linkIds(), id, nullptr));
        if (!item->links()) {
            if (!ctx.isMasterScore()) {       // DEBUG
                LOGD("---link %d not found (%zu)", id, ctx.linkIds().size());
            }
            item->setLinks(new LinkedObjects(item->score(), id));
            ctx.linkIds().insert({ id, item->links() });
        }
#ifndef NDEBUG
        else {
            for (EngravingObject* eee : *item->links()) {
                EngravingItem* ee = static_cast<EngravingItem*>(eee);
                if (ee->type() != item->type()) {
                    ASSERT_X(String(u"link %1(%2) type mismatch %3 linked to %4")
                             .arg(String::fromAscii(ee->typeName()))
                             .arg(id)
                             .arg(String::fromAscii(ee->typeName()), String::fromAscii(item->typeName())));
                }
            }
        }
#endif
        assert(!item->links()->contains(item));
        item->links()->push_back(item);
    } else if (tag == "tick") {
        int val = e.readInt();
        if (val >= 0) {
            ctx.setTick(Fraction::fromTicks(ctx.fileDivision(val)));             // obsolete
        }
    } else if (tag == "pos") {           // obsolete
        PropertyRW::readProperty(item, e, ctx, Pid::OFFSET);
    } else if (tag == "voice") {
        item->setVoice(e.readInt());
    } else if (tag == "tag") {
        String val(e.readText());
        for (int i = 1; i < MAX_TAGS; i++) {
            if (ctx.score()->layerTags()[i] == val) {
                item->setTag(1 << i);
                break;
            }
        }
    } else if (PropertyRW::readProperty(item, tag, e, ctx, Pid::PLACEMENT)) {
    } else if (tag == "z") {
        item->setZ(e.readInt());
    } else {
        return false;
    }
    return true;
}
