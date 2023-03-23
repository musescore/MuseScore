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
#include "chordrestrw.h"

#include "../../libmscore/chordrest.h"
#include "../../libmscore/score.h"
#include "../../libmscore/sig.h"
#include "../../libmscore/factory.h"
#include "../../libmscore/articulation.h"
#include "../../libmscore/part.h"
#include "../../libmscore/staff.h"
#include "../../libmscore/lyrics.h"

#include "../../types/typesconv.h"

#include "../xmlreader.h"

#include "readcontext.h"
#include "articulationrw.h"
#include "engravingitemrw.h"
#include "lyricsrw.h"

#include "log.h"

using namespace mu::engraving::rw400;

bool ChordRestRW::readProperties(ChordRest* ch, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    //! TODO Should be replaced with `ctx.mscVersion()`
    //! But at the moment, `ctx` is not set everywhere
    int mscVersion = ch->score()->mscVersion();

    if (tag == "durationType") {
        ch->setDurationType(TConv::fromXml(e.readAsciiText(), DurationType::V_QUARTER));
        if (ch->actualDurationType().type() != DurationType::V_MEASURE) {
            if (mscVersion < 112 && (ch->type() == ElementType::REST)
                &&            // for backward compatibility, convert V_WHOLE rests to V_MEASURE
                              // if long enough to fill a measure.
                              // OTOH, freshly created (un-initialized) rests have numerator == 0 (< 4/4)
                              // (see Fraction() constructor in fraction.h; this happens for instance
                              // when pasting selection from clipboard): they should not be converted
                ch->ticks().numerator() != 0
                &&            // rest durations are initialized to full measure duration when
                              // created upon reading the <Rest> tag (see Measure::read() )
                              // so a V_WHOLE rest in a measure of 4/4 or less => V_MEASURE
                (ch->actualDurationType() == DurationType::V_WHOLE && ch->ticks() <= Fraction(4, 4))) {
                // old pre 2.0 scores: convert
                ch->setDurationType(DurationType::V_MEASURE);
            } else {    // not from old score: set duration fraction from duration type
                ch->setTicks(ch->actualDurationType().fraction());
            }
        } else {
            if (mscVersion <= 114) {
                SigEvent event = ch->score()->sigmap()->timesig(e.context()->tick());
                ch->setTicks(event.timesig());
            }
        }
    } else if (tag == "BeamMode") {
        ch->setBeamMode(TConv::fromXml(e.readAsciiText(), BeamMode::AUTO));
    } else if (tag == "Articulation") {
        Articulation* atr = Factory::createArticulation(ch);
        atr->setTrack(ch->track());
        ArticulationRW::read(atr, e, ctx);
        ch->add(atr);
    } else if (tag == "leadingSpace" || tag == "trailingSpace") {
        LOGD("ChordRest: %s obsolete", tag.ascii());
        e.skipCurrentElement();
    } else if (tag == "small") {
        ch->setSmall(e.readInt());
    } else if (tag == "duration") {
        ch->setTicks(e.readFraction());
    } else if (tag == "ticklen") {      // obsolete (version < 1.12)
        int mticks = ch->score()->sigmap()->timesig(e.context()->tick()).timesig().ticks();
        int i = e.readInt();
        if (i == 0) {
            i = mticks;
        }
        if ((ch->type() == ElementType::REST) && (mticks == i)) {
            ch->setDurationType(DurationType::V_MEASURE);
            ch->setTicks(Fraction::fromTicks(i));
        } else {
            Fraction f = Fraction::fromTicks(i);
            ch->setTicks(f);
            ch->setDurationType(TDuration(f));
        }
    } else if (tag == "dots") {
        ch->setDots(e.readInt());
    } else if (tag == "staffMove") {
        ch->setStaffMove(e.readInt());
        if (ch->vStaffIdx() < ch->part()->staves().front()->idx() || ch->vStaffIdx() > ch->part()->staves().back()->idx()) {
            ch->setStaffMove(0);
        }
    } else if (tag == "Spanner") {
        Spanner::readSpanner(e, ch, ch->track());
    } else if (tag == "Lyrics") {
        Lyrics* lyr = Factory::createLyrics(ch);
        lyr->setTrack(e.context()->track());
        LyricsRW::read(lyr, e, ctx);
        ch->add(lyr);
    } else if (tag == "pos") {
        PointF pt = e.readPoint();
        ch->setOffset(pt * ch->spatium());
    }
//      else if (tag == "offset")
//            DurationElement::readProperties(e);
    else if (!EngravingItemRW::readProperties(ch, e, ctx)) {
        return false;
    }
    return true;
}
