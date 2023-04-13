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

#include "segment.h"
#include "staff.h"

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
    UNREACHABLE;
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
            xml.tag("aeolus", { { "group", i } }, m_aeolusStops[i]);
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

void StaffTextBase::clear()
{
    for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
        _channelNames[voice].clear();
    }
    clearAeolusStops();
}

//---------------------------------------------------------
//   clearAeolusStops
//---------------------------------------------------------

void StaffTextBase::clearAeolusStops()
{
    for (int i = 0; i < 4; ++i) {
        m_aeolusStops[i] = 0;
    }
}

//---------------------------------------------------------
//   setAeolusStop
//---------------------------------------------------------

void StaffTextBase::setAeolusStop(int group, int idx, bool val)
{
    if (val) {
        m_aeolusStops[group] |= (1 << idx);
    } else {
        m_aeolusStops[group] &= ~(1 << idx);
    }
}

void StaffTextBase::setAeolusStop(int group, int val)
{
    m_aeolusStops[group] = val;
}

//---------------------------------------------------------
//   getAeolusStop
//---------------------------------------------------------

bool StaffTextBase::getAeolusStop(int group, int idx) const
{
    return m_aeolusStops[group] & (1 << idx);
}

int StaffTextBase::aeolusStop(int group) const
{
    return m_aeolusStops[group];
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
