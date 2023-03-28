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
#include "tread.h"

#include "../../types/typesconv.h"

#include "../../libmscore/stafftext.h"
#include "../../libmscore/stafftextbase.h"
#include "../../libmscore/dynamic.h"

#include "../xmlreader.h"

#include "textbaserw.h"

using namespace mu::engraving;
using namespace mu::engraving::rw400;

void TRead::read(StaffText* t, XmlReader& xml, ReadContext& ctx)
{
    read(static_cast<StaffTextBase*>(t), xml, ctx);
}

void TRead::read(StaffTextBase* t, XmlReader& xml, ReadContext& ctx)
{
    t->clear();

    while (xml.readNextStartElement()) {
        if (!readProperties(t, xml, ctx)) {
            xml.unknown();
        }
    }
}

bool TRead::readProperties(StaffTextBase* t, XmlReader& e, ReadContext& ctx)
{
    const AsciiStringView tag(e.name());

    if (tag == "MidiAction") {
        int channel = e.intAttribute("channel", 0);
        String name = e.attribute("name");
        bool found = false;
        size_t n = t->channelActions().size();
        for (size_t i = 0; i < n; ++i) {
            ChannelActions* a = &t->channelActions()[i];
            if (a->channel == channel) {
                a->midiActionNames.append(name);
                found = true;
                break;
            }
        }
        if (!found) {
            ChannelActions a;
            a.channel = channel;
            a.midiActionNames.append(name);
            t->channelActions().push_back(a);
        }
        e.readNext();
    } else if (tag == "channelSwitch" || tag == "articulationChange") {
        voice_idx_t voice = static_cast<voice_idx_t>(e.intAttribute("voice", -1));
        if (voice < VOICES) {
            t->setChannelName(voice, e.attribute("name"));
        } else if (voice == mu::nidx) {
            // no voice applies channel to all voices for
            // compatibility
            for (voice_idx_t i = 0; i < VOICES; ++i) {
                t->setChannelName(i, e.attribute("name"));
            }
        }
        e.readNext();
    } else if (tag == "aeolus") {
        int group = e.intAttribute("group", -1);
        if (group >= 0 && group < 4) {
            t->setAeolusStop(group, e.readInt());
        } else {
            e.readNext();
        }
        t->setSetAeolusStops(true);
    } else if (tag == "swing") {
        DurationType swingUnit = TConv::fromXml(e.asciiAttribute("unit"), DurationType::V_INVALID);
        int unit = 0;
        if (swingUnit == DurationType::V_EIGHTH) {
            unit = Constants::division / 2;
        } else if (swingUnit == DurationType::V_16TH) {
            unit = Constants::division / 4;
        } else if (swingUnit == DurationType::V_ZERO) {
            unit = 0;
        }
        int ratio = e.intAttribute("ratio", 60);
        t->setSwing(true);
        t->setSwingParameters(unit, ratio);
        e.readNext();
    } else if (tag == "capo") {
        int fretId = e.intAttribute("fretId", 0);
        t->setCapo(fretId);
        e.readNext();
    } else if (!TextBaseRW::readProperties(t, e, ctx)) {
        return false;
    }
    return true;
}

void TRead::read(Dynamic* d, XmlReader& e, ReadContext& ctx)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag = e.name();
        if (tag == "subtype") {
            d->setDynamicType(e.readText());
        } else if (tag == "velocity") {
            d->setVelocity(e.readInt());
        } else if (tag == "dynType") {
            d->setDynRange(TConv::fromXml(e.readAsciiText(), DynamicRange::STAFF));
        } else if (tag == "veloChange") {
            d->setChangeInVelocity(e.readInt());
        } else if (tag == "veloChangeSpeed") {
            d->setVelChangeSpeed(TConv::fromXml(e.readAsciiText(), DynamicSpeed::NORMAL));
        } else if (!TextBaseRW::readProperties(d, e, ctx)) {
            e.unknown();
        }
    }
}
