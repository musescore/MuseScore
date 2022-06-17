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

#include "stafftextbase.h"
#include "rw/xml.h"
#include "types/typesconv.h"

#include "system.h"
#include "staff.h"
#include "score.h"
#include "measure.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   StaffTextBase
//---------------------------------------------------------

StaffTextBase::StaffTextBase(const ElementType& type, Segment* parent, TextStyleType tid, ElementFlags flags)
    : TextBase(type, parent, tid, flags)
{
    setSwingParameters(Constants::division / 2, 60);
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffTextBase::write(XmlWriter& xml) const
{
    if (!xml.context()->canWrite(this)) {
        return;
    }
    xml.startElement(this);

    for (const ChannelActions& s : _channelActions) {
        int channel = s.channel;
        for (const String& name : s.midiActionNames) {
            xml.tag("MidiAction", { { "channel", channel }, { "name", name } });
        }
    }
    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
        if (!_channelNames[voice].isEmpty()) {
            xml.tag("channelSwitch", { { "voice", voice }, { "name", _channelNames[voice] } });
        }
    }
    if (_setAeolusStops) {
        for (int i = 0; i < 4; ++i) {
            xml.tag("aeolus", { { "group", i } }, aeolusStops[i]);
        }
    }
    if (swing()) {
        DurationType swingUnit;
        if (swingParameters().swingUnit == Constants::division / 2) {
            swingUnit = DurationType::V_EIGHTH;
        } else if (swingParameters().swingUnit == Constants::division / 4) {
            swingUnit = DurationType::V_16TH;
        } else {
            swingUnit = DurationType::V_ZERO;
        }
        int swingRatio = swingParameters().swingRatio;
        xml.tag("swing", { { "unit", TConv::toXml(swingUnit) }, { "ratio", swingRatio } });
    }
    if (capo() != 0) {
        xml.tag("capo", { { "fretId", capo() } });
    }
    TextBase::writeProperties(xml);

    xml.endElement();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffTextBase::read(XmlReader& e)
{
    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
        _channelNames[voice].clear();
    }
    clearAeolusStops();
    while (e.readNextStartElement()) {
        if (!readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool StaffTextBase::readProperties(XmlReader& e)
{
    const AsciiStringView tag(e.name());

    if (tag == "MidiAction") {
        int channel = e.intAttribute("channel", 0);
        String name = e.attribute("name");
        bool found = false;
        size_t n = _channelActions.size();
        for (size_t i = 0; i < n; ++i) {
            ChannelActions* a = &_channelActions[i];
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
            _channelActions.push_back(a);
        }
        e.readNext();
    } else if (tag == "channelSwitch" || tag == "articulationChange") {
        voice_idx_t voice = static_cast<voice_idx_t>(e.intAttribute("voice", -1));
        if (voice < VOICES) {
            _channelNames[voice] = e.attribute("name");
        } else if (voice == mu::nidx) {
            // no voice applies channel to all voices for
            // compatibility
            for (voice_idx_t i = 0; i < VOICES; ++i) {
                _channelNames[i] = e.attribute("name");
            }
        }
        e.readNext();
    } else if (tag == "aeolus") {
        int group = e.intAttribute("group", -1);
        if (group >= 0 && group < 4) {
            aeolusStops[group] = e.readInt();
        } else {
            e.readNext();
        }
        _setAeolusStops = true;
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
        setSwing(true);
        setSwingParameters(unit, ratio);
        e.readNext();
    } else if (tag == "capo") {
        int fretId = e.intAttribute("fretId", 0);
        setCapo(fretId);
        e.readNext();
    } else if (!TextBase::readProperties(e)) {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   clearAeolusStops
//---------------------------------------------------------

void StaffTextBase::clearAeolusStops()
{
    for (int i = 0; i < 4; ++i) {
        aeolusStops[i] = 0;
    }
}

//---------------------------------------------------------
//   setAeolusStop
//---------------------------------------------------------

void StaffTextBase::setAeolusStop(int group, int idx, bool val)
{
    if (val) {
        aeolusStops[group] |= (1 << idx);
    } else {
        aeolusStops[group] &= ~(1 << idx);
    }
}

//---------------------------------------------------------
//   getAeolusStop
//---------------------------------------------------------

bool StaffTextBase::getAeolusStop(int group, int idx) const
{
    return aeolusStops[group] & (1 << idx);
}

//---------------------------------------------------------
//   segment
//---------------------------------------------------------

Segment* StaffTextBase::segment() const
{
    if (!explicitParent()->isSegment()) {
        LOGD("parent %s", explicitParent()->typeName());
        return 0;
    }
    Segment* s = toSegment(explicitParent());
    return s;
}
}
